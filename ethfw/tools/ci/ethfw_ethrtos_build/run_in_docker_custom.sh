#!/usr/bin/env bash

# Custom wrapper for run_in_docker.sh that uses a custom Docker image with mono-complete
# This script builds the custom Docker image and then delegates to the original run_in_docker.sh
# but with the image name overridden

SCRIPT_DIR="$(cd "$(dirname "$0")"; pwd)"

# Find the lcpd-docker directory (should be in workspace)
LCPD_DOCKER_DIR=""
if [ -d "${WORKSPACE}/lcpd-docker" ]; then
    LCPD_DOCKER_DIR="${WORKSPACE}/lcpd-docker"
elif [ -d "$(dirname "$SCRIPT_DIR")/../../../../lcpd-docker" ]; then
    LCPD_DOCKER_DIR="$(cd "$(dirname "$SCRIPT_DIR")/../../../../lcpd-docker" && pwd)"
fi

if [ -z "$LCPD_DOCKER_DIR" ] || [ ! -d "$LCPD_DOCKER_DIR" ]; then
    echo "ERROR: Cannot find lcpd-docker directory"
    exit 1
fi

echo "Using lcpd-docker from: $LCPD_DOCKER_DIR"

# Build custom Docker image with mono-complete
echo "Building custom Docker image with mono-complete..."
docker build -t ethfw-build-with-mono:latest \
  --build-arg ssh_prv_key="$SSH_PRV_KEY" \
  -f ${SCRIPT_DIR}/Dockerfile \
  ${SCRIPT_DIR}/

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to build custom Docker image"
    exit 1
fi

# Tag it with the Artifactory name so lcpd-docker uses it
echo "Tagging custom image to override Artifactory image..."
docker tag ethfw-build-with-mono:latest artifactory.itg.ti.com/docker-lcpd-docker-local/shared-ubuntu-22.04:latest

# Disable docker pull by patching docker.sh to not pull from Artifactory
echo "Disabling docker pull to use local image..."
sed -i 's/^fsDockerPull="true"/fsDockerPull="false"/' ${LCPD_DOCKER_DIR}/lib/docker.sh

echo "Custom Docker image ready. Calling original run_in_docker.sh..."

# Call the original run_in_docker.sh
exec ${LCPD_DOCKER_DIR}/run_in_docker.sh "$@"
