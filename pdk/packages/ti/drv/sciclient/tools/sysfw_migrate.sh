#!/bin/bash
#
# Copyright (c) 2018-2022, Texas Instruments Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# *  Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# *  Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# *  Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Usage : sysfw_migrate.sh <release tag> [OPTIONS]

################################################################################
export RM=rm
export MV=mv
export MKDIR=mkdir
export MAKE=gcc
export ECHO=echo
export CHMOD=chmod
export COPY=cp
export CAT=cat

################################################################################
# Parse CLI arguments
SOC_LIST="j721e j7200 j721s2 j784s4 j742s2"
version=""
sci_version=""
for i in "$@"; do
case $i in
    -sr|--skip-reset) # Skips the PDK reset and rebase step
        SKIP_RESET=YES
        shift
        ;;
    -sb|--skip-build) # Skips the sciclient binaries build step
        SKIP_BUILD=YES
        shift
        ;;
    -sc|--skip-commit) # Skips the PDK commit step
        SKIP_COMMIT=YES
        shift
        ;;
    -uv|--update-version-header) # Update version header using arguments
        UPDATE_VERSION_HEADER=YES
        shift
        ;;
    --soc=*) #List of SOC's
        if [ "${i#*=}" != "" ]; then
            SOC_LIST="${i#*=}"
        fi
        shift
        ;;
    -h|--help)
        $ECHO "Usage : sysfw_migrate.sh <rmpmhal release tag> <sci release tag> [OPTIONS]"
        $ECHO
        $ECHO "<rmpmhal release tag> : vXX.XX.XX format"
        $ECHO "<sci release tag>     : MM_NN_PP_OO format"
        $ECHO
        $ECHO "OPTIONS:-"
        $ECHO " -sr or --skip-reset            : Skips the PDK reset and rebase step"
        $ECHO " -sb or --skip-build            : Skips the sciclient binaries build step"
        $ECHO " -sc or --skip-commit           : Skips the PDK commit step"
        $ECHO " -uv or --update-version-header : Update version header using arguments"
        $ECHO " --soc=\"\<soc_list\>\" : List of SOCs. Default will be all supported SOCs"
        $ECHO "     Supported SOCs:-"
        $ECHO "     - j721e"
        $ECHO "     - j7200"
        $ECHO "     - j721s2"
        $ECHO "     - j784s4"
        $ECHO "    For example, --soc=\"j721e\" or  --soc=\"j721e j7200\""
        exit 0
        ;;
    v*.*.*) #rmpmhal version
        $ECHO "$i"
        version="$i"
        shift
        ;;
    *_*_*_*) #sci version
        $ECHO "$i"
        sci_version="$i"
        shift
        ;;
    -*) # Invalid flag
        $ECHO "!!!WARNING!!! - IGNORING INVALID FLAG: $1"
        shift
        ;;
esac
done

################################################################################
# Specify paths relative to script
export SCRIPT_DIR=$(cd "$( dirname "${BASH_SOURCE[0]}")" && pwd )
export SCI_CLIENT_DIR=$(cd "$SCRIPT_DIR/.." && pwd )
export ROOTDIR=$(cd "$SCI_CLIENT_DIR/../../.." && pwd )
export PDK_DIR=$(cd "$ROOTDIR/.." && pwd )

$ECHO " Starting DM Migration for $SOC_LIST "

################################################################################
# Rebase to PDK master

if [ "$SKIP_RESET" != "YES" ]; then
    $ECHO "Reset PDK branch and rebase onto master"
    git reset --hard HEAD
    git fetch origin; git rebase origin/master

    cd $PDK_DIR/docs/
    $ECHO "Reset PDK_DOCS branch and rebase onto master"
    git reset --hard HEAD
    git fetch origin; git rebase origin/master
fi

################################################################################
# Update version headers and bypass updating using git

if [ "$UPDATE_VERSION_HEADER" == "YES" ]; then
    cd $ROOTDIR/..
    $ECHO "Applying patch to skip version headers updating during build"
    git apply $ROOTDIR/ti/drv/sciclient/tools/DM_Migration.patch

    cd $ROOTDIR/ti/drv/sciclient/src/version
    sci_version_short=`echo ${sci_version} | cut -d "_" -f -3`
    sci_version_short_dot=`echo ${sci_version_short} | sed -e "s|\_|.|g"`
    sci_build_num=`git tag | grep REL.PSDK.${sci_version_short_dot} | sort | tail -n1 | cut -d "." -f6`
    if [[ "${sci_build_num}" == "" ]]; then
        sci_build_num="00"     #Handle for first build/tag for a release
    fi
    sci_build_num=`echo "${sci_build_num} + 1" | bc`
    sci_build_num=$(printf "%02d" ${sci_build_num})
    sci_version=`echo ${sci_version_short}_${sci_build_num}`
    sci_version_dot=`echo ${sci_version} | sed -e "s|\_|.|g"`
    major_version=`echo ${sci_version_short_dot} | cut -d "." -f1 | awk '{sub(/^0*/,"")}1' | awk '{$0=$0+0}1'`
    sed -i -e "s/.*SCISERVER_MAJOR_VERSION_NAME.*/#define SCISERVER_MAJOR_VERSION_NAME\t${major_version}/" sciserver_version.h
    sub_version=`echo ${sci_version_short_dot} | cut -d "." -f2 | awk '{sub(/^0*/,"")}1' | awk '{$0=$0+0}1'`
    sed -i -e "s/.*SCISERVER_SUBVERSION.*/#define SCISERVER_SUBVERSION\t${sub_version}/" sciserver_version.h
    patch_version=`echo ${sci_version_short_dot} | cut -d "." -f3 | awk '{sub(/^0*/,"")}1' | awk '{$0=$0+0}1'`
    sed -i -e "s/.*SCISERVER_PATCHVERSION.*/#define SCISERVER_PATCHVERSION\t${patch_version}/" sciserver_version.h
    sed -i -e "s/.*SCISERVER_SCMVERSION.*/#define SCISERVER_SCMVERSION\t\"-REL.PSDK.${sci_version_dot}\"/" sciserver_version.h
    sed -i -e "s/.*SCISERVER_DMVERSION.*/#define SCISERVER_DMVERSION     \"PSDK\.${sci_version_dot}\"/" sciserver_version.h
    current_year=$(date +%Y)
    sed -i -e "s/\(Copyright (C) [0-9]\{4\}-\)[0-9]\{4\}/\1${current_year}/" sciserver_version.h

    # Update rmpmhal_version.h using the rmpmhal version tag (vXX.XX.XX)
    if [ -n "$version" ]; then
        cd $ROOTDIR/ti/drv/sciclient/src/version
        # Strip leading 'v' and split into components
        version_stripped="${version#v}"
        rmpmhal_major=`echo ${version_stripped} | cut -d "." -f1 | awk '{sub(/^0*/,"")}1' | awk '{$0=$0+0}1'`
        rmpmhal_sub=`echo ${version_stripped} | cut -d "." -f2 | awk '{sub(/^0*/,"")}1' | awk '{$0=$0+0}1'`
        rmpmhal_patch=`echo ${version_stripped} | cut -d "." -f3 | awk '{sub(/^0*/,"")}1' | awk '{$0=$0+0}1'`
        sed -i -e "s/.*RMPMHAL_SCMVERSION.*/#define RMPMHAL_SCMVERSION\t\t\"${version}\"/" rmpmhal_version.h
        sed -i -e "s/.*RMPMHAL_MAJORVERSION.*/#define RMPMHAL_MAJORVERSION\t${rmpmhal_major}/" rmpmhal_version.h
        sed -i -e "s/.*RMPMHAL_SUBVERSION.*/#define RMPMHAL_SUBVERSION\t\t${rmpmhal_sub}/" rmpmhal_version.h
        sed -i -e "s/.*RMPMHAL_PATCHVERSION.*/#define RMPMHAL_PATCHVERSION\t${rmpmhal_patch}/" rmpmhal_version.h
        sed -i -e "s/\(Copyright (C) [0-9]\{4\}-\)[0-9]\{4\}/\1${current_year}/" rmpmhal_version.h
    fi
fi

################################################################################
# Build sciclient_ccs_init for use with launch.js
if [ "$SKIP_BUILD" != "YES" ]; then
    cd $ROOTDIR/ti/build

    for SOC in $SOC_LIST
    do
        make -j -s allclean
        if [ "$SOC" != "j742s2" ]; then
            make -j -s sciclient_boardcfg BOARD="$SOC"_evm
            make -j -s sciclient_boardcfg BOARD="$SOC"_evm BUILD_HS=yes
        fi
        make -j -s sciclient_ccs_init_clean BOARD="$SOC"_evm
        make -j -s sciclient_ccs_init BOARD="$SOC"_evm
        make -j -s sciserver_testapp_freertos_clean BOARD="$SOC"_evm
        make -j -s sciserver_testapp_freertos BOARD="$SOC"_evm
        $COPY $ROOTDIR/ti/binary/sciclient_ccs_init/bin/"$SOC"/sciclient_ccs_init_mcu1_0_release.xer5f $SCI_CLIENT_DIR/tools/ccsLoadDmsc/"$SOC"/
        $COPY $ROOTDIR/ti/binary/sciserver_testapp_freertos/bin/"$SOC"/sciserver_testapp_freertos_mcu1_0_release.xer5f $SCI_CLIENT_DIR/tools/ccsLoadDmsc/"$SOC"/
        $COPY $ROOTDIR/ti/binary/sciserver_testapp_freertos/bin/"$SOC"/sciserver_testapp_freertos_mcu1_0_release.rprc $SCI_CLIENT_DIR/tools/ccsLoadDmsc/"$SOC"/

        make -j -s sciserver_testapp_safertos_clean BOARD="$SOC"_evm
        make -j -s sciserver_testapp_safertos BOARD="$SOC"_evm

        if [ "$SOC" = "j7200" ] || [ "$SOC" = "j721s2" ] || [ "$SOC" = "j784s4" ]; then
            make -j -s sciclient_boardcfg_combined BOARD="$SOC"_evm
        fi
    done

    cd -
fi

################################################################################
# Revert applied version update bypass patch
if [ "$UPDATE_VERSION_HEADER" == "YES" ]; then
    cd $ROOTDIR/..
    $ECHO "Reverting the applied patch"
    git apply -R $ROOTDIR/ti/drv/sciclient/tools/DM_Migration.patch
fi

################################################################################
# Commit changes to PDK
if [ "$SKIP_COMMIT" != "YES" ]; then
    $ECHO "Commit changes to PDK"
    cd $SCRIPT_DIR

    for SOC in $SOC_LIST
    do
        case $SOC in
            "j721e")
                git add $SCI_CLIENT_DIR/soc/V1
                git add $SCI_CLIENT_DIR/tools/ccsLoadDmsc/j721e
                shift
                ;;
            "j7200")
                git add $SCI_CLIENT_DIR/soc/V2
                git add $SCI_CLIENT_DIR/tools/ccsLoadDmsc/j7200
                shift
                ;;
            "j721s2")
                git add $SCI_CLIENT_DIR/soc/V4
                git add $SCI_CLIENT_DIR/tools/ccsLoadDmsc/j721s2
                shift
                ;;
            "j784s4")
                git add $SCI_CLIENT_DIR/soc/V6
                git add $SCI_CLIENT_DIR/tools/ccsLoadDmsc/j784s4
                shift
                ;;
            "j742s2")
                git add $SCI_CLIENT_DIR/soc/V6
                git add $SCI_CLIENT_DIR/tools/ccsLoadDmsc/j742s2
                shift
                ;;
        esac
    done

    git add $SCI_CLIENT_DIR/src/version/*

    git commit -s -m "Migrating to DM version $version "
fi


################################################################################

$ECHO "Done."
