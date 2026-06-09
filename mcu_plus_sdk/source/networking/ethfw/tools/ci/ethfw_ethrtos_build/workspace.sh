#!/bin/bash

echo "Current directory: $(pwd)"
echo "Timestamp: $(date +"%I:%M:%S %p")"
printenv | sort
#set -Eeuo pipefail
set -x
#set -o functrace

# On failure, ensure artifacts are archived so logs can be retrieved
trap 'if [ -d ./artifacts ] && [ ! -f ./artifacts.tar.gz ]; then tar -czf ./artifacts.tar.gz ./artifacts 2>/dev/null || true; fi' EXIT

#rm -rf /workdir/clone
ethfw_manifest_repo="ssh://git@bitbucket.itg.ti.com/processor-sdk-gateway/repo_manifests.git"
doxygen_install_path="/usr/bin"

# Define Environment Variables with defaults (can be overridden by environment)
: ${work_dir:="/workdir"}
: ${clone_dir:="/workdir/clone"}
: ${workarea_dir:="/workdir/workarea"}
: ${artifacts_dir:="/workdir/artifacts"}
: ${tools_dir:="${HOME}/ti"}

JSON_FILE="./envfile.json"

# Function to read JSON value (handles both quoted strings and unquoted booleans)
get_json_value() {
    key=$1
    # Match either: "string_value" or boolean/number value (no quotes)
    grep -oP "\"$key\":\s*(?:\"([^\"]*)\"|([^,}]+))" "$JSON_FILE" | sed 's/.*:\s*//;s/"//g' | tr -d ' '
}

# Read values
manifest_tag=$(get_json_value manifest_tag)
product_family=$(get_json_value product_family)
release_build=$(get_json_value release_build)
release_version=$(get_json_value release_version)
fsdk_link=$(get_json_value fsdk_link)
ethfw_build=$(get_json_value ethfw_build)
docs_build=$(get_json_value docs_build)
ethfw_tag=$(get_json_value ethfw_tag)
enet_tag=$(get_json_value enet_tag)
tsn_tag=$(get_json_value tsn_tag)
pdk_tag=$(get_json_value pdk_tag)
csl_tag=$(get_json_value csl_tag)
mcusw_tag=$(get_json_value mcusw_tag)
sdk_builder_tag=$(get_json_value sdk_builder_tag)
concerto_tag=$(get_json_value concerto_tag)
release_version_short_dot=$(get_json_value release_version_short_dot)
kw_build=$(get_json_value kw_build)
quick_ethfw_build=$(get_json_value quick_ethfw_build)
enet_build=$(get_json_value enet_build)
cplusplus_build=$(get_json_value cplusplus_build)
pdk_docs_tag=$(get_json_value pdk_docs_tag)
build_number=$(get_json_value build_number)
build_timestamp=$(get_json_value build_timestamp)
trigger_tests=$(get_json_value trigger_tests)

# Print values
echo "========================================"
echo "BUILD CONFIGURATION"
echo "========================================"
echo "Manifest Tag    : $manifest_tag"
echo "Product Family  : $product_family"
echo "Release Build   : $release_build"
echo "Release Version : $release_version"
echo "FSDK Link       : $fsdk_link"
echo "ethfw_build	  : $ethfw_build"
echo "enet_build      : $enet_build"
echo "Docs Build      : $docs_build"
echo "EthFW Tag       : $ethfw_tag"
echo "Enet Tag        : $enet_tag"
echo "TSN Tag         : $tsn_tag"
echo "PDK Tag         : $pdk_tag"
echo "CSL Tag         : $csl_tag"
echo "MCUSW Tag       : $mcusw_tag"
echo "SDK Builder Tag : $sdk_builder_tag"
echo "Concerto Tag    : $concerto_tag"
echo "Release Version Short Dot : $release_version_short_dot"
echo "KW Build        : $kw_build"
echo "Quick EthFW Build : $quick_ethfw_build"
echo "C++ Build       : $cplusplus_build"
echo "PDK Docs Tag    : $pdk_docs_tag"
echo "build_number	  : $build_number"
echo "build_timestamp : $build_timestamp"
echo "trigger_tests : $trigger_tests"
echo "========================================"

# Check if mono is available (required for SBL appimage generation)
echo "=========================================="
echo "Checking for mono runtime..."
echo "=========================================="
if command -v mono &> /dev/null; then
    echo "mono is available:"
    mono --version
else
    echo "WARNING: mono runtime not found!"
    echo "Appimage generation will fail without mono."
    echo "Please ensure mono-complete is installed in the Docker image."
fi


# Create an SSH agent and load the key baked into the image at build time
eval $(ssh-agent -s)
ssh-add /root/.ssh/id_rsa
export GIT_SSH_COMMAND="ssh -i /root/.ssh/id_rsa"

total_build_start_time=`date +%s`
date=`date '+%b_%d_%Y_%I_%M_%p'`
date_print=`date '+%d-%b-%Y %I:%M:%S %p'`
git_path="/usr/bin"


#Default value if not provided
if [ "$working_dir" == "" ]; then
  working_dir=`pwd`
fi
: ${product_family:="jacinto"}
: ${release_build:="false"}
: ${release_version:="mm_nn_pp_bb"}
: ${ethfw_build:="false"}
: ${quick_ethfw_build:="false"}
: ${enet_build:="false"}
: ${stable_build:="false"}
: ${docs_build:="false"}
: ${cplusplus_build:="true"}
: ${manifest_tag:="master"}
: ${ethfw_manifest_repo:="ssh://git@bitbucket.itg.ti.com/processor-sdk-gateway/repo_manifests.git"}
: ${doxygen_install_path:="/usr/bin"}

: ${kw_build:="false"}
: ${kw_build_label:="ETHFW_KW_BUILD"}
: ${kw_project:="ETHSWITCHFW"}
: ${kw_release_build:="true"}
: ${kw_install_path:="/kw_path/bin"}
: ${kw_url:="https://klocworkweb.india.ti.com:8095"}
: ${kw_user:="a0490934"}
# To get Klocwork ltoken use: cat ~/.klocwork/ltoken and paste only token from right KW/Port entry
: ${kw_ltoken:="9948b12a0055fd1619da5883d8369e98ef4fd79e432223eb1484625ec18bc235"}

if [ "${release_build}" == "true" ]; then
   profile_list="release"
elif [ "${product_family}" != "jacinto" ]; then
   # For non-jacinto builds (specific device builds), only build release profile
   profile_list="release"
else
   profile_list="debug release"
fi

print_time_diff() {
  local start_time=$1
  local message=$2

  local end_time=$(date +%s)
  local diff=$((end_time - start_time))

  echo "$message: $diff seconds"
}

tag_replace() {
  local file=$1
  local repo_name=$2
  local tag=$3
  sed -i -e "s|${repo_name}|${tag}|g" ${file}
}

if [ -d "$clone_dir" ]; then
  # Check if the repository is properly cloned
  if [ -f "$clone_dir/ethfw_j7200.xml" ]; then
    echo "Repository is already cloned, skipping cloning step"
  else
    # Remove the existing clone directory and re-clone the repository
    echo "Repository is not properly cloned, re-cloning..."
    rm -rf "$clone_dir"
    git clone "$ethfw_manifest_repo" "$clone_dir"
    echo "Repository cloned successfully to $clone_dir"
  fi
else
  echo "Repository is not cloned, cloning..."
  git clone "$ethfw_manifest_repo" "$clone_dir"
  echo "Repository cloned successfully to $clone_dir"
fi

#Derived variables
if [ "$product_family" == "jacinto" ]; then
    if [ "$enet_build" == "true" ]; then    
       : ${board_list:="j721e_evm j7200_evm j721s2_evm j784s4_evm j742s2_evm"}
    else
       : ${board_list:="j721e_evm j7200_evm j784s4_evm j742s2_evm"}
    fi
fi
if [ "$product_family" == "j721e" ]; then
   : ${board_list:="j721e_evm"}
fi
if [ "$product_family" == "j7200" ]; then
   : ${board_list:="j7200_evm"}
fi
if [ "$product_family" == "j784s4" ]; then
   : ${board_list:="j784s4_evm"}
fi
if [ "$product_family" == "j742s2" ]; then
   : ${board_list:="j742s2_evm"}
fi
if [ "$product_family" == "j721s2" ]; then
   : ${board_list:="j721s2_evm"}
fi

release_version_dot=`echo ${release_version} | sed -e "s|\_|.|g"`
release_version_short=`echo ${release_version} | cut -d "_" -f -3`
# release_version_short_dot=`echo ${release_version_short} | sed -e "s|\_|.|g"`



soc=`echo ${board_list} | cut -d "_" -f 1`

fsdk_folder=`echo "$fsdk_link" | rev`
fsdk_folder=`echo ${fsdk_folder} | cut -d "/" -f 1`
fsdk_folder=`echo "$fsdk_folder" | rev`
fsdk_folder=`echo ${fsdk_folder} | cut -d "." -f 1`

pdk_version=`echo "$fsdk_folder" | rev`
pdk_version=`echo ${pdk_version} | cut -d "-" -f 1`
pdk_version=`echo "$pdk_version" | rev`



if [ "${release_build}" == "true" ]; then
    # For ETHFW release builds, use a common PDK folder name (not device-specific)
    # Even though multiple boards are built, they all use the same PDK
    pdk_folder=pdk_${pdk_version}
else
    # For ENET daily builds, use release version naming
    pdk_folder=pdk_${release_version}
fi



ethfw_folder=ethfw_${release_version_short}

if [ "${ethfw_build}" == "true" ]; then
product_name=ethfw
else
product_name=enet-lld
fi

release_folder=${product_name}-rtos-${soc}-${release_version}

if [ "$product_family" == "jacinto" ]; then
  release_folder=${product_name}-rtos-allSocs-${release_version}
fi

release_folder_binary_only=${release_folder}-binary_only

release_folder_docs_only=${release_folder}-docs_only

echo "soc: ${soc}"
echo "release_folder: ${release_folder}"
echo "release_folder_binary_only: ${release_folder_binary_only}"
echo "release_folder_docs_only: ${release_folder_docs_only}"

#Get the provided xml details
if [ "$product_family" == "jacinto" ]; then
  ethfw_manifest_xml=ethfw_j721e.xml
  fsdk_manifest_xml=fsdk_j721e.xml
else
  if [ "${ethfw_build}" == "true" ]; then
    ethfw_manifest_xml=ethfw_${soc}.xml
  fi
  fsdk_manifest_xml=fsdk_${soc}.xml
fi

if [ "${stable_build}" == "true" ]; then
    ethfw_manifest_xml=ethfw_${soc}_stable.xml
    fsdk_manifest_xml=fsdk_${soc}_stable.xml
fi

set_proxies() {
    echo "Set required proxies ..."
    export HTTPS_PROXY=http://webproxy.ext.ti.com:80
    export https_proxy=http://webproxy.ext.ti.com:80
    export HTTP_PROXY=http://webproxy.ext.ti.com:80
    export http_proxy=http://webproxy.ext.ti.com:80
    export ftp_proxy=http://webproxy.ext.ti.com:80
    export FTP_PROXY=http://webproxy.ext.ti.com:80
    export no_proxy=ti.com
    echo "Set required proxies ... Done"
}
set_proxies

create_build_target_file() {
	echo "Creating build target files...!!!"
    local file_name=${1}
    local file_ext=${2}
    local log_file=${3}
    echo "${file_name}${file_ext}"      >> ${build_targets_dir}/${file_name}.bt
    if [ -s "${log_file}" ]; then
        echo "${file_name}.bt:FAILED"   >> ${build_targets_dir}/build_targets
        echo "Error Log..."             >> ${build_targets_dir}/${file_name}.bt
        cat ${log_file}                 >> ${build_targets_dir}/${file_name}.bt
    else
        echo "${file_name}.bt:PASSED"   >> ${build_targets_dir}/build_targets
    fi
}

clean_folders() {
  rm -rf $clone_dir $workarea_dir ${artifacts_dir} ${build_targets_dir} ${log_dir}
  rm -rf $release_folder_dir ${release_folder_dir}.tar.gz
}
#clean_folders

# Define derived path variables (base directories already defined at top of script)
release_folder_dir=$work_dir/$release_folder

build_targets_dir=${artifacts_dir}/output
repo_revs_file=${artifacts_dir}/repo-revs.txt
log_dir=${artifacts_dir}/logs

build_log=${log_dir}/build.log
build_error_log=${log_dir}/error_build.log
pdk_log=${log_dir}/pdk.log
enet_log=${log_dir}/enet.log
ethfw_log=${log_dir}/ethfw.log

# Calculate num jobs to run in parallel - take one more to consider IO bound delays etc..
num_proc=$(nproc)
num_jobs=$((num_proc + 1))
jobs_option="-j${num_jobs}"
echo "Num Parallel Jobs : ${jobs_option}"

pdk_clone_path=$clone_dir/pdk
sdk_install_path=$workarea_dir
pdk_install_path=${workarea_dir}/${pdk_folder}
ethfw_clone_path=$clone_dir
ethfw_comp_install_path=$workarea_dir/ethfw

mkdir -p ${clone_dir}
mkdir -p ${artifacts_dir}
mkdir -p ${workarea_dir}
mkdir -p ${log_dir}
mkdir -p ${build_targets_dir}
mkdir -p ${release_folder_dir}
mkdir -p $release_folder_binary_only
mkdir -p $release_folder_docs_only
mkdir -p ${tools_dir}

touch ${artifacts_dir}/fsdk_j721e.xml
touch ${repo_revs_file}
touch ${build_log}
touch ${build_error_log}
touch ${pdk_log}
touch ${enet_log}
touch ${ethfw_log}


####################### Clone all required repos ######################
repo_init() {
  echo "Doing repo init and sync in $clone_dir ..."
  local start_time=`date +%s`
  if [ ! -d ${clone_dir}/.repo ]; then
    cd $clone_dir
    rm -rf $ethfw_manifest_xml
    rm -rf $fsdk_manifest_xml
    rm -rf common.sh
    rm -rf ethfw_tag_input.cfg
    if [ "${release_build}" == "true" ]; then
      git archive --remote=$ethfw_manifest_repo $manifest_tag $ethfw_manifest_xml > temp.tar
      tar -xf temp.tar --strip-components 0
      rm -rf temp.tar
    else
      if [ "${ethfw_build}" == "true" ]; then
        git archive --remote=$ethfw_manifest_repo $manifest_tag $ethfw_manifest_xml > temp.tar
        tar -xf temp.tar --strip-components 0
        rm -rf temp.tar
      fi
      git archive --remote=$ethfw_manifest_repo $manifest_tag $fsdk_manifest_xml > temp.tar
      tar -xf temp.tar --strip-components 0
      rm -rf temp.tar
    fi
    git archive --remote=$ethfw_manifest_repo $manifest_tag scripts/common.sh > temp.tar
    tar -xf temp.tar --strip-components 1
    rm -rf temp.tar

    git archive --remote=$ethfw_manifest_repo $manifest_tag releases/${release_version_short}/ethfw_tag_input.cfg > temp.tar
    tar -xf temp.tar --strip-components 2
    rm -rf temp.tar

    source common.sh
    source ethfw_tag_input.cfg

    if [ "${release_build}" == "false" ]; then
      echo "Overrding tag ..."
      #Override the branch in repo xml file
      if [ "${ethfw_build}" == "true" ]; then
        tag_replace $ethfw_manifest_xml "sdk_builder"             $sdk_builder_tag
        tag_replace $ethfw_manifest_xml "concerto"                $concerto_tag
        tag_replace $ethfw_manifest_xml "ethfw"                   $ethfw_tag
      fi
      tag_replace $fsdk_manifest_xml  "enet-lld"                $enet_tag
      tag_replace $fsdk_manifest_xml  "enet-tsn-stack"          $tsn_tag
      tag_replace $fsdk_manifest_xml  "pdk"                     $pdk_tag
      tag_replace $fsdk_manifest_xml  "pdk_docs"                $pdk_docs_tag
      tag_replace $fsdk_manifest_xml  "common-csl-ip"           $csl_tag
      tag_replace $fsdk_manifest_xml  "mcusw"                   $mcusw_tag
    fi
    #replace the repo xml with modified file
    if [ "${ethfw_build}" == "true" ]; then
      repo init -q --no-clone-bundle --no-repo-verify -u ${ethfw_manifest_repo} -b ${manifest_tag} -m $ethfw_manifest_xml
      rm -rf .repo/manifests/$ethfw_manifest_xml
      cp -f $ethfw_manifest_xml .repo/manifests
    fi
    if [ "${enet_build}" == "true" ]; then
      repo init -q --no-clone-bundle --no-repo-verify -u ${ethfw_manifest_repo} -b ${manifest_tag} -m $fsdk_manifest_xml
      rm -rf .repo/manifests/$fsdk_manifest_xml
      cp -f $fsdk_manifest_xml .repo/manifests
    fi

    if [ "${ethfw_build}" == "true" ]; then
      #Clone
      echo "Cloning as per provided XML: $ethfw_manifest_xml ..."
      repo init -q --no-repo-verify -m $ethfw_manifest_xml
    fi

    if [ "${enet_build}" == "true" ]; then
      #Clone
      echo "Cloning with XML: $fsdk_manifest_xml ..."
      repo init -q --no-repo-verify -m $fsdk_manifest_xml
    fi

    #repo sync -q
    repo sync -j10
    if [ $? -ne 0 ]; then
        echo "ERROR: repo sync failed! Aborting build."
        exit 1
    fi
    repo start dev --all
    #Show the current branch/git status
    repo forall -c "pwd;git branch -vv | cut -d ' ' -f 1-4"

    #Copy the manifest to artifacts folder
    if [ "${ethfw_build}" == "true" ]; then
      cp -f ${ethfw_manifest_xml} ${artifacts_dir}
    fi

    if [ "${release_build}" == "false" ]; then
        cp -f ${fsdk_manifest_xml} ${artifacts_dir}
    fi
    # Print clone time
    echo "Cloning completed!!"
    cd - > /dev/null

  fi
  print_time_diff $start_time "Clone Time"
  echo "Repo init and sync in $clone_dir ... Done"
  echo ""
}
repo_init


echo "pdk_install_path: ${pdk_install_path}"
echo "pdk_folder: ${pdk_folder}"
echo "DEBUG: release_build = ${release_build}, product_family = ${product_family}, pdk_version = ${pdk_version}"
echo "DEBUG: pdk_install_path resolves to: $([ -d ${pdk_install_path} ] && echo exists || echo DOES NOT EXIST)"
ls -la ${workarea_dir} 2>&1 | grep "^d.*pdk" | sed 's/^/DEBUG: Found PDK folder: /'

pdk_doxygen_build() {
    echo "Starting Doxygen Build..."
    local start_time=`date +%s`
    cd $pdk_install_path/docs/internal_docs/doxygen

    for board in $board_list
    do
        echo "  Doxygen Build for Board:${board} ..."
        make -s apiguide BOARD=$board SDK_INSTALL_PATH=$sdk_install_path PDK_INSTALL_PATH=$pdk_install_path/packages DOXYGEN=doxygen TOOLS_INSTALL_PATH=${tools_dir} 1>>${pdk_log} 2>>${build_error_log}
        echo "  Doxygen Build for Board:${board} completed!!"
    done

    cd - > /dev/null
    print_time_diff $start_time "Doxygen Time"
    echo "Doxygen Build completed!!"
    echo ""
}



pdk_sphinx_build() {
    echo "Starting Sphinx Build..."
    local start_time=`date +%s`

    cd $pdk_install_path/docs/internal_docs/sphinx
    export PATH=/workdir/.local/bin:$PATH
    make -s all > /dev/null
    cd - > /dev/null

    print_time_diff $start_time "Sphinx Time"
    echo "Sphinx Build completed!!"
    echo ""
}




get_repo_revs() {
    local xmlFile=$1
    ## Set release name and directory
    local git_name=`git remote show origin -n | grep "Fetch URL:" | cut -d":" -f2- | cut -d" " -f2 | sed -e "s|\:|;|g"`
    local repo_name=`git remote show origin -n | grep "Fetch URL:" | cut -d":" -f2- | cut -d" " -f2 | cut -d"/" -f5- | cut -d"." -f1`

    ## Get latest commit ID and branch
    local commit_id=`git log -n 1 | grep commit | grep -v 'this commit' | cut -d" " -f 2`
    local git_branch=`git branch -vv | cut -d ' ' -f 2-4`
    local commit_msg=`git log -1 --pretty=format:%s | sed -e "s|:|\||g"`

    ## Write into the repo-revs info file
    echo -n ${git_name}:${commit_id}:${git_branch}: >> ${repo_revs_file}
    echo ${commit_msg}                              >> ${repo_revs_file}

    ## Replace the branch with the fixed commit in artifact XML file
    tag_replace ${artifacts_dir}/${xmlFile} ${repo_name} ${commit_id}
    #Clone depth already set. Revert the duplicate change due to above command
    sed -i -e 's|clone-depth="1" clone-depth="1"|clone-depth="1"|g'  ${artifacts_dir}/${xmlFile}
}

repo_revs_pdk() {
    if [ -d ${clone_dir}/pdk ]; then
        cd ${clone_dir}/pdk
        get_repo_revs $fsdk_manifest_xml 
        cd - 1>/dev/null
    fi
    if [ -d ${clone_dir}/pdk/docs ]; then
        cd ${clone_dir}/pdk/docs
        get_repo_revs $fsdk_manifest_xml
        cd - 1>/dev/null
    fi
    if [ -d ${clone_dir}/pdk/packages/ti/csl ]; then
        cd ${clone_dir}/pdk/packages/ti/csl
        get_repo_revs $fsdk_manifest_xml
        cd - 1>/dev/null
    fi
    if [ -d ${clone_dir}/pdk/packages/ti/drv/pm ]; then
        cd ${clone_dir}/pdk/packages/ti/drv/pm
        get_repo_revs $fsdk_manifest_xml
        cd - 1>/dev/null
    fi
    if [ -d ${clone_dir}/pdk/packages/ti/drv/pmic ]; then
        cd ${clone_dir}/pdk/packages/ti/drv/pmic
        get_repo_revs $fsdk_manifest_xml
        cd - 1>/dev/null
    fi
    if [ -d ${clone_dir}/pdk/packages/ti/drv/enet ]; then
        cd ${clone_dir}/pdk/packages/ti/drv/enet
        get_repo_revs $fsdk_manifest_xml
        cd - 1>/dev/null
    fi
    if [ -d ${clone_dir}/pdk/packages/ti/drv/csirx ]; then
        cd ${clone_dir}/pdk/packages/ti/drv/csirx
        get_repo_revs $fsdk_manifest_xml
        cd - 1>/dev/null
    fi
    if [ -d ${clone_dir}/pdk/packages/ti/drv/csitx ]; then
        cd ${clone_dir}/pdk/packages/ti/drv/csitx
        get_repo_revs $fsdk_manifest_xml
        cd - 1>/dev/null
    fi
    if [ -d ${clone_dir}/pdk/packages/ti/drv/sa ]; then
        cd ${clone_dir}/pdk/packages/ti/drv/sa
        get_repo_revs $fsdk_manifest_xml
        cd - 1>/dev/null
    fi
    if [ -d ${clone_dir}/pdk/packages/ti/drv/sciclient/src/rm_pm_hal ]; then
        cd ${clone_dir}/pdk/packages/ti/drv/sciclient/src/rm_pm_hal
        get_repo_revs $fsdk_manifest_xml
        cd - 1>/dev/null
    fi
}


download_components() {
    echo "Downloading components in $workarea_dir ..."
    local start_time=`date +%s`
    cd $workarea_dir

    for board in $board_list
    do
      soc=`echo ${board} | cut -d "_" -f 1`
      $clone_dir/.repo/manifests/download_${soc}_components.sh
    done

    cd - > /dev/null
 	#print_time_diff $start_time "Download Time"
    echo "Downloading components in $workarea_dir ... Done"
    echo ""

    #Exports the compiler path variables
    export PSDK_TOOLS_PATH=${tools_dir}
    export TOOLS_INSTALL_PATH=${tools_dir}

    if [ "${release_build}" == "true" ]; then
      echo "Downloading FSDK Tarball ..."
      echo "DEBUG: fsdk_link = ${fsdk_link}"
      echo "DEBUG: fsdk_folder = ${fsdk_folder}"

      # Fix fsdk_link if it starts with // (missing protocol)
      local fixed_fsdk_link="${fsdk_link}"
      if [[ "${fsdk_link}" == //* ]]; then
          fixed_fsdk_link="http:${fsdk_link}"
          echo "DEBUG: Fixed fsdk_link to: ${fixed_fsdk_link}"
      fi

      # Get the base directory URL and find the actual available tarball via pattern matching.
      # The version in fsdk_link may be stale; the server may have a newer patch version.
      local fsdk_base_url
      fsdk_base_url="$(dirname "${fixed_fsdk_link}")"
      echo "DEBUG: FSDK base URL: ${fsdk_base_url}"

      local actual_tarball
      actual_tarball=$(wget -q -O - "${fsdk_base_url}/" 2>/dev/null | \
          grep -oE "ti-processor-sdk-rtos-${soc}-[0-9_]+\\.tar\\.gz" | \
          grep -v 'docs_only' | grep -v 'windows_codegen_tools' | \
          sort -u | head -1)

      if [ -z "${actual_tarball}" ]; then
          echo "ERROR: Could not find FSDK tarball matching ti-processor-sdk-rtos-${soc}-*.tar.gz on server"
          exit 1
      fi

      echo "DEBUG: Found FSDK tarball on server: ${actual_tarball}"
      fixed_fsdk_link="${fsdk_base_url}/${actual_tarball}"

      # Update fsdk_folder and derived variables to match the actual tarball found
      fsdk_folder="${actual_tarball%.tar.gz}"
      pdk_version=$(echo "$fsdk_folder" | rev | cut -d "-" -f 1 | rev)
      pdk_folder="pdk_${pdk_version}"
      pdk_install_path="${workarea_dir}/${pdk_folder}"
      echo "DEBUG: Updated fsdk_folder    = ${fsdk_folder}"
      echo "DEBUG: Updated pdk_version    = ${pdk_version}"
      echo "DEBUG: Updated pdk_folder     = ${pdk_folder}"
      echo "DEBUG: Updated pdk_install_path = ${pdk_install_path}"

      # download the FSDK Tarball and use that directly to package
      cd ${workarea_dir}
      echo "DEBUG: About to wget from: ${fixed_fsdk_link}"
      wget -q "${fixed_fsdk_link}"
      if [ $? -ne 0 ]; then
          echo "ERROR: wget failed to download FSDK tarball from ${fixed_fsdk_link}"
          exit 1
      fi

      echo "DEBUG: Checking for downloaded tar file: $fsdk_folder.tar.gz"
      if [ -f "$fsdk_folder.tar.gz" ]; then
          echo "DEBUG: Tarball found, extracting..."
          tar -xf $fsdk_folder.tar.gz
          echo "DEBUG: Extraction complete"
          rm -rf $fsdk_folder.tar.gz
      else
          echo "ERROR: Tarball not found at $fsdk_folder.tar.gz"
          ls -la ${workarea_dir}/*.tar.gz 2>&1 | sed 's/^/  /'
          exit 1
      fi
      cd - 1>/dev/null

      echo "DEBUG: After extraction, contents of ${workarea_dir}:"
      ls -la ${workarea_dir} | head -20 | sed 's/^/  /'

      if [ -d "${workarea_dir}/$fsdk_folder" ]; then
          echo "DEBUG: Moving extracted files..."
          mv -f ${workarea_dir}/$fsdk_folder/*  ${workarea_dir}
          echo "DEBUG: Move complete"

          echo "DEBUG: After moving, checking for PDK folder..."
          ls -la ${workarea_dir} | grep "^d.*pdk" | sed 's/^/  /'

          # The extracted PDK might have device-specific name (e.g., pdk_j784s4_11_02_01_01)
          # but we need it as pdk_11_02_01_01. Rename if needed.
          for extracted_pdk in ${workarea_dir}/pdk_*_${pdk_version}; do
              if [ -d "$extracted_pdk" ]; then
                  echo "DEBUG: Found extracted PDK at: $extracted_pdk"
                  target_pdk="${workarea_dir}/${pdk_folder}"
                  if [ -d "$target_pdk" ]; then
                      echo "DEBUG: Target PDK already exists at $target_pdk"
                  else
                      echo "DEBUG: Renaming $extracted_pdk to $target_pdk"
                      mv "$extracted_pdk" "$target_pdk"
                  fi
              fi
          done

          echo "DEBUG: After rename, checking for PDK folder..."
          ls -la ${workarea_dir} | grep "^d.*pdk" | sed 's/^/  /'
      else
          echo "ERROR: Extracted folder not found at ${workarea_dir}/$fsdk_folder"
          exit 1
      fi
    fi
}
download_components

###################### Package Build ######################
package_pdk() {
    echo "  Package PDK ..."
    local start_time=`date +%s`

    #Repo rev PDK
    repo_revs_pdk

    cd ${clone_dir}/pdk/packages/ti/build
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to cd to ${clone_dir}/pdk/packages/ti/build - PDK not properly cloned!"
        exit 1
    fi

	for board in ${board_list}
	do
    	echo "    PDK Packaging Board:${board} ..."
    	timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
    	pdk_board_log=${log_dir}/pdk_${board}_${timestamp}.log
    	touch ${pdk_board_log}
    	make -s $jobs_option allcores_package PACKAGE_SELECT=all BOARD=${board} TOOLS_INSTALL_PATH:=${tools_dir} SDK_INSTALL_PATH=$sdk_install_path PDK_INSTALL_PATH=$pdk_clone_path/packages &> ${pdk_board_log}
    	echo "    PDK Packaging Board:${board} completed!!"
    	#cat ${pdk_board_log}
	done

    cd - 1>/dev/null


    #Copy the packaged files to build folder
    if [ ! -d "${clone_dir}/pdk/packages/ti/binary/package/all/pdk_" ]; then
        echo "ERROR: PDK packaging failed - packaged directory not found at ${clone_dir}/pdk/packages/ti/binary/package/all/pdk_"
        exit 1
    fi

    # Use file locking to prevent race conditions when multiple builds try to copy PDK in parallel
    # (common when running promotion_complete.py with multiple devices)
    echo "  Acquiring lock for PDK copy operation..."
    pdk_lock_file="${workarea_dir}/.pdk_copy_lock"
    mkdir -p "${workarea_dir}"

    # Wait for lock with timeout
    local lock_timeout=600  # 10 minutes timeout
    local lock_elapsed=0
    while [ -f "$pdk_lock_file" ] && [ $lock_elapsed -lt $lock_timeout ]; do
        echo "  Waiting for PDK lock (${lock_elapsed}s)..."
        sleep 2
        lock_elapsed=$((lock_elapsed + 2))
    done

    if [ -f "$pdk_lock_file" ]; then
        echo "WARNING: PDK lock timed out after ${lock_timeout}s, proceeding anyway"
    fi

    # Create lock file
    touch "$pdk_lock_file"
    trap "rm -f $pdk_lock_file" EXIT

    # Always copy fresh PDK to prevent cross-contamination between parallel builds
    # (e.g., SafeRTOS setup for one device corrupting PDK used by next device)
    # File locking ensures only one build copies at a time
    echo "  Copying PDK to workarea..."
    rm -rf ${workarea_dir}/${pdk_folder}
    cp -rf ${clone_dir}/pdk/packages/ti/binary/package/all/pdk_     ${workarea_dir}
    mv -f ${workarea_dir}/pdk_                                    ${workarea_dir}/${pdk_folder}
    echo "  PDK copied successfully"

    rm -f "$pdk_lock_file"


    #Copy non-source items not packaged through make package
    cp -rf ${clone_dir}/pdk/docs                 ${workarea_dir}/${pdk_folder}
    cp -rf ${clone_dir}/pdk/packages/makefile    ${workarea_dir}/${pdk_folder}/packages
    #echo "hear2 $(pwd)"
    if [ -d ${workarea_dir}/${pdk_folder}/packages/ti/transport/lwip/lwip-contrib/ ]; then
        #Delete .git folder of lwip-contrib
        rm -rf ${workarea_dir}/${pdk_folder}/packages/ti/transport/lwip/lwip-contrib/.git*
    fi
    if [ -d ${workarea_dir}/${pdk_folder}/packages/ti/transport/lwip/lwip-stack/ ]; then
        #Delete .git folder of lwip-stack
        rm -rf ${workarea_dir}/${pdk_folder}/packages/ti/transport/lwip/lwip-stack/.git*
    fi

    if [ -d ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet* ]; then
        #enet documentation is part of driver itself and not part of pdk_docs, copy the same
        mkdir -p ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/internal_docs
        cp -rf ${clone_dir}/pdk/packages/ti/drv/enet/internal_docs/doxygen  ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/internal_docs/

        # Copy enet_component.mk from source PDK to workarea (packaging workaround)
        # make allcores_package PACKAGE_SELECT=all does not include component .mk files, but they are
        # required by the PDK top-level makefile to discover enet app build targets (enet_loopback_test, etc.)
        if [ ! -f ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk ]; then
            if [ -f ${clone_dir}/pdk/packages/ti/drv/enet/enet_component.mk ]; then
                echo "  Copying enet_component.mk (packaging workaround)..."
                mkdir -p ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet
                cp -f ${clone_dir}/pdk/packages/ti/drv/enet/enet_component.mk \
                    ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/
                echo "  enet_component.mk copied"
            else
                echo "  WARNING: enet_component.mk not found in source PDK"
            fi
        fi

        #Enable SBL appimage generation for enet examples if not already set (FIX for missing .appimage files)
        if [ -f ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk ]; then
            if ! grep -q "SBL_APPIMAGEGEN" ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk; then
                echo "  WARNING: SBL_APPIMAGEGEN not found - enabling for all enet examples..."
                sed -i '/export enet_lwip_example_\$(1)_SOCLIST/a export enet_lwip_example_$(1)_SBL_APPIMAGEGEN = yes' ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk
                sed -i '/export enet_loopback_test_\$(1)_SOCLIST/a export enet_loopback_test_$(1)_SBL_APPIMAGEGEN = yes' ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk
                sed -i '/export enet_est_test_\$(1)_SOCLIST/a export enet_est_test_$(1)_SBL_APPIMAGEGEN = yes' ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk
                sed -i '/export enet_tsn_gptp_example_\$(1)_SOCLIST/a export enet_tsn_gptp_example_$(1)_SBL_APPIMAGEGEN = yes' ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk
                sed -i '/export enet_tsn_lldp_example_\$(1)_SOCLIST/a export enet_tsn_lldp_example_$(1)_SBL_APPIMAGEGEN = yes' ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk
                sed -i '/export enet_tsn_netconf_example_\$(1)_SOCLIST/a export enet_tsn_netconf_example_$(1)_SBL_APPIMAGEGEN = yes' ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk
                sed -i '/export enet_tsn_est_example_\$(1)_SOCLIST/a export enet_tsn_est_example_$(1)_SBL_APPIMAGEGEN = yes' ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk
                sed -i '/export enet_cli_test_\$(1)_SOCLIST/a export enet_cli_test_$(1)_SBL_APPIMAGEGEN = yes' ${workarea_dir}/${pdk_folder}/packages/ti/drv/enet/enet_component.mk
                echo "  SBL_APPIMAGEGEN enabled!"
            else
                echo "  SBL_APPIMAGEGEN already set - skipping"
            fi
        fi
    fi

    #Copy build directory (workaround for packaging bug in PDK master)
    #The packaging process fails to include packages/ti/build/* with makefile, Rules.make, etc.
    if [ -d ${clone_dir}/pdk/packages/ti/build ]; then
        echo "  Copying build directory (packaging workaround)..."
        mkdir -p ${workarea_dir}/${pdk_folder}/packages/ti
        cp -rf ${clone_dir}/pdk/packages/ti/build ${workarea_dir}/${pdk_folder}/packages/ti/
        echo "  Build directory copied"
    fi

    #Copy OSAL arch and src directories (workaround for packaging bug in PDK master)
    #The packaging process fails to include arch/core/* and src/* source files needed for OSAL builds
    if [ -d ${clone_dir}/pdk/packages/ti/osal/arch ]; then
        echo "  Copying OSAL arch directory (packaging workaround)..."
        mkdir -p ${workarea_dir}/${pdk_folder}/packages/ti/osal
        cp -rf ${clone_dir}/pdk/packages/ti/osal/arch ${workarea_dir}/${pdk_folder}/packages/ti/osal/
        echo "  OSAL arch directory copied"
    fi
    if [ -d ${clone_dir}/pdk/packages/ti/osal/src ]; then
        echo "  Copying OSAL src directory (packaging workaround)..."
        mkdir -p ${workarea_dir}/${pdk_folder}/packages/ti/osal
        cp -rf ${clone_dir}/pdk/packages/ti/osal/src ${workarea_dir}/${pdk_folder}/packages/ti/osal/
        echo "  OSAL src directory copied"
    fi


    #Changes after package
    cd ${workarea_dir}/${pdk_folder}/packages/ti/build
    sed -i -e "s|..\...\...\...|${release_version_dot}|g"                                           makefile
    #Replace only the first occurrence - that is for release build
    sed -i -e '0,/DISABLE_RECURSE_DEPS....no/s//DISABLE_RECURSE_DEPS \?\= yes/'                     Rules.make
    sed -i -e "s|PDK_VERSION_STR=_\$[(]PDK_SOC[)]_\$[(]PDK_VERSION[)]|PDK_VERSION_STR=|g"           pdk_tools_path.mk
    sed -i -e "s|PDK_VERSION_STR=|PDK_VERSION_STR=_${product_family}_${release_version}|g"          pdk_tools_path.mk
    cd - 1>/dev/null
    print_time_diff $start_time "  Package PDK Time"
    echo "  Package PDK ... Done"
}

package_ethfw_comp() {
  echo "Packaging ETHFW to $workarea_dir ..."
  local start_time=`date +%s`

  #Repo rev
  cd ${clone_dir}/ethfw
  get_repo_revs $ethfw_manifest_xml
  cd - 1>/dev/null
  cd ${clone_dir}/sdk_builder
  get_repo_revs $ethfw_manifest_xml
  cd - 1>/dev/null
  cd ${clone_dir}/sdk_builder/concerto
  get_repo_revs $ethfw_manifest_xml
  cd - 1>/dev/null
  
  cd $clone_dir

  #Copy the packaged files to build folder
  cp -rf $clone_dir/ethfw $workarea_dir
  cp -rf $clone_dir/sdk_builder $workarea_dir

  #Change component paths
  sed -i -e "s|\/pdk|\/${pdk_folder}|g"   ${workarea_dir}/ethfw/ethfw_tools_path.mak

  rm -rf $workarea_dir/ethfw/.git
  rm -rf $workarea_dir/ethfw/.gitignore
  rm -rf $workarea_dir/sdk_builder/.git
  rm -rf $workarea_dir/sdk_builder/.gitignore
  rm -rf $workarea_dir/sdk_builder/concerto/.git
  rm -rf $workarea_dir/sdk_builder/concerto/.gitignore

  cd - > /dev/null
  print_time_diff $start_time "Package Time"
  echo "Packaging ETHFW to $workarea_dir ... Done"
  echo ""
}

enet_app_freertos_build()
{
  local board=$1
  # Base app list - these should work on all cores
  local enet_lld_appList_base="enet_loopback_test_freertos enet_lwip_example_freertos enet_est_test_freertos enet_helloworld_example enet_cli_test_freertos"
  # Advanced features - may not be available on all cores/boards
  local enet_lld_appList_advanced="enet_tsn_gptp_example_freertos enet_tsn_lldp_example_freertos enet_tsn_netconf_example_freertos enet_tsn_est_example_freertos"

  # Device-specific core list - J7200 doesn't have mcu3_0, c66, c7x cores
  local coreList="mcu2_0 mcu2_1 mcu1_0"
  if [ "${board}" != "j7200_evm" ]; then
      coreList="mcu2_0 mcu2_1 mcu1_0 mcu3_0"
  fi

  echo "Building Enet-lld Apps on freeRTOS for Board:${board} (Cores: ${coreList})..."
  for profile in $profile_list
  do
    for core in $coreList
    do
      # For mcu1_0, build only basic targets to avoid unavailable features
      if [ "$core" == "mcu1_0" ]; then
          local enet_lld_appList="$enet_lld_appList_base"
      else
          local enet_lld_appList="$enet_lld_appList_base $enet_lld_appList_advanced enet_ecc_test"
      fi

      echo "  Building apps for CORE=${core} BOARD=${board} BUILD_PROFILE=${profile}..."
      echo "  DEBUG: enet_lld_appList = $enet_lld_appList"
      echo "  DEBUG: Current directory = $(pwd)"
      echo "  DEBUG: Makefile exists = $([ -f Makefile ] && echo yes || echo no)"
      echo "  DEBUG: pdk_install_path = $pdk_install_path"
      echo "  DEBUG: Checking if enet_component.mk exists:"
      find $pdk_install_path -name "enet_component.mk" -type f 2>/dev/null | sed 's/^/    /'
      echo "  DEBUG: Available make targets containing 'enet_loopback':"
      make -s $jobs_option --print-data-base CORE=$core BOARD=$board BUILD_PROFILE=$profile PDK_INSTALL_PATH=$pdk_install_path/packages 2>/dev/null | grep "enet_loopback\|^\.PHONY" | head -30 | sed 's/^/    /' || echo "    (make --print-data-base failed)"

      make -s $jobs_option $enet_lld_appList CORE=$core BOARD=$board BUILD_PROFILE=$profile DISABLE_RECURSE_DEPS=yes TOOLS_INSTALL_PATH=${tools_dir} PDK_INSTALL_PATH=$pdk_install_path/packages 1>>${enet_log} 2>>${build_error_log}
      local enet_freertos_exit_code=$?

      echo "  DEBUG: Make exit code = ${enet_freertos_exit_code}"
      if [ -f "${build_error_log}" ]; then
          echo "  DEBUG: Last 5 lines of error_build.log:"
          tail -5 "${build_error_log}" | sed 's/^/    /'
      fi

      if [ ${enet_freertos_exit_code} -ne 0 ]; then
          echo "ERROR: Enet-lld FreeRTOS app build failed for CORE=${core} BOARD=${board} BUILD_PROFILE=${profile} with exit code ${enet_freertos_exit_code}"
          echo "DEBUG: Attempting to skip this core and continue (instead of full exit)"
          echo "DEBUG: Checking if any binaries were created before failure..."
          find $pdk_install_path/packages/ti/binary -type f \( -name "*.out" -o -name "*.xer5f" \) 2>/dev/null | head -10 | sed 's/^/    /'
          # Continue instead of exiting - allow other cores to build
          continue
      fi
    done
  done
}

enet_app_safertos_build()
{
  local board=$1
  # Basic SafeRTOS app targets - these should work on all cores
  local enet_lld_appList="enet_loopback_test_safertos enet_lwip_example_safertos enet_est_test_safertos"

  # Device-specific core list - J7200 doesn't have mcu3_0, c66, c7x cores
  local coreList="mcu2_0 mcu2_1 mcu1_0"
  if [ "${board}" != "j7200_evm" ]; then
      coreList="mcu2_0 mcu2_1 mcu1_0 mcu3_0"
  fi

  if [ "${board}" != "j742s2_evm" ]; then
    # Only build SafeRTOS apps if SafeRTOS setup succeeded
    echo "  DEBUG: Checking SafeRTOS setup for Board:${board}"
    echo "  DEBUG: SAFERTOS_KERNEL_INSTALL_PATH = '${SAFERTOS_KERNEL_INSTALL_PATH}'"
    if [ -n "${SAFERTOS_KERNEL_INSTALL_PATH}" ]; then
        echo "  DEBUG: SAFERTOS_KERNEL_INSTALL_PATH is set"
        if [ -d "${SAFERTOS_KERNEL_INSTALL_PATH}" ]; then
            echo "  DEBUG: SAFERTOS_KERNEL_INSTALL_PATH directory exists"
        else
            echo "  DEBUG: SAFERTOS_KERNEL_INSTALL_PATH directory does NOT exist"
        fi
    else
        echo "  DEBUG: SAFERTOS_KERNEL_INSTALL_PATH is empty or unset"
    fi

    if [ -n "${SAFERTOS_KERNEL_INSTALL_PATH}" ] && [ -d "${SAFERTOS_KERNEL_INSTALL_PATH}" ]; then
      echo "Building Enet-lld Apps on safeRTOS for Board:${board} (Cores: ${coreList})..."
      for profile in $profile_list
      do
        for core in $coreList
        do
          echo "  Building apps for CORE=${core} BOARD=${board} BUILD_PROFILE=${profile}..."
          echo "  DEBUG: SafeRTOS enet_lld_appList = $enet_lld_appList"
          echo "  DEBUG: SafeRTOS Current directory = $(pwd)"
          echo "  DEBUG: SafeRTOS pdk_install_path = $pdk_install_path"
          echo "  DEBUG: SafeRTOS SAFERTOS_KERNEL_INSTALL_PATH = $SAFERTOS_KERNEL_INSTALL_PATH"
          echo "  DEBUG: Checking if safertos path and enet_component.mk exist:"
          [ -d "$SAFERTOS_KERNEL_INSTALL_PATH" ] && echo "    SafeRTOS path exists" || echo "    WARNING: SafeRTOS path DOES NOT EXIST"
          find $pdk_install_path -name "enet_component.mk" -type f 2>/dev/null | sed 's/^/    /' || echo "    (enet_component.mk not found)"
          echo "  DEBUG: Available make targets containing 'enet_loopback_test_safertos':"
          make -s $jobs_option --print-data-base CORE=$core BOARD=$board BUILD_PROFILE=$profile PDK_INSTALL_PATH=$pdk_install_path/packages SAFERTOS_KERNEL_INSTALL_PATH=$SAFERTOS_KERNEL_INSTALL_PATH 2>/dev/null | grep "enet_loopback_test_safertos" | head -10 | sed 's/^/    /' || echo "    (target not found in make database)"
          echo "  DEBUG: Make command: make -s $jobs_option $enet_lld_appList CORE=$core BOARD=$board BUILD_PROFILE=$profile DISABLE_RECURSE_DEPS=yes TOOLS_INSTALL_PATH=${tools_dir} PDK_INSTALL_PATH=$pdk_install_path/packages"
          make -s $jobs_option $enet_lld_appList CORE=$core BOARD=$board BUILD_PROFILE=$profile DISABLE_RECURSE_DEPS=yes TOOLS_INSTALL_PATH=${tools_dir} PDK_INSTALL_PATH=$pdk_install_path/packages 1>>${enet_log} 2>>${build_error_log}
          local enet_safertos_exit_code=$?
          echo "  DEBUG: SafeRTOS build exit code for CORE=${core}: ${enet_safertos_exit_code}"
          if [ ${enet_safertos_exit_code} -ne 0 ]; then
              echo "ERROR: Enet-lld SafeRTOS app build failed for CORE=${core} BOARD=${board} BUILD_PROFILE=${profile} with exit code ${enet_safertos_exit_code}"
              echo "Last 20 lines of error_build.log:"
              tail -20 ${build_error_log} | sed 's/^/  /'
              echo "Check error_build.log for details"
              exit 1
          fi
        done
      done
    else
      echo "Skipping SafeRTOS Enet-lld Apps build for Board:${board} (SafeRTOS setup failed or not available)"
    fi
  fi
}

get_component_versions()
{
  echo "release_version_short:${release_version_short}"
  local versions_file="$clone_dir/.repo/manifests/releases/${release_version_short}/.component_versions"
  if [ -f "${versions_file}" ]; then
    source "${versions_file}"
  else
    echo "WARNING: Component versions file not found: ${versions_file}"
    echo "SafeRTOS builds may fail without version information"
  fi
}

#################### Setup SafeRTOS environment for a specific board ####################
setup_safertos_for_board(){
    local board=$1
    get_component_versions

    cd $pdk_install_path/packages/ti/build

    echo "  Setting up SafeRTOS environment for Board:${board} ..."

    # Initialize SafeRTOS setup flag
    export SAFERTOS_KERNEL_INSTALL_PATH=""

    if [ "$board" == "j721e_evm" ]; then
        safertos_log=$(./safertos_setup.sh --soc=j721e --isa=r5f --path=${sdk_install_path}/safertos_j721e_r5f_${SAFERTOS_J721E_R5F_VERSION} $2 2>&1)
        safertos_exit_code=$?
        if [ ${safertos_exit_code} -eq 0 ] && [ -d "${sdk_install_path}/safertos_j721e_r5f_${SAFERTOS_J721E_R5F_VERSION}" ]; then
            ./safertos_setup.sh --soc=j721e --isa=c66 --path=${sdk_install_path}/safertos_j721e_c66_${SAFERTOS_J721E_C66_VERSION} $2 > /dev/null 2>&1
            ./safertos_setup.sh --soc=j721e --isa=c7x --path=${sdk_install_path}/safertos_j721e_c7x_${SAFERTOS_J721E_C7X_VERSION} $2 > /dev/null 2>&1
            export SAFERTOS_KERNEL_INSTALL_PATH=${sdk_install_path}/safertos_j721e_r5f_${SAFERTOS_J721E_R5F_VERSION}
            echo "  SafeRTOS setup successful for Board:${board}"
        else
            echo "  WARNING: SafeRTOS setup failed for Board:${board} (j721e r5f). Skipping SafeRTOS targets."
            if [ ${safertos_exit_code} -ne 0 ]; then
                echo "  SafeRTOS setup error (exit code: ${safertos_exit_code}):"
                echo "$safertos_log" | head -20
            fi
        fi
    fi
    if [ "$board" == "j7200_evm" ]; then
        # J7200 SafeRTOS: Some make targets don't exist in PDK, causing safertos_setup.sh to fail
        # Even though it fails, the SafeRTOS path is still created and usable
        # Solution: Don't check exit code, only check if the path was created
        echo "  Setting up SafeRTOS environment for Board:${board} ..."

        safertos_log=$(./safertos_setup.sh --soc=j7200 --isa=r5f --path=${sdk_install_path}/safertos_j7200_r5f_${SAFERTOS_J7200_R5F_VERSION} $2 2>&1)
        safertos_exit_code=$?

        echo "  DEBUG: SafeRTOS setup script exit code: ${safertos_exit_code} (may fail due to missing targets, but path should be created)"

        # For J7200, safertos_setup.sh may return non-zero due to missing make targets
        # Check if the SafeRTOS directory was created, which is the real indicator of success
        if [ -d "${sdk_install_path}/safertos_j7200_r5f_${SAFERTOS_J7200_R5F_VERSION}" ]; then
            export SAFERTOS_KERNEL_INSTALL_PATH=${sdk_install_path}/safertos_j7200_r5f_${SAFERTOS_J7200_R5F_VERSION}
            echo "  SafeRTOS setup successful for Board:${board}"
            echo "  DEBUG: SAFERTOS_KERNEL_INSTALL_PATH set to: ${SAFERTOS_KERNEL_INSTALL_PATH}"
        else
            echo "  WARNING: SafeRTOS path not created for Board:${board} (j7200 r5f). SafeRTOS apps will be skipped."
            if [ ${safertos_exit_code} -ne 0 ]; then
                echo "  SafeRTOS setup error (exit code: ${safertos_exit_code}):"
                echo "$safertos_log" | head -20
            fi
        fi
    fi
    if [ "$board" == "j721s2_evm" ]; then
        safertos_log=$(./safertos_setup.sh --soc=j721s2 --isa=r5f --path=${sdk_install_path}/safertos_j721s2_r5f_${SAFERTOS_J721S2_R5F_VERSION} $2 2>&1)
        safertos_exit_code=$?
        if [ ${safertos_exit_code} -eq 0 ] && [ -d "${sdk_install_path}/safertos_j721s2_r5f_${SAFERTOS_J721S2_R5F_VERSION}" ]; then
            ./safertos_setup.sh --soc=j721s2 --isa=c7x --path=${sdk_install_path}/safertos_j721s2_c7x_${SAFERTOS_J721S2_C7X_VERSION} $2 > /dev/null 2>&1
            export SAFERTOS_KERNEL_INSTALL_PATH=${sdk_install_path}/safertos_j721s2_r5f_${SAFERTOS_J721S2_R5F_VERSION}
            echo "  SafeRTOS setup successful for Board:${board}"
        else
            echo "  WARNING: SafeRTOS setup failed for Board:${board} (j721s2 r5f). Skipping SafeRTOS targets."
            if [ ${safertos_exit_code} -ne 0 ]; then
                echo "  SafeRTOS setup error (exit code: ${safertos_exit_code}):"
                echo "$safertos_log" | head -20
            fi
        fi
    fi
    if [ "$board" == "j784s4_evm" ]; then
        safertos_log=$(./safertos_setup.sh --soc=j784s4 --isa=r5f --path=${sdk_install_path}/safertos_j784s4_r5f_${SAFERTOS_J784S4_R5F_VERSION} $2 2>&1)
        safertos_exit_code=$?
        if [ ${safertos_exit_code} -eq 0 ] && [ -d "${sdk_install_path}/safertos_j784s4_r5f_${SAFERTOS_J784S4_R5F_VERSION}" ]; then
            ./safertos_setup.sh --soc=j784s4 --isa=c7x --path=${sdk_install_path}/safertos_j784s4_c7x_${SAFERTOS_J784S4_C7X_VERSION} $2 > /dev/null 2>&1
            export SAFERTOS_KERNEL_INSTALL_PATH=${sdk_install_path}/safertos_j784s4_r5f_${SAFERTOS_J784S4_R5F_VERSION}
            echo "  SafeRTOS setup successful for Board:${board}"
        else
            echo "  WARNING: SafeRTOS setup failed for Board:${board} (j784s4 r5f). Skipping SafeRTOS targets."
            if [ ${safertos_exit_code} -ne 0 ]; then
                echo "  SafeRTOS setup error (exit code: ${safertos_exit_code}):"
                echo "$safertos_log" | head -20
            fi
        fi
    fi
    if [ "$board" == "j742s2_evm" ]; then
        echo "  SafeRTOS not supported on Board:${board}, nothing to build"
    fi
    echo "  SafeRTOS environment setup for Board:${board} completed!!"

    cd - 1> /dev/null
}

#################### Build SafeRTOS ####################
build_safertos(){
    echo "Build SafeRTOS ..."
    local start_time=`date +%s`

    for board in $board_list
    do
        setup_safertos_for_board $board
    done

    print_time_diff $start_time "Build SafeRTOS Time"
    echo "Build SafeRTOS ... Done"
}


###################### SDL build ######################
build_sdl() {
    #copy SDL from clone dir to sdk dir
    cp -rf ${clone_dir}/sdl ${sdk_install_path}
    if [ -d ${sdk_install_path}/sdl* ]; then
        echo "  Build SDL ..."
        local start_time=`date +%s`
        #Build SDL Libraries
        cd ${sdk_install_path}/sdl
        for profile in ${profile_list}
        do
            for board in ${board_list}
            do
                if [ "${board}" == "j784s4_evm" ] || [ "${board}" == "j721s2_evm" ] || [ "${board}" == "j7200_evm" ] || [ "${board}" == "j721e_evm" ]; then
                    soc=`echo ${board} | cut -d "_" -f 1`
                    echo "  Building SDL Libs for Board:${board} ..."
                    make -s $jobs_option sdl_libs SOC=${soc} PROFILE=${profile} TOOLS_INSTALL_PATH=${tools_dir} TOOLCHAIN_PATH_R5=${tools_dir}/ti-cgt-armllvm_4.0.4.LTS 1>>$log_dir/build.log 2>>${build_error_log}
                    local sdl_exit_code=$?
                    if [ ${sdl_exit_code} -ne 0 ]; then
                        echo "WARNING: SDL libs build failed for Board:${board} Profile:${profile} with exit code ${sdl_exit_code}"
                        echo "SDL is optional - continuing with build (check error_build.log for details)"
                    else
                        echo "  Building SDL Libs for Board:${board} completed!!!"
                    fi
                fi 
            done
        done
        cd - 1>/dev/null
        print_time_diff $start_time "  Build SDL Time"
        echo "  Build SDL ... Done"
    fi
}



pdk_cpp_build() {
    echo "C++ Build ..."
    local start_time=`date +%s`
    cd $pdk_install_path/packages/ti/build

    cpp_build_core_list="mcu1_0 mcu1_1 c66xdsp_1"
    # c++ build to be checked only in release build
    cpp_build_profile_list="release"
    for profile in $cpp_build_profile_list
    do
        for board in $board_list
        do
        echo "  Building Board:$board Profile:$profile (c++ build) ..."
        for core in $cpp_build_core_list
        do
            make -s $jobs_option pdk_libs_clean BOARD=$board BUILD_PROFILE=$profile CORE=$core SDK_INSTALL_PATH=$sdk_install_path PDK_INSTALL_PATH=$pdk_install_path/packages CPLUSPLUS_BUILD=yes TOOLS_INSTALL_PATH=${tools_dir} 1>>${pdk_log} 2>>${build_error_log}
            make -s $jobs_option pdk_libs       BOARD=$board BUILD_PROFILE=$profile CORE=$core SDK_INSTALL_PATH=$sdk_install_path PDK_INSTALL_PATH=$pdk_install_path/packages CPLUSPLUS_BUILD=yes TOOLS_INSTALL_PATH=${tools_dir} 1>>${pdk_log} 2>>${build_error_log}
            local cpp_exit_code=$?
            if [ ${cpp_exit_code} -ne 0 ]; then
                echo "WARNING: C++ PDK libs build failed for Board:${board} Profile:${profile} Core:${core} with exit code ${cpp_exit_code}"
                echo "C++ build is non-critical, continuing with rest of build..."
            fi
        done
        echo "  Building Board:$board Profile:$profile (c++ build) completed!!"
        done
    done
    # clean all the binaries
    for board in $board_list
    do
        make -s allclean BOARD=$board SDK_INSTALL_PATH=$sdk_install_path PDK_INSTALL_PATH=$pdk_install_path/packages TOOLS_INSTALL_PATH=${tools_dir}
    done

    cd - > /dev/null
    print_time_diff $start_time "C++ Build Time"
    echo "C++ Build ... Done"
    echo ""
}


###################### PDK build ######################
pdk_build () {
    local start_time=`date +%s`

    # Build cpp libs first.
    if [ "${cplusplus_build}" == "true" ] && [ "${release_build}" == "false" ]; then
        pdk_cpp_build
    fi

    print_time_diff $start_time "  cpp libs first..."
    echo "  cpp libs first..."

    for profile in $profile_list
    do
      for board in $board_list
      do
        # First build libs
        local libs_start_time=`date +%s`
        echo "  Building PDK Libs for Board:${board} Profile:${profile} ..."
        echo "  Start time: $(date '+%Y-%m-%d %H:%M:%S')"
        echo "  DEBUG: pdk_install_path = ${pdk_install_path}"
        echo "  DEBUG: pdk_folder = ${pdk_folder}"
        echo "  DEBUG: Checking ${pdk_install_path}/packages/ti/build directory:"
        if [ -d "${pdk_install_path}/packages/ti/build" ]; then
            echo "    Directory EXISTS - proceeding with make"
            ls -la "${pdk_install_path}" 2>&1 | head -10 | sed 's/^/    /'
        else
            echo "    ERROR: Directory DOES NOT EXIST!"
            echo "    Available PDK directories:"
            ls -la ${workarea_dir} 2>&1 | grep "^d.*pdk" | sed 's/^/      /'
            exit 1
        fi
        if [ "${enet_build}" == "true" ]; then
            # ENET builds always run with release_build=false — package_pdk() created a binary-only
            # workarea PDK (no .c source files). Must build libs from SOURCE PDK (clone_dir) which
            # has actual .c files. make from workarea exits 0 but generates nothing (0 .aer5f files).
            echo "  Building PDK libs from SOURCE PDK (${clone_dir}/pdk/packages/ti/build)..."
            cd ${clone_dir}/pdk/packages/ti/build
            make -s $jobs_option custom_target BUILD_TARGET_LIST_ALL=all_libs BOARD=$board BUILD_PROFILE=$profile BUILD_PROFILE_LIST_ALL=$profile SDK_INSTALL_PATH=$sdk_install_path PDK_INSTALL_PATH=${clone_dir}/pdk/packages TOOLS_INSTALL_PATH=${tools_dir} 1>>${pdk_log} 2>>${build_error_log}
        else
            # ETHFW builds: workarea PDK from FSDK tarball (release) or source package (debug) - build from workarea
            cd $pdk_install_path/packages/ti/build
            make -s $jobs_option custom_target BUILD_TARGET_LIST_ALL=all_libs BOARD=$board BUILD_PROFILE=$profile BUILD_PROFILE_LIST_ALL=$profile SDK_INSTALL_PATH=$sdk_install_path PDK_INSTALL_PATH=$pdk_install_path/packages TOOLS_INSTALL_PATH=${tools_dir} 1>>${pdk_log} 2>>${build_error_log}
        fi
        local libs_exit_code=$?
        print_time_diff $libs_start_time "  PDK Libs Build Time (${board} ${profile})"
        echo "  Building PDK Libs for Board:${board} Profile:${profile} completed!! Exit code: ${libs_exit_code}"

        # DEBUG: Log what libraries were actually built
        echo "  DEBUG: PDK Libraries generated for Board:${board} Profile:${profile}:"
        find ${pdk_install_path}/packages/ti/drv -name "*.aer5f" -type f 2>/dev/null | grep -E "(uart|osal|csl)" | sed 's/^/    /' || echo "    (No matching libraries found)"
        echo "  DEBUG: Checking specifically for UART libraries:"
        find ${pdk_install_path}/packages/ti/drv/uart/lib/${board}/r5f/${profile}/ -name "*.aer5f" 2>/dev/null | sed 's/^/    /' || echo "    (UART library NOT found for Board:${board})"
        echo "  DEBUG: Total .aer5f files in drv folder:"
        find ${pdk_install_path}/packages/ti/drv -name "*.aer5f" 2>/dev/null | wc -l | sed 's/^/    /'

        if [ ${libs_exit_code} -ne 0 ]; then
            echo "ERROR: PDK Libs build failed for Board:${board} Profile:${profile} with exit code ${libs_exit_code}"
            echo "Check error_build.log for details"
            exit 1
        fi

        # Verify sciserver_testapp_freertos was actually built (only for ETHFW builds)
        # Note: make exit code can be 0 even if targets weren't built, so we validate actual artifacts
        # ENET-only builds don't generate sciserver_testapp_freertos in custom_target phase, so skip validation for those
        if [ "${ethfw_build}" == "true" ]; then
            if ! find ${pdk_install_path}/packages -name "sciserver_testapp_freertos*" -type f 2>/dev/null | grep -q .; then
                echo "ERROR: PDK Libs build did not generate sciserver_testapp_freertos for Board:${board} Profile:${profile}"
                echo "Check error_build.log and pdk.log for make target failures"
                exit 1
            fi
        fi

        # For ENET builds: copy ALL built library directories from SOURCE PDK to workarea PDK.
        # Apps are built against workarea PDK (pdk_install_path), so the libs must be present there.
        # Dynamic discovery: finds every top-level packages/ti/ subdir in SOURCE that contains .aer5f
        # files (e.g. ti/drv, ti/csl, ti/osal, ti/board, ...) and copies it to workarea.
        # This avoids hardcoding the list and missing new dirs as all_libs target evolves.
        if [ "${enet_build}" == "true" ] && [ ${libs_exit_code} -eq 0 ]; then
            echo "  Copying built libraries from SOURCE PDK to workarea PDK..."
            for src_subdir in $(find ${clone_dir}/pdk/packages/ti -name "*.aer5f" -type f 2>/dev/null | \
                    sed "s|${clone_dir}/pdk/packages/ti/||" | cut -d'/' -f1 | sort -u); do
                echo "    Copying ti/${src_subdir}/..."
                mkdir -p "${pdk_install_path}/packages/ti/${src_subdir}"
                cp -rf "${clone_dir}/pdk/packages/ti/${src_subdir}/"* \
                    "${pdk_install_path}/packages/ti/${src_subdir}/" 2>/dev/null || true
            done
            echo "  Library copy complete"
            echo "  DEBUG: Total .aer5f files after copy:"
            find ${pdk_install_path}/packages/ti -name "*.aer5f" 2>/dev/null | wc -l | sed 's/^/    /'
        fi

        cd - > /dev/null
      done
    done

    # Setup safeRTOS for ETHFW builds
    # For ENET-only builds, SafeRTOS will be set up per-board in the enet build loop
    if [ "${ethfw_build}" == "true" ]; then
        build_safertos
    fi

    for board in $board_list
    do
      echo "Building Required pdk examples for Board:${board} "

      # Reset SafeRTOS environment for this board before building enet apps
      # This ensures SAFERTOS_KERNEL_INSTALL_PATH points to the correct SafeRTOS for this board
      if [ "${enet_build}" == "true" ]; then
          echo "DEBUG: Setting up SafeRTOS for enet apps on Board:${board}"
          setup_safertos_for_board $board
      fi

      cd ${pdk_install_path}/packages/ti/build

      # Build sciserver_testapp_freertos for ETHFW only (it's ETHFW-specific)
      if [ "${ethfw_build}" == "true" ]; then
          echo "  DEBUG: Attempting to build sciserver_testapp_freertos for Board:${board}"
          echo "  DEBUG: Make command: make -j${num_jobs} -s sciserver_testapp_freertos BOARD=$board CORE=mcu1_0 BUILD_PROFILE=release"
          make -j${num_jobs} -s sciserver_testapp_freertos BOARD=$board CORE=mcu1_0 BUILD_PROFILE=release TOOLS_INSTALL_PATH=${tools_dir} 1>>${pdk_log} 2>>${build_error_log}
          local sciserver_exit_code=$?
          echo "  DEBUG: sciserver_testapp_freertos build exit code: ${sciserver_exit_code}"
          if [ ${sciserver_exit_code} -ne 0 ]; then
              echo "ERROR: PDK sciserver_testapp_freertos build failed for Board:${board} with exit code ${sciserver_exit_code}"
              echo "Last 20 lines of error_build.log:"
              tail -20 ${build_error_log} | sed 's/^/  /'
              echo "Check error_build.log for details"
              exit 1
          fi
      fi

      # Build sbl_uart_img for BOTH ETHFW and ENET builds (needed for device booting in tests)
      # For ETHFW: fatal if it fails
      # For ENET: non-fatal if it fails (some boards like J7200 may not support it)
      echo "  DEBUG: Attempting to build sbl_uart_img for Board:${board}"
      echo "  DEBUG: Make command: make -j${num_jobs} -s sbl_uart_img BOARD=$board CORE=mcu1_0 BUILD_PROFILE=release"
      make -j${num_jobs} -s sbl_uart_img BOARD=$board CORE=mcu1_0 BUILD_PROFILE=release TOOLS_INSTALL_PATH=${tools_dir} 1>>${pdk_log} 2>>${build_error_log}
      local sbl_exit_code=$?
      echo "  DEBUG: sbl_uart_img build exit code: ${sbl_exit_code}"
      if [ ${sbl_exit_code} -ne 0 ]; then
          if [ "${ethfw_build}" == "true" ]; then
              echo "ERROR: PDK sbl_uart_img build failed for Board:${board} with exit code ${sbl_exit_code}"
              echo "Last 20 lines of error_build.log:"
              tail -20 ${build_error_log} | sed 's/^/  /'
              echo "Check error_build.log for details"
              exit 1
          else
              echo "WARNING: PDK sbl_uart_img build failed for Board:${board} with exit code ${sbl_exit_code} (non-fatal for ENET builds)"
              echo "Last 10 lines of error_build.log:"
              tail -10 ${build_error_log} | sed 's/^/  /'
              echo "Note: sbl_uart_img is needed for device booting - test execution may fail if this file is not available"
          fi
      fi

      if [ "${enet_build}" == "true" ]; then
          # Build Enet-lld Apps for freeRTOS
          echo "DEBUG: About to build enet_app_freertos for ${board}"
          enet_app_freertos_build $board
          echo "DEBUG: enet_app_freertos_build completed for ${board}"
          echo "DEBUG: Checking for ENET binaries in pdk_install_path:"
          find ${pdk_install_path}/packages/ti/binary -type f \( -name "*.out" -o -name "*.xer5f" -o -name "*.appimage*" \) 2>/dev/null | head -10 | sed 's/^/    /'

          # Build Enet-lld Apps for SafeRTOS
          echo "DEBUG: About to build enet_app_safertos for ${board}"
          enet_app_safertos_build $board
          echo "DEBUG: enet_app_safetos_build completed for ${board}"
      fi
      cd - > /dev/null

      echo "DEBUG: After enet build, checking pdk_install_path binaries:"
      find ${pdk_install_path} -type f \( -name "*.out" -o -name "*.xer5f" \) 2>/dev/null | wc -l | sed 's/^/  Total binaries: /'
    done
    
    print_time_diff $start_time "  pdk examples for Board..."
    echo "  pdk examples for Board..."
    
    if [ "${release_build}" == "false" ]; then
      # PDK docs build
      pdk_doxygen_build
      pdk_sphinx_build
    fi

    print_time_diff $start_time "  Build PDK Time"
    echo "  Build PDK ... Done"
}

if [ "${release_build}" == "false" ]; then
  package_pdk

  # Copy SafeRTOS source files from cloned PDK to packaged PDK for enet builds
  # The packaged PDK excludes SafeRTOS sources, so we need to copy them manually
  if [ "${enet_build}" == "true" ]; then
    echo "=========================================="
    echo "Copying SafeRTOS sources to packaged PDK"
    echo "=========================================="
    if [ -d "${pdk_clone_path}/packages/ti/osal/src/safertos" ]; then
      mkdir -p ${pdk_install_path}/packages/ti/osal/src
      cp -rf ${pdk_clone_path}/packages/ti/osal/src/safertos ${pdk_install_path}/packages/ti/osal/src/
      echo "  Copied safertos directory"
    fi
    if [ -f "${pdk_clone_path}/packages/ti/osal/src/src_common_safertos.mk" ]; then
      cp -f ${pdk_clone_path}/packages/ti/osal/src/src_common_safertos.mk ${pdk_install_path}/packages/ti/osal/src/
      echo "  Copied src_common_safertos.mk"
    fi
    if [ -f "${pdk_clone_path}/packages/ti/build/makefile_safertos.mk" ]; then
      cp -f ${pdk_clone_path}/packages/ti/build/makefile_safertos.mk ${pdk_install_path}/packages/ti/build/
      echo "  Copied makefile_safertos.mk"
    fi
    echo "SafeRTOS source files copied successfully"
  fi
fi
if [ "${docs_build}" == "false" ] && [ "${quick_ethfw_build}" == "false" ]; then
  # Copy SDL source files for all builds (enet_ecc_test needs them)
  # But only build SDL libraries for ethfw builds
  if [ "${enet_build}" == "true" ]; then
    echo "Copying SDL source files for enet build..."
    cp -rf ${clone_dir}/sdl ${sdk_install_path}
    echo "SDL source files copied (libraries will not be built for enet builds)"
  else
    build_sdl
  fi
  pdk_build
fi


###################### ETHFW build ######################
ethfw_build () {
  local start_time=`date +%s`

  cd $ethfw_comp_install_path

  for board in ${board_list}
  do
    local soc=`echo ${board} | cut -d "_" -f 1`
    local soc_caps=${soc^^}

    for profile in $profile_list
    do
      if [ "$kw_build" == "true" ]; then
          echo "Building Ethfw with Klockwork for Profile:${profile} ..."
          kw_out_list=$kw_out_list$log_dir/"kw_ethfw_"$board"_"$profile".out "

          # Build EthFw libs and apps only for KW, don't build UT test app in KW.
          $kwinject_cmd -o $kw_ethfw_out make -s $jobs_option remoteswitchcfg_all BUILD_SOC_LIST=${soc_caps} PROFILE=$profile PSDK_TOOLS_PATH=${tools_dir} PDK_PATH=${pdk_install_path} 1>>${ethfw_log} 2>>${build_error_log}
          local kw_exit_code=$?
          if [ ${kw_exit_code} -ne 0 ]; then
              echo "ERROR: Klockwork EthFw build failed for Board:${board} Profile:${profile} with exit code ${kw_exit_code}"
              echo "Check error_build.log for details"
              exit 1
          fi
          echo "Building ETHFW for Profile:${profile} completed!!"
        else
          echo "    Building EthFw SOC:${soc_caps} Profile:${profile} ..."
          make $jobs_option -s ethfw_all BUILD_SOC_LIST=${soc_caps} PROFILE=${profile} PSDK_TOOLS_PATH=${tools_dir} 1>>${ethfw_log} 2>>${build_error_log}
          local ethfw_exit_code=$?
          if [ ${ethfw_exit_code} -ne 0 ]; then
              echo "ERROR: EthFw build failed for Board:${board} Profile:${profile} with exit code ${ethfw_exit_code}"
              echo "Check error_build.log for details"
              exit 1
          fi

          if [ "${board}" != "j742s2_evm" ]; then
            make -j -s ethfw_all BUILD_SOC_LIST=${soc_caps} PROFILE=$profile  BUILD_APP_FREERTOS?=no BUILD_APP_SAFERTOS?=yes PSDK_TOOLS_PATH=${tools_dir} 1>>${ethfw_log} 2>>${build_error_log}
            local safertos_exit_code=$?
            if [ ${safertos_exit_code} -ne 0 ]; then
                echo "ERROR: EthFw SafeRTOS build failed for Board:${board} Profile:${profile} with exit code ${safertos_exit_code}"
                echo "Check error_build.log for details"
                exit 1
            fi
          fi
          echo "    Building EthFw SOC:${soc_caps} Profile:${profile} Done"
        fi
    done

    mkdir ${build_targets_dir}/$soc

    # Verify that expected binary files were generated
    if [ ! -f out/$soc_caps/R5Ft/FREERTOS/release/app_remoteswitchcfg_server_strip.xer5f ]; then
        echo "ERROR: ETHFW FreeRTOS binary not found: out/$soc_caps/R5Ft/FREERTOS/release/app_remoteswitchcfg_server_strip.xer5f"
        echo "Build failed - compilation errors prevented binary generation"
        exit 1
    fi

    # Verify SafeRTOS binary for boards that support it
    if [ "${board}" != "j742s2_evm" ]; then
        if [ ! -f out/$soc_caps/R5Ft/SAFERTOS/release/app_remoteswitchcfg_server_strip.xer5f ]; then
            echo "ERROR: ETHFW SafeRTOS binary not found: out/$soc_caps/R5Ft/SAFERTOS/release/app_remoteswitchcfg_server_strip.xer5f"
            echo "Build failed - SafeRTOS compilation errors prevented binary generation"
            exit 1
        fi
    fi

    cp -f out/$soc_caps/R5Ft/FREERTOS/release/app_remoteswitchcfg_server_strip.xer5f ${build_targets_dir}/$soc
  done

  cd - > /dev/null
  print_time_diff $start_time "  Build EthFw Time"
  echo "  Build EthFw ... Done"
}


ethfw_doxygen_build() {
  echo "Starting Doxygen Build..."
  local start_time=`date +%s`
  cd $ethfw_comp_install_path

  for board in $board_list
  do
    echo "  EthFw User and API Guide generation for Board:${board} ..."
    make -C internal_docs/doxygen -s all DOXYGEN=doxygen 1>>${ethfw_log} 2>>${build_error_log}
    echo "    EthFw User and API Guides completed!!"
    echo "    EthFw Datasheet generation..."
    make -C internal_docs/doxygen -s datasheet DOXYGEN=doxygen 1>>${ethfw_log} 2>>${build_error_log}
    cp -R internal_docs/datasheet/ docs/.
    echo "    EthFw Datasheet completed!!"
  done

  rm -rf $ethfw_comp_install_path/internal_docs

  cd - > /dev/null
  print_time_diff $start_time "Doxygen Time"
  echo "Doxygen Build completed!!"
  echo ""
}


recur_copy() {
    local pattern=$1
    local dest=$2
    local count=$(find ./ -iname "$pattern" 2>/dev/null | wc -l)
    if [ $count -gt 0 ]; then
        echo "        DEBUG recur_copy: Found $count files matching pattern '$pattern'"
        find ./ -iname "$pattern"  | \
        while read filepath; do
            cp --parent --target-directory=$dest "$filepath" 2>&1
        done
    fi
}



copy_binary() {
    echo "    DEBUG: copy_binary called with: from=$1, to=$2"
    echo "    DEBUG: release_folder_dir = ${release_folder_dir}"
    echo "    DEBUG: Source path = ${release_folder_dir}/$1"

    if [ -d ${release_folder_dir}/$1 ]; then
        if [ ${1} != "mcusw" ]; then
            echo "  Copying $1 ..."
        fi
        echo "    DEBUG: Source directory exists, listing content..."
        find ${release_folder_dir}/$1 -type f \( -name "*.xer5f" -o -name "*.out" \) 2>/dev/null | head -5 | sed 's/^/      Found: /'

        mkdir -p $2
        cd ${release_folder_dir}/$1
        echo "    DEBUG: Changed to $(pwd)"

        #copy binary files
        recur_copy '*.xer5f'    "../../$2"
        recur_copy '*.xem4'     "../../$2"
        recur_copy '*.xem4f'    "../../$2"
        recur_copy '*.xe66'     "../../$2"
        recur_copy '*.xa53fg'   "../../$2"
        recur_copy '*.xa72fg'   "../../$2"
        recur_copy '*.xe71'     "../../$2"
        recur_copy '*.lib'      "../../$2"
        recur_copy '*.out'      "../../$2"
        #Map files
        recur_copy '*.map'      "../../$2"
        #copy sbl images
        recur_copy '*.bin'      "../../$2"
        recur_copy '*.tiimage'  "../../$2"
        #SBL App Images
        recur_copy '*.appimage'         "../../$2"
        recur_copy '*.appimage.hs_fs'   "../../$2"
        recur_copy '*.appimage.signed'  "../../$2"
        recur_copy '*.img'              "../../$2"
        recur_copy '*-image-evm.tar.gz' "../../$2"
        #XIP App Images
        recur_copy '*.appimage_xip'     "../../$2"
        #ROV files
        recur_copy '*.rov.xs'   "../../$2"
        cd - 1>/dev/null
        echo "    DEBUG: After copy, destination has:"
        find $2 -type f \( -name "*.xer5f" -o -name "*.out" \) 2>/dev/null | head -5 | sed 's/^/      Found: /'
    else
        echo "    DEBUG: Source directory does NOT exist: ${release_folder_dir}/$1"
        echo "    DEBUG: Available directories in ${release_folder_dir}:"
        ls -la ${release_folder_dir} 2>/dev/null | sed 's/^/      /'
    fi
}


copy_docs() {
    if [ -d ${release_folder_dir}/$1 ]; then
        echo "  Copying $1 ..."
        mkdir -p ${release_folder_docs_only}/$1
        cp ${release_folder_dir}/$1/*.html                  ${release_folder_docs_only}/$1      2>/dev/null
        cp ${release_folder_dir}/$1/*.htm                   ${release_folder_docs_only}/$1      2>/dev/null
        cp ${release_folder_dir}/$1/*.gif                   ${release_folder_docs_only}/$1      2>/dev/null
        cp ${release_folder_dir}/$1/*.txt                   ${release_folder_docs_only}/$1      2>/dev/null
        cp ${release_folder_dir}/$1/docs/                   ${release_folder_docs_only}/$1 -r   2>/dev/null
        cp ${release_folder_dir}/$1/doc/                    ${release_folder_docs_only}/$1 -r   2>/dev/null
        
        if [ $2 == "pdk" ]; then
          mkdir -p ${release_folder_docs_only}/$1/packages/ti/drv/udma/docs/
          mkdir -p ${release_folder_docs_only}/$1/packages/ti/drv/csirx/docs/
          mkdir -p ${release_folder_docs_only}/$1/packages/ti/drv/enet/docs/
          mkdir -p ${release_folder_docs_only}/$1/packages/ti/drv/sciclient/soc/sysfw/binaries
          cp ${release_folder}/$1/packages/ti/drv/udma/docs/*     ${release_folder_docs_only}/$1/packages/ti/drv/udma/docs/   2>/dev/null
          cp ${release_folder}/$1/packages/ti/drv/csirx/docs/*    ${release_folder_docs_only}/$1/packages/ti/drv/csirx/docs/  2>/dev/null
          cp -r ${release_folder}/$1/packages/ti/drv/enet/docs/*  ${release_folder_docs_only}/$1/packages/ti/drv/enet/docs/   2>/dev/null
          cp -r ${release_folder}/$1/packages/ti/drv/sciclient/soc/sysfw/binaries/system-firmware-public-documentation  ${release_folder_docs_only}/$1/packages/ti/drv/sciclient/soc/sysfw/binaries   2>/dev/null
        fi
        echo "  Copying $1 ... Done !!!"
    fi
}


make_docs_only_package() {
    echo "Make Docs Only Package ..."
    local start_time=`date +%s`

    if [ -d ${release_folder_docs_only} ]; then

        if [ -d ${release_folder}/ethfw* ]; then
            copy_docs ${ethfw_folder} "ethfw"
        fi

        if [ -d ${release_folder}/pdk* ]; then
            copy_docs ${pdk_folder} "pdk"
        fi

        cd ${release_folder_docs_only}
        tar czf ${release_folder_docs_only}.tar.gz ./* 1>>${build_log} 2>>${build_error_log}
        mv ${release_folder_docs_only}.tar.gz ../ 1>>${build_log} 2>>${build_error_log}
        cd - 1>/dev/null

        create_build_target_file ${release_folder_docs_only} ".tar.gz" ${build_error_log}
    fi

    print_time_diff $start_time "Make Docs Only Package Time"
    echo "Make Docs Only Package ... Done"
}

if [ "${docs_build}" == "true" ]; then
  make_docs_only_package
fi




make_tarball_package() {
    echo "Make TarBall Package ..."
    echo "DEBUG make_tarball_package: workarea_dir = ${workarea_dir}"
    echo "DEBUG make_tarball_package: release_folder_dir = ${release_folder_dir}"
    echo "DEBUG make_tarball_package: pdk_folder = ${pdk_folder}"
    echo "DEBUG make_tarball_package: ethfw_build = ${ethfw_build}"
    echo "DEBUG make_tarball_package: enet_build = ${enet_build}"

    echo "DEBUG: Contents of ${workarea_dir}:"
    ls -lah ${workarea_dir} 2>&1 | head -30 | sed 's/^/  /'

    echo "DEBUG: Checking for PDK in workarea:"
    find ${workarea_dir} -maxdepth 1 -type d -name "pdk*" 2>/dev/null | sed 's/^/  Found: /'

    echo "DEBUG: Checking for binaries in workarea PDK:"
    find ${workarea_dir}/pdk* -type f \( -name "*.out" -o -name "*.xer5f" \) 2>/dev/null | wc -l | sed 's/^/  Binary count in source: /'

    local start_time=`date +%s`
    cd $workarea_dir

    if [ "${ethfw_build}" == "true" ]; then
      echo "DEBUG: Copying ETHFW folder..."
      if [ -d "$workarea_dir/ethfw" ]; then
          echo "  Source exists: $workarea_dir/ethfw"
          cp -rf $workarea_dir/ethfw $release_folder_dir/$ethfw_folder
          echo "  Copied to: $release_folder_dir/$ethfw_folder"
      else
          echo "  WARNING: Source does not exist: $workarea_dir/ethfw"
      fi

      echo "DEBUG: Copying PDK folder for ETHFW..."
      if [ -d "$workarea_dir/$pdk_folder" ]; then
          echo "  Source exists: $workarea_dir/$pdk_folder"
          echo "  Listing source PDK binary count:"
          find "$workarea_dir/$pdk_folder" -type f \( -name "*.out" -o -name "*.xer5f" \) 2>/dev/null | wc -l | sed 's/^/    /'
          cp -rf $workarea_dir/$pdk_folder $release_folder_dir/$pdk_folder
          echo "  Copied to: $release_folder_dir/$pdk_folder"
      else
          echo "  ERROR: Source does not exist: $workarea_dir/$pdk_folder"
          ls -la "$workarea_dir" | grep pdk | sed 's/^/    /'
      fi
    fi

    if [ "${enet_build}" == "true" ]; then
      echo "DEBUG: Copying PDK folder for ENET..."
      if [ -d "$workarea_dir/$pdk_folder" ]; then
          echo "  Source exists: $workarea_dir/$pdk_folder"
          echo "  Listing source PDK binary count:"
          find "$workarea_dir/$pdk_folder" -type f \( -name "*.out" -o -name "*.xer5f" \) 2>/dev/null | wc -l | sed 's/^/    /'
          cp -rf $workarea_dir/$pdk_folder $release_folder_dir/$pdk_folder
          echo "  Copied to: $release_folder_dir/$pdk_folder"
      else
          echo "  ERROR: Source does not exist: $workarea_dir/$pdk_folder"
          ls -la "$workarea_dir" | grep pdk | sed 's/^/    /'
      fi
    fi

    cd - > /dev/null

    echo "DEBUG: After copying, release_folder_dir contents:"
    ls -lah ${release_folder_dir} 2>&1 | head -20 | sed 's/^/  /'

    echo "DEBUG: Checking binaries in release_folder_dir:"
    find ${release_folder_dir} -type f \( -name "*.out" -o -name "*.xer5f" \) 2>/dev/null | wc -l | sed 's/^/  Binary count in destination: /'
    if [ "${release_build}" == "true" ]; then
        if [ -d ${release_folder_dir}/ethfw* ]; then
            cd ${release_folder_dir}
            zip -rq ethfw_${release_version_short}.zip ethfw_${release_version_short} 1>>${ethfw_log} 2>>${build_error_log}
            mv ethfw_${release_version_short}.zip ../
            cd - 1>/dev/null
            create_build_target_file ethfw_${release_version_short} ".zip" ${build_error_log}
        fi
        cd $release_folder_dir
        tar czf ${release_folder_dir}.tar.gz ./* 1>>$log_dir/build.log  2>>$log_dir/error.log
        cd - > /dev/null
    fi
    print_time_diff $start_time "Make TarBall Package Time"
    echo "Make TarBall Package ... Done"
}



make_binary_package() {
    echo "Make Binary Only Package ..."
    echo "DEBUG: release_folder_dir = ${release_folder_dir}"
    echo "DEBUG: release_folder_binary_only = ${release_folder_binary_only}"
    echo "DEBUG: pdk_folder = ${pdk_folder}"
    echo "DEBUG: Checking if ${release_folder_dir} exists..."
    ls -la ${release_folder_dir} 2>&1 | head -20 | sed 's/^/  /'

    local start_time=`date +%s`
    cd ${work_dir}

    if [ -d ${release_folder_binary_only} ]; then
      if [ "${quick_ethfw_build}" == "false" ]; then
          if [ "${ethfw_build}" == "true" ]; then
            echo "DEBUG: Copying ETHFW binaries..."
            copy_binary ${ethfw_folder}   ${release_folder_binary_only}/$ethfw_folder
            copy_binary ${pdk_folder}     ${release_folder_binary_only}/$pdk_folder
          fi
      fi

      if [ "${enet_build}" == "true" ]; then
        echo "DEBUG: Copying ENET (PDK) binaries..."
        echo "DEBUG: Source directory = ${release_folder_dir}/${pdk_folder}"
        echo "DEBUG: Dest directory = ${release_folder_binary_only}/${pdk_folder}"
        echo "DEBUG: Checking source content..."
        find ${release_folder_dir}/${pdk_folder} -type f -name "*.out" -o -name "*.xer5f" 2>/dev/null | head -10 | sed 's/^/    Found: /'
        copy_binary ${pdk_folder}     ${release_folder_binary_only}/$pdk_folder
        echo "DEBUG: After copy, checking destination..."
        find ${release_folder_binary_only}/${pdk_folder} -type f -name "*.out" -o -name "*.xer5f" 2>/dev/null | head -10 | sed 's/^/    Found: /'
      fi

      cd ${release_folder_binary_only}
      tar czf ${release_folder_binary_only}.tar.gz ./*
      mv ${release_folder_binary_only}.tar.gz ../
      cd - 1>/dev/null

      create_build_target_file ${release_folder_binary_only} ".tar.gz" ${build_error_log}
    else
      echo "ERROR: release_folder_binary_only does not exist: ${release_folder_binary_only}"
    fi

    print_time_diff $start_time "Make Binary Only Package Time"
    echo "Make Binary Only Package ... Done"
}




make_artifact_tarball_package() {
	echo "Moving the tae.gz, .zip, xlsx files"
    mv *.tar.gz   ${build_targets_dir} 1>/dev/null 2>&1
    mv *.zip      ${build_targets_dir} 1>/dev/null 2>&1
    mv *.xlsx     ${build_targets_dir} 1>/dev/null 2>&1

if [ "${docs_build}" == "true" ]; then
    cd ${build_targets_dir}
    #untar docs only package
    untar_gz ${release_folder_docs_only}.tar.gz
    rm -rf ${release_folder_docs_only}.tar.gz
    cd - 1>/dev/null
fi
    echo "Making artifacts Tarball..."
    tar -czf ./artifacts.tar.gz ./artifacts

    echo "Making artifacts Tarball... Done"
}





if [ "${ethfw_build}" == "true" ]; then
	echo "now package_ethfw_comp will call...!!!"
  package_ethfw_comp
  if [ "${docs_build}" == "true" ]; then
    ethfw_doxygen_build
  else
    ethfw_build
    if [ "$kw_build" == "true" ]; then
      ethfw_kw_build
    else
	  echo "now ethfw_doxygen_build will call...!!!"
      ethfw_doxygen_build
    fi
  fi
fi

echo "DEBUG: quick_ethfw_build = ${quick_ethfw_build}, enet_build = ${enet_build}, ethfw_build = ${ethfw_build}"
if [ "${quick_ethfw_build}" == "false" ] || [ "${enet_build}" == "true" ]; then
    echo "DEBUG: Calling make_tarball_package (reason: quick_ethfw_build=${quick_ethfw_build} or enet_build=${enet_build})"
    make_tarball_package
else
    echo "DEBUG: Skipping make_tarball_package"
fi


# Define binary validation function
validate_binaries_generated() {
    echo "Validating that binaries were generated..."
    echo "DEBUG: release_folder_binary_only = ${release_folder_binary_only}"
    echo "DEBUG: enet_build = ${enet_build}"
    echo "DEBUG: ethfw_build = ${ethfw_build}"
    echo "DEBUG: pdk_folder = ${pdk_folder}"
    echo "DEBUG: ethfw_folder = ${ethfw_folder}"

    local binary_count=0
    local binary_files=""

    if [ "${enet_build}" == "true" ]; then
        # Check for ENET binaries in release_folder_binary_only
        echo "DEBUG: Checking ENET binaries..."
        echo "DEBUG: Looking in ${release_folder_binary_only}/${pdk_folder}"
        echo "DEBUG: Directory exists? $([ -d "${release_folder_binary_only}/${pdk_folder}" ] && echo yes || echo no)"

        if [ -d "${release_folder_binary_only}/${pdk_folder}" ]; then
            echo "DEBUG: Listing content of ${release_folder_binary_only}/${pdk_folder}:"
            find "${release_folder_binary_only}/${pdk_folder}" -type f \( -name "*.out" -o -name "*.appimage*" -o -name "*.tiimage" -o -name "*.bin" \) 2>/dev/null | head -20 | sed 's/^/    /'

            binary_count=$(find "${release_folder_binary_only}/${pdk_folder}" -type f \( -name "*.out" -o -name "*.appimage*" -o -name "*.tiimage" -o -name "*.bin" \) 2>/dev/null | wc -l)
            echo "DEBUG: Found $binary_count binary files"

            if [ $binary_count -gt 0 ]; then
                echo "SUCCESS: Found $binary_count binary files for ENET build"
                return 0
            fi
        else
            echo "DEBUG: ENET folder does not exist"
        fi

        echo "ERROR: No ENET binaries found in ${release_folder_binary_only}/${pdk_folder}"
        echo "Build completed but no binaries were generated. This indicates a build failure."
        echo "DEBUG: Dumping entire release_folder_binary_only content:"
        find "${release_folder_binary_only}" -type f 2>/dev/null | head -30 | sed 's/^/    /'
        exit 1
    fi

    if [ "${ethfw_build}" == "true" ]; then
        # Check for ETHFW binaries (can be .xer5f, .out, .appimage, .tiimage, .bin, or other executable formats)
        echo "DEBUG: Checking ETHFW binaries..."
        if [ -d "${release_folder_binary_only}/${ethfw_folder}" ]; then
            binary_count=$(find "${release_folder_binary_only}/${ethfw_folder}" -type f \( -name "*.xer5f" -o -name "*.out" -o -name "*.appimage*" -o -name "*.tiimage" -o -name "*.bin" \) 2>/dev/null | wc -l)
            if [ $binary_count -gt 0 ]; then
                echo "SUCCESS: Found $binary_count binary files for ETHFW build"
                return 0
            fi
        fi
        echo "ERROR: No ETHFW binaries found in ${release_folder_binary_only}/${ethfw_folder}"
        echo "DEBUG: Listing ALL files in ${release_folder_binary_only}/${ethfw_folder}:"
        find "${release_folder_binary_only}/${ethfw_folder}" -type f 2>/dev/null | head -30 | sed 's/^/  /'
        echo "Build completed but no binaries were generated. This indicates a build failure."
        exit 1
    fi
}

# Package binaries
echo "========================================"
echo "DEBUG: Starting binary packaging phase..."
echo "========================================"
echo "DEBUG: workarea_dir = ${workarea_dir}"
echo "DEBUG: release_folder_dir = ${release_folder_dir}"
echo "DEBUG: release_folder_binary_only = ${release_folder_binary_only}"
echo "DEBUG: enet_build = ${enet_build}"
echo "DEBUG: ethfw_build = ${ethfw_build}"
echo "DEBUG: docs_build = ${docs_build}"

echo "DEBUG STAGE 1: Contents of workarea_dir before packaging:"
find ${workarea_dir} -maxdepth 1 -type d -name "pdk*" -o -name "ethfw*" 2>/dev/null | while read dir; do
    count=$(find "$dir" -type f \( -name "*.out" -o -name "*.xer5f" \) 2>/dev/null | wc -l)
    echo "  $dir: $count binaries"
done

echo "DEBUG STAGE 2: Contents of release_folder_dir before packaging:"
if [ -d "${release_folder_dir}" ]; then
    ls -lah ${release_folder_dir} 2>&1 | head -20 | sed 's/^/  /'
else
    echo "  Directory does not exist yet"
fi

if [ "${docs_build}" == "false" ]; then
    echo "DEBUG: Calling make_binary_package"
    make_binary_package
else
    echo "DEBUG: Skipping make_binary_package (docs_build is true)"
fi

echo "DEBUG STAGE 3: Contents of release_folder_binary_only after binary package:"
find ${release_folder_binary_only} -type f \( -name "*.out" -o -name "*.xer5f" -o -name "*.appimage*" \) 2>/dev/null | head -20 | while read f; do
    echo "  FOUND BINARY: $f"
done

# Validate that binaries were actually generated
echo "DEBUG STAGE 4: About to call validate_binaries_generated"
validate_binaries_generated

echo "DEBUG STAGE 5: About to call make_artifact_tarball_package"
make_artifact_tarball_package

############################### copying artifacts to nas server #####################################
copying_artifacts_nas() {
  local NAS_HTTP_LINK="http://epswnas.itg.ti.com/"
  local NAS_DIR=""
  # Derived variables
  if [ "$product_family" == "jacinto" ]; then
    if [ "$enet_build" == "true" ]; then    
       board_list="j721e_evm j7200_evm j721s2_evm j784s4_evm j742s2_evm"
    else
       board_list="j721e_evm j7200_evm j784s4_evm"
    fi
  fi
  if [ "$product_family" == "j721e" ]; then
     board_list="j721e_evm"
  fi
  if [ "$product_family" == "j7200" ]; then
     board_list="j7200_evm"
  fi
  if [ "$product_family" == "j784s4" ]; then
     board_list="j784s4_evm"
  fi
  if [ "$product_family" == "j742s2" ]; then
     board_list="j742s2_evm"
  fi
  if [ "$product_family" == "j721s2" ]; then
     board_list="j721s2_evm"
  fi
  if [ "$enet_build" == "true" ]; then
    job_type="cpsw"
    if [ "$trigger_tests" == "true" ]; then
	  NAS_DIR="/data/eth_rtos_builds/enet/ethrtos_enet_lld_rtos_daily_${build_number}"
	  echo "${NAS_HTTP_LINK}/eth_rtos_builds/enet/ethrtos_enet_lld_rtos_daily_${build_number}"
	  RTOS_BINS_BASE="http://epswnas.itg.ti.com/eth_rtos_builds/enet/ethrtos_enet_lld_rtos_daily_${build_number}/artifacts/output/${release_folder}-binary_only.tar.gz"
    else
	  NAS_DIR="/data/eth_rtos_cicd/enet/ethrtos_enet_lld_rtos_daily_${build_number}"
	  echo "${NAS_HTTP_LINK}/eth_rtos_cicd/enet/ethrtos_enet_lld_rtos_daily_${build_number}"
	  RTOS_BINS_BASE="http://epswnas.itg.ti.com/eth_rtos_cicd/enet/ethrtos_enet_lld_rtos_daily_${build_number}/artifacts/output/${release_folder}-binary_only.tar.gz"
    fi
  else
    job_type="ethfw"
    if [ "$release_build" == "true" ]; then
      NAS_DIR="/data/eth_rtos_cicd/ethfw/ethrtos_ethfw_rtos_${build_number}"
      echo "${NAS_HTTP_LINK}/eth_rtos_cicd/ethfw/ethrtos_ethfw_rtos_${build_number}"
      RTOS_BINS_BASE="http://epswnas.itg.ti.com/eth_rtos_cicd/ethfw/ethrtos_ethfw_rtos_${build_number}/artifacts/output/${release_folder}-binary_only.tar.gz"
    else
      NAS_DIR="/data/eth_rtos_builds/ethfw/ethrtos_ethfw_rtos_${build_number}"
      echo "${NAS_HTTP_LINK}/eth_rtos_builds/ethfw/ethrtos_ethfw_rtos_${build_number}"
      RTOS_BINS_BASE="http://epswnas.itg.ti.com/eth_rtos_builds/ethfw/ethrtos_ethfw_rtos_${build_number}/artifacts/output/${release_folder}-binary_only.tar.gz"
    fi
  fi
  
  if [ "${release_build}" == "false" ]; then 
  # Only trigger tests if the builds have not been triggered by promotion job.
  # In case of promotion job, it also handles the test triggers.

    for board in $board_list; do
      soc=$(echo $board | cut -d "_" -f1)
      JOB=""
      JIRA=""
      
      if [ "$job_type" == "cpsw" ]; then
	      if [[ "$trigger_tests" == "true" ]]; then
	        case $soc in
	          j7200) JOB="cpsw-high-j7200-full-test"; JIRA="ETHFW-2768" ;;
	          j721e) JOB="cpsw-high-j721e-full-test"; JIRA="ETHFW-2793" ;;
	          j784s4) JOB="cpsw-high-j784s4-full-test"; JIRA="ETHFW-2803" ;;
	          j742s2) JOB="cpsw-high-j742s2-hsfs-full-test"; JIRA="ETHFW-2338" ;;
	          j721s2) JOB="cpsw-high-j721s2-full-test"; JIRA="ETHFW-2768" ;;
	        esac
	      fi
      else
        if [[ "$trigger_tests" == "true" ]]; then
          case $soc in
            j7200) JOB="ethfw-j7200-pg2.0-full-test"; JIRA="ETHFW-2915" ;;
            j721e) JOB="view/CPSW/job/ethfw-j721e-pg1.1-full-test"; JIRA="ETHFW-2916" ;;
            j784s4) JOB="ethfw-j784s4-pg2.0-full-test"; JIRA="ETHFW-2917" ;;
          esac
        fi
      fi

      if [ -n "$JOB" ]; then
        curl -k "https://jenkins-proc.itg.ti.com/job/${JOB}/buildWithParameters?token=abcd&RELEASE_VERSION=${release_version}&INSTALLER_BUILD_ID=${build_number}-${build_timestamp}&RTOS_BINS=${RTOS_BINS_BASE}&REPO_REVISION=NA&TEST_LABEL=NIGHTLY&BUILD_TAG=None&RELEASE_TAG=NA&TEST_TYPE=FULL"
      fi
    done
  fi
  # Ensure directory exists
  ssh -o "StrictHostKeyChecking=no" epswbld@epswnas.itg.ti.com "mkdir -p ${NAS_DIR}"

  # Copy (directory-safe)
  rsync --rsync-path="/usr/bin/rsync" -e "ssh -o StrictHostKeyChecking=no" -Wav --inplace "/workdir/artifacts" epswbld@epswnas.itg.ti.com:"${NAS_DIR}"

  # Verify the binary tarball actually landed on the NAS (skip for quick builds which don't upload binaries)
  if [ "${quick_ethfw_build}" != "true" ] && [ "${docs_build}" != "true" ]; then
    echo "Verifying binary upload to NAS: ${NAS_DIR}/artifacts/output/"
    NAS_BINARY_COUNT=$(ssh -o "StrictHostKeyChecking=no" epswbld@epswnas.itg.ti.com \
        "ls ${NAS_DIR}/artifacts/output/*-binary_only.tar.gz 2>/dev/null | wc -l")
    if [ "${NAS_BINARY_COUNT}" -eq 0 ] 2>/dev/null; then
        echo "ERROR: Binary tarball not found on NAS at ${NAS_DIR}/artifacts/output/"
        echo "ERROR: rsync may have failed or binary packaging did not produce output"
        exit 1
    fi
    echo "NAS binary upload verified: ${NAS_BINARY_COUNT} tarball(s) present"
  fi
}
copying_artifacts_nas



############################### triggering to test jobs #####################################

echo "list_of_workdir"
ls -l /workdir

echo "list_of_clone"
ls -la /workdir/clone

echo "list_of_clone/.repo"
ls -la /workdir/clone/.repo

echo "list_of_clone_ethfw"
ls -la /workdir/clone/ethfw

echo "list of artifacts"
ls -l /workdir/artifacts

echo "list of artifacts/output"
ls -l /workdir/artifacts/output

echo "list of workarea"
ls -l /workdir/workarea

echo "list of workarea"
ls -l /workdir/workarea

echo "list of artifacts/logs"
ls -l /workdir/artifacts/logs


############################### NAS cleanup: remove artifacts older than 30 days #####################################

echo "========================================"
echo "NAS CLEANUP: removing build artifacts older than 30 days"
echo "========================================"
ssh -o "StrictHostKeyChecking=no" epswbld@epswnas.itg.ti.com \
    "find /data/eth_rtos_cicd /data/eth_rtos_builds -mindepth 2 -maxdepth 2 -type d -not -path '*/.snapshot/*' -mtime +30 -exec rm -rf {} + && echo 'NAS cleanup complete'" \
    || echo "WARNING: NAS cleanup failed (non-fatal, build result unaffected)"

print_time_diff $total_build_start_time "Total Build Time"
exit
