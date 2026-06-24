#!/bin/bash
set -e

#############################################################
########################[ Variables ]########################

CICD_TAG="CICD.PSDK.ANALYTICS.12.00.00.02"
REPO_URL="https://github.com/TexasInstruments/ProcessorSDK"
RELEASE_ARTIFACTS_URL="$REPO_URL/releases/download/$CICD_TAG"

MMALIB_VERSION="11_02_00_11"
DSPLIB_VERSION="3_4_0_0"
MATHLIB_VERSION="3_1_2_1"

#############################################################
########################[ Functions ]########################

download_file(){
    # This function takes in the URL of a file and the output directory, 
    # and uses wget to download the file to the specified directory.
    local FILE_URL="$1"
    local OUTPUT_DIR="$2"
    local FILE_NAME=$(basename "$FILE_URL")

    if [ -f "$OUTPUT_DIR/$FILE_NAME" ]; then
        echo "[ Info ]: $FILE_NAME already exists in $OUTPUT_DIR, skipping download."
        return 0
    fi
    
    wget -P "$OUTPUT_DIR" "$FILE_URL"
}

install_mmalib(){
    # This function takes in the URL of the mmalib tar file, the C7x DSP version, and the mmalib version.
    # It downloads the tar file, extracts the relevant contents to a directory named "mmalib_<MMALIB_VERSION>_<SOC>",
    # and then creates a symbolic link named "mmalib_<MMALIB_VERSION>" that points to the extracted directory.
    local FILE_URL="$1"
    local DSP_VERSION="$2"
    local MMALIB_VERSION="$3"
    local TAR_FILE_NAME=$(basename "$FILE_URL")
    local OUT_DIR="mmalib_${MMALIB_VERSION}_${SOC}"

    if [ -d "$OUT_DIR" ]; then
        echo "[ Info ]: mmalib version $MMALIB_VERSION for SOC $SOC already exists in $OUT_DIR, skipping download and extraction."
        ln -sfn "$OUT_DIR" "mmalib_$MMALIB_VERSION"
        return 0
    fi
    
    # Download the tar file, extract it, and organize the contents
    wget $FILE_URL
    mkdir -p "$OUT_DIR"
    tar -xf "$TAR_FILE_NAME" --strip=2 -C "$OUT_DIR"
    rm -rf "$TAR_FILE_NAME"
    ln -s "$OUT_DIR" "mmalib_$MMALIB_VERSION"
}

install_bin(){
    # This function takes in the URL of a .bin file, downloads it,
    # executes it with silent mode and a specified output directory, and then removes the .bin file.
    local FILE_URL="$1"
    local OUT_DIR="$2"
    local BIN_FILE_NAME=$(basename "$FILE_URL")

    if [ -d "$OUT_DIR" ]; then
        echo "[ Info ]: $OUT_DIR already exists, skipping download and installation of $BIN_FILE_NAME."
        return 0
    fi
    
    # Download the .bin file, execute it, and clean up    
    wget $FILE_URL
    chmod +x "$BIN_FILE_NAME"
    ./"$BIN_FILE_NAME" --mode silent --prefix "$OUT_DIR"
    rm -rf "$BIN_FILE_NAME"
}

check_env() {
    # This function checks if a given environment variable is set. 
    # It takes in the name of the variable and a string of options for that variable.
    local var_name=$1
    local options=$2
    
    if [ -z "${!var_name}" ]; then
        echo "[ Error ]: $var_name is not set. Options: $options" >&2
        return 1
    fi
    return 0
}

#############################################################
##########################[ Main ]###########################

# validate the required environment variables
MISSING_ENV=0
check_env "SOC" "j721s2, j784s4, j742s2, j721e, j722s" || MISSING_ENV=1
check_env "TISDK_IMAGE" "adas, edgeai" || MISSING_ENV=1

if [ "$MISSING_ENV" -eq 1 ]; then
    exit 1
fi

######################################################

# Downlaod Linux targetfs and bootfs
echo "[ Info ] Starting download of Linux targetfs and bootfs for SOC: $SOC"

LINUX_TARGETFS_URL="$RELEASE_ARTIFACTS_URL/tisdk-adas-image-${SOC}-evm.tar.xz"
LINUX_BOOTFS_URL="$RELEASE_ARTIFACTS_URL/boot-adas-${SOC}-evm.tar.gz"
download_file "$LINUX_TARGETFS_URL" "."
download_file "$LINUX_BOOTFS_URL" "."

######################################################

# Download mmalib, dsplib and mathlib dependencies
echo "[ Info ] Starting download of mmalib, dsplib and mathlib dependencies for SOC: $SOC"

if [ "$SOC" = "j721s2" ] || [ "$SOC" = "j784s4" ] || [ "$SOC" = "j742s2" ]; then
    C7X_DSP_VERSION="C7120"
    MMALIB_URL="$RELEASE_ARTIFACTS_URL/mmalib_obj_${C7X_DSP_VERSION}_${MMALIB_VERSION}.tar.gz"
    # DSPLIB and MATHLIB are not required for these SOCs, so we only download and install mmalib
    install_mmalib "$MMALIB_URL" "$C7X_DSP_VERSION" "$MMALIB_VERSION"

elif [ "$SOC" = "j721e" ]; then
    C7X_DSP_VERSION="C7100"
    MMALIB_URL="$RELEASE_ARTIFACTS_URL/mmalib_obj_${C7X_DSP_VERSION}_${MMALIB_VERSION}.tar.gz"
    DSPLIB_URL="$RELEASE_ARTIFACTS_URL/dsplib_c66x_${DSPLIB_VERSION}_Linux.bin"
    MATHLIB_URL="$RELEASE_ARTIFACTS_URL/mathlib_c66x_${MATHLIB_VERSION}_Linux.bin"
    # For j721e, we need to download and install mmalib, dsplib and mathlib
    install_mmalib "$MMALIB_URL" "$C7X_DSP_VERSION" "$MMALIB_VERSION"
    install_bin "$DSPLIB_URL" "dsplib_c66x_${DSPLIB_VERSION}"
    install_bin "$MATHLIB_URL" "mathlib_c66x_${MATHLIB_VERSION}"

elif [ "$SOC" = "j722s" ]; then
    C7X_DSP_VERSION="C7524"
    MMALIB_URL="$RELEASE_ARTIFACTS_URL/mmalib_obj_${C7X_DSP_VERSION}_${MMALIB_VERSION}.tar.gz"
    install_mmalib "$MMALIB_URL" "$C7X_DSP_VERSION" "$MMALIB_VERSION"
else
    echo "Error: Unsupported SOC: $SOC"
    exit 1
fi

######################################################

# Download toolchain
echo "[ Info ] Starting download of toolchain for SOC: $SOC"
TOOLCHAIN_URL="$RELEASE_ARTIFACTS_URL/linux-devkit-${SOC}.sh"
download_file "$TOOLCHAIN_URL" "toolchain"
mv "toolchain/linux-devkit-${SOC}.sh" "toolchain/linux-devkit.sh"
chmod +x toolchain/linux-devkit.sh

######################################################

# Download and extract tidl libs
echo "[ Info ] Starting download of tidl libs"
TIDL_LIBS_URL="$RELEASE_ARTIFACTS_URL/tidl_libs.tar.gz"
download_file "$TIDL_LIBS_URL" "."
tar -xf "tidl_libs.tar.gz"

######################################################

# Set the environment variables for SOC and TISDK_IMAGE before running the setup script
echo "[ Info ] Running the PSDK RTOS setup script for SOC: $SOC and TISDK_IMAGE: $TISDK_IMAGE"
export SOC
export TISDK_IMAGE
./sdk_builder/scripts/setup_psdk_rtos.sh

######################################################