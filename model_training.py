# from ultralytics import YOLO
# import multiprocessing
# import torch
# import sys
# import platform


# def main():
#     # Diagnostic: print torch / CUDA info
#     try:
#         print(f"python: {sys.executable} {platform.python_version()}")
#         print("torch:", getattr(torch, '__version__', 'unknown'))
#         print("torch build cuda:", getattr(torch.version, 'cuda', None))
#         # print("torch.cuda.is_built():", torch.cuda.is_built())
#         print("torch.cuda.is_available():", torch.cuda.is_available())
#         try:
#             print("torch.cuda.device_count():", torch.cuda.device_count())
#             if torch.cuda.is_available() and torch.cuda.device_count() > 0:
#                 try:
#                     print("torch.cuda.get_device_name(0):", torch.cuda.get_device_name(0))
#                 except Exception as e:
#                     print("get_device_name failed:", e)
#         except Exception:
#             pass
#     except Exception as e:
#         print("CUDA diagnostic failed:", e)

#     # Load YOLOv11-small backbone with previous weights for fine-tuning
#     model = YOLO("best.pt")

#     # Train settings
#     # If running on Windows and having multiprocessing issues, set workers=0
#     workers = 1
#     if platform.system().lower().startswith("win"):
#         # allow user override via env var, otherwise keep 4
#         workers = 4

#     model.train(
#         data="./Training/data.yaml",
#         epochs=100,
#         imgsz=640,         # larger image for better accuracy
#         batch=4,           # adjust to fit GPU memory
#         device=0,
#         workers=0,         # Windows safe
#         patience=30,
#         project="models",
#         name="yolo11s_shapes_finetune640",
#         optimizer="AdamW"
#     )



# if __name__ == "__main__":
#     multiprocessing.freeze_support()
#     main()

from ultralytics import YOLO
import platform
import multiprocessing

# --------------------------
# CONFIGURATION
# --------------------------

# Path to your starting model (backbone from previous best)
PRETRAINED_MODEL = "D:/AI_Models/NewGen-Recognizor/weights/best.pt"  # or yolo11s.pt if starting fresh

# Path to merged dataset
DATA_YAML = "New-Data/data.yaml"

# Training parameters
EPOCHS = 100
IMGSZ = 640
BATCH_SIZE = 4
DEVICE = 0  # GPU index, use 'cpu' if GPU unavailable
PATIENCE = 30  # early stopping
OPTIMIZER = "AdamW"

# Determine workers for Windows multiprocessing
WORKERS = 2
if platform.system().lower().startswith("win"):
    WORKERS = 2  # Adjust if needed

# --------------------------
# TRAINING
# --------------------------

def main():
    # Load model
    model = YOLO(PRETRAINED_MODEL)

    # Start training
    model.train(
        data=DATA_YAML,
        epochs=EPOCHS,
        imgsz=IMGSZ,
        batch=BATCH_SIZE,
        device=DEVICE,
        workers=WORKERS,
        patience=PATIENCE,
        project="models",          # Output parent folder
        name="D:/AI_Models/NewGen-Recognizor-O",
        optimizer=OPTIMIZER,
    )

if __name__ == "__main__":
    multiprocessing.freeze_support()  # Required for Windows
    main()