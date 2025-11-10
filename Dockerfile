FROM arm64v8/debian:bookworm

# Install build dependencies and SSH server
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    file \
    libyaml-cpp-dev \
    libusb-1.0-0-dev \
    zlib1g-dev \
    wget \
    bzip2 \
    openssh-server \
    sudo \
    vim \
    && rm -rf /var/lib/apt/lists/*

# install uldaq (MCC Universal Library for Linux)
ENV ULDAQ_VERSION=1.2.1
RUN wget -N https://github.com/mccdaq/uldaq/releases/download/v${ULDAQ_VERSION}/libuldaq-${ULDAQ_VERSION}.tar.bz2 && \
    tar -xvjf libuldaq-${ULDAQ_VERSION}.tar.bz2 && \
    cd libuldaq-${ULDAQ_VERSION} && \
    ./configure && make && make install && \
    cd .. && rm -rf libuldaq-${ULDAQ_VERSION} libuldaq-${ULDAQ_VERSION}.tar.bz2

# Create SSH user (default user: dev, password: dev)
RUN useradd -m -s /bin/bash dev && \
    echo "dev:dev" | chpasswd && \
    adduser dev sudo

# Configure SSH to allow password authentication and disable PAM restrictions
RUN sed -i 's/#\?PasswordAuthentication .*/PasswordAuthentication yes/' /etc/ssh/sshd_config && \
    sed -i 's/#\?PermitRootLogin .*/PermitRootLogin no/' /etc/ssh/sshd_config && \
    mkdir -p /var/run/sshd

# Set working directory
WORKDIR /home/dev/app

# Default command: start SSH service
CMD ["/usr/sbin/sshd", "-D"]

# EXPOSE SSH port
EXPOSE 22
