# Linux-Based Multi-Threaded Log Processing Service

## Role
C++ / Linux / Docker Engineer (Fresher)

## Description
A C++ application that monitors a log file in real time, processes logs using multiple threads,
categorizes them by level, and runs inside Docker.

## Features
- Multithreaded producer-consumer model
- Thread-safe queue using mutex and condition variable
- Real-time log processing
- ERROR logs written to a separate file
- Summary printed every 10 seconds
- Graceful shutdown using SIGINT
- Dockerized execution

## Thread Model
- Log Reader Thread: Reads app.log continuously
- Log Processor Thread: Parses and categorizes logs
- Main Thread: Prints summary

## Build & Run
```bash
make
./log_processor
