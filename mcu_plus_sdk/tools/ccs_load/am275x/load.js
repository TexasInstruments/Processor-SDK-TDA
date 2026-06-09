/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
Description:
   Load executables to various CPUs on the SOC

Usage:

1. Modify "sdkPath" (search for EDIT THIS) to point to the absolute path of the SDK.
    - On windows make sure to use '/' or '\\' as path separator

2. Launch am275x_GP_EVM target connection in CCS, however DO NOT connect to any CPUs.

3. From CCS Scripting console do (CCS Tool Bar > View > Scripting Console)

        js:> loadJSFile "<path/to/sdk>/tools/ccs_load/am275x/load.js"

4. After successful execution you should see a log like below

        js:> loadJSFile "C:/ti/mcu_plus_sdk/tools/ccs_load/am275x/load.js"
        Connecting, halting, reseting ...
        [r5fss0-0] Loading ... C:/ti/mcu_plus_sdk/examples/drivers/ipc/ipc_rpmsg_echo/am275x-evm/r5fss0-0_freertos/ti-arm-clang/ipc_rpmsg_echo.debug.out
        [r5fss0-1] Loading ... C:/ti/mcu_plus_sdk/examples/drivers/ipc/ipc_rpmsg_echo/am275x-evm/r5fss0-1_nortos/ti-arm-clang/ipc_rpmsg_echo.debug.out
        [r5fss1-0] Loading ... C:/ti/mcu_plus_sdk/examples/drivers/ipc/ipc_rpmsg_echo/am275x-evm/r5fss1-0_nortos/ti-arm-clang/ipc_rpmsg_echo.debug.out
        [r5fss1-1] Loading ... C:/ti/mcu_plus_sdk/examples/drivers/ipc/ipc_rpmsg_echo/am275x-evm/r5fss1-1_nortos/ti-arm-clang/ipc_rpmsg_echo.debug.out
        [r5fss0-1] Running ...
        [r5fss1-0] Running ...
        [r5fss1-1] Running ...
        [r5fss0-0] Running ...
        All DONE !!!

5. If any of the logs in step 4 show "fail" or "error" messages then
   check your EVM, CCS, SDK setup, Executables path and try again.

6. To reload without power cycles, repeat step 4 onwards.

*/

// !!! EDIT THIS
isAvailableA53 = true;

// !!! EDIT THIS !!! Add absolute path to SDK in your environment variables
//                   OR set this variable to the absolute path of the SDK
sdkPath = System.getenv("MCU_PLUS_SDK_AM275X_PATH");
if(sdkPath == null)
{
    home = System.getenv("HOME");
    sdkPath = home + "/ti/mcu_plus_sdk_am275/mcu_plus_sdk";
}

// !!! EDIT THIS !!! Edit all of below based on the example you want to run
runAfterLoad = true;
examplePath = "examples/drivers/ipc/ipc_rpmsg_echo";
exampleName = "ipc_rpmsg_echo";
os = "freertos";
profile = "release"
board = "am275x-evm";

// !!! EDIT THIS !!! Comment lines below for CPUs that are not needed to be loaded, change 'os' based on the 'os' combo that you have
var elf_file = {};
elf_file["wkup-r5fss0-0"] = makeElfFileName("wkup-r5fss0-0", os, "ti-arm-clang", exampleName, profile);
elf_file["r5fss0-0"] = makeElfFileName("r5fss0-0", os, "ti-arm-clang", exampleName, profile);
elf_file["r5fss0-1"] = makeElfFileName("r5fss0-1", os, "ti-arm-clang", exampleName, profile);
elf_file["r5fss1-0"] = makeElfFileName("r5fss1-0", os, "ti-arm-clang", exampleName, profile);
elf_file["r5fss1-1"] = makeElfFileName("r5fss1-1", os, "ti-arm-clang", exampleName, profile);
elf_file["c75ss0-0"] = makeElfFileName("c75ss0-0", os, "ti-c7000", exampleName, profile);
elf_file["c75ss1-0"] = makeElfFileName("c75ss1-0", os, "ti-c7000", exampleName, profile);

function makeElfFileName(cpu, os, compiler, exampleName, profile)
{
    return sdkPath + "/" + examplePath + "/" + board + "/" + cpu + "_" + os + "/" + compiler + "/" + exampleName + "." + profile + ".out"
}

function connectHaltResetCpu(cpu)
{
    dsCPU[cpu].target.connect();
    dsCPU[cpu].target.halt();
    dsCPU[cpu].target.reset();
}

function runCpu(cpu)
{
    if(cpu && File(elf_file[cpu]).isFile() )
    {
        if(runAfterLoad)
        {
            print("[" + cpu + "] Running ... ");
            dsCPU[cpu].target.runAsynch();
        }
    }
}

function loadCpu(cpu)
{
    if(cpu && File(elf_file[cpu]).isFile() )
    {
        print("[" + cpu + "] Loading ... " + elf_file[cpu] );
        dsCPU[cpu].memory.loadProgram(elf_file[cpu]);
    }
    else
    {
        print("[" + cpu + "] Skipping load, " + elf_file[cpu] + " file NOT FOUND ... " );
    }
}

function doEverything()
{
    /* Set timeout of in units of msecs */
    script.setScriptTimeout(10 * 1000);

    // Open a debug session
    dsCPU["wkup-r5fss0-0"] = debugServer.openSession( ".*WKUP_R5F_0" );
    dsCPU["r5fss0-0"] = debugServer.openSession( ".*MAIN_Cortex_R5_0_0" );
    dsCPU["r5fss0-1"] = debugServer.openSession( ".*MAIN_Cortex_R5_0_1" );
    dsCPU["r5fss1-0"] = debugServer.openSession( ".*MAIN_Cortex_R5_1_0" );
    dsCPU["r5fss1-1"] = debugServer.openSession( ".*MAIN_Cortex_R5_1_1" );
    dsCPU["c75ss0-0"] = debugServer.openSession( ".*C75X_0" );
    dsCPU["c75ss1-0"] = debugServer.openSession( ".*C75X_1" );

    print("Connecting, halting, reseting ...");
    connectHaltResetCpu("wkup-r5fss0-0");
    connectHaltResetCpu("r5fss0-0");
    connectHaltResetCpu("r5fss0-1");
    connectHaltResetCpu("r5fss1-0");
    connectHaltResetCpu("r5fss1-1");
    connectHaltResetCpu("c75ss0-0");
    connectHaltResetCpu("c75ss1-0");

    loadCpu("wkup-r5fss0-0");
    loadCpu("r5fss0-0");
    loadCpu("r5fss0-1");
    loadCpu("r5fss1-0");
    loadCpu("r5fss1-1");
    loadCpu("c75ss0-0");
    loadCpu("c75ss1-0");

    runCpu("r5fss0-1");
    runCpu("r5fss1-0");
    runCpu("r5fss1-1");
    runCpu("r5fss0-0");
    runCpu("c75ss0-0");
    runCpu("c75ss1-0");
    runCpu("wkup-r5fss0-0");

    print("All DONE !!!");
}

// Import the DSS packages into our namespace to save on typing
importPackage(Packages.com.ti.debug.engine.scripting)
importPackage(Packages.com.ti.ccstudio.scripting.environment)
importPackage(Packages.java.lang)
importPackage(java.io);
importPackage(java.lang);

var dsCPU = {};
var ds;
var debugServer;
var script;

// Check to see if running from within CCSv4 Scripting Console
var withinCCS = (ds !== undefined);

// Create scripting environment and get debug server if running standalone
if (!withinCCS)
{
    // Import the DSS packages into our namespace to save on typing
    importPackage(Packages.com.ti.debug.engine.scripting);
    importPackage(Packages.com.ti.ccstudio.scripting.environment);
    importPackage(Packages.java.lang);

    // Create our scripting environment object - which is the main entry point into any script and
    // the factory for creating other Scriptable ervers and Sessions
    script = ScriptingEnvironment.instance();

    // Get the Debug Server and start a Debug Session
    debugServer = script.getServer("DebugServer.1");

    debugServer.setConfig(fileCcxml);
    doEverything();
}
else // otherwise leverage existing scripting environment and debug server
{
    debugServer = ds;
    script = env;
    doEverything();
}
