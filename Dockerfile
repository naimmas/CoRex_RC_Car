#Fetch Debian image
FROM ubuntu:24.04

# Default command
CMD ["/bin/bash"]

#Update system packages
RUN apt-get update && apt-get upgrade -y

#Install required packages for building
RUN apt-get install -y --no-install-recommends \
        build-essential pkg-config libusb-1.0-0-dev

#Install required tools
RUN apt-get install -y --no-install-recommends \
        git nano wget unzip \
        python3 \
        python3-pip \
        clang-tidy-19 clang-format-19 \
        ruby \
        ca-certificates

RUN pip install --no-cache-dir \
                --break-system-packages \
                gcovr \
                flawfinder \
                lizard \
                junitparser

#Check clang tools
RUN clang-tidy-19 --version
RUN clang-format-19 --version

#Install ceedling
RUN gem install ceedling
#Check ceedling
RUN ceedling version

#Check tools
RUN gcovr --version
RUN flawfinder --version
RUN lizard --version
RUN junitparser --version

#Download arm-none-eabi-gcc and cmake
RUN if [ "$(uname -m)" = "aarch64" ]; then \
        echo "[INFO] Running on aarch64."; \
        wget -q --show-progress https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-aarch64-arm-none-eabi.tar.xz \
        -O /tmp/arm-gnu-toolchain.tar.xz; \
        wget -q --show-progress https://github.com/Kitware/CMake/releases/download/v4.0.2/cmake-4.0.2-linux-aarch64.tar.gz \
        -O /tmp/cmake.tar.gz; \
    else \
        echo "[INFO] Running on x86_64."; \
        apt-get install -y --no-install-recommends libc6-dev-i386; \
        wget -q --show-progress https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz \
        -O /tmp/arm-gnu-toolchain.tar.xz; \
        wget -q --show-progress https://github.com/Kitware/CMake/releases/download/v4.0.2/cmake-4.0.2-linux-x86_64.tar.gz \
        -O /tmp/cmake.tar.gz; \
    fi

#Install arm-none-eabi-gcc
RUN mkdir -p /opt/arm-gnu-toolchain \
    && tar -xf /tmp/arm-gnu-toolchain.tar.xz -C /opt/arm-gnu-toolchain --strip-components=1 \
    && ln -s /opt/arm-gnu-toolchain/bin/* /usr/local/bin/

# Check arm-none-eabi-gcc
RUN arm-none-eabi-gcc --version

#Install cmake
RUN mkdir -p /opt/cmake \
    && tar -xf /tmp/cmake.tar.gz -C /opt/cmake --strip-components=1 \
    && ln -s /opt/cmake/bin/* /usr/local/bin/

#Check cmake
RUN cmake --version

#Install RP2040 toolchain
RUN git clone --branch 2.1.1 --single-branch https://github.com/raspberrypi/pico-sdk.git /opt/pico-sdk
RUN git clone --branch 2.1.1 --single-branch https://github.com/raspberrypi/picotool.git /tmp/picotool

WORKDIR /opt/pico-sdk
RUN git submodule update --init --recursive
ENV PICO_SDK_PATH=/opt/pico-sdk

WORKDIR /tmp/picotool
RUN mkdir build
WORKDIR /tmp/picotool/build
RUN cmake -DCMAKE_INSTALL_PREFIX=/opt/picobuild -DPICOTOOL_FLAT_INSTALL=1 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..
RUN make -j$(nproc) install
ENV PICO_TOOL_PATH=/opt/picobuild/picotool/

#Clean up tmp
RUN rm -rf /tmp/*
#Clean up apt cache
RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ARG USERNAME=jenkins
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# 1) If that UID already exists, rename it to $USERNAME.
# 2) Otherwise create a fresh user + group with that UID/GID.
RUN set -eux; \
    if getent passwd "${USER_UID}" >/dev/null; then \
        olduser=$(getent passwd "${USER_UID}" | cut -d: -f1); \
        echo "[INFO] Re-using UID ${USER_UID}: renaming ${olduser} → ${USERNAME}"; \
        usermod  -l "${USERNAME}" "$olduser"; \
        groupmod -n "${USERNAME}" "$olduser" || true; \
    else \
        if getent group "${USER_GID}" >/dev/null; then \
            echo "[INFO] Re-using GID ${USER_GID}"; \
            groupname=$(getent group "${USER_GID}" | cut -d: -f1); \
        else \
            groupadd -g "${USER_GID}" "${USERNAME}"; \
        fi; \
        useradd  -m -u "${USER_UID}" -g "${USER_GID}" \
                 -s /usr/sbin/nologin "${USERNAME}"; \
    fi

# Writable workspace owned by that user
RUN mkdir -p /home/${USERNAME}/workspace \
 && chown -R "${USER_UID}:${USER_GID}" /home/${USERNAME}

WORKDIR /home/${USERNAME}/workspace
USER ${USERNAME}
