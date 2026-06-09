
let common = system.getScript("/common");

const driverVer = {
    "ethphy": {
        version: "v1",
    },
    "flash": {
        version: "v0",
    },
    "pmic": {
        version: "v0",
    },
    "serialFlash": {
        version: "v0",
    },
    "panel" : {
        version: "v0",
    },
};

const topModules_mcu_r5 = [
    "/board/eeprom/eeprom",
    "/board/led/led",
    "/board/pmic/pmic",
];
const topModules_wkup_r5 = [
    "/board/eeprom/eeprom",
    "/board/ethphy_cpsw_icssg/ethphy_cpsw_icssg",
    "/board/flash/flash",
    "/board/led/led",
    "/board/panel/panel"
];

exports = {
    getTopModules: function() {
        if (common.getSelfSysCfgCoreName().match(/mcu*/))
        {
            return topModules_mcu_r5;
        }
        return topModules_wkup_r5;
    },
    getDriverVer: function(driverName) {
        return driverVer[driverName].version;
    },
    getDriverInstanceValid: function(driverName) {
        let valid = false;
        if(driverName in driverVer)
        {
            valid = true;
        }
        return valid;
    }
};
