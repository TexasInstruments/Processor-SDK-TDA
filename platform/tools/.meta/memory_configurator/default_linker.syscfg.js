let common = system.getScript("/common");
let selfCoreName = common.getSelfSysCfgCoreName();
const device = system.deviceData.device;

exports = {
    displayName: "Default Linker Config",
	config: [
        {
            name: "scriptingButton",
            displayName: "Add Default Linker Config",
            buttonText: "CLICK",
            scriptingOnComplete: (inst) => {
                    let memory_regions_count = 0
                    let sections_count = 0

                    if(device == "J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x") {
                        // mcu-r5fss checks must come before r5fss0 checks (substring match)
                        if(selfCoreName.includes("mcu-r5fss0-0")){
                            memory_regions_count = 13;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss0-0")){
                            memory_regions_count = 15;  // primary core, owns shared regions
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss0-1")){
                            memory_regions_count = 13;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss1-0")){
                            memory_regions_count = 12;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss1-1")){
                            memory_regions_count = 12;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c71ss0-0")){
                            memory_regions_count = 16;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c71ss1-0")){
                            memory_regions_count = 15;
                            sections_count = 0;
                        }
                    }
                    else if(device == "J721E_DRA829_TDA4VM_AM752x") {
                        // mcu-r5fss checks must come before r5fss0 checks (substring match)
                        if(selfCoreName.includes("mcu-r5fss0-0")){
                            memory_regions_count = 8;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss0-0")){
                            memory_regions_count = 17;  // primary core, owns shared regions
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss0-1")){
                            memory_regions_count = 8;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss1-0")){
                            memory_regions_count = 7;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss1-1")){
                            memory_regions_count = 7;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c71ss0-0")){
                            memory_regions_count = 11;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c66ss0-0")){
                            memory_regions_count = 7;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c66ss1-0")){
                            memory_regions_count = 7;
                            sections_count = 0;
                        }
                    }
                    else if(device == "J742S2") {
                        // mcu-r5fss checks must come before r5fss0 checks (substring match)
                        if(selfCoreName.includes("mcu-r5fss0-0")){
                            memory_regions_count = 8;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss0-0")){
                            memory_regions_count = 17;  // primary core, owns shared regions
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss0-1")){
                            memory_regions_count = 9;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss1-0")){
                            memory_regions_count = 7;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss1-1")){
                            memory_regions_count = 7;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss2-0")){
                            memory_regions_count = 8;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss2-1")){
                            memory_regions_count = 7;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c71ss0-0")){
                            memory_regions_count = 17;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c71ss1-0")){
                            memory_regions_count = 17;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c71ss2-0")){
                            memory_regions_count = 13;
                            sections_count = 0;
                        }
                    }
                    else if(device == "J784S4_TDA4AP_TDA4VP_TDA4AH_TDA4VH_AM69x" || device == "J784S4") {
                        // mcu-r5fss checks must come before r5fss0 checks (substring match)
                        if(selfCoreName.includes("mcu-r5fss0-0")){
                            memory_regions_count = 8;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss0-0")){
                            memory_regions_count = 17;  // primary core, owns shared regions
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss0-1")){
                            memory_regions_count = 8;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss1-0")){
                            memory_regions_count = 7;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss1-1")){
                            memory_regions_count = 7;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss2-0")){
                            memory_regions_count = 9;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("r5fss2-1")){
                            memory_regions_count = 7;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c71ss0-0")){
                            memory_regions_count = 25;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c71ss1-0")){
                            memory_regions_count = 25;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c71ss2-0")){
                            memory_regions_count = 25;
                            sections_count = 0;
                        }
                        else if(selfCoreName.includes("c71ss3-0")){
                            memory_regions_count = 25;
                            sections_count = 0;
                        }
                    }

                    //let generalInst = scripting.addModule("/memory_configurator/general.syscfg.js", {}, false).addInstance()

                    let region_module = system.modules['/memory_configurator/region'];
                    let regionInst;

                    if(region_module && region_module.$instances && region_module.$instances.length > 0) {
                        regionInst = region_module.$instances[0];
                    } else {
                        regionInst = scripting.addModule("/memory_configurator/region.syscfg.js", {}, false).addInstance()
                    }

                    // product.json puts every SoC's .meta dir on the include path, so the
                    // script name must be SoC-unique or the first SoC listed always wins.
                    const socScript = "/mem_map_default_cfg/default_linker_config_" + common.getSocName();

                    regionInst.memory_region.create(memory_regions_count);
                    for(let i=0;i<memory_regions_count;i++){
                        system.getScript(socScript).populate_memory_regions(regionInst, i, device, selfCoreName)
                    }



                    // let sectionMod = scripting.addModule("/memory_configurator/section.syscfg.js", {}, false)
                    // for(let i=0;i<sections_count;i++){
                    //     let sectionInst= sectionMod.addInstance()
                    //     system.getScript("/memory_configurator/default_linker_config").populate_sections_regions(sectionInst, i+1, device, selfCoreName)
                    // }

                }
    }],
    maxInstances: 1,
    minInstances: 0,
}
