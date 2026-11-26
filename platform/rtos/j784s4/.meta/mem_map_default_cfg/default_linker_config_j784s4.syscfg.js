let common = system.getScript("/common");

function populate_memory_regions(regionInst, ind, device, core){

    if(device == "J784S4_TDA4AP_TDA4VP_TDA4AH_TDA4VH_AM69x" || device == "J784S4") {

        // MCU1_0 (mcu-r5fss0-0) - 8 regions
        if(core.includes("mcu-r5fss0-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // R5F_TCMA_VECS
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:  // R5F_TCMA
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 2:  // R5F_TCMB0_VECS
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // R5F_TCMB0
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010040;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU1_0_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_0_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU1_0_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_0_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA0100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_MCU1_0
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_0";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA0100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 7:  // DDR_MCU1_0_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_0_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                
            }
        }

        // MCU2_0 (r5fss0-0) - 16 regions (primary core, owns shared regions)
        else if(core.includes("r5fss0-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // R5F_TCMA_VECS
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:  // R5F_TCMA
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 2:  // R5F_TCMB0
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // MAIN_OCRAM_MCU2_0
                    regionInst.memory_region[ind].type = "OCRAM_ALL";
                    regionInst.memory_region[ind].$name = "MAIN_OCRAM_MCU2_0";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x60000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU2_0_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA2000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU2_0_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA2100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_MCU2_0
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0";
                    regionInst.memory_region[ind].size = 0x1EFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA2100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 7:  // IPC_VRING_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "IPC_VRING_MEM";
                    regionInst.memory_region[ind].size = 0x3000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAC000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0","c71ss3-0"];
                    break;
                case 8:  // APP_LOG_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_LOG_MEM";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAF000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0","c71ss3-0"];
                    break;
                case 9:  // TIOVX_OBJ_DESC_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_OBJ_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x1FC0000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAF040000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0","c71ss3-0"];
                    break;
                case 10:  // APP_FILEIO_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_FILEIO_MEM";
                    regionInst.memory_region[ind].size = 0x400000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB1000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0","c71ss3-0"];
                    break;
                case 11:  // DDR_MCU2_0_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x600000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBB000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 12:  // DDR_MCU2_0_VISS_CONFIG_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0_VISS_CONFIG_HEAP";
                    regionInst.memory_region[ind].size = 0x200000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBB600000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 13:  // INTERCORE_ETH_DESC_MEM
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "INTERCORE_ETH_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBE000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 14:  // INTERCORE_ETH_DATA_MEM
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "INTERCORE_ETH_DATA_MEM";
                    regionInst.memory_region[ind].size = 0x1800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBE800000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 15:  // DDR_SHARED_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_SHARED_MEM";
                    regionInst.memory_region[ind].size = 0x3C000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xC0000000;
                    regionInst.memory_region[ind].attributes = ["R"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","r5fss2-0","r5fss2-1","c71ss0-0","c71ss1-0","c71ss2-0","c71ss3-0"];
                    break;
            }
        }

        // MCU1_1 (mcu-r5fss0-1) - 8 regions
        else if(core.includes("mcu-r5fss0-1")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // R5F_TCMA_VECS
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:  // R5F_TCMA
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 2:  // R5F_TCMB0_VECS
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // R5F_TCMB0
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010040;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU1_1_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA1000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU1_1_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA1100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_MCU1_1
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_1";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA1100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 7:  // DDR_MCU1_1_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBA800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                
            }
        }

        // MCU2_1 (r5fss0-1) - 8 regions
        else if(core.includes("r5fss0-1")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // R5F_TCMA_VECS
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:  // R5F_TCMA
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 2:  // R5F_TCMB0
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // MAIN_OCRAM_MCU2_1
                    regionInst.memory_region[ind].type = "OCRAM_ALL";
                    regionInst.memory_region[ind].$name = "MAIN_OCRAM_MCU2_1";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x60040000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU2_1_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA4000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU2_1_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA4100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_MCU2_1
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA4100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                
                case 7:  // DDR_MCU2_1_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBB800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                
            }
        }

        // MCU3_0 (r5fss1-0) - 7 regions
        else if(core.includes("r5fss1-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // R5F_TCMA_VECS
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:  // R5F_TCMA
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 2:  // R5F_TCMB0
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // DDR_MCU3_0_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA5000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU3_0_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA5100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU3_0
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA5100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                
                case 6:  // DDR_MCU3_0_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBC000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                
            }
        }

        // MCU3_1 (r5fss1-1) - 7 regions
        else if(core.includes("r5fss1-1")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // R5F_TCMA_VECS
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:  // R5F_TCMA
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 2:  // R5F_TCMB0
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // DDR_MCU3_1_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA6000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU3_1_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA6100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU3_1
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA6100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_MCU3_1_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBC800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                
            }
        }

        // MCU4_0 (r5fss2-0) - 9 regions
        else if(core.includes("r5fss2-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // R5F_TCMA_VECS
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:  // R5F_TCMA
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 2:  // R5F_TCMB0
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // MAIN_OCRAM_MCU4_0
                    regionInst.memory_region[ind].type = "OCRAM_ALL";
                    regionInst.memory_region[ind].$name = "MAIN_OCRAM_MCU4_0";
                    regionInst.memory_region[ind].size = 0x80000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x60080000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU4_0_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU4_0_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA7000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU4_0_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU4_0_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA7100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_MCU4_0
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU4_0";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA7100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 7:  // DDR_MCU4_0_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU4_0_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x600000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBD000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 8:  // DDR_MCU4_0_VISS_CONFIG_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU4_0_VISS_CONFIG_HEAP";
                    regionInst.memory_region[ind].size = 0x200000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBD600000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                
            }
        }

        // MCU4_1 (r5fss2-1) - 7 regions
        else if(core.includes("r5fss2-1")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // R5F_TCMA_VECS
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:  // R5F_TCMA
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 2:  // R5F_TCMB0
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // DDR_MCU4_1_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU4_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU4_1_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU4_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU4_1
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU4_1";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_MCU4_1_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU4_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBD800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                
            }
        }

        // C7X_1 (c71ss0-0) - 25 regions
        else if(core.includes("c71ss0-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // L2RAM_C7x_1
                    regionInst.memory_region[ind].type = "L2_C71SS0-0";
                    regionInst.memory_region[ind].$name = "L2RAM_C7x_1";
                    regionInst.memory_region[ind].size = 0x70000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x64800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 1:  // L1RAM_C7x_1
                    regionInst.memory_region[ind].type = "L1_C71SS0-0";
                    regionInst.memory_region[ind].$name = "L1RAM_C7x_1";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x64E00000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 2:  // MSMC_C7x_1
                    regionInst.memory_region[ind].type = "MSMC_C71SS0-0";
                    regionInst.memory_region[ind].$name = "MSMC_C7x_1";
                    regionInst.memory_region[ind].size = 0x300000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x68000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // DDR_C7x_1_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_C7x_1_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_C7x_1_BOOT
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_BOOT";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2200000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_C7x_1_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2400000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 7:  // DDR_C7x_1_SECURE_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_SECURE_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2600000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 8:  // DDR_C7x_1
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1";
                    regionInst.memory_region[ind].size = 0x1800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 9:  // DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x100000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 10:  // DDR_C7X_1_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x110000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 11:  // DDR_C7X_1_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x120000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 12:  // DDR_C7X_1_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x130000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 13:  // DDR_C7X_1_2_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_2_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x140000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 14:  // DDR_C7X_1_3_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_3_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x150000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 15:  // DDR_C7X_1_4_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_4_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x160000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 16:  // DDR_C7X_1_2_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_2_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x170000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 17:  // DDR_C7X_1_3_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_3_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x180000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 18:  // DDR_C7X_1_4_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_4_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x190000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 19:  // DDR_C7X_1_2_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_2_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1A0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 20:  // DDR_C7X_1_3_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_3_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1B0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 21:  // DDR_C7X_1_4_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_4_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1C0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 22:  // DDR_C7X_1_2_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_2_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1D0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 23:  // DDR_C7X_1_3_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_3_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1E0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 24:  // DDR_C7X_1_4_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_4_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1F0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
            }
        }

        // C7X_2 (c71ss1-0) - 25 regions
        else if(core.includes("c71ss1-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // L2RAM_C7x_2
                    regionInst.memory_region[ind].type = "L2_C71SS1-0";
                    regionInst.memory_region[ind].$name = "L2RAM_C7x_2";
                    regionInst.memory_region[ind].size = 0x70000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x65800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 1:  // L1RAM_C7x_2
                    regionInst.memory_region[ind].type = "L1_C71SS1-0";
                    regionInst.memory_region[ind].$name = "L1RAM_C7x_2";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x65E00000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 2:  // MSMC_C7x_2
                    regionInst.memory_region[ind].type = "MSMC_C71SS1-0";
                    regionInst.memory_region[ind].$name = "MSMC_C7x_2";
                    regionInst.memory_region[ind].size = 0x300000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x69000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // DDR_C7x_2_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB4000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_C7x_2_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB4100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_C7x_2_BOOT
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2_BOOT";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB4200000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_C7x_2_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB4400000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 7:  // DDR_C7x_2_SECURE_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2_SECURE_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB4600000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 8:  // DDR_C7x_2
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2";
                    regionInst.memory_region[ind].size = 0x1800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB4800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 9:  // DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x100000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 10:  // DDR_C7X_2_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x110000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 11:  // DDR_C7X_2_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x120000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 12:  // DDR_C7X_2_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x130000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 13:  // DDR_C7X_2_1_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_1_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x140000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 14:  // DDR_C7X_2_3_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_3_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x150000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 15:  // DDR_C7X_2_4_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_4_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x160000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 16:  // DDR_C7X_2_1_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x170000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 17:  // DDR_C7X_2_3_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_3_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x180000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 18:  // DDR_C7X_2_4_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_4_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x190000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 19:  // DDR_C7X_2_1_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_1_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1A0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 20:  // DDR_C7X_2_3_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_3_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1B0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 21:  // DDR_C7X_2_4_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_4_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1C0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 22:  // DDR_C7X_2_1_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_1_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1D0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 23:  // DDR_C7X_2_3_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_3_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1E0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 24:  // DDR_C7X_2_4_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_4_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1F0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
            }
        }

        // C7X_3 (c71ss2-0) - 25 regions
        else if(core.includes("c71ss2-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // L2RAM_C7x_3
                    regionInst.memory_region[ind].type = "L2_C71SS2-0";
                    regionInst.memory_region[ind].$name = "L2RAM_C7x_3";
                    regionInst.memory_region[ind].size = 0x70000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x66800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 1:  // L1RAM_C7x_3
                    regionInst.memory_region[ind].type = "L1_C71SS2-0";
                    regionInst.memory_region[ind].$name = "L1RAM_C7x_3";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x66E00000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 2:  // MSMC_C7x_3
                    regionInst.memory_region[ind].type = "MSMC_C71SS2-0";
                    regionInst.memory_region[ind].$name = "MSMC_C7x_3";
                    regionInst.memory_region[ind].size = 0x300000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x6A000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // DDR_C7x_3_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_3_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_C7x_3_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_3_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_C7x_3_BOOT
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_3_BOOT";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6200000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_C7x_3_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_3_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6400000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 7:  // DDR_C7x_3_SECURE_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_3_SECURE_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6600000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 8:  // DDR_C7x_3
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_3";
                    regionInst.memory_region[ind].size = 0x1800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 9:  // DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x100000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 10:  // DDR_C7X_3_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x110000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 11:  // DDR_C7X_3_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x120000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 12:  // DDR_C7X_3_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x130000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 13:  // DDR_C7X_3_1_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_1_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x140000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 14:  // DDR_C7X_3_2_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_2_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x150000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 15:  // DDR_C7X_3_4_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_4_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x160000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 16:  // DDR_C7X_3_1_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x170000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 17:  // DDR_C7X_3_2_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_2_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x180000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 18:  // DDR_C7X_3_4_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_4_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x190000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 19:  // DDR_C7X_3_1_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_1_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1A0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 20:  // DDR_C7X_3_2_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_2_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1B0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 21:  // DDR_C7X_3_4_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_4_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1C0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 22:  // DDR_C7X_3_1_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_1_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1D0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 23:  // DDR_C7X_3_2_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_2_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1E0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 24:  // DDR_C7X_3_4_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_3_4_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1F0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
            }
        }

        // C7X_4 (c71ss3-0) - 25 regions
        else if(core.includes("c71ss3-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:  // L2RAM_C7x_4
                    regionInst.memory_region[ind].type = "L2_C71SS3-0";
                    regionInst.memory_region[ind].$name = "L2RAM_C7x_4";
                    regionInst.memory_region[ind].size = 0x70000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x67800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 1:  // L1RAM_C7x_4
                    regionInst.memory_region[ind].type = "L1_C71SS3-0";
                    regionInst.memory_region[ind].$name = "L1RAM_C7x_4";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x67E00000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 2:  // MSMC_C7x_4
                    regionInst.memory_region[ind].type = "MSMC_C71SS3-0";
                    regionInst.memory_region[ind].$name = "MSMC_C7x_4";
                    regionInst.memory_region[ind].size = 0x300000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x6B000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 3:  // DDR_C7x_4_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_4_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_C7x_4_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_4_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB8100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_C7x_4_BOOT
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_4_BOOT";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB8200000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // DDR_C7x_4_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_4_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB8400000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 7:  // DDR_C7x_4_SECURE_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_4_SECURE_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB8600000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 8:  // DDR_C7x_4
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_4";
                    regionInst.memory_region[ind].size = 0x1800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB8800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 9:  // DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x100000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 10:  // DDR_C7X_4_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x110000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 11:  // DDR_C7X_4_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x120000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 12:  // DDR_C7X_4_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x130000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 13:  // DDR_C7X_4_1_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_1_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x140000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 14:  // DDR_C7X_4_2_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_2_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x150000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 15:  // DDR_C7X_4_3_LOCAL_HEAP_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_3_LOCAL_HEAP_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x160000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 16:  // DDR_C7X_4_1_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x170000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 17:  // DDR_C7X_4_2_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_2_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x180000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 18:  // DDR_C7X_4_3_LOCAL_HEAP (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_3_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x190000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 19:  // DDR_C7X_4_1_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_1_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1A0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 20:  // DDR_C7X_4_2_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_2_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1B0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 21:  // DDR_C7X_4_3_SCRATCH_NON_CACHEABLE (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_3_SCRATCH_NON_CACHEABLE";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1C0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 22:  // DDR_C7X_4_1_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_1_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1D0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 23:  // DDR_C7X_4_2_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_2_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1E0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 24:  // DDR_C7X_4_3_SCRATCH (virtual)
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_4_3_SCRATCH";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x1F0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
            }
        }

    }
}

exports.populate_memory_regions = populate_memory_regions;
