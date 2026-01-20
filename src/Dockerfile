FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN make

CMD ["./log_processor"]
