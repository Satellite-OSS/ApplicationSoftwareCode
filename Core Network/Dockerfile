FROM docker.xuanyuan.me/ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y build-essential make ca-certificates && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY 103F16 ./



CMD ["./core_nf_simulation"]
