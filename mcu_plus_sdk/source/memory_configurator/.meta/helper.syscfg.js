const common = system.getScript("/common");
const physicalLayout = system.getScript("/memory_configurator/soc/physicalLayout.json")[system.deviceData.device];

function checkNameChange(name) {
    let selfCoreName = common.getSelfSysCfgCoreName();
    let selfCore_shared_module = common.getModuleForCore("/memory_configurator/shared_region_references", selfCoreName);
    let newName = name;

    if (selfCore_shared_module !== undefined) {
        let selfCore_shared_instances = selfCore_shared_module.$instances;
        _.each(selfCore_shared_instances, (each_instance) => {
            if (each_instance.shared_region === name) {
                newName = each_instance.shared_region_name_change;
            }
        });
    }
    return newName;
}


function memoryRegionInformation(mpu_instance, mpu_set) {
    let mr_list = {
        name: [],
        type: [],
        start_addr: [],
        size: [],
        end_addr: [],
        permissions: [],
        shared: [],
        cores: [],
        access_check: []
    };
    let all_core_mr_instances = util_function(mpu_set);
    let mr_instances = [];
    _.each(all_core_mr_instances, (each_core_memory_module) => {
        if (each_core_memory_module.core.length > 0) {
            _.each(each_core_memory_module.instances, (inst) => {
                let obj = Object.assign({}, inst);
                obj.core = each_core_memory_module.core;
                mr_instances.push(obj);
            });
        }
    });
    let sorted_memory_regions = _.chain(mr_instances)
        .sortBy((mr_instance) => (mr_instance.auto ? mr_instance.autoStartAddress : mr_instance.manualStartAddress))
        .value();

    let selfCoreName = common.getSelfSysCfgCoreName();
    let mp_start = mpu_instance.baseAddr;
    let mp_size = Math.pow(2, mpu_instance.size);
    let mp_end = Number(mp_start) + Number(mp_size) - 0x1;
    _.each(sorted_memory_regions, (region) => {
        let mr_start = (region.auto ? region.autoStartAddress : region.manualStartAddress);
        let mr_end = mr_start + region.size - 0x1;
        if (mr_end < mp_start || mp_end < mr_start) {
            // current mp instance doesn't fall inside this mr
        } else {
            let region_name = (region.$name).concat("_", region.core);
            mr_list["name"].push(displayName(region_name));
            mr_list["type"].push(region.type);
            mr_list["start_addr"].push(mr_start);
            mr_list["size"].push(region.size);
            mr_list["end_addr"].push(mr_end);
            mr_list["permissions"].push(JSON.stringify(region.attributes));
            mr_list["cores"].push(region.core);

            if (region.isShared) {
                let arr_cores = region.shared_cores;

                if (!region.core.includes(selfCoreName) && !arr_cores.includes(selfCoreName)) {
                    mr_list.access_check = false;
                }

                _.each(arr_cores, (core) => {
                    let last_value = mr_list["cores"][mr_list.cores.length - 1];
                    let updated_value = "";
                    if (last_value.includes(core) == false) {
                        updated_value = updated_value.concat(core, " ");
                        last_value = updated_value;
                    }
                    if (!updated_value.includes(selfCoreName))
                        updated_value = selfCoreName.concat(" ", updated_value);
                    mr_list["cores"][mr_list.cores.length - 1] = updated_value;
                });
                mr_list["shared"].push(true);
            } else {
                mr_list["shared"].push(false);
            }
        }
    });
    return mr_list;
}


function util_function(mpu_set) {
    let coreNames = common.getSysCfgCoreNames();
    if (!coreNames || coreNames.length === 0) {
        coreNames = ["mcu-r5fss0-0", "wkup-r5fss0-0", "main-r5fss0-0", "c75ss0-0", "c75ss1-0"];
    }
    let memory_region_module_name = '/memory_configurator/memory_region' + mpu_set;

    let all_core_memory_module = [];

    for (let core of coreNames) {
        let obj = {
            instances: [],
            core: "",
        };
        let core_module = common.getModuleForCore(memory_region_module_name, core);
        if (core_module != undefined) {
            let core_module_instances = core_module.$instances;
            obj.instances = core_module_instances;
            obj.core = core;
        }
        all_core_memory_module.push(obj);
    }

    return all_core_memory_module;
}

function displayName(name) {
    let lastIndex = name.lastIndexOf("_");
    let region_name = name.substring(0, lastIndex);

    return region_name;
}

function getMemoryRegionModuleName(core) {
    let region_mod = common.getModuleForCore('/memory_configurator/region', core);
    if (region_mod !== undefined && region_mod.$instances && region_mod.$instances[0].mpu_setting) {
        if (core.includes("c75ss0") || core.includes("c75ss1"))
            return "/memory_configurator/memory_region_mmu";
        return "/memory_configurator/memory_region_mpu";
    }
    return "/memory_configurator/memory_region";
}

function coreList(inst) {
    let coreNames = common.getSysCfgCoreNames();
    let selfCoreName = common.getSelfSysCfgCoreName();

    if (!coreNames || coreNames.length === 0) {
        coreNames = ["main-r5fss0-0", "wkup-r5fss0-0", "c75ss0-0", "c75ss1-0"];
    }

    let tmp_list = coreNames.filter(function (coreName) {
        return coreName !== selfCoreName;
    });

    let memoryRegType = inst.type;
    let applicable_cores = (memoryRegType.slice(memoryRegType.lastIndexOf('_')+1)).toLowerCase();
    if ( memoryRegType.indexOf('_')== -1 ) memoryRegType = memoryRegType+"_ALL"; // This is for types which do not follow the _ALL type format, it's assumed that this region could be shared among all

    let options = [];

    for (let core of tmp_list)
    {
        let ele = {name: core, displayName: core}
        if( memoryRegType.includes("_ALL") ) // This means that the memory type can be accessed across all cores
            options.push(ele)
        else if ( core.includes(applicable_cores) && physicalLayout[memoryRegType].access === "all" ) //Display only those which cores with which the current core can access given the memory time
            options.push(ele)
    }
	return options;
}

function computeCoreNum()
{
    let options = coreList();
    let default_shared_cores = [];

    for(let i = 0; i <  options.length; i++){
        default_shared_cores.push(options[i].name);
    }

    return default_shared_cores;
}

/*
 * Rounds a byte count up to the smallest power-of-2 that a RAT region can
 * cover and returns it as a log2 exponent, matching the "size" options of
 * the addr_translate module (5 = 32 B ... 32 = 4 GB).
 *
 * RAT regions must be a power of 2, so a linker region whose size is not a
 * power of 2 is covered by the next power of 2 above it. The result is
 * clamped to the addr_translate option range; addr_translate's validate()
 * reports an error when the clamped size no longer covers the owning linker
 * region.
 *
 * Parameters:
 *   sz - number — the linker region size in bytes.
 *
 * Returns: number — log2 of the RAT region size, in the range [5, 32].
 * Side effects: none.
 */
function ratSizeLog2(sz) {
    let log2 = 5;
    while (log2 < 32 && Math.pow(2, log2) < sz) log2++;
    return log2;
}

/*
 * Builds the optional addr_translate child descriptor for one memory region
 * instance whose size is stored in bytes (memory_region module).
 *
 * Shared with memory_region_mpu via getRatModuleInstancesExp() below, which
 * covers instances whose size is already a log2 exponent. The descriptor is
 * returned only when the instance's hidden ratAutomated flag is set;
 * region.syscfg.js forces that flag from its "Automate RAT Setting" toggle
 * via requiredArgs.
 *
 * Only size and isAutomated are passed as requiredArgs. The local address is
 * deliberately not passed: addr_translate derives it lazily via getValue from
 * $ownedBy, because evaluating autoStartAddress during moduleInstances()
 * triggers a SysConfig cross-context error (same constraint as mpu_armv7).
 *
 * Parameters:
 *   inst - memory_region instance. Reads inst.ratAutomated and inst.size
 *          (bytes).
 *
 * Returns: object[] — empty, or one descriptor (useArray, max 1 instance).
 * Side effects: none.
 */
function getRatModuleInstances(inst) {
    if (!inst.ratAutomated) return [];

    return [{
        name: "rat_config",
        displayName: "RAT Region",
        moduleName: "/kernel/dpl/addr_translate",
        useArray: true,
        minInstanceCount: 0,
        maxInstanceCount: 1,
        collapsed: false,
        requiredArgs: {
            size:        ratSizeLog2(inst.size),
            isAutomated: true,
        },
    }];
}

/*
 * Builds the optional addr_translate child descriptor for one memory_region_mpu
 * instance, whose size is already a log2 exponent (5 = 32 B ... 32 = 4 GB)
 * and is therefore passed through directly instead of via ratSizeLog2().
 * See getRatModuleInstances() above for the full contract.
 *
 * Parameters:
 *   inst - memory_region_mpu instance. Reads inst.ratAutomated and inst.size
 *          (log2 exponent).
 *
 * Returns: object[] — empty, or one descriptor (useArray, max 1 instance).
 * Side effects: none.
 */
function getRatModuleInstancesExp(inst) {
    if (!inst.ratAutomated) return [];

    return [{
        name: "rat_config",
        displayName: "RAT Region",
        moduleName: "/kernel/dpl/addr_translate",
        useArray: true,
        minInstanceCount: 0,
        maxInstanceCount: 1,
        collapsed: false,
        requiredArgs: {
            size:        inst.size,
            isAutomated: true,
        },
    }];
}

exports = {
    memoryRegionInformation,
    util_function,
    displayName,
    coreList,
    getMemoryRegionModuleName,
    ratSizeLog2,
    getRatModuleInstances,
    getRatModuleInstancesExp,
}