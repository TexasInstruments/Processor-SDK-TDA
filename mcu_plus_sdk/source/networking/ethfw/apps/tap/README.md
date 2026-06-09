# TAP User-space Application
While building TAP application for non ADAS Linux, please update memory map addresses for intercore shared descriptors and data buffers from <b>`<ethfw>/apps/app_remoteswitchcfg_server/<soc>/mcu_2_0/linker_mem_map.cmd`</b> to <b>`<ethfw>/apps/tap/<soc>.conf`</b>.
See INTERCORE_ETH_DESC_MEM and INTERCORE_ETH_DATA_MEM memory starting address (ORIGIN) with length (LENGTH) from <b>`<ethfw>/apps/app_remoteswitchcfg_server/<soc>/mcu_2_0/linker_mem_map.cmd`</b>.
Match the same with ICQ_BASE_ADDR and BUFPOOL_BASE_ADDR (with their corresponding memory length) in <b>`<ethfw>/apps/tap/<soc>.conf`</b>.
As an extra check corresponding values should match with Linux device tree overlay's <b>reserved_memory</b> (i.e. main_r5fss0_core0_shared_memory_queue_region and main_r5fss0_core0_shared_memory_bufpool_region) for inter-core network communication as well. Download and install PSDK Linux, instructions are [here](https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/vision_apps/docs/user_guide/ENVIRONMENT_SETUP.html).
Device-tree overlays are present in <b>${PSDKL_PATH}/board-support/ti-linux-kernel-<version>/arch/arm64/boot/dts/ti</b> folder.
Overlay names are k3-j721e-evm-virt-mac-client.dtso for J721E, k3-j7200-evm-virt-mac-client.dtso for J7200 and k3-j784s4-evm-virt-mac-client.dtso for J784S4 respectively.
### Introduction
The TAP user-space application serves as a medium to facilitate the exchange of Ethernet frames between the A72 Linux and R5_0 (MCU2_0) master core.
To achieve this, a TAP device is used to read from and write to the Linux network stack. Ethernet frames are copied from/to the shared memory region to allow other cores to access it.  
### Compiling and installing TAP application
The TAP application is provided as part of the Ethernet Firmware software component in the Processor SDK, it can be found at `<ethfw>/apps/tap`.
The TAP application needs to be cross-compiled using Linux toolchain and then installed to the Linux filesystem.
The steps listed below assume that an SD card is used for Linux booting of the TI EVM.  
1. Download the Linux toolchain via setuptools.sh helper script. 
```shell
    $ ./setuptools.sh
```
2. Cross-compile the TAP application and install in SD card. Here,<aarch64-none-linux-gnu install dir> is the absolute path where the ARM64 A72 Linux compiler was installed using setuptools.sh in previous step. $SOC is the soc (J7200, J721E or J784S4) for which we are running TAP application. 
```shell
    $ make CROSS_COMPILE=<aarch64-none-linux-gnu install dir>/bin/aarch64-none-linux-gnu- install DESTDIR=<Path to the root file system on SD card> SOC=<SOC>
```
For example, if the ARM64 A72 Linux compiler was installed in <PSDK_RTOS_PATH>/ethfw/apps/tap/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu, and the SD card is mounted at /media/username and the file system is at /media/username/root and soc is SOC, then the make command should be:
```shell
    $ make CROSS_COMPILE=<PSDK_RTOS_PATH>/ethfw/apps/tap/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu- install DESTDIR=/media/username/root SOC=<SOC>
```
3. Boot the TI EVM from SD card as usual, and run below command to ensure that the systemd service launch_tap.service starts up automatically on boot. With this, on the next boot, the user-space application should be running automatically in the background. 
```shell
    $ systemctl enable launch_tap.service
```
4. On successful startup of launch_tap.service, a new network interface called tap0 will be created and it will get an IP address assigned using DHCP (valid only if the switch port is connected to the network hosting a DHCP server). The network interface tap0 should also show up in the list of available network interfaces using the ifconfig command:
```shell
    $ ifconfig tap0
      tap0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500  metric 1
            inet 192.168.1.220  netmask 255.255.255.0  broadcast 192.168.1.255
            inet6 fe80::201:2ff:fe04:506  prefixlen 64  scopeid 0x20<link>
            ether 00:01:02:04:05:06  txqueuelen 1000  (Ethernet)
            RX packets 82  bytes 6932 (6.7 KiB)
            RX errors 0  dropped 0  overruns 0  frame 0
            TX packets 44  bytes 5534 (5.4 KiB)
            TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

### Debugging
By default, the systemd service launch_tap.service will run the shell script tapif.sh during boot up. However, it is possible to manually relaunch the application either for testing purposes or in case of errors during automatic startup:
1. Navigate to the directory containing tapif.sh file and the tapif executable (typically /usr/bin).
2. Execute the shell script tapif.sh which shall initialize the TAP device and launch the user-space application. 
```shell
    $ cd /usr/bin
    $ bash tapif.sh&
```
3. The inter-core virtual Ethernet interface and tapif user-space application can be shutdown if needed by executing the script cleantapif.sh which is provided in the same directory.
```shell
    $ cd /usr/bin
    $ bash ./cleantapif.sh
```
