let path = require('path');

let device = "j722s";

const files = {
    common: [
        "dm_power_analysis_app.c",
        "main.c",
    ],
};

/* Relative to where the makefile will be generated
 * Typically at <example_folder>/<BOARD>/<core_os_combo>/<compiler>
 */
const filedirs = {
    common: [
        "..",                 /*J722S specific main file */
        "../../../src",       /* Example base*/
    ],
};

const syscfgfile = "../example.syscfg";

const readmeDoxygenPageTag = "EXAMPLES_DM_POWER_ANALYSIS";

const defines_wkup_r5 = {
    common: [
        "ENABLE_SCICLIENT_DIRECT",
    ],
};

const libdirs_freertos_wkup_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/sciserver/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/rm_pm_hal/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/sciclient_direct/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/self_reset/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils/lib",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/dm_stub/lib",
    ],
};

const libdirs_freertos_main_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils/lib",
    ],
};
const libdirs_freertos_c75ss0 = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils/lib",
    ],
}

const libdirs_freertos_c75ss1 = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils/lib",
    ],
}
const libdirs_freertos_mcu_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils/lib",
    ],
};

const includes_freertos_c75ss0 = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/FreeRTOS-Kernel/include",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/portable/TI_CGT/DSP_C75X",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/config/j722s/c75x",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils/src/soc/j722s",
        "${MCU_PLUS_SDK_PATH}/source/drivers/",
        "${MCU_PLUS_SDK_PATH}/source/drivers/hw_include/",
    ],
};

const includes_freertos_c75ss1 = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/FreeRTOS-Kernel/include",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/portable/TI_CGT/DSP_C75X",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/config/j722s/c75x",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils/src/soc/j722s",
        "${MCU_PLUS_SDK_PATH}/source/drivers/",
        "${MCU_PLUS_SDK_PATH}/source/drivers/hw_include/",
    ],
};

const includes_freertos_wkup_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/FreeRTOS-Kernel/include",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/config/j722s/r5f",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/rm_pm_hal/rm_pm_hal_src/include",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/rm_pm_hal/rm_pm_hal_src/pm/include",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/rm_pm_hal/rm_pm_hal_src/pm",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/rm_pm_hal/rm_pm_hal_src/",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/sciclient_direct/soc/j722s/",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/rm_pm_hal/rm_pm_hal_src/pm/soc/j722s/include/soc/j722s/",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/rm_pm_hal/rm_pm_hal_src/include/soc/j722s/",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/rm_pm_hal/rm_pm_hal_src/pm/soc/j722s/include/",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils/src/soc/j722s",
        "${MCU_PLUS_SDK_PATH}/source/drivers/",
        "${MCU_PLUS_SDK_PATH}/source/drivers/hw_include/",
    ],
};

const includes_freertos_main_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/FreeRTOS-Kernel/include",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/config/j722s/r5f",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils/src/soc/j722s",
        "${MCU_PLUS_SDK_PATH}/source/drivers/",
        "${MCU_PLUS_SDK_PATH}/source/drivers/hw_include/",
    ],
};

const includes_freertos_mcu_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/FreeRTOS-Kernel/include",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/config/j722s/r5f",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils",
        "${MCU_PLUS_SDK_PATH}/source/pdm_utils/src/soc/j722s",
        "${MCU_PLUS_SDK_PATH}/source/drivers/",
        "${MCU_PLUS_SDK_PATH}/source/drivers/hw_include/",
    ],
};

const libs_freertos_wkup_r5f = {
    common: [
        "rm_pm_hal.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "sciclient_direct.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "self_reset.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "freertos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "pdm_utils.j722s.wkup-r5fss0-0.ti-arm-clang.${ConfigName}.lib",
        "sciserver.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "dm_stub.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};

const libs_freertos_main_r5f = {
    common: [
        "freertos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.j722s.main-r5f.ti-arm-clang.${ConfigName}.lib",
        "pdm_utils.j722s.main-r5fss0-0.ti-arm-clang.${ConfigName}.lib",
    ],
};

const libs_freertos_mcu_r5f = {
    common: [
        "freertos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.j722s.main-r5f.ti-arm-clang.${ConfigName}.lib",
        "pdm_utils.j722s.mcu-r5fss0-0.ti-arm-clang.${ConfigName}.lib",
    ],
};

const libs_freertos_c75ss0 = {
    common: [
        "freertos.j722s.c75x.ti-c7000.${ConfigName}.lib",
        "drivers.j722s.c75ss0-0.ti-c7000.${ConfigName}.lib",
        "pdm_utils.j722s.c75ss0-0.ti-c7000.${ConfigName}.lib",
    ],
};

const libs_freertos_c75ss1 = {
    common: [
        "freertos.j722s.c75x.ti-c7000.${ConfigName}.lib",
        "drivers.j722s.c75ss1-0.ti-c7000.${ConfigName}.lib",
        "pdm_utils.j722s.c75ss1-0.ti-c7000.${ConfigName}.lib",
    ],
};

const lnkfiles = {
    common: [
        "linker.cmd",
    ]
};

const buildOptionCombos = [
    { device: device, cpu: "wkup-r5fss0-0", cgt: "ti-arm-clang", board: "j722s-evm", os: "freertos"},
    { device: device, cpu: "mcu-r5fss0-0",  cgt: "ti-arm-clang", board: "j722s-evm", os: "freertos"},
    { device: device, cpu: "main-r5fss0-0", cgt: "ti-arm-clang", board: "j722s-evm", os: "freertos"},
    { device: device, cpu: "c75ss0-0",      cgt: "ti-c7000",     board: "j722s-evm", os: "freertos"},
    { device: device, cpu: "c75ss1-0",      cgt: "ti-c7000",     board: "j722s-evm", os: "freertos"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "executable";
    property.name = "dm_power_analysis_app";
    property.isInternal = false;
    property.tirexResourceSubClass = [ "example.gettingstarted" ];
    property.description = "DM Power Analysis app"
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = {};

    build_property.files = files;
    build_property.filedirs = filedirs;
    build_property.lnkfiles = lnkfiles;
    build_property.syscfgfile = syscfgfile;
    build_property.readmeDoxygenPageTag = readmeDoxygenPageTag;

    if(buildOption.cpu.match(/wkup-r5f*/)) {
        if(buildOption.os.match(/freertos*/) )
        {
            build_property.defines = defines_wkup_r5;
            build_property.includes = includes_freertos_wkup_r5f;
            build_property.libdirs = libdirs_freertos_wkup_r5f;
            build_property.libs = libs_freertos_wkup_r5f;
        }
    }
    else if(buildOption.cpu.match(/main-r5f*/)) {
        if(buildOption.os.match(/freertos*/) )
        {
            build_property.includes = includes_freertos_main_r5f;
            build_property.libdirs = libdirs_freertos_main_r5f;
            build_property.libs = libs_freertos_main_r5f;
        }
    }
    else if(buildOption.cpu.match(/mcu-r5f*/)) {
        if(buildOption.os.match(/freertos*/) )
        {
            build_property.includes = includes_freertos_mcu_r5f;
            build_property.libdirs = libdirs_freertos_mcu_r5f;
            build_property.libs = libs_freertos_mcu_r5f;
        }
   }
   else if(buildOption.cpu.match(/c75ss0-0*/)) {
        if(buildOption.os.match(/freertos*/) )
        {
            build_property.includes = includes_freertos_c75ss0;
            build_property.libdirs = libdirs_freertos_c75ss0;
            build_property.libs = libs_freertos_c75ss0;
        }
   }
   else if(buildOption.cpu.match(/c75ss1-0*/)) {
        if(buildOption.os.match(/freertos*/) )
        {
            build_property.includes = includes_freertos_c75ss1;
            build_property.libdirs = libdirs_freertos_c75ss1;
            build_property.libs = libs_freertos_c75ss1;
        }
  }
  return build_property;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};
