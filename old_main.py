# ---------------------------------------------------------
# YOLOv11-S Real-time Webcam Detection (Proof of Concept)
# ---------------------------------------------------------

# Import required libraries
from ultralytics import YOLO            # Loads YOLO models
import cv2                               # Used for webcam + display

# Load YOLOv11-S pretrained model
# (this will auto-download on first run)
# D:/AI_Models/Shape-Recognizor4/weights/best.pt
model = YOLO("D:/AI_Models/NewGen-Recognizor/weights/best.pt")               # Choose yolo11s, yolo11m, etc.
# model.predict(source=0, show=True, conf=0.75)

# Create a resizable window
cv2.namedWindow("YOLOv11-S Live", cv2.WINDOW_NORMAL)

# Open default webcam (0 = the first camera)
cap = cv2.VideoCapture(0)

# Loop continuously to read webcam frames
while True:
    ret, frame = cap.read()              # Read a frame from webcam
    if not ret:
        break                            # Stop if camera error

    # Run YOLO object detection on the frame
    results = model(frame)

    # Draw detection bounding boxes & labels onto image
    annotated_frame = results[0].plot()

    # Display result in window titled "YOLOv11-S Live"
    cv2.imshow("YOLOv11-S Live", annotated_frame)

    # Press 'q' to exit
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Cleanup: release camera and close window
cap.release()
cv2.destroyAllWindows()