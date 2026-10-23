#!/bin/bash
#
# Set up c7x-mma-tidl dependencies for a given SOC.
#
# Usage:
#   ./setup.sh <SOC>           # SOC passed as argument
#   SOC=j784s4 ./setup.sh      # SOC from environment variable
#
#   SOC: j721e | j721s2 | j784s4 | j742s2 | j722s | am62a | tda54
#
# Steps:
#   1. git submodule init       (arm-tidl, tidl-kernels)
#   2. git submodule update
#   3. Download mmalib for the target SOC
#   4. Configure repo for device via configureRepoForDevice.py
#

set -e

# Save original calling directory
CALLING_DIR="$(pwd)"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Resolve SOC: argument takes priority, then environment variable
if [ -n "$1" ]; then
    SOC="$1"
elif [ -n "${SOC}" ]; then
    : # use SOC from environment
else
    echo "[setup] ERROR: SOC not specified. Pass as argument or set \$SOC env var."
    echo "[setup]   Usage: $0 <SOC>"
    echo "[setup]   SOC: j721e j721s2 j784s4 j742s2 j722s am62a tda54"
    exit 1
fi

VALID_SOCS="j721e j721s2 j784s4 j742s2 j722s am62a tda54"
case " ${VALID_SOCS} " in
    *" ${SOC} "*) : ;;
    *) echo "[setup] ERROR: Invalid SOC '${SOC}'. Supported: ${VALID_SOCS}"; exit 1 ;;
esac

echo "[setup] SOC=${SOC}"

# Step 1 & 2: Initialize and update submodules (arm-tidl, tidl-kernels)
# Skip if .git directory is absent (e.g. released/packaged with .git removed).
if [ -d "${SCRIPT_DIR}/.git" ]; then
    echo "[setup] Initializing submodules ..."
    git -C "${SCRIPT_DIR}" submodule init
    echo "[setup] Updating submodules ..."
    git -C "${SCRIPT_DIR}" submodule update
    echo "[setup] Submodules done"
else
    echo "[setup] No .git directory found, skipping submodule init/update"
fi

# Step 3: Download mmalib
# Download is skipped if mmalib_<VERSION>_<SOC>/ already exists.
if [ -f "${SCRIPT_DIR}/mmalib-kernels/download_mmalib.sh" ]; then
    echo "[setup] Downloading mmalib for ${SOC} ..."
    if ! bash "${SCRIPT_DIR}/mmalib-kernels/download_mmalib.sh" "${SOC}"; then
        echo "[setup] ERROR: mmalib download failed for SOC=${SOC}"
        exit 1
    fi
fi

# Step 4: Configure repo for device
if [ -f "${SCRIPT_DIR}/ti_dl/utils/python/configureRepoForDevice.py" ]; then
    echo "[setup] Configuring repo for ${SOC} ..."
    (cd "${SCRIPT_DIR}/ti_dl/utils/python" && python3 configureRepoForDevice.py "${SOC}")
    echo "[setup] Configure done"
fi

echo "[setup] Done"

# Return to original calling directory
cd "${CALLING_DIR}"
