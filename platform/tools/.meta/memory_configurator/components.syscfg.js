let common = system.getScript("/common");
let selfCoreName = common.getSelfSysCfgCoreName();
let selfSocName = common.getSocName();

let condition = (!selfCoreName.includes("a53")) && (!selfCoreName.includes("pru")) && (common.getSocName() != "awr294x") && (system.compiler != "gcc");

/* MPU configuration is only applicable to R5F cores */
let isR5fCore = selfCoreName && selfCoreName.includes("r5fss");

/* MMU configuration is only applicable to C7x cores */
let isC7xCore = selfCoreName && selfCoreName.includes("c71ss");

/* RAT configuration is applicable to R5F and C66x cores */
let isRatCore = selfCoreName && (selfCoreName.includes("r5fss") || selfCoreName.includes("c66"));

/* Mirrors the RTOS make variable set by build_flags.mak */
let isSafeRTOS = (process.env.RTOS || "").toUpperCase() === "SAFERTOS";
let rtos = (typeof process !== "undefined" && process.env.RTOS) ? process.env.RTOS.toLowerCase() : "freertos";

let memory_configurator_module = {}
if (condition)
  {
      memory_configurator_module = {
          displayName: "TI Memory Configurator",
          topModules: [
              {
                  displayName: "Memory Configurator",
                  modules: [
                      "memory_configurator/default_linker",
                      "memory_configurator/region",
                  ],
              },
              ...(isR5fCore ? [{
                  displayName: "MPU Configurator",
                  modules: [
                      "memory_configurator/mpu_cfg",
                  ],
              }] : []),
              ...(isC7xCore ? [{
                  displayName: "MMU Configurator",
                  modules: [
                      "memory_configurator/mmu_cfg",
                  ],
              }] : []),
              ...(isRatCore ? [{
                  displayName: "RAT Configurator",
                  modules: [
                      "memory_configurator/rat_cfg",
                  ],
              }] : []),
          ],
          templates: [
              {
                  name: "/memory_configurator/templates/linker_mem_map_base.xdt",
                  outputPath: "linker_mem_map_" + rtos + ".cmd",
              },
              {
                  name: "/memory_configurator/templates/mpu_cfg_" + rtos + "_base.xdt",
                  outputPath: "mpu_cfg_" + rtos + ".c",
                  alwaysRun: isR5fCore,
              },
              {
                  name: "/memory_configurator/templates/mmu_cfg_base.xdt",
                  outputPath: "mmu_cfg.c",
                  alwaysRun: isC7xCore,
              },
              {
                  name: "/memory_configurator/templates/app_mem_map_base.xdt",
                  outputPath: "app_mem_map.h",
                  alwaysRun: selfCoreName.includes("r5fss0-0"),
              },
              {
                  name: "/memory_configurator/templates/rat_cfg_base.xdt",
                  outputPath: "rat_cfg.c",
                  alwaysRun: isRatCore,
              },
              {
                  name: "/memory_configurator/templates/linux_rtos_memory_map_base.xdt",
                  outputPath: "k3-" + common.getSocName() + "-rtos-memory-map.dtsi",
                  alwaysRun: selfCoreName.includes("r5fss0-0"),
              },
          ],                                                                                                                                                                                                
          views: [{                                                                                                                                                                                         
              name: "memory_configurator/Summary",                                                                                                                                                          
              displayName: "Memory Summary",                                                                                                                                                                
              viewType: "markdown",                                                                                                                                                                         
              icon: "table-large",                                                                                                                                                                          
              ignoreErrors: true,                                                                                                                                                                           
          }],                                                                                                                                                                                               
      }                                                                                                                                                                                                     
  } 
exports = memory_configurator_module
