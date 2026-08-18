FROM docker.xuanyuan.me/ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libopencv-dev \
    libgdal-dev \
    libopenjp2-7-dev \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Copy images folder to the root directory
COPY images /images

# Set the working directory to /app
WORKDIR /app


# Copy EarthPlusSim from the local 'origin' directory to /app/EarthPlusSim in the container
COPY 503F128/ .

ENTRYPOINT ["./EarthPlusSim"]
