FROM docker.xuanyuan.me/ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai


RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    libopencv-dev \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

COPY ./data /data


RUN mkdir /app

WORKDIR /app



COPY 50FF48/ .

ENV DISPLAY=:0

ENTRYPOINT ["/app/ovx1_app_cn"]
CMD ["../data/video.mp4"]
