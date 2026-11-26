ARG DOCKER_IMAGE_LOCATION='artifactory.itg.ti.com/docker-public/library/ubuntu:22.04'
FROM ${DOCKER_IMAGE_LOCATION}

WORKDIR /app

# Install git, openssh-client, and Python dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    git \
    openssh-client \
    rsync \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip3 install requests

# No SSH keys baked into the image: the build agent's own ~/.ssh is bind-mounted
# into the container at runtime instead (same pattern as lcpd-docker's docker_run(),
# see ethrtos_ethfw_rtos's workspace.sh flow) - that identity is already authorized
# on both bitbucket.itg.ti.com and epswnas.itg.ti.com.
RUN mkdir -p /root/.ssh && chmod 700 /root/.ssh && \
    ssh-keyscan -H bitbucket.itg.ti.com >> /root/.ssh/known_hosts 2>&1 && \
    ssh-keyscan -t rsa bitbucket.itg.ti.com >> /root/.ssh/known_hosts 2>&1 && \
    ssh-keyscan -H epswnas.itg.ti.com >> /root/.ssh/known_hosts 2>&1 && \
    chmod 600 /root/.ssh/known_hosts

# Copy application files
COPY promotion_complete.py .
COPY ethfw_jenkins.cfg .
COPY ethfw_branch.cfg .

CMD ["python3", "promotion_complete.py"]

