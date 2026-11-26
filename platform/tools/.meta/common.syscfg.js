
/*
 * Returns the name of the core that owns the current SysConfig context.
 *
 * When system.deviceData.core is populated (i.e. a single-core context), the
 * core name is read directly from that object. Otherwise, the function falls
 * back to system.context, which is the string SysConfig assigns to the active
 * editing context for multi-context devices (J784S4, J721S2, J721E, J742S2).
 *
 * Returns: string — the core name, e.g. "r5fss0-0" or "c71ss0-0".
 *          Returns undefined if neither deviceData.core nor a recognised SoC
 *          is found (effectively an unhandled device).
 */
function getSelfSysCfgCoreName() {
    if(system.deviceData.core)
        return system.deviceData.core.name;


    /* when system.deviceData.core is not defined return a valid core for current SoC */
    switch(getSocName()) {
        case "j784s4":
            return system.context;
        case "j721s2":
            return system.context;
        case "j721e":
            return system.context;
        case "j742s2":
            return system.context;
    }
};

/*
 * Returns true if the SafeRTOS RTOS variant is supported on the current SoC.
 *
 * Consults getSocName() to determine the active device and checks it against
 * a hard-coded allowlist (j721s2, j721e, j742s2, j784s4). Any unrecognised
 * device returns false.
 *
 * Returns: boolean — true when SafeRTOS is a valid RTOS choice, false otherwise.
 * Side effects: none.
 */
function isSafeRTOSSupported() {
    switch(getSocName()) {
        case "j721s2":
            return true;
        case "j721e":
            return true;
        case "j742s2":
            return true;
        case "j784s4":
            return true;
        default:
            return false;
    }
};

/*
 * Maps the full SysConfig device string to a short, canonical SoC name.
 *
 * system.deviceData.device can carry several alias strings for the same
 * physical device (e.g. "J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x" or simply
 * "J721S2"). This function normalises all known variants to a single
 * lower-case token used throughout the codebase for device-specific branching.
 *
 * Returns: string — one of "j784s4", "j721s2", "j721e", "j742s2".
 *          Returns undefined for unrecognised devices (no default branch).
 * Side effects: none.
 */
function getSocName() {
    if(system.deviceData.device == "J784S4_TDA4AP_TDA4VP_TDA4AH_TDA4VH_AM69x" || system.deviceData.device == "J784S4")
        return "j784s4";
    if(system.deviceData.device == "J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x" || system.deviceData.device == "J721S2")
        return "j721s2";
    if(system.deviceData.device == "J721E_DRA829_TDA4VM_AM752x" || system.deviceData.device == "J721E")
        return "j721e";
    if(system.deviceData.device == "J742S2")
        return "j742s2";
};

/*
 * Maps the full SysConfig device string to the canonical EVM board name.
 *
 * This is the human-readable board identifier used in build system targets,
 * linker script names, and documentation (e.g. "j721e-evm"). It mirrors the
 * same device-alias normalisation performed by getSocName() but returns the
 * hyphenated EVM string instead.
 *
 * Returns: string — one of "j784s4-evm", "j721s2-evm", "j721e-evm",
 *          "j742s2-evm". Returns undefined for unrecognised devices.
 * Side effects: none.
 */
function getDeviceName() {
    if(system.deviceData.device == "J784S4_TDA4AP_TDA4VP_TDA4AH_TDA4VH_AM69x" || system.deviceData.device == "J784S4")
        return "j784s4-evm";
    if(system.deviceData.device == "J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x" || system.deviceData.device == "J721S2")
        return "j721s2-evm";
    if(system.deviceData.device == "J721E_DRA829_TDA4VM_AM752x" || system.deviceData.device == "J721E")
        return "j721e-evm";
    if(system.deviceData.device == "J742S2")
        return "j742s2-evm";
};

/*
 * Validates whether a string is a legal C identifier.
 *
 * A valid C identifier must start with a letter or underscore and contain
 * only alphanumeric characters or underscores. This is the same rule enforced
 * by C compilers. The check is implemented via a regex match.
 *
 * Parameters:
 *   id - string to validate. May be null; null is handled gracefully.
 *
 * Returns: boolean — true if id is a non-null string matching
 *          /^[a-zA-Z_][0-9a-zA-Z_]*$/, false otherwise.
 * Side effects: none.
 */
function isCName(id) {
    if ( (id != null) && (id.match(/^[a-zA-Z_][0-9a-zA-Z_]*$/) != null) )
    {
        return true;
    }
    return false;
}

/*
 * Converts a whitespace- or symbol-separated string to camelCase.
 *
 * Leading/trailing spaces are trimmed implicitly, then the entire string is
 * lowercased before capitalising the first letter of each word boundary
 * (any non-alphanumeric character acts as a separator). The very first word
 * starts lower-case, matching standard camelCase convention.
 *
 * Parameters:
 *   str - the input string to convert, e.g. "Hello World" → "helloWorld".
 *
 * Returns: string — the camelCase version of str.
 * Side effects: none.
 */
function camelSentence(str) {
    return  (" " + str).toLowerCase().replace(/[^a-zA-Z0-9]+(.)/g, function(match, chr)
    {
        return chr.toUpperCase();
    });
}

/*
 * Returns an array of all core names defined for the current device.
 *
 * Iterates over system.deviceData.cores and collects each core's name string.
 * The returned array is the authoritative list of cores that SysConfig knows
 * about for the active device; it is used widely to drive cross-context
 * validation loops.
 *
 * Returns: string[] — e.g. ["mcu-r5fss0-0", "r5fss0-0", "c71ss0-0", ...].
 *          Returns an empty array if no cores are defined in deviceData.
 * Side effects: none.
 */
function getSysCfgCoreNames()
{
    let core_names = [];

    for( let core of system.deviceData.cores)
    {
        core_names.push(core.name);
    }
    return core_names;
}

/*
 * Returns the $static configuration object of a module as seen from a
 * specific core context.
 *
 * Each core has its own SysConfig context; this function looks up that
 * context by coreName, retrieves the named module from it, and returns the
 * module's $static property (the singleton, non-instance configuration).
 * This is used to compare static settings across cores during validation.
 *
 * Parameters:
 *   moduleName - string — the SysConfig module path, e.g. "/memory_configurator/region".
 *   coreName   - string — the target core name, e.g. "r5fss0-0".
 *
 * Returns: object | undefined — module.$static if the module exists in that
 *          core's context and has a $static property; undefined otherwise.
 * Side effects: none.
 */
function getStaticModuleForCore(moduleName, coreName)
{
    let context = system.contexts[coreName];
    let modules = context ? context.system.modules : {};
    let module = modules[moduleName];

    if(module && module.$static)
        return module.$static;

    return undefined;
}

/*
 * Returns the module object for a given module path as seen from a specific
 * core context.
 *
 * Unlike getStaticModuleForCore(), this returns the full module object (not
 * just $static), which exposes $instances and other module-level properties.
 * Used when iterating over per-core instance lists during cross-context
 * validation or memory region discovery.
 *
 * Parameters:
 *   moduleName - string — the SysConfig module path, e.g. "/memory_configurator/memory_region".
 *   coreName   - string — the target core name, e.g. "c71ss0-0".
 *
 * Returns: object | undefined — the module object if it exists in that core's
 *          context, undefined otherwise (module not added for that core).
 * Side effects: none.
 */
function getModuleForCore(moduleName, coreName)
{
    let context = system.contexts[coreName];
    let modules = context ? context.system.modules : {};
    let module = modules[moduleName];

    return module;
}


/*
 * Builds a SysConfig dropdown (options-list) config object from an array of
 * static configuration entries.
 *
 * This is a convenience factory used by ui.makeConfig and ui.makeInstanceConfig
 * to avoid repeating boilerplate when constructing SysConfig configurable
 * descriptors. The first entry in staticConfigArr becomes the default value.
 * Each entry may carry an optional description that is forwarded to the
 * option's description field.
 *
 * Parameters:
 *   staticConfigArr - object[] — array of objects with at least a .name
 *                     property and an optional .description property.
 *   fieldname       - string — the configurable name (config.name).
 *   displayName     - string — the human-readable label shown in the GUI.
 *
 * Returns: object | undefined — a SysConfig config descriptor with name,
 *          displayName, description, default, and options fields.
 *          Returns undefined if staticConfigArr is empty.
 * Side effects: none.
 */
function makeConfig(staticConfigArr, fieldname, displayName)
{
    let config = {};

    if(staticConfigArr.length == 0)
        return undefined;

    config.name = `${fieldname}`;
    config.displayName = `${displayName}`;
    config.description = `Select ${displayName}`;
    config.default = staticConfigArr[0].name;
    config.options = [];

    for (let i = 0; i < staticConfigArr.length; i++) {
        let option = {};

        option.name = staticConfigArr[i].name;
        if(staticConfigArr[i].description)
        {
            option.description = staticConfigArr[i].description;
        }
        config.options.push(option);
    }

    return config;
}

/*
 * Validates that no two instances of the same module share an identical value
 * for a given field, and reports an error on the current instance if they do.
 *
 * Iterates over all instances of instance.$module and compares each one's
 * fieldname value to the current instance's value. The current instance is
 * excluded from the comparison to avoid a self-match false positive.
 *
 * Parameters:
 *   instance  - SysConfig module instance — the instance being validated.
 *   fieldname - string — the name of the configurable field to check for
 *               duplicates (e.g. "instance" or "region_name").
 *   report    - SysConfig report object — used to call logError when a
 *               duplicate is found.
 *
 * Returns: void.
 * Side effects: calls report.logError() for every conflicting instance found.
 */
function checkSameFieldName(instance, fieldname, report)
{
    let moduleInstances = instance.$module.$instances;

    for (let i = 0; i < moduleInstances.length; i++) {
        if (instance[fieldname] === moduleInstances[i][fieldname] &&
            instance !== moduleInstances[i]) {
            report.logError(`Same ${fieldname} cannot be selected`, instance, fieldname);
        }
    }
}

/*
 * Same as checkSameFieldName() but searches across every core's context, not
 * just the current module's instance list.
 *
 * Iterates over all cores returned by getSysCfgCoreNames(), looks up the
 * module in each core's context, and checks every instance in that module for
 * a duplicate fieldname value. This ensures uniqueness constraints hold even
 * when the same logical module is added in multiple core contexts.
 *
 * Parameters:
 *   moduleName - string — the SysConfig module path to check in each core
 *                context, e.g. "/memory_configurator/memory_region".
 *   instance   - SysConfig module instance — the instance being validated.
 *   fieldname  - string — the configurable field to check for duplicates.
 *   report     - SysConfig report object — used to call logError.
 *
 * Returns: void.
 * Side effects: calls report.logError() for every cross-core conflict found.
 */
function checkSameFieldNameOnAllCores(moduleName, instance, fieldname, report)
{
    let coreNames = getSysCfgCoreNames();
    for ( let remoteCoreName of coreNames)
    {
        let remote_core_instance = getModuleForCore(moduleName, remoteCoreName);
        if(remote_core_instance)
        {
            let moduleInstances = remote_core_instance.$instances;
            for (let i = 0; i < moduleInstances.length; i++) {
                if (instance[fieldname] === moduleInstances[i][fieldname] &&
                    instance !== moduleInstances[i]) {
                    report.logError(`Same ${fieldname} cannot be selected`, instance, fieldname);
                }
            }
        }
    }
}

function findDuplicates(arrayToCheck)
{
    const count = arrayToCheck =>
      arrayToCheck.reduce((a, b) => ({ ...a,
        [b]: (a[b] || 0) + 1
      }), {})

    const duplicates = dict =>
      Object.keys(dict).filter((a) => dict[a] > 1)

    return {count: count(arrayToCheck), duplicates: duplicates(count(arrayToCheck))};
}

/*
 * Checks whether a given string is empty and returns a string
 * If stringToCheck is empty, an empty string is returned otherwise
 * stringToAdd is returned
 *
 * stringToCheck - The string to be compared to empty string
 * stringToAdd - The string to return when stringToCheck is non empty
 *
 */
function stringOrEmpty(stringToCheck, stringToAdd)
{
    if (stringToCheck != "")
    {
        return stringToAdd;
    }
    return "";
}

/*
 *  Check that HW signal type matches a specified array of types
 *
 *  Example: within a module's filterHardware(component) method:
 *      for (sig in component.signals) {
 *          let type = component.signals[sig].type;
 *          if (Common.typeMatches(type, ["PWM", "DOUT"])) {
 *              :
 *          }
 *      }
 *
 *  type      - a string or array of strings that are valid signal types
 *  nameArray - array of signal name types that must match one of the signal
 *              types named by type
 *
 *  Returns true iff nameArray contains at least one type name that's
 *          specified the type parameter.
 */
function typeMatches(type, nameArray)
{
    let options = {};

    if (type instanceof Array || typeof type == "object") {
        for (var i = 0; i < type.length; i++) {
            options[type[i]] = 1;
        }
    }
    else if (typeof type == "string" || type instanceof String) {
        options[type] = 1;
    }

    for (var i = 0; i < nameArray.length; i++) {
        let name = nameArray[i];
        if (name in options) {
            return (true);
        }
    }

    return (false);
}

/*
 * Returns the ordered list of driver names defining the recommended
 * initialization sequence for system bring-up.
 *
 * The order matters for correct hardware initialisation: UART is opened first
 * to enable early logging; MCU BIST is started before DDR and OSPI tuning so
 * it can run in parallel in the background; DDR must be available before OSPI
 * tuning; QoS configuration follows DDR.
 *
 * This list is consumed by code generators that emit the driver open sequence
 * in system initialisation files.
 *
 * Returns: string[] — driver name tokens in the required initialisation order.
 * Side effects: none.
 */
function getDriverOpenOrder()
{
    return [
        "watchdog",
        "uart", /* UART to opened first for logging */
        "mcu_bist", /* MCU BIST to be started before DDR init and OSPI tuning so that it can run in parallel to these in the background */
        "ddr",
        "qos",
        "ospi",
    ];
}

exports = {
    getSelfSysCfgCoreName,
    isSafeRTOSSupported,
    getSocName,
    getDeviceName,
    camelSentence,
    getStaticModuleForCore,
    getModuleForCore,
    getSysCfgCoreNames,
    findDuplicates,
    stringOrEmpty,
    typeMatches,
    getDriverOpenOrder,

    validate: {
        /*
         * Validates that no two instances of the same module share the same
         * "instance" field value. Delegates to checkSameFieldName with the
         * fixed fieldname "instance".
         *
         * Parameters:
         *   instance - SysConfig module instance being validated.
         *   report   - SysConfig report object for logging errors.
         */
        checkSameInstanceName : function (instance, report) {
            checkSameFieldName(instance, "instance", report);
        },
        /*
         * Same as checkSameInstanceName but searches across all core contexts.
         * Delegates to checkSameFieldNameOnAllCores with fieldname "instance".
         *
         * Parameters:
         *   moduleName - string — module path to check in each core context.
         *   instance   - SysConfig module instance being validated.
         *   report     - SysConfig report object for logging errors.
         */
        checkSameInstanceNameOnAllCores : function (moduleName, instance, report) {
            checkSameFieldNameOnAllCores(moduleName, instance, "instance", report);
        },
        checkSameFieldName : checkSameFieldName,
        checkSameFieldNameOnAllCores : checkSameFieldNameOnAllCores,
        /*
         * Validates that a numeric configurable falls within [min, max].
         * Logs a formatted error showing the allowed range in the requested
         * display base (decimal or hex).
         *
         * Parameters:
         *   instance      - SysConfig module instance being validated.
         *   report        - SysConfig report object for logging errors.
         *   property      - string — name of the numeric configurable to check.
         *   min           - number — minimum allowed value (inclusive).
         *   max           - number — maximum allowed value (inclusive).
         *   displayFormat - string — "hex" formats the range bounds in hex;
         *                   any other value uses decimal.
         */
        checkNumberRange: function(instance, report, property, min, max, displayFormat) {
            if(instance[property] > max || instance[property] < min) {
                let base = 10;

                if(displayFormat == "hex") {
                    base = 16;

                    report.logError( `Value must be between 0x${min.toString(base).toUpperCase()}`
                    + ` .. 0x${max.toString(base).toUpperCase()}`,
                     instance, property);
                }
                else {
                    report.logError( `Value must be between ${min.toString(base).toUpperCase()}`
                    + ` .. ${max.toString(base).toUpperCase()}`,
                     instance, property);
                }
            }
        },
        /*
         * Validates that a configurable's current value is a legal C identifier.
         * Delegates to isCName(). Logs an error if the value fails the check.
         *
         * Parameters:
         *   instance - SysConfig module instance being validated.
         *   report   - SysConfig report object for logging errors.
         *   property - string — name of the configurable field to check.
         */
        checkValidCName: function(instance, report, property) {
            if(isCName((instance[property])) === false) {
                report.logError(`Provide a valid C name`, instance, property);
            }
        },
        /*
         * Validates that a static configurable's value matches across all core
         * contexts. Used to enforce that settings which must be identical on
         * every core (e.g. memory region mode) are kept in sync.
         *
         * Iterates over all cores from getSysCfgCoreNames(), skips the current
         * core, and compares instance[configurable] against the same field in
         * each remote core's $static. Reports an error on the first mismatch
         * and stops.
         *
         * Parameters:
         *   moduleName    - string — SysConfig module path to look up per core.
         *   instance      - SysConfig module instance being validated.
         *   report        - SysConfig report object for logging errors.
         *   configurable  - string — the configurable field name to compare.
         *   core_instances - unused parameter (retained for API compatibility).
         */
        checkConfigurableValueMatchForAllCores(moduleName, instance, report, configurable, core_instances)
        {
            let coreNames = getSysCfgCoreNames();
            let selfCoreName = getSelfSysCfgCoreName();

            for ( let remoteCoreName of coreNames)
            {
                if( remoteCoreName != selfCoreName)
                {
                    let remote_core_instance = getStaticModuleForCore(moduleName, remoteCoreName);

                    if(remote_core_instance && instance[configurable] != remote_core_instance[configurable])
                    {
                        report.logError("Value MUST match with other cores", instance, configurable);
                        break;
                    }
                }
            }
        },
    },

    ui:
    {
        makeInstanceConfig: function(staticConfigArr) {
            return makeConfig(staticConfigArr, "instance", "Instance");
        },
        makeConfig: makeConfig,
    }
};
