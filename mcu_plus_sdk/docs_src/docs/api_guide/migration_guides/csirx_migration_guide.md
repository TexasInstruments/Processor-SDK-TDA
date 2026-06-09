# CSIRX Migration Guide {#CSIRX_MIGRATION_GUIDE}

This section describes the differences between CSIRX driver in MCU+ SDK and Processor SDK RTOS (PDK).
This can be used as migration aid when moving from Processor SDK RTOS (PDK) to MCU+ SDK.


## API changes

None

## Changes in Directory structure

- In PDK, CSIRX driver is partitioned into CSL and driver layer.
  In MCU+ SDK, files in CSL are moved to hw_include which is part of source/drivers/csrix folder.


## Examples supported

- csirx_capture_testapp with Fusion-1 board is supported in MCU+ SDK.
- This example validates simulataneous streaming from 2 CSIRX instances with 4 channels each.
- Read more about Fusion-1 board here

## Features supported

- Supports capture stream
- Up to 32 virtual channels (Fusion board supports only 4 channels per instance)
- Supports Fusion-1 board.
- Max 2.5 GBPS DPHY lane speed with 2 or 4-lane configuration
- Min lane speed : 80 Mbps to 100 Mbps 
- Max lane speed : 2250 Mbps to 2500 Mbps
- Support for interleaving virtual channels at line level.
- Supports following frame data types:
  RGB: RGB88, RGB444, RGB555, RGB565, RGB666, RGB888
  YUV: YUV422_8, YUV422_10, YUV420_8, YUV420_10
  RAW: RAW6, RAW7, RAW8, RAW10, RAW12, RAW14, RAW16, RAW20
- Error Detection for following
   Stream FIFO overflow
   ECC error
   CRC error
   DataID error
   Truncated/Elongated Packets

