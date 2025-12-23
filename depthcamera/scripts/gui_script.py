import pyrealsense2 as rs
import numpy as np
import cv2
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import os
from datetime import datetime
import serial
import tkinter as tk
from tkinter import ttk, messagebox
from PIL import Image, ImageTk
import threading

# Constants
TARGET_WIDTH = 32
TARGET_HEIGHT = 32
MAX_PIN_DISPLACEMENT_M = 0.05  # ~5cm
DEPTH_SCALE = 0.001
MIN_DEPTH_M = 0.15
MAX_DEPTH_M = 1.0

SERIAL_PORT = '/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_44238313838351308040-if00'
BAUD_RATE = 115200


class LivePinGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("LivePin")
        self.root.geometry("1000x700")
        self.root.configure(bg='#2b2b2b')
        
        # State variables
        self.pipeline = None
        self.camera_running = False
        self.captured_frame = None
        self.depth_image = None
        self.roi = None
        self.roi_selected = False
        self.processing = False
        
        # Create UI
        self.create_ui()
        
    def create_ui(self):
        # Title
        title_frame = tk.Frame(self.root, bg='#2b2b2b')
        title_frame.pack(pady=20)
        
        title = tk.Label(
            title_frame,
            text="LivePin",
            font=("Arial", 36, "bold"),
            bg='#2b2b2b',
            fg='#4CAF50'
        )
        title.pack()
        
        subtitle = tk.Label(
            title_frame,
            text="Dynamic Pin Display Controller",
            font=("Arial", 12),
            bg='#2b2b2b',
            fg='#888888'
        )
        subtitle.pack()
        
        # Video display area
        self.video_frame = tk.Frame(self.root, bg='#1e1e1e', relief=tk.SUNKEN, bd=2)
        self.video_frame.pack(pady=20, padx=20, fill=tk.BOTH, expand=True)
        
        self.video_label = tk.Label(
            self.video_frame,
            text="Camera feed will appear here",
            font=("Arial", 14),
            bg='#1e1e1e',
            fg='#888888'
        )
        self.video_label.pack(expand=True)
        
        # Control buttons frame
        button_frame = tk.Frame(self.root, bg='#2b2b2b')
        button_frame.pack(pady=10)
        
        self.start_button = tk.Button(
            button_frame,
            text="Start Camera",
            font=("Arial", 14, "bold"),
            bg='#4CAF50',
            fg='white',
            activebackground='#45a049',
            command=self.start_camera,
            width=15,
            height=2,
            cursor="hand2"
        )
        self.start_button.pack(side=tk.LEFT, padx=10)
        
        self.capture_button = tk.Button(
            button_frame,
            text="Capture (S)",
            font=("Arial", 14, "bold"),
            bg='#2196F3',
            fg='white',
            activebackground='#0b7dda',
            command=self.capture_frame,
            width=15,
            height=2,
            state=tk.DISABLED,
            cursor="hand2"
        )
        self.capture_button.pack(side=tk.LEFT, padx=10)
        
        self.reset_button = tk.Button(
            button_frame,
            text="Reset",
            font=("Arial", 14, "bold"),
            bg='#FF9800',
            fg='white',
            activebackground='#F57C00',
            command=self.reset_app,
            width=15,
            height=2,
            state=tk.DISABLED,
            cursor="hand2"
        )
        self.reset_button.pack(side=tk.LEFT, padx=10)
        
        self.quit_button = tk.Button(
            button_frame,
            text="Quit (Q)",
            font=("Arial", 14, "bold"),
            bg='#f44336',
            fg='white',
            activebackground='#da190b',
            command=self.quit_app,
            width=15,
            height=2,
            cursor="hand2"
        )
        self.quit_button.pack(side=tk.LEFT, padx=10)
        
        # Status bar
        self.status_frame = tk.Frame(self.root, bg='#1e1e1e', relief=tk.SUNKEN, bd=1)
        self.status_frame.pack(side=tk.BOTTOM, fill=tk.X)
        
        self.status_label = tk.Label(
            self.status_frame,
            text="Ready",
            font=("Arial", 10),
            bg='#1e1e1e',
            fg='#4CAF50',
            anchor=tk.W,
            padx=10,
            pady=5
        )
        self.status_label.pack(fill=tk.X)
        
        # Bind keyboard shortcuts
        self.root.bind('<s>', lambda e: self.capture_frame())
        self.root.bind('<q>', lambda e: self.quit_app())
        
    def update_status(self, message, color='#4CAF50'):
        self.status_label.config(text=message, fg=color)
        self.root.update()
    
    def reset_app(self):
        """Reset the application to initial state"""
        # Stop camera if running
        if self.camera_running and self.pipeline:
            self.camera_running = False
            try:
                self.pipeline.stop()
            except:
                pass
        
        # Clean up any lingering OpenCV windows
        cv2.destroyAllWindows()
        cv2.waitKey(1)
        
        # Reset all state variables
        self.pipeline = None
        self.camera_running = False
        self.captured_frame = None
        self.depth_image = None
        self.roi = None
        self.roi_selected = False
        self.processing = False
        
        # Reset UI
        self.start_button.config(state=tk.NORMAL)
        self.capture_button.config(state=tk.DISABLED)
        self.reset_button.config(state=tk.DISABLED)
        
        # Clear video display
        self.video_label.config(image="", text="Camera feed will appear here")
        self.video_label.imgtk = None
        
        self.update_status("Ready", '#4CAF50')
        
    def start_camera(self):
        if self.camera_running:
            return
            
        self.update_status("Starting RealSense camera...", '#FFA500')
        self.start_button.config(state=tk.DISABLED)
        
        try:
            self.pipeline = rs.pipeline()
            config = rs.config()
            config.enable_stream(rs.stream.depth, 1280, 720, rs.format.z16, 30)
            
            self.pipeline.start(config)
            
            # Warm up camera
            for _ in range(10):
                self.pipeline.wait_for_frames()
            
            self.camera_running = True
            self.capture_button.config(state=tk.NORMAL)
            self.update_status("Camera running - Press 'Capture' or 'S' to capture frame", '#4CAF50')
            
            # Start video feed thread
            self.video_thread = threading.Thread(target=self.update_video_feed, daemon=True)
            self.video_thread.start()
            
        except Exception as e:
            messagebox.showerror("Camera Error", f"Failed to start camera:\n{str(e)}")
            self.update_status("Failed to start camera", '#f44336')
            self.start_button.config(state=tk.NORMAL)
            
    def update_video_feed(self):
        while self.camera_running and not self.captured_frame:
            try:
                frames = self.pipeline.wait_for_frames()
                depth_frame = frames.get_depth_frame()
                
                if not depth_frame:
                    continue
                
                # Process depth image for display
                depth_image = np.asanyarray(depth_frame.get_data())
                depth_m = depth_image.astype(np.float32) * DEPTH_SCALE
                depth_m = np.clip(depth_m, MIN_DEPTH_M, MAX_DEPTH_M)
                
                depth_max = np.nanmax(depth_m)
                if depth_max > 0:
                    depth_display = (depth_m / depth_max * 255).astype(np.uint8)
                    depth_display = cv2.applyColorMap(depth_display, cv2.COLORMAP_JET)
                else:
                    depth_display = np.zeros((depth_m.shape[0], depth_m.shape[1], 3), dtype=np.uint8)
                
                # Add text overlay
                cv2.putText(depth_display, "Press 'Capture' or 'S' to capture", 
                           (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
                
                # Convert to tkinter image
                depth_display_rgb = cv2.cvtColor(depth_display, cv2.COLOR_BGR2RGB)
                
                # Resize to fit window
                h, w = depth_display_rgb.shape[:2]
                max_w, max_h = 960, 540
                scale = min(max_w/w, max_h/h)
                new_w, new_h = int(w*scale), int(h*scale)
                depth_display_rgb = cv2.resize(depth_display_rgb, (new_w, new_h))
                
                img = Image.fromarray(depth_display_rgb)
                imgtk = ImageTk.PhotoImage(image=img)
                
                self.video_label.imgtk = imgtk
                self.video_label.configure(image=imgtk, text="")
                
            except Exception as e:
                print(f"Error updating video feed: {e}")
                break
                
    def capture_frame(self):
        if not self.camera_running or self.captured_frame:
            return
            
        try:
            # Stop the video feed loop first
            self.camera_running = False
            
            # Get one more frame for capture
            frames = self.pipeline.wait_for_frames()
            depth_frame = frames.get_depth_frame()
            
            if not depth_frame:
                messagebox.showerror("Capture Error", "Failed to capture frame")
                self.camera_running = True  # Resume if capture failed
                return
            
            self.captured_frame = depth_frame
            
            # Stop pipeline
            if self.pipeline:
                try:
                    self.pipeline.stop()
                except:
                    pass
            
            self.capture_button.config(state=tk.DISABLED)
            self.update_status("Frame captured! Preparing ROI selection...", '#FFA500')
            
            # Schedule ROI selection on main thread after a short delay
            self.root.after(100, self.show_roi_selection)
            
        except Exception as e:
            messagebox.showerror("Capture Error", f"Failed to capture frame:\n{str(e)}")
            self.camera_running = True  # Resume if capture failed
            
    def show_roi_selection(self):
        """Show ROI selection window - runs on main thread"""
        try:
            # Get depth image
            depth_image = np.asanyarray(self.captured_frame.get_data())
            self.depth_image = depth_image.astype(np.float32) * DEPTH_SCALE
            self.depth_image = np.clip(self.depth_image, MIN_DEPTH_M, MAX_DEPTH_M)
            
            # Create display for ROI selection
            depth_display = (self.depth_image / np.nanmax(self.depth_image) * 255).astype(np.uint8)
            depth_display = cv2.applyColorMap(depth_display, cv2.COLORMAP_JET)
            
            # Ensure any existing OpenCV windows are destroyed first
            cv2.destroyAllWindows()
            cv2.waitKey(1)  # Process any pending events
            
            self.update_status("Select ROI in the window...", '#FFA500')
            
            # Use OpenCV for ROI selection - MUST run on main thread
            cv2.namedWindow("Select ROI", cv2.WINDOW_NORMAL)
            cv2.resizeWindow("Select ROI", 960, 540)
            roi = cv2.selectROI("Select ROI", depth_display, showCrosshair=True, fromCenter=False)
            cv2.destroyAllWindows()
            cv2.waitKey(1)  # Process any pending events
            
            x, y, w, h = roi
            if w == 0 or h == 0:
                self.update_status("No ROI selected", '#f44336')
                self.reset_button.config(state=tk.NORMAL)
                messagebox.showwarning("ROI Selection", "No ROI selected. Click 'Reset' to try again.")
                return
            
            # Store ROI and process in background thread
            self.roi = (x, y, w, h)
            threading.Thread(target=self.process_captured_frame, daemon=True).start()
            
        except Exception as e:
            self.update_status("ROI selection failed", '#f44336')
            self.reset_button.config(state=tk.NORMAL)
            messagebox.showerror("ROI Error", f"Failed to select ROI:\n{str(e)}\n\nClick 'Reset' to try again.")
            
    def process_captured_frame(self):
        try:
            self.update_status("Processing depth data...", '#FFA500')
            
            # Get ROI coordinates
            x, y, w, h = self.roi
            
            # Process ROI
            cropped = self.depth_image[y:y+h, x:x+w]
            h_crop, w_crop = cropped.shape
            aspect_target = TARGET_WIDTH / TARGET_HEIGHT
            aspect_image = w_crop / h_crop
            
            # Pad to match aspect ratio
            if aspect_image > aspect_target:
                new_h = int(round(w_crop / aspect_target))
                pad_top = (new_h - h_crop) // 2
                pad_bottom = new_h - h_crop - pad_top
                padded = np.pad(cropped, ((pad_top, pad_bottom), (0, 0)), constant_values=np.nan)
            else:
                new_w = int(round(h_crop * aspect_target))
                pad_left = (new_w - w_crop) // 2
                pad_right = new_w - w_crop - pad_left
                padded = np.pad(cropped, ((0, 0), (pad_left, pad_right)), constant_values=np.nan)
            
            median_depth = np.nanmedian(padded)
            padded = np.nan_to_num(padded, nan=median_depth)
            
            # Resize to target dimensions
            resized = cv2.resize(padded, (TARGET_WIDTH, TARGET_HEIGHT), interpolation=cv2.INTER_AREA)
            min_d, max_d = np.min(resized), np.max(resized)
            normalized = (resized - min_d) / (max_d - min_d + 1e-6)
            pin_heights_m = normalized * MAX_PIN_DISPLACEMENT_M
            
            # Save outputs
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            output_folder = f"../pin_outputs/{timestamp}"
            os.makedirs(output_folder, exist_ok=True)
            
            output_csv = os.path.join(output_folder, "pins_32x32.csv")
            output_plot = os.path.join(output_folder, "pin_height_map.png")
            np.savetxt(output_csv, pin_heights_m, delimiter=",", fmt="%.5f")
            
            plt.figure(figsize=(6, 4))
            plt.imshow(pin_heights_m, cmap='gray', origin='upper')
            plt.title(f"{TARGET_WIDTH}x{TARGET_HEIGHT} Pin Height Map")
            plt.colorbar(label="Height (m)")
            plt.xlabel("X (pins)")
            plt.ylabel("Y (pins)")
            plt.savefig(output_plot, dpi=150)
            plt.close()
            
            self.update_status("Sending data to Arduino...", '#FFA500')
            
            # Send over UART
            data_bytes = pin_heights_m.astype(np.float32).tobytes()
            try:
                with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
                    ser.write(data_bytes)
                    
                success_msg = f"✓ Successfully sent {len(data_bytes)} bytes to Arduino!\n\nData saved to: {output_folder}\n\nClick 'Reset' to capture another frame."
                self.update_status("Data sent successfully!", '#4CAF50')
                self.reset_button.config(state=tk.NORMAL)
                self.root.after(0, lambda: messagebox.showinfo("Success", success_msg))
                
            except Exception as e:
                error_msg = f"Failed to send data over UART:\n{str(e)}\n\nData was saved to: {output_folder}\n\nClick 'Reset' to try again."
                self.update_status("Failed to send data to Arduino", '#f44336')
                self.reset_button.config(state=tk.NORMAL)
                self.root.after(0, lambda: messagebox.showerror("UART Error", error_msg))
                
        except Exception as e:
            self.update_status("Processing failed", '#f44336')
            self.reset_button.config(state=tk.NORMAL)
            self.root.after(0, lambda: messagebox.showerror("Processing Error", f"Failed to process frame:\n{str(e)}\n\nClick 'Reset' to try again."))
            
    def quit_app(self):
        if self.camera_running and self.pipeline:
            self.camera_running = False
            try:
                self.pipeline.stop()
            except:
                pass
        self.root.quit()


def main():
    root = tk.Tk()
    app = LivePinGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()