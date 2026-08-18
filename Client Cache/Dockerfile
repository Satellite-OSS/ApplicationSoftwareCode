FROM docker.xuanyuan.me/python:3.9

WORKDIR /binary
copy client .

RUN pip install --no-cache-dir grpcio grpcio-tools protobuf Flask

WORKDIR /app
COPY 5 .


CMD ["./start.sh"]
