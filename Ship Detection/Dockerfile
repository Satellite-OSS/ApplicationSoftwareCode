FROM --platform=linux/arm64 docker.xuanyuan.me/ubuntu:22.04


ADD Arial.ttf Arial.Unicode.ttf /root/.config/Ultralytics/


ENV DEBIAN_FRONTEND noninteractive
RUN apt update
RUN TZ=Etc/UTC apt install -y tzdata
RUN apt install --no-install-recommends -y python3-pip git zip curl htop gcc libgl1 libglib2.0-0 libpython3-dev

#COPY requirements.txt .

COPY best.pt /best.pt


COPY requirements.txt .
RUN python3 -m pip install --upgrade pip wheel
RUN pip install --no-cache -r requirements.txt albumentations gsutil notebook \
    coremltools onnx onnxruntime

RUN mkdir -p /app
WORKDIR /app


COPY 5/ ./


CMD ["python3", "detect.py", "--weights","../best.pt", "--source", "img.jpg"]


ENV DEBIAN_FRONTEND teletype
