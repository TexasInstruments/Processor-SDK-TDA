# CSIRX {#DRIVERS_CSIRX_PAGE}

[TOC]

CSI (Camera Serial Interface) module allows device to stream video inputs from one or more cameras.
CSIRX has four streams:

- one capture stream
- one loopback stream
- two OTF streams

- If capture stream is enabled, video input from camera modules are captured and stored
  in DRAM location provided by the application.
- If loopback stream is enabled, video input from camera modules are redirected to CSITX.
- If OTF streams are enabled, video input is redirected to video accelerator.

\attention  CSIRX driver supports only capture. Loopback, and OTF are not yet enabled



## Features Supported

- Supports Capture stream
- Up to 32 virtual channels (Fusion board supports only 4 channels per instance)
- Supports Fusion-1 and Fusion-2 boards.
- Max 2.5 GBPS DPHY lane speed with 2 or 4-lane configuration
- Min lane speed : 80 Mbps to 100 Mbps 
- Max lane speed : 2250 Mbps to 2500 Mbps
- Support for interleaving virtual channels at line level.
- Supports following frame data types:
  RGB: RGB88, RGB444, RGB555, RGB565, RGB666, RGB888
  YUV: YUV422_8, YUV422_10, YUV420_8, YUV420_10
  RAW: RAW6, RAW7, RAW8, RAW10, RAW12, RAW14, RAW16, RAW20
- Error Detection
- Stream FIFO overflow
- ECC error
- CRC error
- DataID error
- Truncated/Elongated Packets

## Features NOT Supported

- loopback stream
- OTF streams

## APIs
- CSIRX driver APIs are integrated with FVID2 interface.
- Hence application writer can directly use FVID2 APIs to create & delete the driver instance, Start and stop a particular stream etc.

- For reference, here are the top-level driver APIs for CSIRX

\ref DRV_CSIRX_MODULE
