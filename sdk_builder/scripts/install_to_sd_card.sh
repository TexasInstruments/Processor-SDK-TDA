THIS_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
source ${THIS_SCRIPT_DIR}/board_env.sh

bootfs_file=$PWD/boot-${TISDK_IMAGE}-${TI_DEV_BOARD}.tar.gz
rootfs_file=$PWD/tisdk-${TISDK_IMAGE}-image-${TI_DEV_BOARD}.tar.xz

if [ ${SOC} = "am62a" ]
then
    rootfs_file=$PWD/tisdk-${TISDK_IMAGE}-image-${TI_DEV_BOARD}.rootfs.tar.xz
fi

bootfs_folder=/media/$USER/boot
rootfs_folder=/media/$USER/root

untar_bootfs()
{
    if [ -d $bootfs_folder ]
    then
        echo "Installing $bootfs_file to $bootfs_folder ..."
        if [ -f $bootfs_file ]
        then
            cd $bootfs_folder
            tar -xf $bootfs_file
            cd -
            sync
            echo "Installing $bootfs_file to $bootfs_folder ... Done"
        else
            echo "ERROR: $bootfs_file not found !!!"
        fi
    else
        echo "ERROR: $bootfs_folder not found !!!"
    fi
}

untar_rootfs()
{
    if [ -d $rootfs_folder ]
    then
        echo "Installing $rootfs_file to $rootfs_folder ..."
        if [ -f $rootfs_file ]
        then
            cd $rootfs_folder
            sudo chmod 777 .
            sudo tar --same-owner -xf ${rootfs_file} .
            cd -
            if [ "${TI_DEV_BOARD}" = "j721e-evm" ] || [ "${TI_DEV_BOARD}" = "am62a-evm" ] || [ "${TI_DEV_BOARD}" = "j721s2-evm" ] || [ "${TI_DEV_BOARD}" = "j784s4-evm" ] || [ "${TI_DEV_BOARD}" = "j742s2-evm" ] || [ "${TI_DEV_BOARD}" = "j722s-evm" ]
            then
                sudo chmod 777 -R $rootfs_folder/opt/
                sudo chmod 777 $rootfs_folder/usr/lib/firmware/
                sudo chmod 777 -R $rootfs_folder/usr/include/processor_sdk/
                if [ "${TISDK_IMAGE}" = "adas" ] && [ "${SOC}" != "am62a" ]
                then
                    sudo chmod 777 -R $rootfs_folder/usr/lib/firmware/vision_apps_evm/
                fi
                if [ "${SOC}" = "am62a" ]
                then
                    sudo chmod 777 -R $rootfs_folder/usr/lib/firmware/ti-ipc/am62axx/
                fi
                if [ "${TISDK_IMAGE}" = "edgeai" ] && [ "${SOC}" != "am62a" ]
                then
                    sudo chmod 777 -R $rootfs_folder/usr/lib/firmware/vision_apps_eaik/
                fi
                sudo chmod 777 -R $rootfs_folder/usr/lib/
                sudo chmod 777 -R $rootfs_folder/usr/bin/
                if [ "${TISDK_IMAGE}" = "edgeai" ]
                then
                sudo chmod 777 -R $rootfs_folder/usr/include/edgeai-apps-utils/
                sudo chmod 777 -R $rootfs_folder/usr/include/edgeai-tiovx-kernels/
                sudo chmod 777 -R $rootfs_folder/usr/include/edgeai-tiovx-apps/
                sudo chmod 777 -R $rootfs_folder/usr/include/edgeai-tiovx-modules/
                sudo chmod 777 $rootfs_folder/usr/bin/edgeai-tiovx-kernels-test
                sudo chmod 777 $rootfs_folder/usr/bin/edgeai-tiovx-modules-test
                fi
                sudo chmod 666 $rootfs_folder/etc/security/limits.conf
                sudo chmod 644 $rootfs_folder/usr/lib/systemd/system/*
                sudo chmod 644 $rootfs_folder/usr/lib/systemd/system.conf.d/*.conf
                sudo chmod 755 -R $rootfs_folder/usr/lib/systemd/system-generators
            else
                sudo chmod 777 $rootfs_folder/opt/
                sudo chmod 777 $rootfs_folder/usr/lib/firmware
                sudo chmod 777 $rootfs_folder/etc/security/
                sudo chmod 666 $rootfs_folder/etc/security/limits.conf
                sudo chmod 777 $rootfs_folder/boot
                sudo chmod 777 $rootfs_folder/usr/lib
                sudo chmod 777 $rootfs_folder/usr/lib/gstreamer-1.0/
                sudo chmod 777 $rootfs_folder/usr/lib/pkgconfig/
                sudo chmod 777 $rootfs_folder/usr/bin
                sudo chmod 777 $rootfs_folder/usr/include
            fi
            sync
            echo "Installing $rootfs_file to /media/$USER/$rootfs_folder ... Done"
        else
            echo "ERROR: $rootfs_file not found !!!"
        fi
    else
        echo "ERROR: $rootfs_folder not found !!!"
    fi
}

if [[ `id -u` -ne 0 ]]
then
  untar_bootfs
  untar_rootfs
else
  echo "Please run without sudo"
  exit
fi
