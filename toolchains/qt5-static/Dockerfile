# Base image is version-tagged; switch to a digest-pinned reference when refreshed online.
FROM ubuntu:16.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
    bison \
    build-essential \
    ca-certificates \
    curl \
    file \
    flex \
    gperf \
    # libdbus-1-dev \
    libglib2.0-dev \
    # libsqlite3-dev \
    libssl-dev \
    libx11-dev \
    libx11-xcb-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxdamage-dev \
    libxext-dev \
    libxfixes-dev \
    libxi-dev \
    # libxml2-dev \
    libxrandr-dev \
    libxrender-dev \
    # libxslt1-dev \
    libxcb1-dev \
    patch \
    perl \
    pkg-config \
    python \
    ruby \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
