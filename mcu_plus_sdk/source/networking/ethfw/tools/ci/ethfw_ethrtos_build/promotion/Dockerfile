ARG DOCKER_IMAGE_LOCATION='artifactory.itg.ti.com/docker-public/library/ubuntu:22.04'
FROM ${DOCKER_IMAGE_LOCATION}

WORKDIR /app

# Define build arguments for SSH keys (passed during docker build)
ARG ssh_prv_key
ARG ssh_pub_key

# Install git, openssh-client, and Python dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    git \
    openssh-client \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip3 install requests

# Setup SSH keys from build arguments
RUN mkdir -p /root/.ssh && chmod 700 /root/.ssh && \
    echo "$ssh_prv_key" > /root/.ssh/id_rsa && \
    echo "$ssh_pub_key" > /root/.ssh/id_rsa.pub && \
    chmod 600 /root/.ssh/id_rsa && \
    chmod 600 /root/.ssh/id_rsa.pub

# Add Bitbucket to known hosts to avoid SSH prompts
# Use multiple methods to ensure it works
RUN ssh-keyscan -H bitbucket.itg.ti.com >> /root/.ssh/known_hosts 2>&1 && \
    ssh-keyscan -t rsa bitbucket.itg.ti.com >> /root/.ssh/known_hosts 2>&1 && \
    chmod 600 /root/.ssh/known_hosts

# Create SSH config to disable strict host key checking as fallback
RUN echo "Host bitbucket.itg.ti.com" > /root/.ssh/config && \
    echo "    StrictHostKeyChecking no" >> /root/.ssh/config && \
    echo "    UserKnownHostsFile /dev/null" >> /root/.ssh/config && \
    chmod 600 /root/.ssh/config

# Copy application files
COPY promotion_complete.py .
COPY ethfw_jenkins.cfg .
COPY ethfw_branch.cfg .

CMD ["python3", "promotion_complete.py"]

