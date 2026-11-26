let common = system.getScript("/common");

function populate_memory_regions(regionInst, ind, device, core){

    if(device == "J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x" || device == "J721S2") {

        // MCU1_0 (mcu-r5fss0-0) - 13 regions
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
                case 7:  // IPC_VRING_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "IPC_VRING_MEM";
                    regionInst.memory_region[ind].size = 0x2000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 8:  // APP_LOG_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_LOG_MEM";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 9:  // TIOVX_OBJ_DESC_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_OBJ_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x3FC0000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA040000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 10:  // APP_FILEIO_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_FILEIO_MEM";
                    regionInst.memory_region[ind].size = 0x400000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAE000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 11:  // DDR_MCU1_0_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU1_0_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 12:  // DDR_SHARED_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_SHARED_MEM";
                    regionInst.memory_region[ind].size = 0x20000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xC0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
            }
        }
        // MCU2_0 (r5fss0-0) - 15 regions
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
                    regionInst.memory_region[ind].type = "OCRAM_SHARED_ALL";
                    regionInst.memory_region[ind].$name = "MAIN_OCRAM_MCU2_0";
                    regionInst.memory_region[ind].size = 0x80000;
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
                    regionInst.memory_region[ind].size = 0x2000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 8:  // APP_LOG_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_LOG_MEM";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 9:  // TIOVX_OBJ_DESC_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_OBJ_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x3FC0000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA040000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 10:  // APP_FILEIO_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_FILEIO_MEM";
                    regionInst.memory_region[ind].size = 0x400000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAE000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 11:  // DDR_MCU2_0_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0xE00000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB9000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 12:  // DDR_MCU2_0_VISS_CONFIG_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_0_VISS_CONFIG_HEAP";
                    regionInst.memory_region[ind].size = 0x200000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB9E00000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 13:  // DDR_SHARED_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_SHARED_MEM";
                    regionInst.memory_region[ind].size = 0x20000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xC0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 14:  // TIOVX_LOG_RT_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_LOG_RT_MEM";
                    regionInst.memory_region[ind].size = 0x1C00000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAE400000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","mcu-r5fss0-1","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
            }
        }

        // MCU2_1 (r5fss0-1) - 13 regions
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
                case 3:  // DDR_MCU2_1_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA4000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU2_1_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA4100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU2_1
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1";
                    regionInst.memory_region[ind].size = 0x1EFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA4100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // IPC_VRING_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "IPC_VRING_MEM";
                    regionInst.memory_region[ind].size = 0x2000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 7:  // APP_LOG_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_LOG_MEM";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 8:  // TIOVX_OBJ_DESC_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_OBJ_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x3FC0000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA040000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 9:  // APP_FILEIO_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_FILEIO_MEM";
                    regionInst.memory_region[ind].size = 0x400000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAE000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 10:  // DDR_MCU2_1_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU2_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x1000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 11:  // DDR_SHARED_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_SHARED_MEM";
                    regionInst.memory_region[ind].size = 0x20000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xC0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss1-0","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;

                case 12:  // MAIN_OCRAM_MCU2_1
                    regionInst.memory_region[ind].type = "OCRAM_SHARED_ALL";
                    regionInst.memory_region[ind].$name = "MAIN_OCRAM_MCU2_1";
                    regionInst.memory_region[ind].size = 0x00080000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x60080000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
            }
        }

        // MCU3_0 (r5fss1-0) - 12 regions
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
                    regionInst.memory_region[ind].manualStartAddress = 0xA6000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU3_0_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA6100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU3_0
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA6100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // IPC_VRING_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "IPC_VRING_MEM";
                    regionInst.memory_region[ind].size = 0x2000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 7:  // APP_LOG_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_LOG_MEM";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 8:  // TIOVX_OBJ_DESC_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_OBJ_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x3FC0000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA040000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 9:  // APP_FILEIO_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_FILEIO_MEM";
                    regionInst.memory_region[ind].size = 0x400000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAE000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
                case 10:  // DDR_MCU3_0_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_0_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBB000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 11:  // DDR_SHARED_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_SHARED_MEM";
                    regionInst.memory_region[ind].size = 0x20000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xC0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-1","c71ss0-0","c71ss1-0"];
                    break;
            }
        }

        // MCU3_1 (r5fss1-1) - 12 regions
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
                    regionInst.memory_region[ind].manualStartAddress = 0xA7000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 4:  // DDR_MCU3_1_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA7100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 5:  // DDR_MCU3_1
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1";
                    regionInst.memory_region[ind].size = 0xEFFC00;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA7100400;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 6:  // IPC_VRING_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "IPC_VRING_MEM";
                    regionInst.memory_region[ind].size = 0x2000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","c71ss0-0","c71ss1-0"];
                    break;
                case 7:  // APP_LOG_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_LOG_MEM";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","c71ss0-0","c71ss1-0"];
                    break;
                case 8:  // TIOVX_OBJ_DESC_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_OBJ_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x3FC0000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA040000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","c71ss0-0","c71ss1-0"];
                    break;
                case 9:  // APP_FILEIO_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_FILEIO_MEM";
                    regionInst.memory_region[ind].size = 0x400000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAE000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","c71ss0-0","c71ss1-0"];
                    break;
                case 10:  // DDR_MCU3_1_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_MCU3_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x800000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xBB800000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 11:  // DDR_SHARED_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_SHARED_MEM";
                    regionInst.memory_region[ind].size = 0x20000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xC0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","c71ss0-0","c71ss1-0"];
                    break;
            }
        }

        // C7X_1 (c71ss0-0) - 16 regions
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
                case 2:  // IPC_VRING_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "IPC_VRING_MEM";
                    regionInst.memory_region[ind].size = 0x2000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss1-0"];
                    break;
                case 3:  // APP_LOG_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_LOG_MEM";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss1-0"];
                    break;
                case 4:  // TIOVX_OBJ_DESC_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_OBJ_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x3FC0000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA040000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss1-0"];
                    break;
                case 5:  // APP_FILEIO_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_FILEIO_MEM";
                    regionInst.memory_region[ind].size = 0x400000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAE000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss1-0"];
                    break;
                case 6:  // DDR_C7x_1_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 7:  // DDR_C7x_1_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB0100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 8:  // DDR_C7x_1_BOOT
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_BOOT";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB0200000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 9:  // DDR_C7x_1_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB0400000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 10:  // DDR_C7x_1_SECURE_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1_SECURE_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB0600000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 11:  // DDR_C7x_1
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_1";
                    regionInst.memory_region[ind].size = 0x59FC000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB0604000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 12:  // DDR_SHARED_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_SHARED_MEM";
                    regionInst.memory_region[ind].size = 0x20000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xC0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss1-0"];
                    break;
                case 13:  // DDR_C7X_1_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x10000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x117000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 14:  // DDR_C7X_1_SCRATCH
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_1_SCRATCH";
                    regionInst.memory_region[ind].size = 0x17000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x100000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 15:  // MSMC_C7x_1
                    regionInst.memory_region[ind].type = "MSMC_SHARED_ALL";
                    regionInst.memory_region[ind].$name = "MSMC_C7x_1";
                    regionInst.memory_region[ind].size = 0x3C8000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x70020000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
            }
        }

        // C7X_2 (c71ss1-0) - 15 regions
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
                case 2:  // IPC_VRING_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "IPC_VRING_MEM";
                    regionInst.memory_region[ind].size = 0x2000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xA8000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0"];
                    break;
                case 3:  // APP_LOG_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_LOG_MEM";
                    regionInst.memory_region[ind].size = 0x40000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0"];
                    break;
                case 4:  // TIOVX_OBJ_DESC_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "TIOVX_OBJ_DESC_MEM";
                    regionInst.memory_region[ind].size = 0x3FC0000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAA040000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0"];
                    break;
                case 5:  // APP_FILEIO_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "APP_FILEIO_MEM";
                    regionInst.memory_region[ind].size = 0x400000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xAE000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0"];
                    break;
                case 6:  // DDR_C7x_2_IPC
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2_IPC";
                    regionInst.memory_region[ind].size = 0x100000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 7:  // DDR_C7x_2_RESOURCE_TABLE
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2_RESOURCE_TABLE";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6100000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 8:  // DDR_C7x_2_BOOT
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2_BOOT";
                    regionInst.memory_region[ind].size = 0x400;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6200000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 9:  // DDR_C7x_2_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6400000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 10:  // DDR_C7x_2_SECURE_VECS
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2_SECURE_VECS";
                    regionInst.memory_region[ind].size = 0x4000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6600000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 11:  // DDR_C7x_2
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_C7x_2";
                    regionInst.memory_region[ind].size = 0x19FC000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xB6604000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 12:  // DDR_SHARED_MEM (shared)
                    regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                    regionInst.memory_region[ind].$name = "DDR_SHARED_MEM";
                    regionInst.memory_region[ind].size = 0x20000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0xC0000000;
                    regionInst.memory_region[ind].attributes = ["R", "W"];
                    regionInst.memory_region[ind].isShared = true;
                    regionInst.memory_region[ind].shared_cores = ["mcu-r5fss0-0","r5fss0-0","r5fss0-1","r5fss1-0","r5fss1-1","c71ss0-0"];
                    break;
                case 13:  // DDR_C7X_2_LOCAL_HEAP
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_LOCAL_HEAP";
                    regionInst.memory_region[ind].size = 0x1000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x127000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
                case 14:  // DDR_C7X_2_SCRATCH
                    regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                    regionInst.memory_region[ind].addressType = "Virtual";
                    regionInst.memory_region[ind].$name = "DDR_C7X_2_SCRATCH";
                    regionInst.memory_region[ind].size = 0x4000000;
                    regionInst.memory_region[ind].auto = false;
                    regionInst.memory_region[ind].manualStartAddress = 0x128000000;
                    regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                    break;
            }
        }

    }
}

exports.populate_memory_regions = populate_memory_regions;
