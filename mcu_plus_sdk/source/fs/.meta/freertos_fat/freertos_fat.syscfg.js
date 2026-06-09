let common = system.getScript("/common");

let freertos_fat_module_name = "/fs/freertos_fat/freertos_fat";

let freertos_fat_module = {
	displayName: "FreeRTOS FAT",
	templates: {
        "/drivers/system/system_config.h.xdt": {
            driver_config: "/fs/freertos_fat/templates/freertos_fat.h.xdt",
        },
	    "/drivers/system/drivers_open_close.c.xdt": {
	        driver_open_close_config: "/fs/freertos_fat/templates/freertos_fat_open_close_config.c.xdt",
	        driver_open: "/fs/freertos_fat/templates/freertos_fat_open.c.xdt",
	        driver_close: "/fs/freertos_fat/templates/freertos_fat_close.c.xdt",
	    },
	    "/drivers/system/drivers_open_close.h.xdt": {
	        driver_open_close_config: "/fs/freertos_fat/templates/freertos_fat_open_close.h.xdt",
	    },
	},
	defaultInstanceName: "CONFIG_FREERTOS_FAT",
    config:getConfigurables(),
	moduleInstances: moduleInstances,
};

function getConfigurables()
{
    let config = [];

	config.push(
		{
			name: "media",
			displayName: "Select Media",
			description: "Select the media which is to be used underneath the virtual file system provided by FreeRTOS FAT",
			default: "SD",
			options: [
				{ name: "SD" },
                { name: "EMMC" },
			]
		},
	)

    if(common.isDMWithBootSupported())
    {
        config.push(common.getDMWithBootConfig());
    }
    return config;
}

function moduleInstances(inst) {

    let modInstances = new Array();
    let requiredArgs = {};

    if(system.deviceData.device !== "AM275x"){
        switch(inst.media) {
            case "SD":
                modInstances.push({
                    name: "peripheralDriver",
                    displayName: "MMCSD Configuration",
                    moduleName: '/drivers/mmcsd/mmcsd',
                    useArray: false,
                    requiredArgs: {
                        moduleSelect: "MMC1",
                    },
                });
                break;
            case "EMMC":
                modInstances.push({
                    name: "peripheralDriver",
                    displayName: "MMCSD Configuration",
                    moduleName: '/drivers/mmcsd/mmcsd',
                    useArray: false,
                    requiredArgs: {
                        moduleSelect: "MMC0",
                    },
                });
                break;
        }
    } else{
        if(common.isDMWithBootSupported())
        {
            requiredArgs.addedByBootloader = inst.addedByBootloader;
            requiredArgs.moduleSelect = "MMC";
        }
        else{
            requiredArgs.moduleSelect = "MMC";
        }
        modInstances.push({
            name: "peripheralDriver",
            displayName: "MMCSD Configuration",
            moduleName: '/drivers/mmcsd/mmcsd',
            useArray: false,
            requiredArgs:requiredArgs
        });
    }

    return (modInstances);
}

exports = freertos_fat_module;