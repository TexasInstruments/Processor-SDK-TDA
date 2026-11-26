let common = system.getScript("/common");
let selfCoreName = common.getSelfSysCfgCoreName();

let condition = (!selfCoreName.includes("a53")) && (!selfCoreName.includes("pru")) && (common.getSocName() != "awr294x") && (system.compiler != "gcc");

let rtos = (typeof process !== "undefined" && process.env.RTOS) ? process.env.RTOS.toLowerCase() : "freertos";

// NOTE: outputPath is shown verbatim in the SysConfig GUI file list, so these
// stay bare filenames, uniform with every other entry. The sdk_builder
// platform_syscfg targets plant a symlink under each name in every core's
// generated/ directory before SysConfig runs, so the write follows the link
// to the canonical platform/memory_map/<soc>/ location.

let memory_configurator_module = {}
if (condition)
  {
      memory_configurator_module = {
          displayName: "TI Memory Configurator",
          templates: [
              {
                  name: "/memory_configurator/templates/linker_mem_map_base.xdt",
                  outputPath: "linker_mem_map_" + rtos + ".cmd",
              },
              {
                  name: "/memory_configurator/templates/app_mem_map_base.xdt",
                  outputPath: "app_mem_map.h",
              },
              {
                  name: "/memory_configurator/templates/linux_rtos_memory_map_base.xdt",
                  outputPath: "k3-j722s-rtos-memory-map.dtsi",
              },
          ],
          topModules: [
              "memory_configurator/default_linker",
              "memory_configurator/region",
          ],
      }                                                                                                                                                                                                     
  } 
exports = memory_configurator_module
