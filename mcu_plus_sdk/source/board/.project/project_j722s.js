let path = require('path');

let device = "j722s";


const files_r5f={
    common: [
        "board_control.c",
        "board_utils.c",
        "eeprom.c",
        "eeprom_at24c.c",
        "flash.c",
        "flash_nor_ospi.c",
        "flash_nand_ospi.c",
        "led.c",
        "led_gpio.c",
        "led_ioexp.c",
        "ioexp_tca6424.c",
        "ioexp_tca6416.c",
        "nor_spi_sfdp.c",
        "phy_common_priv.c",
        "dp83tc812.c",
		"dp83tg720.c",
		"dp83869.c",
		"dp83867.c",
		"dp83822.c",
		"dp83826.c",
        "generic_phy.c",
        "eeprom_at24c512c.c",
    ],
};

const files_c75x={
    common: [
        "eeprom.c",
        "eeprom_at24c.c",
        "led.c",
        "led_ioexp.c",
        "ioexp_tca6424.c",
        "ioexp_tca6416.c",
    ],
};

const filedirs = {
    common: [
        "control",
        "utils",
        "eeprom",
        "flash",
        "flash/ospi",
        "flash/sfdp",
        "ioexp",
        "null",
        "led",
        "ethphy/enet/rtos_drivers/src",
        "ethphy/enet/rtos_drivers/include",
    ],
};

const includes = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/board/ethphy/enet/rtos_drivers/include",
        "${MCU_PLUS_SDK_PATH}/source/board/ethphy/port",
    ],
}

const buildOptionCombos = [
    { device: device, cpu: "r5f", cgt: "ti-arm-clang"},
    { device: device, cpu: "c75x", cgt: "ti-c7000"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "library";
    property.name = "board";
    property.isInternal = false;
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = {};

    build_property.filedirs = filedirs;
    build_property.includes = includes;
    if(buildOption.cpu.match(/r5f*/))
    {
        build_property.files = files_r5f;
    }
    else if(buildOption.cpu.match(/c75x*/))
    {
        build_property.files = files_c75x;
    }

    return build_property;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};
