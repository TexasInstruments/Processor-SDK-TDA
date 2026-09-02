#! /bin/bash

data_set_file=$1
psdk_path=$PWD
built_images=$psdk_path/out
rootfs_folder=$built_images/mnt

untar_file()
{
    if [ ! -d $rootfs_folder ]
    then
        echo "Mount point $rootfs_folder not found, creating it ..."
        mkdir -p $rootfs_folder
    fi

    if [ -d $rootfs_folder ]
    then
        echo "Installing $data_set_file to $test_data_folder ..."
        if [ -f $data_set_file ]
        then
            echo "Unmounting $rootfs_folder (if mounted) ..."
            sudo umount $rootfs_folder >/dev/null 2>&1 || true
            echo "Attaching $built_images/rootfs-img.wic as loop device ..."
            loop_dev=$(sudo losetup --find --partscan --show $built_images/rootfs-img.wic)
            echo "Mounting rootfs partition ${loop_dev}p2 to $rootfs_folder ..."
            sudo mount ${loop_dev}p2 $rootfs_folder
            echo "Extracting $data_set_file to $test_data_folder ..."
            sudo mkdir -p $test_data_folder
            sudo tar -xf $data_set_file $tar_arg -C $test_data_folder
            echo "Syncing to disk ..."
            sync
            echo "Unmounting $rootfs_folder ..."
            sudo umount $rootfs_folder
            echo "Detaching loop device $loop_dev ..."
            sudo losetup -d $loop_dev
            echo "Installing $data_set_file to $test_data_folder/ ... Done"
        else
            echo "ERROR: $data_set_file not found !!!"
        fi
    else
        echo "ERROR: $rootfs_folder not found !!!"
    fi
}

if [[ $data_set_file == *_data_set_*.tar.gz ]]
then

    if [[ $data_set_file == *psdk_rtos_ti_data_set_ptk*.tar.gz ]]
    then
        test_data_folder=$rootfs_folder/opt/vision_apps
    else
        test_data_folder=$rootfs_folder/opt/vision_apps/test_data
        tar_arg="--strip 2"
    fi
    untar_file

else

    echo "Usage: $0 <path/to/*_data_set*.tar.gz>"
    echo "       Pass one of the data sets provided on the SDK download page."

fi
