# Extend the existing shared-ubuntu-22.04 image from Artifactory
FROM artifactory.itg.ti.com/docker-lcpd-docker-local/shared-ubuntu-22.04:latest

# Define build argument for SSH private key (passed during docker build, never stored in repo)
ARG ssh_prv_key

# Set proxy for apt (needed for TI network)
ENV http_proxy=http://webproxy.ext.ti.com:80
ENV https_proxy=http://webproxy.ext.ti.com:80

# Install required tools:
# - mono-complete: for SBL appimage generation
# - repo: for Google repo tool to manage multiple git repositories
# - curl: needed to download repo tool
# This runs as root during docker build, so no sudo needed
RUN apt-get update && apt-get install -y \
    mono-complete \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Install Google repo tool
RUN curl https://storage.googleapis.com/git-repo-downloads/repo > /usr/local/bin/repo && \
    chmod a+x /usr/local/bin/repo

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
