# Base image is version-tagged; switch to a digest-pinned reference when refreshed online.
FROM ubuntu:18.04

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
    libdbus-1-dev \
    libfreetype6-dev \
    libglib2.0-dev \
    libicu-dev \
    libjpeg-dev \
    libpng-dev \
    libsqlite3-dev \
    libssl-dev \
    libx11-dev \
    libx11-xcb-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxdamage-dev \
    libxext-dev \
    libxfixes-dev \
    libxi-dev \
    libxml2-dev \
    libxrandr-dev \
    libxrender-dev \
    libxslt1-dev \
    libxcb1-dev \
    libxcb-glx0-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-render0-dev \
    libxcb-shape0-dev \
    libxcb-shm0-dev \
    libxcb-sync-dev \
    libxcb-util0-dev \
    libxcb-xfixes0-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    patch \
    perl \
    pkg-config \
    python \
    ruby \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
