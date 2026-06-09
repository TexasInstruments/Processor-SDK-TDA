# BootFlow Migration Guide {#BOOTFLOW_MIGRATION_GUIDE}

This section describes the difference between SBL boot flow of MCU+ SDK and Processor SDK RTOS (PDK).
This can be used as a migration aid when moving from Processor SDK RTOS (PDK) to MCU+ SDK.

- In PDK, both combined boot and legacy boot are supported, whereas in MCU+ SDK only combined boot is supported
\imageStyle{sbl_legacy_bootflow.png,width:30%}
\image html sbl_legacy_bootflow.png "Legacy Boot Flow"

\imageStyle{sbl_combined_bootflow.png,width:30%}
\image html sbl_combined_bootflow.png "Combined Boot Flow"

- For more details see - \ref EXAMPLES_DRIVERS_SBL