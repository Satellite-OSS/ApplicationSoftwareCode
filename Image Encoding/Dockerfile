FROM docker.xuanyuan.me/python:3.9-slim

RUN pip install Pillow

COPY ./data /data/


WORKDIR /app


COPY 5 .


CMD ["python", "encoding.py", "/data/input.png", "50"]
