FROM docker.xuanyuan.me/python:3.9-slim

RUN pip install --no-cache-dir scikit-image torch torchvision numpy PyYAML Pillow

COPY checkpoints /checkpoints
COPY configs /configs
COPY input.jpg /input.jpg

WORKDIR /app

COPY 5/ ./



CMD ["python", "compress.py", "--image", "../input.jpg", "--config", "../configs/low_rate.yaml", "--model", "../checkpoints/low_rate_epoch50.pth"]
