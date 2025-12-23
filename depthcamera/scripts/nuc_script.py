import pyrealsense2 as rs
import numpy as np
import cv2
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import os
from datetime import datetime

TARGET_WIDTH = 32
TARGET_HEIGHT = 32
MAX_PIN_DISPLACEMENT_M = 0.05 #~5cm
DEPTH_SCALE = 0.001
MIN_DEPTH_M = 0.15
MAX_DEPTH_M = 1.0

timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
output_folder = f"../pin_outputs/{timestamp}"
os.makedirs(output_folder, exist_ok=True)

pipeline = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.depth, 1280, 720, rs.format.z16, 30)

print("Starting RealSense camera...")
pipeline.start(config)

for _ in range(10):
    pipeline.wait_for_frames()

print("\nLive preview started!")
print("Press 's' to capture the current frame")
print("Press 'q' to quit without capturing")

cv2.namedWindow("Depth Preview", cv2.WINDOW_NORMAL)
cv2.resizeWindow("Depth Preview", 960, 540)

depth_frame = None
try:
    while True:
        frames = pipeline.wait_for_frames()
        current_depth_frame = frames.get_depth_frame()
        
        if not current_depth_frame:
            continue
        
        depth_image_preview = np.asanyarray(current_depth_frame.get_data())
        depth_m_preview = depth_image_preview.astype(np.float32) * DEPTH_SCALE
        depth_m_preview = np.clip(depth_m_preview, MIN_DEPTH_M, MAX_DEPTH_M)
        
        depth_max = np.nanmax(depth_m_preview)
        if depth_max > 0:
            depth_display = (depth_m_preview / depth_max * 255).astype(np.uint8)
            depth_display = cv2.applyColorMap(depth_display, cv2.COLORMAP_JET)
        else:
            depth_display = np.zeros((depth_m_preview.shape[0], depth_m_preview.shape[1], 3), dtype=np.uint8)
        
        cv2.putText(depth_display, "Press 's' to capture | 'q' to quit", 
                   (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
        
        cv2.imshow("Depth Preview", depth_display)
        
        key = cv2.waitKey(1) & 0xFF
        if key == ord('s'):
            depth_frame = current_depth_frame
            print("Frame captured!")
            break
        elif key == ord('q'):
            print("Quitting without capturing...")
            pipeline.stop()
            cv2.destroyAllWindows()
            exit(0)
    
    cv2.destroyWindow("Depth Preview")
    
    if not depth_frame:
        raise RuntimeError("Failed to capture depth frame.")
    depth_image = np.asanyarray(depth_frame.get_data())
finally:
    pipeline.stop()
    print("Camera stopped.")

depth_m = depth_image.astype(np.float32) * DEPTH_SCALE
depth_m = np.clip(depth_m, MIN_DEPTH_M, MAX_DEPTH_M)

depth_display = (depth_m / np.nanmax(depth_m) * 255).astype(np.uint8)
depth_display = cv2.applyColorMap(depth_display, cv2.COLORMAP_JET)

cv2.namedWindow("Select ROI", cv2.WINDOW_NORMAL)
cv2.resizeWindow("Select ROI", 960, 540)
roi = cv2.selectROI("Select ROI", depth_display, showCrosshair=True, fromCenter=False)
cv2.destroyWindow("Select ROI")

x, y, w, h = roi
if w == 0 or h == 0:
    raise ValueError("No ROI selected.")

cropped = depth_m[y:y+h, x:x+w]
h_crop, w_crop = cropped.shape
aspect_target = TARGET_WIDTH / TARGET_HEIGHT
aspect_image = w_crop / h_crop

if aspect_image > aspect_target:
    new_h = int(round(w_crop / aspect_target))
    pad_top = (new_h - h_crop) // 2
    pad_bottom = new_h - h_crop - pad_top
    padded = np.pad(cropped, ((pad_top, pad_bottom), (0,0)), constant_values=np.nan)
else:
    new_w = int(round(h_crop * aspect_target))
    pad_left = (new_w - w_crop) // 2
    pad_right = new_w - w_crop - pad_left
    padded = np.pad(cropped, ((0,0), (pad_left, pad_right)), constant_values=np.nan)

median_depth = np.nanmedian(padded)
padded = np.nan_to_num(padded, nan=median_depth)

resized = cv2.resize(padded, (TARGET_WIDTH, TARGET_HEIGHT), interpolation=cv2.INTER_AREA)
min_d, max_d = np.min(resized), np.max(resized)
normalized = (resized - min_d) / (max_d - min_d + 1e-6)
pin_heights_m = normalized * MAX_PIN_DISPLACEMENT_M

output_csv = os.path.join(output_folder, "pins_32x32.csv")
output_plot = os.path.join(output_folder, "pin_height_map.png")
np.savetxt(output_csv, pin_heights_m, delimiter=",", fmt="%.5f")

plt.figure(figsize=(6,4))
plt.imshow(pin_heights_m, cmap='gray', origin='upper')
plt.title(f"{TARGET_WIDTH}x{TARGET_HEIGHT} Pin Height Map")
plt.colorbar(label="Height (m)")
plt.xlabel("X (pins)")
plt.ylabel("Y (pins)")
plt.savefig(output_plot, dpi=150)

print(f"Saved 32x32 pin map to {output_csv}")
print(f"Pin height map saved to {output_plot}")