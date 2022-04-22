FROM ubuntu:20.04

ARG USERNAME=vscode

ARG DEBIAN_FRONTEND=noninteractive

# Install necessary software
RUN apt-get update && apt-get install --no-install-recommends -y \
    git \
    gnupg \
    curl \
    sudo \
    locales \
    python3 \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

# Setup locales
RUN sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen \
    && locale-gen

ENV LC_ALL en_US.UTF-8
ENV LC_CTYPE en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en

# Create a new sudo user with the right permissions
RUN groupadd -g 986 ttyusb && \
    useradd -ms /bin/bash -G dialout,plugdev,ttyusb $USERNAME \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/$USERNAME

USER $USERNAME

WORKDIR /home/$USERNAME

# Install ESPtool
RUN pip3 install esptool

# Install PlatformIO Core CLI
RUN curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o get-platformio.py \
    && python3 get-platformio.py \
    && echo "export PATH=\$PATH:/home/$USERNAME/.platformio/penv/bin" >> ~/.bashrc \
    && rm get-platformio.py
