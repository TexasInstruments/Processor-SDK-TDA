# Copyright (c) 2026, Texas Instruments Incorporated
# SPDX License Identifier: MIT

echo "user.sh called..."

echo Setting additional environment variables...
export PS1='TDA54-VDK@QNX:$(pwd)# '
export PATH=:/proc/boot:/bin:/sbin:/usr/bin:/usr/sbin:/opt/bin:/ti_fs:/ti_fs/bin:/ti_fs/sbin:/ti_fs/usr/bin:/ti_fs/usr/sbin:/ti_fs/tibin:/ti_fs/scripts
export LD_LIBRARY_PATH=:/proc/boot:/lib:/usr/lib:/lib/dll:/opt/lib:/ti_fs/lib:/ti_fs/usr/lib:/ti_fs/lib/dll/mmedia:/ti_fs/lib/dll:/ti_fs/tilib
export VX_TEST_DATA_PATH=/ti_fs/vision_apps/test_data

echo "Starting shmemallocator.."
shmemallocator

echo Starting TI IPC Resource Manager
tiipc-mgr

echo Starting vision_apps_init
cd /ti_fs/vision_apps
vision_apps_init.sh
