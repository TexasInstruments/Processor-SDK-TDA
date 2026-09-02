function getSocName() {
	const selected_device = system.deviceData.device;
	let soc_name = "";
	switch (selected_device) {
		case "J784S4_TDA4AP_TDA4VP_TDA4AH_TDA4VH_AM69x":
			soc_name = "j784s4";
			break;
		case "J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x":
			soc_name = "j721s2";
			break;
		case "J722S_TDA4VEN_TDA4AEN_AM67":
			soc_name = "j722s";
			break;
		case "J742S2":
			soc_name = "j742s2";
			break;
	}
	return soc_name;
}

/* common file can be picked pdm_utils(this file, in case of PDK ) or mcu_plus_sdk,
 PDK is define only if its selected from PDK(as standalone tool), This flag is used
 to check tool is invoked from PDK or MCU+SDK
 */
let PDK = true;

exports = {
	getSocName,
    PDK
};
