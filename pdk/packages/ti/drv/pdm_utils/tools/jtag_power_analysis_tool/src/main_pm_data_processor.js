const pathJ = require("path");
const fsJ = require("fs");

const isRunningFromCcsJ = typeof ds !== "undefined"; /* ds is defined when running in CCS Scripting Console */

if (isRunningFromCcsJ) {
    /* Tool can run from scripting console */
    /* ------------------------------------- EDIT FILE PATH TO ABSOLUTE PATH BEFORE RUNNING ------------------------------------- */
    const filePath = "/home/cgt/ti/pdm_utils/tools/jtag_power_analysis_tool/src/main_pm_data_processor.js";
    if (!fsJ.existsSync(filePath)) {
        throw new Error("jtag_power_analysis_tool: file " + filePath + " not found, please check the path");
    }
    process.chdir(pathJ.dirname(filePath));
}

const common = require("./drv/common.js");
const {buildClkTree, printClkTree, printConfigTable} = require("./app_clk_tree.js");
const {getPllSettings, printPllData} = require("./app_pll.js");
const {getPscData, printPscData} = require("./app_psc.js");

/**
 * JTAG Power Analysis Tool
 *
 * This tool generates detailed power report for various power domains of the SoC.
 *
 * Usage:
 *
 *     node pmDataProcessor.js --soc <soc> --mmrKeyVal <folder containing MMR keys and values in csv format> --selector <all|clock_tree|pll|psc> --level <0-2>
 *
 *     --soc <soc>                                   The SoC to be used. Supported SoCs: j784s4, j721s2, j721e, j722s, j7200
 *
 *     --mmrKeyVal <folder containing MMR keys and values in csv format>
 *                                                   The folder containing csv files containing MMR keys and values.
 *
 *     --selector <all|clock_tree|pll|psc>           (Optional) The tool to be used. Default is 'all'
 *
 *     --level <0-2>                                 (Optional) The print level. Default is '0'
 *
 *                                                   Levels:
 *                                                   0: Minimal print
 *                                                   1: Intermediate print
 *                                                   2: Detailed print
 *
 *     --help                                         Display this help
 *
 * Example:
 *
 *     node pmDataProcessor.js --soc j784s4 --mmrKeyVal /path/to/mmr/keys/folder --selector clock_tree --level 2
 *
 */

function help() {
    console.log("JTAG Power Analysis Tool\n" + "------------------------\n");
    console.log("Usage:");
    console.log(
        "    node pmDataProcessor.js --soc <soc> --mmrKeyVal <folder containing MMR keys and values in csv format> --selector <all|clock_tree|pll|psc> --level <0-2>"
    );
    console.log(
        "    --soc <soc>                                   The SoC to be used. Supported SoCs: " + JtagTool.supportedSoc.join(", ")
    );
    console.log("    --mmrKeyVal <folder containing MMR keys and values in csv format>");
    console.log(
        "                                                   The folder containing csv files containing MMR Register Addresses and its values."
    );
    console.log("    --selector <all|clock_tree|pll|psc>           (Optional) The tool to be used. Default is 'all'");
    console.log("    --level <0-2>                                 (Optional) The print level. Default is '0'");
    console.log("                                                   Levels:");
    console.log("                                                   0: Minimal print");
    console.log("                                                   1: Intermediate print");
    console.log("                                                   2: Detailed print");
    console.log("    --help                                         Display this help");
    console.log();
    console.log("Example:");
    console.log("    node pmDataProcessor.js --soc j784s4 --mmrKeyVal /path/to/mmr/keys/folder --selector clock_tree --level 2");
}

class JtagTool {
    static selectorList = ["all", "clock_tree", "pll", "psc", "pet", "junction_temp"];
    static supportedSoc = ["j784s4", "j742s2", "j721s2", "j721e", "j722s", "j7200"];

    constructor(soc, mmrKeyValDir) {
        this.soc = soc;
        this.mmrKeyValDir = mmrKeyValDir;
    }

    init() {
        common.setSoc(this.soc);

        if (!fsJ.existsSync(this.mmrKeyValDir)) {
            console.error("File " + this.mmrKeyValDir + " does not exist");
            help();
            /* process.exit(-1); */
        } else {
            const filesList = fsJ
                .readdirSync(this.mmrKeyValDir)
                .filter((file) => file.endsWith(".csv"))
                .map((file) => pathJ.join(this.mmrKeyValDir, file));

            if (filesList.length === 0) {
                console.error("No CSV files found in " + pathJ.dirname(this.mmrKeyValDir));
                help();
                /* process.exit(-1); */
            }

            const combinedJsObject = common.getCombinedObjectFromCsv(filesList);

            if (combinedJsObject) {
                common.setMemValues(combinedJsObject);
            }
        }
    }

    deinit() {
        common.setSoc("undefined");
        common.setMemValues(null);
    }

    run(selector = "all", level = 0) {
        if (selector == "all" || selector == "pll") {
            /* call the function to get the PLL settings */
            const pllStatusJson = getPllSettings(this.soc);

            /* save the PLL settings */
            common.saveObjToOutput("plls_status_" + this.soc + ".json", pllStatusJson);

            /* print the PLL settings */
            printPllData(pllStatusJson.pllStatus, level);
        }

        if (selector == "all" || selector == "psc" || selector == "pet" || selector == "clock_tree") {
            /* call the function to get the LPSC settings */
            const pscDataJson = getPscData(this.soc);

            /* save the PSC settings */
            common.saveObjToOutput("psc_status_" + this.soc + ".json", pscDataJson);

            /* print the PSC settings */
            if (selector === "psc" || selector === "all") {
                printPscData(pscDataJson.pscStatus, level);
            }
        }

        if (selector == "all" || selector == "clock_tree" || selector == "pet") {
            /* call the function to build the clock tree */
            const clkTreeJson = buildClkTree(this.soc);

            /* save the clock tree */
            common.saveObjToOutput("clock_ip_tree_" + this.soc + ".json", clkTreeJson);

            /* print the clock tree */
            if (selector == "pet") {
                printConfigTable(clkTreeJson.clkTree); /* print the clock tree as per the pet tool() */
            } else {
                printClkTree(clkTreeJson.clkTree);
            }
        }

        return 0;
    }
}

function pmDataProcessor(soc, selector, level, mmrKeyValDir = "../memory_values_csv/" + soc) {
    if (!JtagTool.supportedSoc.includes(soc)) {
        console.error("Unsupported soc: " + soc);
        console.error("Supported socs: " + JtagTool.supportedSoc.join(", "));
        process.exit(-1);
    }

    if (!JtagTool.selectorList.includes(selector)) {
        console.error("Unsupported selector: " + selector);
        console.error("Supported selectors: " + JtagTool.selectorList.join(", "));
        process.exit(-1);
    }

    const jtag = new JtagTool(soc, mmrKeyValDir);
    jtag.init();
    jtag.run(selector, level);
    jtag.deinit();
}

if (!isRunningFromCcsJ) {
    const minimist = require("minimist");

    const args = minimist(process.argv.slice(2), {
        default: {
            selector: "all",
            level: 0
        }
    });

    const soc = args.soc;
    const mmrKeyValDir = args.mmrKeyVal || pathJ.join(__dirname, "../memory_values_csv", soc);
    const selector = args.selector || "all";
    const level = args.level || 0;

    pmDataProcessor(soc, selector, level, mmrKeyValDir);
}


/* Example usage for j722s: 
  From Terminal: node main_pm_data_processor.js --soc j722s --selector pet --level 0
  or 
  From CCS20 Scripting Console: pmDataProcessor("j722s", "pet", 0);
 */
