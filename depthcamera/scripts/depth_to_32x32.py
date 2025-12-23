import cv2
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import os
from datetime import datetime

depth_image_path = "../realsense_captures/depthmaps/me_Depth.png"

target_width = 32
target_height = 32
max_pin_displacement_m = 0.042 # ~4.2cm
depth_scale = 0.001 # meters

depth = cv2.imread(depth_image_path, cv2.IMREAD_UNCHANGED).astype(np.float32)

if depth.ndim == 3:
    depth = depth[:, :, 0]

depth_m = depth * depth_scale
mask = (depth_m > 0.15) & (depth_m < 1.0)
depth_m = np.where(mask, depth_m, np.nan)

depth_m = cv2.medianBlur(np.nan_to_num(depth_m, nan=0).astype(np.float32), 5) # noise reduction

median_depth = np.nanmedian(depth_m)
depth_m = np.where(depth_m == 0, median_depth, depth_m)

depth_display = (depth_m / np.nanmax(depth_m) * 255).astype(np.uint8)
depth_display = cv2.applyColorMap(depth_display, cv2.COLORMAP_JET)

cv2.namedWindow("Select ROI", cv2.WINDOW_NORMAL)
r = cv2.selectROI("Select ROI", depth_display, showCrosshair=True, fromCenter=False)
cv2.destroyWindow("Select ROI")

x, y, w, h = r
cropped = depth_m[y:y+h, x:x+w]

h, w = cropped.shape
aspect_target = target_width / target_height
aspect_image = w / h

if aspect_image > aspect_target: # too wide
    new_h = int(round(w / aspect_target))
    pad_top = (new_h - h) // 2
    pad_bottom = new_h - h - pad_top
    padded = np.pad(cropped, ((pad_top, pad_bottom), (0, 0)), constant_values=np.nan)
else: # too tall
    new_w = int(round(h * aspect_target))
    pad_left = (new_w - w) // 2
    pad_right = new_w - w - pad_left
    padded = np.pad(cropped, ((0, 0), (pad_left, pad_right)), constant_values=np.nan)

median_depth = np.nanmedian(padded)
padded = np.nan_to_num(padded, nan=median_depth)

resized = cv2.resize(padded, (target_width, target_height), interpolation=cv2.INTER_AREA) # downsampling

min_d = np.min(resized)
max_d = np.max(resized)
normalized = (resized - min_d) / (max_d - min_d + 1e-6)
pin_heights_m = normalized * max_pin_displacement_m

timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
output_folder = f"../realsense_captures/pin_output_{timestamp}"
os.makedirs(output_folder, exist_ok=True)

output_csv = os.path.join(output_folder, "pins_32x24_centered.csv")
output_plot = os.path.join(output_folder, "pin_height_map.png")

np.savetxt(output_csv, pin_heights_m, delimiter=",", fmt="%.5f")
print(f"Saved {target_width}x{target_height} pin map to {output_csv}")

plt.figure(figsize=(6, 4))
plt.imshow(pin_heights_m, cmap='gray', origin='upper')
plt.title(f"{target_width}x{target_height} Centered Pin Height Map")
plt.colorbar(label="Height (m)")
plt.xlabel("X (pins)")
plt.ylabel("Y (pins)")
plt.savefig(output_plot, dpi=150)
print(f"Pin height map saved as {output_plot}")