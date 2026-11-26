let common = system.getScript("/common");

function populate_memory_regions(regionInst, ind, device, core){

    if(device == "J721E_DRA829_TDA4VM_AM752x" || device == "J721E") {

        // MCU1_0 (mcu-r5fss0-0) - 8 regions
        if(core.includes("mcu-r5fss0-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 2:
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 3:
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010040;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 4:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_0_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 5:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_0_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA0100000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 6:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_0";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA0100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 7:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_0_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xD8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                
            }
        }
        // MCU2_0 (r5fss0-0) - 16 regions
        else if(core.includes("r5fss0-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 2:
                    regionInst.memory_region[ind].type = "OCRAM_SHARED_ALL";
                    regionInst.memory_region[ind].$name = "OCRAM_MCU2_0";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x03600000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 3:
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 4:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA2000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 5:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA2100000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 6:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0";
                    regionInst.memory_region[ind].size = 0x1EFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA2100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 7:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "IPC_VRING_MEM";
                    regionInst.memory_region[ind].size = 0x2000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
                    break;
                case 8:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_LOG_MEM";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAC000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
                    break;
                case 9:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_OBJ_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x3FC0000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAC040000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
                    break;
                case 10:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_FILEIO_MEM";
                    regionInst.memory_region[ind].size = 0x400000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
                    break;
                case 11:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_SHARED_MEM";
                    regionInst.memory_region[ind].size = 0x20000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
                    break;
                case 12:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0xF00000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xD9000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 13:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0_RESERVED";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xD9F00000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 14:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "INTERCORE_ETH_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xE4000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
                    break;
                case 15:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "INTERCORE_ETH_DATA_MEM";
                    regionInst.memory_region[ind].size = 0x1800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xE4800000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
                    break;
                case 16:  // TIOVX_LOG_RT_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_LOG_RT_MEM";
                    regionInst.memory_region[ind].size = 0x01C00000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB0400000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c66ss0-0","c66ss1-0","c71ss0-0"];
                    break;
            }
        }
        // MCU2_1 (r5fss0-1) - 8 regions
        else if(core.includes("r5fss0-1")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 2:
                    regionInst.memory_region[ind].type = "OCRAM_SHARED_ALL";
                    regionInst.memory_region[ind].$name = "OCRAM_MCU2_1";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x03640000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 3:
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 4:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA4000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 5:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA4100000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 6:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1";
                    regionInst.memory_region[ind].size = 0x1EFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA4100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 7:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x1000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xDA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                
            }
        }
        // MCU3_0 (r5fss1-0) - 7 regions
        else if(core.includes("r5fss1-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 2:
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 3:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA6000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 4:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA6100000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 5:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA6100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 6:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xDB000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                
            }
        }
        // MCU3_1 (r5fss1-1) - 7 regions
        else if(core.includes("r5fss1-1")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA_VECS";
                    regionInst.memory_region[ind].size = 0x40;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000000;
                    regionInst.memory_region[ind].attributes = ["X"];
                    break;
                case 1:
                    regionInst.memory_region[ind].type = "TCMA_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMA";
                    regionInst.memory_region[ind].size = 0x7FC0;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00000040;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 2:
                    regionInst.memory_region[ind].type = "TCMB_R5F";
                    regionInst.memory_region[ind].$name = "R5F_TCMB0";
                    regionInst.memory_region[ind].size = 0x8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x41010000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 3:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA7000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 4:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA7100000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 5:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA7100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 6:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xDB800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                
            }
        }
        // C7X_1 (c71ss0-0) - 11 regions
        else if(core.includes("c71ss0-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:
                    regionInst.memory_region[ind].type = "L2_C71SS0-0";
                    regionInst.memory_region[ind].$name = "L2RAM_C7x_1";
                    regionInst.memory_region[ind].size = 0x70000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x64800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 1:
                    regionInst.memory_region[ind].type = "L1_C71SS0-0";
                    regionInst.memory_region[ind].$name = "L1RAM_C7x_1";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x64E00000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 2:
                    regionInst.memory_region[ind].type = "MSMC_C71SS0-0";
                    regionInst.memory_region[ind].$name = "MSMC_C7x_1";
                    regionInst.memory_region[ind].size = 0x7C8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x70020000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 3:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 4:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2100000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 5:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_BOOT";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2200000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 6:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2400000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 7:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_SECURE_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2600000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 8:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1";
                    regionInst.memory_region[ind].size = 0x59FC000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB2604000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 9:
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_SCRATCH";
                    regionInst.memory_region[ind].size = 0x17000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x100000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 10:
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x117000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
            }
        }
        // C66X_1 (c66ss0-0) - 7 regions
        else if(core.includes("c66ss0-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:
                    regionInst.memory_region[ind].type = "L2_C66SS0-0";
                    regionInst.memory_region[ind].$name = "L2RAM_C66x_1";
                    regionInst.memory_region[ind].size = 0x38000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 1:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66X_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA9000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 2:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66X_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8100000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 3:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66X_1";
                    regionInst.memory_region[ind].size = 0xDFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8200400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 4:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66X_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x1000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xDC000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 5:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66X_1_SCRATCH";
                    regionInst.memory_region[ind].size = 0x3000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xDD000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 6:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66x_1_BOOT";
                    regionInst.memory_region[ind].size = 0x00000400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8200000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
            }
        }
        // C66X_2 (c66ss1-0) - 7 regions
        else if(core.includes("c66ss1-0")){
            let coreNames = common.getSysCfgCoreNames();
            switch(ind) {
                case 0:
                    regionInst.memory_region[ind].type = "L2_C66SS1-0";
                    regionInst.memory_region[ind].$name = "L2RAM_C66x_2";
                    regionInst.memory_region[ind].size = 0x38000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x00800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 1:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66X_2_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 2:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66X_2_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA9100000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
                case 3:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66X_2";
                    regionInst.memory_region[ind].size = 0xDFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA9200400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 4:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66X_2_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x1000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xE0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 5:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66X_2_SCRATCH";
                    regionInst.memory_region[ind].size = 0x3000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xE1000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X"];
                    break;
                case 6:
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C66x_2_BOOT";
                    regionInst.memory_region[ind].size = 0x00000400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA9200000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    break;
            }
        }
    }
}

exports.populate_memory_regions = populate_memory_regions;
