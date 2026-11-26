const common = system.getScript("/common");
const physicalLayout = system.getScript("/memory_configurator/physicalLayout.json")[system.deviceData.device];

/*
 * Checks whether a shared memory region has been given a display-name alias
 * via the shared_region_references module, and returns the effective name.
 *
 * When a core consumes a shared region owned by another core, an entry in
 * the shared_region_references module can rename that region for the local
 * context (e.g. to avoid symbol conflicts). This function walks the
 * shared_region_references instances of the current core and returns the
 * renamed string if one exists, or the original name if no rename entry is
 * found.
 *
 * Parameters:
 *   name - string — the original $name of the shared memory region instance.
 *
 * Returns: string — the renamed display name if an alias exists, or name
 *          unchanged if no alias is configured.
 * Side effects: none.
 */
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


/*
 * Collects all memory regions across all cores whose address range overlaps
 * with a given MPU/MPU-cfg instance's address span, and returns a structured
 * summary of those regions.
 *
 * Calls util_function(mpu_set) to gather every memory region instance from
 * every core, then sorts them by start address. For each region that
 * intersects the MPU instance's range [baseAddr, baseAddr + 2^size - 1],
 * the function accumulates name, type, start address, size, end address,
 * attributes (permissions), shared status, and owning/sharing core list.
 *
 * When a region is shared (isShared == true), the cores list in the returned
 * object is extended with the shared_cores array; an access_check flag is
 * also set to false when neither the region's owning core nor its shared_cores
 * list includes the current core.
 *
 * Parameters:
 *   mpu_instance - SysConfig instance of an MPU region (mpu_cfg), expected to
 *                  have .baseAddr (start) and .size (log2 exponent) properties.
 *   mpu_set      - string — module-name suffix passed to util_function to
 *                  select between plain (""), MPU ("_mpu"), or MMU ("_mmu")
 *                  memory region modules.
 *
 * Returns: object — {
 *     name: string[],         instance names of overlapping regions,
 *     type: string[],         memory type keys (e.g. "DDR_ALL"),
 *     start_addr: number[],   start address of each region,
 *     size: number[],         size in bytes of each region,
 *     end_addr: number[],     inclusive end address of each region,
 *     permissions: string[],  JSON-stringified attributes arrays,
 *     shared: boolean[],      true if the corresponding region is shared,
 *     cores: string[],        owning + sharing core names per region,
 *     access_check: boolean | false  (false when access is denied)
 *   }
 * Side effects: none (read-only walk of module instances).
 */
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


/*
 * Returns all memory region instances for every core, grouped into an array
 * of {core, instances} objects, for the memory region module variant
 * identified by mpu_set.
 *
 * The full module path used to look up each core's module is:
 *   '/memory_configurator/memory_region' + mpu_set
 * where mpu_set is typically "" (plain), "_mpu", or "_mmu".
 *
 * When system.deviceData.cores is empty or undefined (which can happen during
 * early SysConfig load), the function falls back to hard-coded per-device core
 * lists so that downstream callers can still iterate reliably.
 *
 * Parameters:
 *   mpu_set - string — suffix appended to the base module path to select the
 *             correct memory region module variant. Pass "" for the plain
 *             memory_region module, "_mpu" for memory_region_mpu, etc.
 *
 * Returns: object[] — array where each element has:
 *     .core      string   - core name, empty string if the module is absent
 *     .instances object[] - array of SysConfig module instances for that core
 *   Every known core gets an entry even if its module is undefined (in that
 *   case instances is [] and core is "").
 * Side effects: none.
 */
function util_function(mpu_set) {
    let coreNames = common.getSysCfgCoreNames();
    if (!coreNames || coreNames.length === 0) {
        const device = system.deviceData.device;
        if (device === "J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x" || device === "J721S2") {
            coreNames = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
        } else if (device === "J721E_DRA829_TDA4VM_AM752x" || device === "J721E") {
            coreNames = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
        } else if (device === "J742S2") {
            coreNames = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0"];
        } else if (device === "J784S4_TDA4AP_TDA4VP_TDA4AH_TDA4VH_AM69x" || device === "J784S4") {
            coreNames = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0","c71ss3-0"];
        } else {
            coreNames = ["main-r5fss0-0", "wkup-r5fss0-0", "c71ss0-0", "c71ss1-0"];
        }
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

/*
 * Strips the trailing "_<core>" suffix from a region instance name.
 *
 * Memory region names are internally stored as "<regionName>_<coreName>" so
 * that instances from different cores can coexist without name collisions.
 * This function reverses that concatenation and returns only the user-visible
 * region name by removing the last underscore-delimited segment.
 *
 * Parameters:
 *   name - string — combined instance name in the form "REGION_NAME_coreName",
 *          e.g. "DDR_REGION_r5fss0-0".
 *
 * Returns: string — the base region name with the last "_..." segment removed,
 *          e.g. "DDR_REGION".
 * Side effects: none.
 */
function displayName(name) {
    let lastIndex = name.lastIndexOf("_");
    let region_name = name.substring(0, lastIndex);

    return region_name;
}

/*
 * Returns the SysConfig module path for the memory region module that should
 * be used for a given core, based on whether "Automate MPU/MMU" mode is
 * enabled in that core's region module instance.
 *
 * The selection logic is:
 *   1. If the core has a region module and its first instance has mpu_setting
 *      true, the automated variant is used:
 *        - Cores whose name includes "c71ss" → "/memory_configurator/memory_region_mmu"
 *        - All other cores (R5F)             → "/memory_configurator/memory_region_mpu"
 *   2. Otherwise (manual mode or no region module) → "/memory_configurator/memory_region"
 *
 * Parameters:
 *   core - string — the core name to query, e.g. "r5fss0-0" or "c71ss0-0".
 *
 * Returns: string — one of "/memory_configurator/memory_region",
 *          "/memory_configurator/memory_region_mpu", or
 *          "/memory_configurator/memory_region_mmu".
 * Side effects: none.
 */
function getMemoryRegionModuleName(core) {
    let region_mod = common.getModuleForCore('/memory_configurator/region', core);
    if (region_mod !== undefined && region_mod.$instances && region_mod.$instances[0].mpu_setting) {
        if (core.includes("c71ss"))
            return "/memory_configurator/memory_region_mmu";
        return "/memory_configurator/memory_region_mpu";
    }
    return "/memory_configurator/memory_region";
}

/*
 * Builds the list of memory region options that should be selectable by the
 * current core in the memory_region_list module's "load_memory" dropdown.
 *
 * Walks every core's memory region module (using getMemoryRegionModuleName to
 * select the right variant per core) and includes a region in the returned
 * list when either:
 *   - The region belongs to the current core (selfCoreName is included in the
 *     core's context string), or
 *   - The region is marked shared (isShared == true) and shared_cores includes
 *     selfCoreName, in which case the display name may be overridden by an
 *     alias from checkNameChange().
 *
 * The function also rebuilds the book_keeping array in-place (splice + push)
 * with one {name} entry per region instance, parallel to the memory_regions
 * options array, so that callers can correlate selected region names back to
 * the originating instance.
 *
 * When no region module exists at all, a single placeholder {name:"",
 * displayName:""} entry is returned so the dropdown has a valid (empty)
 * default.
 *
 * Parameters:
 *   book_keeping - object[] — an external array that is cleared and repopulated
 *                  in-place. After return it has one {name} entry per memory
 *                  region instance that was considered (not just those added to
 *                  the visible options list).
 *
 * Returns: { memory_regions: object[] } — object whose memory_regions field
 *          is the array of {name, displayName} option entries for the dropdown.
 * Side effects: mutates book_keeping in-place.
 */
function loadMemoryRegions(book_keeping) {
    let memory_regions = [];

    let coreNames = common.getSysCfgCoreNames();
    let region_module = system.modules['/memory_configurator/region'];
    if (region_module !== undefined) {
        let selfCoreName = common.getSelfSysCfgCoreName();
        book_keeping.splice(0, book_keeping.length);
        for (let core of coreNames) {
            let memory_region_module_name = getMemoryRegionModuleName(core);
            let core_module = common.getModuleForCore(memory_region_module_name, core);
            let core_module_instances;
            if (core_module != undefined) {
                core_module_instances = core_module.$instances;
                _.each(core_module_instances, instance => {
                    let obj = { name: " " };
                    let displayName = instance.$name.concat(" Type: ", instance.type);
                    if (core.includes(selfCoreName)) {
                        memory_regions.push({ name: instance.$name, displayName: displayName });
                        obj.name = instance.$name;
                    } else if (instance.isShared && instance.shared_cores.includes(selfCoreName)) {
                        let newName = checkNameChange(instance.$name);
                        let displayName_changed = newName.concat(" Type: ", instance.type);
                        memory_regions.push({ name: instance.$name, displayName: displayName_changed });
                        obj.name = instance.$name;
                    }
                    book_keeping.push(obj);
                });
            }
        }
    } else {
        memory_regions.push({ name: "", displayName: "" });
    }
    return { memory_regions };
}

/*
 * Returns the list of core options with which the given memory region instance
 * can be shared, filtered by the region's memory type accessibility rules.
 *
 * The current (owning) core is always excluded from the returned list.
 * Cores are further filtered based on the memory type:
 *   - Types ending in "_ALL" (or types with no underscore, treated as _ALL)
 *     can be shared with every other core.
 *   - All other types restrict sharing to cores whose name contains the
 *     lower-case suffix after the last "_" in the type key, provided the
 *     physicalLayout entry for that type has access == "all".
 *
 * Falls back to a hard-coded per-device core list when getSysCfgCoreNames()
 * returns an empty array (early SysConfig load).
 *
 * Parameters:
 *   inst - SysConfig memory region instance. The function reads inst.type to
 *          determine the memory type and derive sharing eligibility.
 *
 * Returns: object[] — array of {name, displayName} entries (both fields equal
 *          to the core name string) for cores eligible to share this region.
 * Side effects: none.
 */
function coreList(inst) {
    let coreNames = common.getSysCfgCoreNames();
    let selfCoreName = common.getSelfSysCfgCoreName();

    if (!coreNames || coreNames.length === 0) {
        const device = system.deviceData.device;
        if (device === "J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x" || device === "J721S2") {
            coreNames = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
        } else if (device === "J721E_DRA829_TDA4VM_AM752x" || device === "J721E") {
            coreNames = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
        } else if (device === "J742S2") {
            coreNames = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0"];
        } else if (device === "J784S4_TDA4AP_TDA4VP_TDA4AH_TDA4VH_AM69x" || device === "J784S4") {
            coreNames = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0","c71ss3-0"];
        } else {
            coreNames = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
        }
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

/*
 * Returns an array of all shareable core names that can be used as the default
 * value for the shared_cores configurable.
 *
 * Calls coreList() with no inst argument to get the full list of eligible
 * cores, then extracts just the name string from each entry. The result is
 * intended to pre-populate the shared_cores multi-select with all possible
 * sharing partners when a region is first marked as shared.
 *
 * Note: coreList() will receive undefined as inst, so any type-based filtering
 * inside coreList() based on inst.type will throw or behave unexpectedly;
 * this function is therefore only safe to call in contexts where coreList()
 * does not access inst properties (or where the caller ensures inst is valid).
 *
 * Returns: string[] — array of core name strings eligible for region sharing.
 * Side effects: none.
 */
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
 * the rat_cfg module (5 = 32 B ... 32 = 4 GB).
 *
 * RAT regions must be a power of 2, so a linker region whose size is not a
 * power of 2 is covered by the next power of 2 above it. The result is
 * clamped to the rat_cfg option range; rat_cfg's validate() reports an error
 * when the clamped size no longer covers the owning linker region.
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
 * Returns the moduleInstances descriptor array that spawns the optional
 * rat_cfg child of a memory region instance in "Automate RAT" mode.
 *
 * Shared by memory_region and memory_region_mpu so that RAT automation works
 * with or without Automate MPU. The descriptor is returned only when the
 * instance's hidden ratAutomated flag is set; region.syscfg.js forces that
 * flag from its "Automate RAT Setting" toggle via requiredArgs.
 *
 * Only size and isAutomated are passed as requiredArgs. The local address is
 * deliberately not passed: rat_cfg derives it lazily via getValue from
 * $ownedBy, because evaluating autoStartAddress during moduleInstances()
 * triggers a SysConfig cross-context error (same constraint as mpu_cfg).
 *
 * Parameters:
 *   inst - memory_region / memory_region_mpu instance. Reads inst.ratAutomated
 *          and inst.size.
 *
 * Returns: object[] — empty, or one descriptor (useArray, max 1 instance).
 * Side effects: none.
 */
function getRatModuleInstances(inst) {
    if (!inst.ratAutomated) return [];

    return [{
        name: "rat_config",
        displayName: "RAT Region",
        moduleName: "/memory_configurator/rat_cfg",
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
 * Returns every core name the memory configurator may legally reference for
 * the current device, regardless of which core context is active.
 *
 * system.deviceData.cores is empty for these devices (the SysConfig device
 * JSON ships "cores": []), so common.getSysCfgCoreNames() returns [] and the
 * hard-coded per-device list below is what is used in practice. Validators
 * must not rely on getSysCfgCoreNames() alone: an empty list makes every
 * shared_cores selection look invalid.
 *
 * Returns: string[] - core names for the current device.
 * Side effects: none.
 */
function getDeviceCoreNames() {
    let coreNames = common.getSysCfgCoreNames();
    if (coreNames && coreNames.length > 0) {
        return coreNames;
    }

    const device = system.deviceData.device;
    if (device === "J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x" || device === "J721S2") {
        return ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
    } else if (device === "J721E_DRA829_TDA4VM_AM752x" || device === "J721E") {
        return ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
    } else if (device === "J742S2") {
        return ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0"];
    } else if (device === "J784S4_TDA4AP_TDA4VP_TDA4AH_TDA4VH_AM69x" || device === "J784S4") {
        return ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0","c71ss3-0"];
    }
    return ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
}

/*
 * Reports an error on the shared_cores configurable when any selected core is
 * not a valid sharing partner, i.e. it is not a known core of the device or it
 * is the core that owns the region.
 *
 * Shared by memory_region, memory_region_mpu and memory_region_mmu so that all
 * three accept the same set of cores. The "at least one core" check stays in
 * each module's validate().
 *
 * Parameters:
 *   inst   - memory region instance. Reads inst.shared_cores.
 *   report - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls report.logError() once if any selected core is invalid.
 */
function validateSharedCores(inst, report) {
    let selfCoreName = common.getSelfSysCfgCoreName();
    let otherCores = getDeviceCoreNames().filter(c => c !== selfCoreName);

    if (inst.shared_cores.some(c => otherCores.indexOf(c) === -1)) {
        report.logError(`Not a valid option.`, inst, "shared_cores");
    }
}

exports = {
    memoryRegionInformation,
    util_function,
    displayName,
    loadMemoryRegions,
    coreList,
    getMemoryRegionModuleName,
    ratSizeLog2,
    getRatModuleInstances,
    getDeviceCoreNames,
    validateSharedCores,
}