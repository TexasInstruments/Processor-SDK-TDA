# Use public ubuntu:22.04 mirror from Artifactory (shared-ubuntu-22.04 blob missing/corrupt)
FROM artifactory.itg.ti.com/docker-public/library/ubuntu:22.04

# Define build argument for SSH private key (passed during docker build, never stored in repo)
ARG ssh_prv_key

# Set proxy for apt (needed for TI network)
ENV http_proxy=http://webproxy.ext.ti.com:80
ENV https_proxy=http://webproxy.ext.ti.com:80
# Prevent tzdata (and other packages) from blocking on interactive timezone prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install all packages needed to replace shared-ubuntu-22.04 base.
# shared-ubuntu-22.04 was built on ghcr.io/crops/poky:ubuntu-22.04 (full Yocto
# toolchain image) plus lcpd-docker additions. Plain ubuntu:22.04 has none of
# that, so we install the full required set here.
RUN apt-get update && apt-get install -y \
    \
    # Mono: SBL appimage generation \
    mono-complete=6.8.0.* \
    \
    # Core build tools \
    build-essential \
    autoconf \
    automake \
    autotools-dev \
    bc \
    bison \
    cmake \
    flex \
    m4 \
    make \
    ninja-build \
    ccache \
    unifdef \
    \
    # Version control \
    git \
    git-lfs \
    \
    # Download / network tools \
    curl \
    wget \
    rsync \
    openssh-client \
    \
    # Python (repo tool + PDK scripts) \
    python3 \
    python3-venv \
    python3-pip \
    \
    # Scripting (PDK Makefiles use perl) \
    perl \
    perl-modules \
    libxml-simple-perl \
    libjson-perl \
    \
    # Archive tools \
    unzip \
    zip \
    zstd \
    liblz4-tool \
    lzop \
    \
    # 32-bit compatibility for TI CGT cross-compiler binaries \
    lib32z1 \
    libc6-i386 \
    libncurses5 \
    libtinfo5 \
    \
    # Utilities \
    jq \
    coreutils \
    time \
    xxd \
    rename \
    expect \
    gawk \
    kmod \
    pkg-config \
    \
    # Libraries used by cross-compiler and build scripts \
    libssl-dev \
    libncurses-dev \
    libelf-dev \
    libxml2-dev \
    libcurl4-gnutls-dev \
    \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Install Google repo tool
RUN curl https://storage.googleapis.com/git-repo-downloads/repo > /usr/local/bin/repo && \
    chmod a+x /usr/local/bin/repo

# Configure wget to retry on failure (handles intermittent connectivity to internal
# TI servers like bangsdowebsvr01.india.ti.com used by download_dep_components.sh)
RUN echo "tries = 3\ntimeout = 60\nwaitretry = 10\nretry-connrefused = on" > /root/.wgetrc

# Verify tools are installed
RUN mono --version && repo --version

# Setup SSH private key from build argument
RUN mkdir -p /root/.ssh && chmod 700 /root/.ssh && \
    echo "$ssh_prv_key" > /root/.ssh/id_rsa && \
    chmod 600 /root/.ssh/id_rsa

# Add known hosts for Bitbucket and NAS server to avoid SSH prompts
RUN ssh-keyscan -H bitbucket.itg.ti.com >> /root/.ssh/known_hosts 2>&1 && \
    ssh-keyscan -t rsa bitbucket.itg.ti.com >> /root/.ssh/known_hosts 2>&1 && \
    ssh-keyscan -H epswnas.itg.ti.com >> /root/.ssh/known_hosts 2>&1 && \
    chmod 600 /root/.ssh/known_hosts
