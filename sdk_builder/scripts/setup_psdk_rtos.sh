#!/bin/bash

#
# Utility script to install all dependant components not included in the
# SDK
#
# - Make sure wget, apt-get, curl proxies are set
#   if required to access these tools from behind corporate firewall.
# - Make sure you have sudo access
# - Tested on Ubuntu 22.04
#
# IMPORANT NOTE:
#   TI provides this as is and may not work as expected on all systems,
#   Please review the contents of this script and modify as needed
#   for your environment
#
#
PSDK_PATH="$(readlink -f "$(dirname "$BASH_SOURCE")/../..")"
PSDK_BUILDER_PATH="${PSDK_PATH}/sdk_builder"
NUM_PROCS=$(nproc)

source "${PSDK_BUILDER_PATH}/scripts/board_env.sh"

if [ $? -ne 0 ]
then
    exit 1
fi

PSDK_LINUX_ROOTFS=tisdk-${TISDK_IMAGE}-image-${TI_DEV_BOARD}.tar.xz

if [ ${SOC} = "am62a" ]
then
    PSDK_LINUX_ROOTFS=tisdk-${TISDK_IMAGE}-image-${TI_DEV_BOARD}.rootfs.tar.xz
fi

PSDK_LINUX_BOOTFS=boot-${TISDK_IMAGE}-${TI_DEV_BOARD}.tar.gz
ATF_TAG=e0c4d3903b382bf34f552af53e6d955fae5283ab
OPTEE_TAG=a9690ae39995af36a31b7a4f446f27ea0787e3a4


: ${PSDK_TOOLS_PATH:=${HOME}/ti}
: ${MCU_SDK_PATH:=${PSDK_PATH}/mcu_sdk}

skip_sudo=0
skip_linux=0
firmware_only=0
pc_emulation=1
qnx_build=1
skip_tools=0
install_tidl_deps=0
skip_atf_optee=0

if [ $# -eq 0 ]
then
    echo "user_args = none"
else
    user_args=$*
    echo "user_args = $user_args"
fi

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"
case $key in
    --skip_sudo)
    skip_sudo=1
    shift # past argument
    ;;
    --qnx_sbl)
    skip_linux=1
    shift # past argument
    ;;
    --firmware_only)
    skip_linux=1
    firmware_only=1
    shift # past argument
    ;;
    --skip_pc_emulation)
    pc_emulation=0
    shift # past argument
    ;;
    --install_tidl_deps)
    install_tidl_deps=1
    shift # past argument
    ;;
    --skip_tools)
    skip_tools=1
    shift # past argument
    ;;
    --skip_atf_optee)
    skip_atf_optee=1
    shift # past argument
    ;;
    --skip_linux)
    skip_linux=1
    shift # past argument
    ;;
    --skip_qnx_build)
    qnx_build=0
    shift # past argument
    ;;
    --install_toolchain)
    skip_linux=1
    skip_atf_optee=1
    qnx_build=0
    pc_emulation=0
    shift # past argument
    ;;
    -h|--help)
    echo Usage: $0 [options]
    echo
    echo Options,
    echo --skip_sudo         use this option to skip steps that need sudo command
    echo --qnx_sbl           use this option for setting up SDK for QNX SBL only, skipping SPL u-boot setup steps
    echo --firmware_only     use this option for setting up SDK for Firmware Build only, skipping linux build dependencies
    echo --skip_pc_emulation use this option for skipping installing packages needed for pc emulation
    echo --skip_tools        use this option for skipping downloading and installing GCC and TI code gen tools
    echo --skip_atf_optee    use this option for skipping downloading and installing ATF and OPTEE
    echo --skip_linux        use this option for skipping downloading and installing linux build
    echo --install_tidl_deps use this option for installing tidl dependencies
    echo --skip_qnx_build    use this option for skipping QNX build steps
    echo --install_toolchain use this option for only installing toolchain, skipping linux, qnx build setups and ATF/OPTEE setup
    exit 0
    ;;
    *)
    # Unknown option - just pass through
    echo "Unknown option $1"
    POSITIONAL+=("$1")
    shift
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

ubuntu_1804=0
VERSION_ID=`cat /etc/os-release | grep VERSION_ID= | sed -e "s|VERSION_ID=\"||" | sed -e "s|\"||"`
if [ ${VERSION_ID} = "18.04" ]
then
    ubuntu_1804=1
fi

# Track which packages didn't install then print out at the end
declare -a err_packages

if [ $skip_sudo -eq 0 ]
then
    sudo apt-get update
    declare -a arr=()

    # This section lists packages to be installed on the host machine (including comments for what it is needed for)

    arr+=("unzip")                      # for unzipping zip tools
    arr+=("build-essential")            # for gnu make utilities
    arr+=("git")                        # for cloning open source repos (later in this script)
    arr+=("curl" "python3-distutils")   # for installing pip (later in this script)
    arr+=("libtinfo5")                  # for TI ARM compiler dependency (22.04 needs this, 18.04 is pre-installed)
    arr+=("bison")                      # for rebuilding the dtb, dtbo from PSDK Linux install directory (make linux-dtbs) after memory map updates
    arr+=("libgnutls28-dev")            # for building u-boot with PSDK Linux top-level makefile using the "make uboot" rule
    arr+=("flex")                       # for building linux uboot with PSDK Linux top-level makefile using the "make uboot" rule (HS and enabling MCU1_0)
    arr+=("swig")                       # for building linux uboot with PSDK Linux top-level makefile using the "make uboot" rule (HS and enabling MCU1_0)
    arr+=("u-boot-tools")               # for building linux sysfw with PSDK Linux top-level makefile using the "make sysfw" rule (HS and enabling MCU1_0)

    if [ $ubuntu_1804 -eq 0 ]
    then
        arr+=("libc6:i386")  # for running the ti-cgt-c6000 installer (only on 22.04, since not preinstalled)
    fi

    if [ $qnx_build -eq 1 ] && [ ${SOC} = "tda54" ]
    then
        arr+=("fdisk")       # for building the PSDK QNX SD card img file
        arr+=("dosfstools")  # for building the PSDK QNX SD card img file
        arr+=("mtools")      # for building the PSDK QNX SD card img file
        arr+=("bmap-tools")  # for building the PSDK QNX SD card img file
    fi

    # If we are doing more than just building RTOS firmware for yocto, there may be more packages we need
    if [ $firmware_only -eq 0 ]
    then

        arr+=("mono-runtime")           # for building sbl bootimage (uses Win32 executable on linux)
        arr+=("device-tree-compiler")   # for sbl_appimage to build base-board.dtb
        arr+=("cmake")                  # for building all edgeai repos
        arr+=("ninja-build" "pkgconf")  # for building edgeai-gst-plugins repo
        arr+=("graphviz")               # for 'dot' tool when running PyTIOVX tool on PC, and tivxExportGraphToDot() in PC emulation
        arr+=("graphviz-dev")           # for tidl model visualization build dependencies
        arr+=("python3-pyelftools")     # for building ATF (for QNX SBL "make sbl_atf_optee")
        arr+=("libncurses5")            # for building tiovx
        arr+=("file")                   # for running linux-devkit to install toolchain

        if [ $ubuntu_1804 -eq 0 ]
        then
            arr+=("python3-pip")         # for TIDL OSRT- ONNX
            arr+=("python3-setuptools")  # for TIDL OSRT- ONNX
            arr+=("libprotobuf-dev")     # for TIDL OSRT- ONNX
            arr+=("protobuf-compiler")   # for TIDL OSRT- ONNX
            arr+=("libprotoc-dev")       # for TIDL OSRT- ONNX
        fi


        # If we are needing to support PC emulation mode
        if [ $pc_emulation -eq 1 ]
        then
            if [ ${SOC} = "j721e" ] || [ ${SOC} = "j721s2" ] || [ ${SOC} = "j784s4" ] || [ ${SOC} = "j742s2" ] || [ ${SOC} = "j722s" ] || [ ${SOC} = "tda54" ]
            then
                echo "${SOC}: [dof] Creating/Updating system link to libDOF.so ..."
                sudo ln -sf $PWD/vhwa_c_models/dmpac/lib/PC/x86_64/LINUX/release/libDOF.so /usr/lib/x86_64-linux-gnu/libDOF.so
            fi

            echo "${SOC}: [glbce] Creating/Updating system link to libApicalSIM.so.1 ..."
            if [ ${SOC} = "j721e" ]
            then
                sudo ln -sf $PWD/vhwa_c_models/vpac1/lib/PC/x86_64/LINUX/release/libglbce.so /usr/lib/x86_64-linux-gnu/libApicalSIM.so.1
            # Temporary: Using VPAC3 c models for TDA54, should be VPAC4 once the models are up
            elif [ ${SOC} = "j721s2" ] || [ ${SOC} = "j784s4" ] || [ ${SOC} = "j742s2" ] || [ ${SOC} = "tda54" ]
            then
                sudo ln -sf $PWD/vhwa_c_models/vpac3/lib/PC/x86_64/LINUX/release/libglbce.so /usr/lib/x86_64-linux-gnu/libApicalSIM.so.1
            else
                # J722S and AM62A
                sudo ln -sf $PWD/vhwa_c_models/vpac3l/lib/PC/x86_64/LINUX/release/libglbce.so /usr/lib/x86_64-linux-gnu/libApicalSIM.so.1
            fi

            if [ ${SOC} = "am62a" ] || [ ${SOC} = "j722s" ]
            then
                echo "Installing Opencv for Imaging host emulation mode"
                arr+=("libopencv-dev") # required for building conformance tests in host emulation mode.
            fi

            if [ $ubuntu_1804 -eq 1 ]
            then
                arr+=("gcc-5" "g++-5")  # for pc emulation compiler (only on 18.04, since not preinstalled)
            fi

            arr+=("libpng-dev")         # for building tiovx/utils/source/tivx_utils_png_rd_wr.c
            arr+=("libgles2-mesa-dev")  # for building vision_apps/utils/opengl/include/app_gl_egl_utils.h
            arr+=("libgbm-dev")  # for building vision_apps/utils/opengl/src/pc/app_gl_egl_utils_pc.cpp
        fi
    fi

    arr+=("libopenexr-dev") # this installs lIlmImf for building tidl_tools/PC_dsp_test_dl_algo.out
    arr+=("autoconf")   # for building protobuf-3.21.12
    arr+=("libtool")    # for building protobuf-3.21.12
    arr+=("libyaml-cpp-dev")    # for building yaml-cpp in host-emulation
    arr+=("libavformat-dev")    # for including avformat headers in host-emulation
    arr+=("libclang-dev")       # for rust bindgen

#    Prior to 9.00.00 SDK release, the following additional packages were also installed but are no longer needed in
#    9.00.00.  The list is kept here as a reference in case something stops working on a new machine installation, this list
#    can be referred in case a missing package is in this list and not called in the script.
#
#    arr+=("zlib1g-dev" "libtiff-dev" "libsdl2-dev" "libsdl2-image-dev" \
#            "libxmu-dev" "libxi-dev" "libgl-dev" "libosmesa-dev" "python3" "python3-pip" \
#            "libz1:i386" "libc6-dev-i386" "libc6:i386" "libstdc++6:i386" "g++-multilib" "diffstat" "texinfo"\
#            "gawk" "chrpath" "libfreetype6-dev" "libssl-dev" "libdevil-dev"  \
#            "python3-dev" "libx11-dev" "pxz" "libglew-dev" "xz-utils" "python3-bs4")

    if ! sudo apt-get install -y "${arr[@]}"; then
        for i in "${arr[@]}"
        do
            if ! sudo apt-get install -y "$i"; then
               err_packages+=("$i")
            fi
        done
    fi

else
    echo ""
    echo "Skipping apt-get install, ensure all required packages are installed on the machine by sudo user"
    echo ""
fi

if [ $skip_linux -eq 0 ]
then
    echo "[psdk linux ${PSDK_LINUX_ROOTFS}] Checking ..."
    if [ -d targetfs ]; then
        echo "targetfs found!"
    else
        if [ -f ${PSDK_LINUX_ROOTFS} ]
        then
            echo "[psdk linux ${PSDK_LINUX_ROOTFS}] Installing files ..."
            mkdir targetfs
            tar xf ${PSDK_LINUX_ROOTFS} -C targetfs/
        else
            echo
            echo "ERROR: Missing $PWD/${PSDK_LINUX_ROOTFS}.  Download and try again"
            exit -1
        fi
    fi
    echo "[psdk linux ${PSDK_LINUX_ROOTFS}] Done "

    echo "[psdk linux ${PSDK_LINUX_BOOTFS}] Checking ... "
    if [ -d bootfs ]; then
        echo "bootfs found!"
    else
        if [ -f ${PSDK_LINUX_BOOTFS} ]
        then
            echo "[psdk linux ${PSDK_LINUX_BOOTFS}] Installing files ..."
            mkdir bootfs
            tar xf ${PSDK_LINUX_BOOTFS} -C bootfs/
        else
            echo
            echo "ERROR: Missing $PWD/${PSDK_LINUX_BOOTFS}.  Download and try again"
            exit -1
        fi
    fi
    echo "[psdk linux ${PSDK_LINUX_BOOTFS}] Done "

    echo "[psdk linux toolchain] Installing ..."
    cd toolchain
    if [ -d sysroots ]; then
        echo "sysroots found!"
    else
        ./linux-devkit.sh -d ./ -y
    fi
    echo "[psdk linux toolchain] Done "
    cd -
fi

if [ $skip_tools -eq 0 ]
then
    # Call makefile to import component versions from PDK Rules.make.  Environment
    # setup is stored for debug purposes.
    THIS_DIR=$(dirname $(realpath $0))
    make --no-print-directory -C ${THIS_DIR} SOC=${SOC} BOARD=${SOC}_evm get_component_versions > .component_versions_${SOC}_env
    cat .component_versions_${SOC}_env
    source .component_versions_${SOC}_env

    CGT_ARMLLVM_VERSION_WEB=$(echo ${CGT_ARMLLVM_VERSION} | sed -r 's:\.:_:g')
    PROTOBUF_VERSION=3.21.12
    OPENCV_VERSION=4.1.0
    FLATBUFF_VERSION=1.12.0
    TVM_VERSION=9.0.0
    OPKG_VERSION=d179a334f7bfbe55dec4839607dac2c38f6b7c8f

    # Create ${PSDK_TOOLS_PATH} folder if not present
    if [ -d ${PSDK_TOOLS_PATH} ]; then
        echo "${PSDK_TOOLS_PATH} found!"
    else
        echo "Creating ${PSDK_TOOLS_PATH} folder"
        mkdir -p ${PSDK_TOOLS_PATH}
    fi

    # Install TI ARM LLVM tools for building on R/M55 cores
    echo "[ti-cgt-armllvm_${CGT_ARMLLVM_VERSION}] Checking ..."
    if [ -d ${PSDK_TOOLS_PATH}/ti-cgt-armllvm_${CGT_ARMLLVM_VERSION} ]; then
        echo "${PSDK_TOOLS_PATH}/ti-cgt-armllvm_${CGT_ARMLLVM_VERSION} found!"
    else
        wget --tries=5 https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-ayxs93eZNN/${CGT_ARMLLVM_VERSION}/ti_cgt_armllvm_${CGT_ARMLLVM_VERSION}_linux-x64_installer.bin -P ${PSDK_TOOLS_PATH} --no-check-certificate
        chmod +x ${PSDK_TOOLS_PATH}/ti_cgt_armllvm_${CGT_ARMLLVM_VERSION}_linux-x64_installer.bin
        ${PSDK_TOOLS_PATH}/ti_cgt_armllvm_${CGT_ARMLLVM_VERSION}_linux-x64_installer.bin --mode unattended --prefix ${PSDK_TOOLS_PATH}
        rm ${PSDK_TOOLS_PATH}/ti_cgt_armllvm_${CGT_ARMLLVM_VERSION}_linux-x64_installer.bin
    fi
    echo "[ti-cgt-armllvm_${CGT_ARMLLVM_VERSION}] Done"

    # Install TI CGT tools for building on C6x DSP cores
    if [ ${SOC} = "j721e" ]
    then
        echo "[ti-cgt-c6000_${CGT_C6X_VERSION}] Checking ..."
        if [ -d ${PSDK_TOOLS_PATH}/ti-cgt-c6000_${CGT_C6X_VERSION} ]; then
            echo "${PSDK_TOOLS_PATH}/ti-cgt-c6000_${CGT_C6X_VERSION} found!"
        else
            wget --tries=5 https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-vqU2jj6ibH/${CGT_C6X_VERSION}/ti_cgt_c6000_${CGT_C6X_VERSION}_linux_installer_x86.bin -P ${PSDK_TOOLS_PATH} --no-check-certificate
            chmod +x ${PSDK_TOOLS_PATH}/ti_cgt_c6000_${CGT_C6X_VERSION}_linux_installer_x86.bin
            ${PSDK_TOOLS_PATH}/ti_cgt_c6000_${CGT_C6X_VERSION}_linux_installer_x86.bin --mode unattended --prefix ${PSDK_TOOLS_PATH}
            rm ${PSDK_TOOLS_PATH}/ti_cgt_c6000_${CGT_C6X_VERSION}_linux_installer_x86.bin
        fi
        echo "[ti-cgt-c6000_${CGT_C6X_VERSION}] Done"
    fi

    # Install TI CGT tools for building on C7x DSP cores
    if [ ${SOC} = "j721e" ] || [ ${SOC} = "j721s2" ] || [ ${SOC} = "j784s4" ] || [ ${SOC} = "j742s2" ] || [ ${SOC} = "am62a" ] || [ ${SOC} = "j722s" ] || [ ${SOC} = "tda54" ]
    then
        echo "[ti-cgt-c7000_${CGT_C7X_VERSION}] Checking ..."
        if [ ! -d ${PSDK_TOOLS_PATH}/ti-cgt-c7000_${CGT_C7X_VERSION} ]
        then
            if [ "${SOC}" == "tda54" ]; then
                if [ ! -f ${PWD}/ti-cgt-c7000_${CGT_C7X_VERSION}.zip ]
                then
                    wget http://bangsdowebsvr01.india.ti.com/PROCESSOR_SDK_RTOS_AUTOMOTIVE/swdownloads/mcu_sdk/tda54/ti-cgt-c7000_${CGT_C7X_VERSION}.zip
                fi
                unzip -q ti-cgt-c7000_${CGT_C7X_VERSION}.zip -d ${PSDK_TOOLS_PATH}
                rm -f ti-cgt-c7000_${CGT_C7X_VERSION}.zip
            else
                wget https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-707zYe3Rik/${CGT_C7X_VERSION}/ti_cgt_c7000_${CGT_C7X_VERSION}_linux-x64_installer.bin -P ${PSDK_TOOLS_PATH} --no-check-certificate
                chmod +x ${PSDK_TOOLS_PATH}/ti_cgt_c7000_${CGT_C7X_VERSION}_linux-x64_installer.bin
                ${PSDK_TOOLS_PATH}/ti_cgt_c7000_${CGT_C7X_VERSION}_linux-x64_installer.bin --mode unattended --prefix ${PSDK_TOOLS_PATH}
                rm ${PSDK_TOOLS_PATH}/ti_cgt_c7000_${CGT_C7X_VERSION}_linux-x64_installer.bin
            fi
        fi
        echo "[ti-cgt-c7000_${CGT_C7X_VERSION}] Done"
    fi

    # Download RISC-V compiler for NPAC on TDA54
    if [ "${SOC}" == "tda54" ]; then
        echo "[riscv-none-elf-gcc-14.2.0-1] Checking ..."
        if [ ! -d ${PSDK_TOOLS_PATH}/xpack-riscv-none-elf-gcc-14.2.0-1 ]; then
            if [ ! -f ${PWD}/xpack-riscv-none-elf-gcc-14.2.0-1-linux-x64.tar.gz ]; then
                wget -c -t 0 https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/download/v14.2.0-1/xpack-riscv-none-elf-gcc-14.2.0-1-linux-x64.tar.gz
            fi
            tar -xzf xpack-riscv-none-elf-gcc-14.2.0-1-linux-x64.tar.gz -C ${PSDK_TOOLS_PATH}
            rm xpack-riscv-none-elf-gcc-14.2.0-1-linux-x64.tar.gz
        fi
        echo "[riscv-none-elf-gcc-14.2.0-1] Done"
    fi

    # Install rustc for Ethernet Switch Firmware on TDA54
    if [ "${SOC}" == "tda54" ]; then
        RUSTC_VERSION=1.94.0
        RUSTC_TARGETS="thumbv8m.main-none-eabihf armv8r-none-eabihf"
        export RUSTUP_HOME=${PSDK_TOOLS_PATH}/rustup_${RUSTC_VERSION}
        export CARGO_HOME=${PSDK_TOOLS_PATH}/cargo_${RUSTC_VERSION}
        echo "[rustc] Checking ..."
        if [ -x "${CARGO_HOME}/bin/rustc" ]; then
            echo "rustc found: $(${CARGO_HOME}/bin/rustc --version)"
            for target in ${RUSTC_TARGETS}; do
                if ! ${CARGO_HOME}/bin/rustup target list --installed 2> /dev/null | grep -qx "${target}"; then
                    echo "Adding missing target ${target}"
                    ${CARGO_HOME}/bin/rustup target add ${target}
                fi
            done
        else
            if [ ! -f ${PSDK_TOOLS_PATH}/rustup-init.sh ]; then
                curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs -o ${PSDK_TOOLS_PATH}/rustup-init.sh
            fi
            chmod +x ${PSDK_TOOLS_PATH}/rustup-init.sh
            ${PSDK_TOOLS_PATH}/rustup-init.sh -y --no-modify-path --default-toolchain ${RUSTC_VERSION} \
                --target $(echo ${RUSTC_TARGETS} | tr ' ' ',')
            rm ${PSDK_TOOLS_PATH}/rustup-init.sh
            ${CARGO_HOME}/bin/rustc --version
            ${CARGO_HOME}/bin/cargo --version
        fi

        RUSTC_BASHRC_MARKER="# rustc for TDA54 ethfw (added by setup_psdk_rtos.sh)"
        if ! grep -qF "${RUSTC_BASHRC_MARKER}" "${HOME}/.bashrc" 2> /dev/null; then
            {
                echo ""
                echo "${RUSTC_BASHRC_MARKER}"
                echo "export RUSTUP_HOME=${RUSTUP_HOME}"
                echo "export CARGO_HOME=${CARGO_HOME}"
                echo "export PATH=\"${CARGO_HOME}/bin:\${PATH}\""
            } >> "${HOME}/.bashrc"
            echo "Added rustc env vars to ~/.bashrc."
        fi
        \. "${HOME}/.bashrc"
        echo "[rustc] Done"
    fi

    # Install sysconfig tool for MCU+SDK build
    if [ ${SOC} = "am62a" ] || [ ${SOC} = "j722s" ]
    then
        echo "[sysconfig_${SYSCONFIG_VERSION}] Checking ..."
        if [ -d ${PSDK_TOOLS_PATH}/sysconfig_${SYSCONFIG_VERSION} ]; then
            echo "${PSDK_TOOLS_PATH}/sysconfig_${SYSCONFIG_VERSION} found!"
        else
            wget --tries=5 https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-nsUM6f7Vvb/${SYSCONFIG_VERSION}.${SYSCONFIG_BUILD}/sysconfig-${SYSCONFIG_VERSION}_${SYSCONFIG_BUILD}-setup.run -P ${PSDK_TOOLS_PATH} --no-check-certificate
            chmod +x ${PSDK_TOOLS_PATH}/sysconfig-${SYSCONFIG_VERSION}_${SYSCONFIG_BUILD}-setup.run
            ${PSDK_TOOLS_PATH}/sysconfig-${SYSCONFIG_VERSION}_${SYSCONFIG_BUILD}-setup.run --mode unattended --prefix ${PSDK_TOOLS_PATH}/sysconfig_${SYSCONFIG_VERSION}
            rm ${PSDK_TOOLS_PATH}/sysconfig-${SYSCONFIG_VERSION}_${SYSCONFIG_BUILD}-setup.run
        fi
        echo "[sysconfig_${SYSCONFIG_VERSION}] Done"
    fi

    # Install GCC only if we need to cross-build host-arm side components
    if [ $firmware_only -eq 0 ]
    then
        echo "[gcc-arm-${GCC_ARCH64_VERSION}-x86_64-aarch64-none-elf] Checking ..."
        if [ -d ${PSDK_TOOLS_PATH}/gcc-arm-${GCC_ARCH64_VERSION}-x86_64-aarch64-none-elf ]; then
            echo "${PSDK_TOOLS_PATH}/gcc-arm-${GCC_ARCH64_VERSION}-x86_64-aarch64-none-elf found!"
        else
            wget --tries=5 https://developer.arm.com/-/media/Files/downloads/gnu-a/${GCC_ARCH64_VERSION}/binrel/gcc-arm-${GCC_ARCH64_VERSION}-x86_64-aarch64-none-elf.tar.xz -P ${PSDK_TOOLS_PATH} --no-check-certificate
            tar xf ${PSDK_TOOLS_PATH}/gcc-arm-${GCC_ARCH64_VERSION}-x86_64-aarch64-none-elf.tar.xz -C ${PSDK_TOOLS_PATH} > /dev/null
            rm ${PSDK_TOOLS_PATH}/gcc-arm-${GCC_ARCH64_VERSION}-x86_64-aarch64-none-elf.tar.xz
        fi
        echo "[gcc-arm-${GCC_ARCH64_VERSION}-x86_64-aarch64-none-elf] Done"

        echo "[gcc-arm-${GCC_ARCH64_VERSION}-x86_64-arm-none-linux-gnueabihf] Checking ..."
        if [ -d ${PSDK_TOOLS_PATH}/gcc-arm-${GCC_ARCH64_VERSION}-x86_64-arm-none-linux-gnueabihf ]; then
            echo "${PSDK_TOOLS_PATH}/gcc-arm-${GCC_ARCH64_VERSION}-x86_64-arm-none-linux-gnueabihf found!"
        else
            wget --tries=5 https://developer.arm.com/-/media/Files/downloads/gnu-a/${GCC_ARCH64_VERSION}/binrel/gcc-arm-${GCC_ARCH64_VERSION}-x86_64-arm-none-linux-gnueabihf.tar.xz -P ${PSDK_TOOLS_PATH} --no-check-certificate
            tar xf ${PSDK_TOOLS_PATH}/gcc-arm-${GCC_ARCH64_VERSION}-x86_64-arm-none-linux-gnueabihf.tar.xz -C ${PSDK_TOOLS_PATH} > /dev/null
            rm ${PSDK_TOOLS_PATH}/gcc-arm-${GCC_ARCH64_VERSION}-x86_64-arm-none-linux-gnueabihf.tar.xz
        fi
        echo "[gcc-arm-${GCC_ARCH64_VERSION}-x86_64-arm-none-linux-gnueabihf] Done"
    fi

    #Install Protobuf needed for building TIDL
    echo "[protobuf-${PROTOBUF_VERSION}] Checking ..."
    if [ -d protobuf-${PROTOBUF_VERSION} ]; then
        echo "protobuf-${PROTOBUF_VERSION} found!"
    else
        wget --tries=5 https://github.com/protocolbuffers/protobuf/archive/refs/tags/v${PROTOBUF_VERSION}.tar.gz --no-check-certificate
        tar xf v${PROTOBUF_VERSION}.tar.gz > /dev/null
        rm v${PROTOBUF_VERSION}.tar.gz
        echo "[protobuf-${PROTOBUF_VERSION} build Done]"
    fi
    echo "[protobuf-${PROTOBUF_VERSION}] Done"

    if [ $install_tidl_deps -eq 1 ]
    then
        echo "[protobuf-${PROTOBUF_VERSION}] Libs Checking ..."
        if [ -d protobuf-${PROTOBUF_VERSION}/src/.libs ]; then
            echo "protobuf-${PROTOBUF_VERSION}/src/.libs found!"
        else
            echo "[Building protobuf-${PROTOBUF_VERSION} libs for TIDL PC emulation mode]"
            cd protobuf-${PROTOBUF_VERSION}
            ./autogen.sh > /dev/null
            ./configure CXXFLAGS=-fPIC --enable-shared=no LDFLAGS="-static" > /dev/null
            make -j${NUM_PROCS} &> .protobuf_build_log
            cd ..
            echo "[protobuf-${PROTOBUF_VERSION} build Done]"
        fi
        echo "[protobuf-${PROTOBUF_VERSION}] Libs Done"

        echo "[opencv-${OPENCV_VERSION}] Checking ..."
        if [ -d opencv-${OPENCV_VERSION} ]; then
            echo "opencv-${OPENCV_VERSION} found!"
        else
            wget --tries=5 https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip --no-check-certificate
            unzip ${OPENCV_VERSION}.zip > /dev/null
            rm ${OPENCV_VERSION}.zip
            echo "[Building opencv-${OPENCV_VERSION} libs for TIDL PC emulation mode]"
            cd opencv-${OPENCV_VERSION}/cmake
            cmake -DBUILD_opencv_highgui:BOOL="1" -DBUILD_opencv_videoio:BOOL="0" -DWITH_IPP:BOOL="0" -DBUILD_WEBP:BOOL="1" -DBUILD_OPENEXR:BOOL="1" -DWITH_IPP_A:BOOL="0" -DBUILD_WITH_DYNAMIC_IPP:BOOL="0" -DBUILD_opencv_cudacodec:BOOL="0" -DBUILD_PNG:BOOL="1" -DBUILD_opencv_cudaobjdetect:BOOL="0" -DBUILD_ZLIB:BOOL="1" -DBUILD_TESTS:BOOL="0" -DWITH_CUDA:BOOL="0" -DBUILD_opencv_cudafeatures2d:BOOL="0" -DBUILD_opencv_cudaoptflow:BOOL="0" -DBUILD_opencv_cudawarping:BOOL="0" -DINSTALL_TESTS:BOOL="0" -DBUILD_TIFF:BOOL="1" -DBUILD_JPEG:BOOL="1" -DBUILD_opencv_cudaarithm:BOOL="0" -DBUILD_PERF_TESTS:BOOL="0" -DBUILD_opencv_cudalegacy:BOOL="0" -DBUILD_opencv_cudaimgproc:BOOL="0" -DBUILD_opencv_cudastereo:BOOL="0" -DBUILD_opencv_cudafilters:BOOL="0" -DBUILD_opencv_cudabgsegm:BOOL="0" -DBUILD_SHARED_LIBS:BOOL="0" -DWITH_ITT=OFF ../ &> /dev/null
            make -j${NUM_PROCS} &> .opencv_build_log
            cd -
            echo "[opencv-${OPENCV_VERSION} build Done]"
        fi
        echo "[opencv-${OPENCV_VERSION}] Done"


        echo "[flatbuffers-${FLATBUFF_VERSION}] Checking ..."
        if [ -d flatbuffers-${FLATBUFF_VERSION} ]; then
            echo "flatbuffers-${FLATBUFF_VERSION} found!"
        else
            wget --tries=5 https://github.com/google/flatbuffers/archive/v${FLATBUFF_VERSION}.zip --no-check-certificate
            unzip v${FLATBUFF_VERSION}.zip > /dev/null
            rm v${FLATBUFF_VERSION}.zip
            echo "[Building flatbuffers-${FLATBUFF_VERSION} libs for TIDL PC emulation mode]"
            cd flatbuffers-${FLATBUFF_VERSION}
            cmake -G "Unix Makefiles" -DCMAKE_POSITION_INDEPENDENT_CODE=ON  -DFLATBUFFERS_BUILD_TESTS=OFF &> /dev/null
            make -j${NUM_PROCS} &> .flatbuffer_build_log
            cd -
            echo "[flatbuffers-${FLATBUFF_VERSION} build Done]"
        fi
        echo "[flatbuffers-${FLATBUFF_VERSION}] Done"

        echo "[tvm] Checking ..."
        if [ -d tvm ]; then
            echo "tvm found!"
        else
            git clone --single-branch --depth 1 --branch TIDL_PSDK_${TVM_VERSION} https://github.com/TexasInstruments/tvm
            cd tvm
            git submodule update --init --recursive --depth 1
            cd -
        fi
        echo "[tvm] Done"

        if [ -d c7x-mma-tidl/ti_dl/utils/python ]
        then
            echo "[Configuring c7x-mma-tidl for ${SOC}] Checking ..."
            cd c7x-mma-tidl/ti_dl/utils/python/
            python3 configureRepoForDevice.py ${SOC} &> /dev/null
            cd -
        fi

        echo "[DL conformance Models] Checking ..."
        if [ -d c7x-mma-tidl/ti_dl/test/testvecs/models/public/caffe/mobileNet1.0v1 ]; then
            echo "c7x-mma-tidl/ti_dl/test/testvecs/models/public/caffe/mobileNet1.0v1 found!"
        else
            mkdir -p c7x-mma-tidl/ti_dl/test/testvecs/models/public/caffe/mobileNet1.0v1
            cd c7x-mma-tidl/ti_dl/test/testvecs/models/public/caffe/mobileNet1.0v1
            wget --tries=5 https://github.com/shicai/MobileNet-Caffe/raw/a0b0c46152e658559956158d7c2bf42bb029c09e/mobilenet.caffemodel
            wget --tries=5 https://raw.githubusercontent.com/shicai/MobileNet-Caffe/a0b0c46152e658559956158d7c2bf42bb029c09e/mobilenet_deploy.prototxt
            cd -
        fi
        echo "[DL conformance Models] Done"
    fi
fi

# core secdev check ( This is used to sign the firmware when using HS devices )
echo "[core secdev] Checking ..."
if [ -d ./core-secdev-k3 ]; then
    echo "core-secdev-k3 found!"
else
    git clone https://github.com/TexasInstruments/core-secdev-k3
fi
echo "[core secdev] Checking ... Done"


if [ $skip_atf_optee -eq 0 ]
then
    # ATF check ( This is only used by RTOS SBL today "make sbl_atf_optee" )
    echo "[ATF] Checking ..."
    if [ -d ./trusted-firmware-a ]; then
        echo "trusted-firmware-a found!"
    else
        git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
        cd trusted-firmware-a
        git fetch origin
        git checkout $ATF_TAG
        git describe
        cd ..
    fi
    echo "[ATF] Checking ... Done"

    # OPTEE check ( This is only used by RTOS SBL today "make sbl_atf_optee" )
    echo "[OPTEE] Checking ..."
    if [ -d ./optee_os ]; then
        echo "optee_os found!"
    else
        git clone https://github.com/OP-TEE/optee_os.git
        cd optee_os
        git fetch origin
        git checkout $OPTEE_TAG
        git describe
        cd ..
    fi
    echo "[OPTEE] Checking ... Done"
fi

echo "[opkg-utils] Checking ..." # opkg is needed for running rule in vision_apps: make ipk
if [ -d opkg-utils-${OPKG_VERSION} ]; then
    echo "opkg-utils-${OPKG_VERSION} found!"
else
    mkdir opkg-utils-${OPKG_VERSION}
    cd opkg-utils-${OPKG_VERSION}
    git init
    git remote add origin https://git.yoctoproject.org/opkg-utils
    git fetch --depth 1 origin $OPKG_VERSION
    git checkout FETCH_HEAD
    cd ..
fi
echo "[opkg-utils] Done"

# If we are needing to support PC emulation mode
if [ $pc_emulation -eq 1 ]
then
    echo "[glm] Checking ..." # GLM needed for SRV PC emulation demo
    if [ ! -d glm ]
    then
        wget --tries=5 https://github.com/g-truc/glm/releases/download/0.9.8.0/glm-0.9.8.0.zip --no-check-certificate
        unzip glm-0.9.8.0.zip > /dev/null
        rm glm-0.9.8.0.zip
    fi
    echo "[glm] Done"
fi

# Download nlohmann/json and stb if QNX build or PC emulation is enabled (skip for AM62A)
if ([ "$qnx_build" = "1" ] || [ "$pc_emulation" = "1" ]) && [ "${SOC}" != "am62a" ]; then
    # Skip this step if SRVDEPS exists
    if [ -d SRVDEPS ]; then
        echo "SRVDEPS found!"
    else
        mkdir -p SRVDEPS
    fi

    if [ -d SRVDEPS/include/nlohmann ]; then
        echo "SRVDEPS/include/nlohmann found!"
    else
        # nlohmann/json clone and checkout
        NLOHMANN_COMMIT="02ac0d6525f2e046f136ca69b5105b4e4f315b2f"
        git clone https://github.com/nlohmann/json.git nlohmann_json_tmp
        cd nlohmann_json_tmp
        if git checkout $NLOHMANN_COMMIT 2>/dev/null; then
            echo "Checked out nlohmann/json at commit $NLOHMANN_COMMIT"
        else
            echo "Commit $NLOHMANN_COMMIT not found, using tip of develop branch"
            git checkout develop
        fi
        cd ..
        cp -r nlohmann_json_tmp/include/ SRVDEPS
        rm -rf nlohmann_json_tmp
    fi

    if [ -d SRVDEPS/include/stb ]; then
        echo "SRVDEPS/include/stb found!"
    else
        # stb clone and checkout
        STB_COMMIT="f1c79c02822848a9bed4315b12c8c8f3761e1296"
        git clone https://github.com/nothings/stb.git stb_tmp
        cd stb_tmp
        if git checkout $STB_COMMIT 2>/dev/null; then
            echo "Checked out stb at commit $STB_COMMIT"
        else
            echo "Commit $STB_COMMIT not found, using tip of master branch"
            git checkout master
        fi
        cd ..
        mkdir -p SRVDEPS/include/stb
        cp stb_tmp/*.h SRVDEPS/include/stb/
        rm -rf stb_tmp

        echo "[SRVDEPS] nlohmann/json and stb headers downloaded for QNX build or PC emulation"
    fi
fi

echo "[pip] Checking ..."
if [ -f ~/.local/bin/pip ]; then
    echo "~/.local/bin/pip found!"
else
    PYTHON3_VERSION=`python3 -c 'import sys; version=sys.version_info[:2]; print("{1}".format(*version))'`
    if [ ${PYTHON3_VERSION} = '3' ]
    then
        curl "https://bootstrap.pypa.io/pip/3.3/get-pip.py" -o "get-pip.py"
    elif [ ${PYTHON3_VERSION} = '4' ]
    then
        curl "https://bootstrap.pypa.io/pip/3.4/get-pip.py" -o "get-pip.py"
    elif [ ${PYTHON3_VERSION} = '5' ]
    then
        curl "https://bootstrap.pypa.io/pip/3.5/get-pip.py" -o "get-pip.py"
    elif [ ${PYTHON3_VERSION} = '6' ]
    then
        curl "https://bootstrap.pypa.io/pip/3.6/get-pip.py" -o "get-pip.py"
    else
        curl "https://bootstrap.pypa.io/pip/get-pip.py" -o "get-pip.py"
    fi
    python3 get-pip.py --user
    rm get-pip.py
fi

python3 -m pip install --upgrade pip

echo "[pip] Checking ... Done"

echo "[pip] Installing dependant python packages ..."
if ! command -v pip3 &> /dev/null
then
    export PATH=${HOME}/.local/bin:$PATH
fi
pip3 install pycryptodomex --user        # for building ATF, OPTEE (built for qnx sbl)
pip3 install meson --user                # for building edegai-gst-plugins
pip3 install 'jsonschema>=4.0.0' --user  # for building linux uboot with PSDK Linux top-level makefile using the "make uboot" rule (HS and enabling MCU1_0)
pip3 install yamllint --user
if [ ${SOC} = 'tda54' ] && [ ! -z "${MCU_SDK_PATH}" ]
then
    if [ -f "${MCU_SDK_PATH}/requirements.txt" ]
    then
        echo "Installing MCU SDK Python dependencies from ${MCU_SDK_PATH}/requirements.txt"
        pip3 install -r ${MCU_SDK_PATH}/requirements.txt
    else
        echo "Skipping MCU SDK Python dependencies (requirements.txt not found)"
    fi
fi
echo "[pip] Installing dependant python packages ... Done"

if [ ${SOC} = 'tda54' ] && [ ! -z "${MCU_SDK_PATH}" ]
then
    echo "Installing dependant mcu_sdk packages ..."
    curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash
    \. "$HOME/.nvm/nvm.sh"
    nvm install 24
    node -v
    npm -v
    if [ -f "${MCU_SDK_PATH}/package.json" ]
    then
        cd ${MCU_SDK_PATH}
        npm install
        cd -
        echo "Installing dependant mcu_sdk packages ... Done"
    else
        echo "Skipping mcu_sdk npm install (package.json not found)"
    fi
fi

# check if there is a err_packages
if [ -z "$err_packages" ]; then
    echo "Packages installed successfully"
else
    echo "ERROR: The following packages were not installed:"
    for i in "${err_packages[@]}"
    do
       echo "$i"
    done
fi
