let common   = system.getScript("/common");
let book_keeping = []
const memoryRegs = system.getScript("/memory_configurator/helper");

let config = [
    {
        name: "$name",
        isCIdentifier: false
    },
    {
        name: "load_memory",
        displayName: "Load Memory",
        default:"",
        description:'Choose a memory region from the ones added in the Memory regions section.',
        options: () => {return memoryRegs.loadMemoryRegions(book_keeping).memory_regions},
    },
]

/*
 * Validates a memory_region_list instance to ensure that a load memory region
 * has been selected.
 *
 * The load_memory field is a dropdown populated dynamically by
 * helper.loadMemoryRegions(). If the user has not yet selected a region (the
 * value is an empty string), this function reports an error so that code
 * generation is blocked until the field is filled in.
 *
 * Parameters:
 *   inst   - SysConfig module instance being validated. Expected to have a
 *            .load_memory string property.
 *   report - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls report.logError() when load_memory is empty.
 */
function validate(inst, report) {

    if(inst.load_memory.length == 0) {
        report.logError("This field can't be kept empty", inst, "load_memory")
    }
}

exports= {
    defaultInstanceName: "Memory Region",
	displayName: "Memory Region",
	config: config,
    validate: validate
}