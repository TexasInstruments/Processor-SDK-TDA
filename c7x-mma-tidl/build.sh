#!/bin/bash
set -o pipefail

# build.sh: Script to simplify the build process for TI Processor SDK

# ============================================================
# Environment Variables Usage:
#
# This script uses environment variables directly if they are already set.
# If not provided as environment variables, they must be specified
# through command-line arguments.
#
# Supported environment variables:
#   - TARGET_BUILD     (Build profile: debug or release)
#   - TARGET_PLATFORM  (Target platform: TI_DEVICE or PC)
#   - PSDK_INSTALL_PATH (Path to the SDK installation)
# ============================================================

# Function to display usage information
show_usage() {
    echo "Usage: ./build.sh [options]"
    echo ""
    echo "Options:"
    echo " --help               Show this help message"
    echo " --target_build       Specify build profile (release, debug, all)"
    echo " --target_platform    Specify target platform (TI_DEVICE, PC, all)"
    echo " --psdk_install_path  Path to SDK"
    echo " --scrub              Scrub and Clean before building"
    echo " -j<N>                Number of parallel build jobs (e.g., -j8, -j16)"
    echo " --install            Copy built libraries, headers and tidl_tools tarball. Do not use during development"
    echo " --install_path       Path to copy built libraries, headers and tidl_tools tarball. Do not use during development"
    echo ""
    echo "Environment Variables:"
    echo " TARGET_BUILD         Used as fallback if --target_build option is not provided"
    echo " TARGET_PLATFORM      Used as fallback if --target_platform option is not provided"
    echo " PSDK_INSTALL_PATH    Used as fallback if --psdk_install_path option is not provided"
    echo ""
}

SCRIPT_PATH=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_PATH")

# Save environment variables to temporary variables
ENV_TARGET_BUILD="$TARGET_BUILD"
ENV_TARGET_PLATFORM="$TARGET_PLATFORM"
ENV_PSDK_INSTALL_PATH="$PSDK_INSTALL_PATH"

# Set default values
ARG_TARGET_BUILD=""
ARG_TARGET_PLATFORM=""
ARG_PSDK_INSTALL_PATH=""

SCRUB=0

INSTALL=0
INSTALL_PATH=""

# Set default build threads to nproc-2
NUM_PROC=$(nproc --ignore=2)
MAKE_JOBS="-j${NUM_PROC}"

# Parse command line arguments (with priority over environment variables)
while [[ $# -gt 0 ]]; do
    case $1 in
        --help)
            show_usage
            exit 0
            ;;
        --target_build)
            ARG_TARGET_BUILD="$2"
            shift 2
            ;;
        --target_platform)
            ARG_TARGET_PLATFORM="$2"
            shift 2
            ;;
        --psdk_install_path)
            ARG_PSDK_INSTALL_PATH="$2"
            shift 2
            ;;
        --scrub)
            SCRUB=1
            shift 1
            ;;
        --install)
            INSTALL=1
            shift 1
            ;;
        --install_path)
            INSTALL_PATH="$2"
            shift 2
            ;;
        -j*)
            # Handle -j8, -j16, etc. directly
            MAKE_JOBS="$1"
            shift 1
            ;;
        *)
            echo "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done


if [ -n "$ARG_TARGET_BUILD" ]; then
    TARGET_BUILD="$ARG_TARGET_BUILD"
elif [ -n "$ENV_TARGET_BUILD" ]; then
    TARGET_BUILD="$ENV_TARGET_BUILD"
else
    TARGET_BUILD="release"  # Default
fi

if [ -n "$ARG_TARGET_PLATFORM" ]; then
    TARGET_PLATFORM="$ARG_TARGET_PLATFORM"
elif [ -n "$ENV_TARGET_PLATFORM" ]; then
    TARGET_PLATFORM="$ENV_TARGET_PLATFORM"
else
    TARGET_PLATFORM="TI_DEVICE" # Default
fi

if [ -n "$ARG_PSDK_INSTALL_PATH" ]; then
    PSDK_INSTALL_PATH="$ARG_PSDK_INSTALL_PATH"
elif [ -n "$ENV_PSDK_INSTALL_PATH" ]; then
    PSDK_INSTALL_PATH="$ENV_PSDK_INSTALL_PATH"
fi

# Normalize input
TARGET_BUILD=$(echo "$TARGET_BUILD" | tr '[:upper:]' '[:lower:]')
TARGET_PLATFORM=$(echo "$TARGET_PLATFORM" | tr '[:lower:]' '[:upper:]')

# Validate target_build
if [[ "$TARGET_BUILD" != "debug" && "$TARGET_BUILD" != "release" && "$TARGET_BUILD" != "all" ]]; then
    echo "Error: Invalid TARGET_BUILD '$TARGET_BUILD'. Use release, debug, or all."
    exit 1
fi

# Validate target_platform
if [[ "$TARGET_PLATFORM" != "TI_DEVICE" && "$TARGET_PLATFORM" != "PC" && "$TARGET_PLATFORM" != "ALL" ]]; then
    echo "Error: Invalid TARGET_PLATFORM '$TARGET_PLATFORM'. Use TI_DEVICE, PC, or ALL."
    exit 1
fi

# Validate install
if [[ "$INSTALL" != "0" && "$INSTALL" != "1" ]]; then
    echo "Error: Invalid INSTALL '$INSTALL'. Use 0 or 1."
    exit 1
fi

if [[ "$INSTALL" == "1" && -z "$INSTALL_PATH" ]]; then
    echo "Error: Please define path to install using --install_path"
    exit 1
else
    # Convert install path to absolute path if it's provided
    if [[ -n "$INSTALL_PATH" ]]; then
        INSTALL_PATH=$(realpath -m "$INSTALL_PATH")
    fi
fi

# Validate scrub
if [[ "$SCRUB" != "0" && "$SCRUB" != "1" ]]; then
    echo "Error: Invalid SCRUB '$SCRUB'. Use 0 or 1."
    exit 1
fi

# Helper: run a make command and abort on failure
run_make() {
    make "$@"
    local status=$?
    if [ $status -ne 0 ]; then
        echo ""
        echo "======================================================================"
        echo "ERROR: make failed with exit code $status"
        echo "  Command: make $*"
        echo "======================================================================"
        exit $status
    fi
}

# Function to build with specific configuration
build_with_config() {
    local build_type=$1
    local platform_type=$2

    # Execute the build
    BUILD_FLAGS="TARGET_BUILD=$build_type"
    if [ "$platform_type" == "PC" ]; then
        BUILD_FLAGS="${BUILD_FLAGS} TARGET_PLATFORM=$platform_type"
    fi
    if [ ! -z "$PSDK_INSTALL_PATH" ]; then
        BUILD_FLAGS="${BUILD_FLAGS} PSDK_INSTALL_PATH=$PSDK_INSTALL_PATH"
    fi

    if [ "$SCRUB" == "1" ]; then
        echo
        echo "Scrubbing c7x-mma-tidl"
        run_make -C ${SCRIPT_DIR} scrub ${BUILD_FLAGS} ${MAKE_JOBS}
        exit 0
    else
        if [ "$INSTALL" == "1" ]; then
            BUILD_FLAGS="${BUILD_FLAGS} TIDL_PATH=$INSTALL_PATH"
        fi

        echo
        echo "======================================================================"
        echo "Building c7x-mma-tidl and arm-tidl with the specified configuration..."
        echo
        echo "  PSDK_INSTALL_PATH: $PSDK_INSTALL_PATH"
        echo "  TARGET_BUILD:      $build_type"
        echo "  TARGET_PLATFORM:   $platform_type"
        echo "  SCRUB:             $SCRUB"
        if [ -n "$MAKE_JOBS" ]; then
            echo "  PARALLEL_JOBS:     $MAKE_JOBS"
        fi
        if [ "$INSTALL" == "1" ]; then
            echo "  INSTALL:           $INSTALL"
            echo "  INSTALL_PATH:      $INSTALL_PATH"
        fi
        echo "======================================================================"
        echo

        run_make -C ${SCRIPT_DIR} all ${BUILD_FLAGS} ${MAKE_JOBS}
        if [ "$INSTALL" == "1" ]; then
            run_make -C ${SCRIPT_DIR} install ${BUILD_FLAGS} ${MAKE_JOBS}
        fi

        run_make -C ${SCRIPT_DIR} tidl_tiovx_arm_kernels ${BUILD_FLAGS} ${MAKE_JOBS}
        if [ "$INSTALL" == "1" ]; then
            run_make -C ${SCRIPT_DIR}/arm-tidl tidl_tiovx_arm_kernels_install ${BUILD_FLAGS} ${MAKE_JOBS}
        fi

        run_make -C ${SCRIPT_DIR} rt ${BUILD_FLAGS} ${MAKE_JOBS}
        if [ "$INSTALL" == "1" ]; then
            run_make -C ${SCRIPT_DIR}/arm-tidl install ${BUILD_FLAGS} ${MAKE_JOBS}
        fi

        if [ "$platform_type" == "PC" ]; then
            cd ${SCRIPT_DIR}
            pushd tidl_tools
            rm -f libtidl_onnxrt_EP.so
            rm -f libtidl_tfl_delegate.so
            rm -f libtidlrt_EP.so
            rm -f libvx_tidl_rt.so
            rm -f libvx_tidl_rt.so.1.0
            if [ "$build_type" == "debug" ]; then
                ln -s ../arm-tidl/onnxrt_ep/out/PC/x86_64/LINUX/debug/libtidl_onnxrt_EP.so
                ln -s ../arm-tidl/tfl_delegate/out/PC/x86_64/LINUX/debug/libtidl_tfl_delegate.so
                ln -s ../arm-tidl/tidlrt_ep/out/PC/x86_64/LINUX/debug/libtidlrt_EP.so
                ln -s ../arm-tidl/rt/out/PC/x86_64/LINUX/debug/libvx_tidl_rt.so
                ln -s ../arm-tidl/rt/out/PC/x86_64/LINUX/debug/libvx_tidl_rt.so.1.0
            else
                ln -s ../arm-tidl/onnxrt_ep/out/PC/x86_64/LINUX/release/libtidl_onnxrt_EP.so
                ln -s ../arm-tidl/tfl_delegate/out/PC/x86_64/LINUX/release/libtidl_tfl_delegate.so
                ln -s ../arm-tidl/tidlrt_ep/out/PC/x86_64/LINUX/release/libtidlrt_EP.so
                ln -s ../arm-tidl/rt/out/PC/x86_64/LINUX/release/libvx_tidl_rt.so
                ln -s ../arm-tidl/rt/out/PC/x86_64/LINUX/release/libvx_tidl_rt.so.1.0
            fi
            popd
            cd - > /dev/null
            if [ "$INSTALL" == "1" ]; then
                run_make -C ${SCRIPT_DIR} install_tidl_tools ${BUILD_FLAGS} ${MAKE_JOBS}
            fi
        fi
    fi
}

if [ "$TARGET_BUILD" == "all" ] && [ "$TARGET_PLATFORM" == "ALL" ]; then
    echo "Building all configurations (release + debug) for all platforms (TI_DEVICE + PC)..."
    build_with_config "release" "TI_DEVICE"
    build_with_config "release" "PC"
    build_with_config "debug" "TI_DEVICE"
    build_with_config "debug" "PC"

elif [ "$TARGET_BUILD" == "all" ]; then
    echo "Building all configurations (release + debug) for platform $TARGET_PLATFORM..."
    build_with_config "release" "$TARGET_PLATFORM"
    build_with_config "debug" "$TARGET_PLATFORM"

elif [ "$TARGET_PLATFORM" == "ALL" ]; then
    echo "Building configuration $TARGET_BUILD for all platforms (TI_DEVICE + PC)..."
    build_with_config "$TARGET_BUILD" "TI_DEVICE"
    build_with_config "$TARGET_BUILD" "PC"

else
    build_with_config "$TARGET_BUILD" "$TARGET_PLATFORM"
fi

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"