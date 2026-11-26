let common = system.getScript("/common");
                                                                                                                                                                                                            
  function populate_memory_regions(regionInst, ind, device, core){                                                                                                                                          
                                                                                                                                                                                                            
      if(device == "J722S_TDA4VEN_TDA4AEN_AM67" || device == "J722S") {                                                                                                                                     
   
          // MCU2_0 (main-r5fss0-0) - 16 regions                                                                                                                                                            
          if(core.includes("main-r5fss0-0")){
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
                  case 1:
                      regionInst.memory_region[ind].type = "TCMA_R5F";  // R5F_TCMA                                                                                                                                                                      
                      regionInst.memory_region[ind].$name = "R5F_TCMA";                                                                                                                                  
                      regionInst.memory_region[ind].size = 0x7FC0;                                                                                                                                          
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x00000040;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["X"];                                                                                                                                     
                      break;                                                                                                                                                                                
                  case 2:  // R5F_TCMB0    
                      regionInst.memory_region[ind].type = "TCMB_R5F";                                                                                                                                                                 
                      regionInst.memory_region[ind].$name = "R5F_TCMB";                                                                                                                                    
                      regionInst.memory_region[ind].size = 0x8000;                                                                                                                                          
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x41010000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 3:  // DDR_MCU2_0_IPC                                                                                                                                                                
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_MCU2_0_IPC";
                      regionInst.memory_region[ind].size = 0x100000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;
                      regionInst.memory_region[ind].manualStartAddress = 0xA2000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 4:  // DDR_MCU2_0_RESOURCE_TABLE                                                                                                                                                     
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_MCU2_0_RESOURCE_TABLE";
                      regionInst.memory_region[ind].size = 0x400;                                                                                                                                           
                      regionInst.memory_region[ind].auto = false;
                      regionInst.memory_region[ind].manualStartAddress = 0xA2100000;
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                      break;
                  case 5:  // DDR_MCU2_0_IPC_TRACE                                                                                                                                                          
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_MCU2_0_IPC_TRACE";                                                                                                                          
                      regionInst.memory_region[ind].size = 0xFFC00;                                                                                                                                         
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xA2100400;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 6:  // DDR_MCU2_0                                                                                                                                                                    
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_MCU2_0";                                                                                                                                    
                      regionInst.memory_region[ind].size = 0x1E00000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xA2200000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 7:  // IPC_VRING_MEM (shared)                                                                                                                                                        
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "IPC_VRING_MEM";                                                                                                                                 
                      regionInst.memory_region[ind].size = 0x1000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xA5000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W"];                                                                                                                                        
                      regionInst.memory_region[ind].isShared = true;
                      regionInst.memory_region[ind].shared_cores = ["wkup-r5fss0-0","c75ss0-0","c75ss1-0"];                                                                                                                                        
                                                                                                                                          
                      break;                                                                                                                                                                                
                  case 8:  // TIOVX_OBJ_DESC_MEM (shared)                                                                                                                                                   
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "TIOVX_OBJ_DESC_MEM";                                                                                                                            
                      regionInst.memory_region[ind].size = 0x800000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xA7000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W"];                                                                                                                                        
                      regionInst.memory_region[ind].isShared = true;
                      regionInst.memory_region[ind].shared_cores = ["wkup-r5fss0-0","c75ss0-0","c75ss1-0"];

                      break;                                                                                                                                                                                
                  case 9:  // APP_FILEIO_MEM (shared)                                                                                                                                                       
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "APP_FILEIO_MEM";                                                                                                                                
                      regionInst.memory_region[ind].size = 0x400000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xA7800000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W"];                                                                                                                                        
                      regionInst.memory_region[ind].isShared = true;
                      regionInst.memory_region[ind].shared_cores = ["wkup-r5fss0-0","c75ss0-0","c75ss1-0"];                                                                                                                                        
                      
                      break;                                                                                                                                                                                
                  case 10:  // APP_LOG_MEM (shared)                                                                                                                                                         
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "APP_LOG_MEM";                                                                                                                                   
                      regionInst.memory_region[ind].size = 0x40000;                                                                                                                                         
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xA7C00000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W"];                                                                                                                                        
                      regionInst.memory_region[ind].isShared = true;
                      regionInst.memory_region[ind].shared_cores = ["wkup-r5fss0-0","c75ss0-0","c75ss1-0"];

                      break;
                  case 11:  // DDR_MCU2_0_LOCAL_HEAP
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_MCU2_0_LOCAL_HEAP";                                                                                                                         
                      regionInst.memory_region[ind].size = 0x2000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB5800000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 12:  // DDR_MCU2_0_VISS_CONFIG_HEAP                                                                                                                                                  
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_MCU2_0_VISS_CONFIG_HEAP";                                                                                                                   
                      regionInst.memory_region[ind].size = 0x400000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB7800000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 13:  // INTERCORE_ETH_DESC_MEM                                                                                                                                                       
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "INTERCORE_ETH_DESC_MEM";                                                                                                                        
                      regionInst.memory_region[ind].size = 0x200000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB8000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W"];                                                                                                                                        
                      break;                                                                                                                                                                                
                  case 14:  // INTERCORE_ETH_DATA_MEM                                                                                                                                                       
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "INTERCORE_ETH_DATA_MEM";                                                                                                                        
                      regionInst.memory_region[ind].size = 0xE00000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB8200000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W"];                                                                                                                                        
                      break;
                  case 15:  // DDR_SHARED_MEM (shared with all cores)                                                                                                                                       
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_SHARED_MEM";                                                                                                                                
                      regionInst.memory_region[ind].size = 0x20000000;                                                                                                                                      
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xC0000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R"];                                                                                                                                        
                      regionInst.memory_region[ind].isShared = true;
                      regionInst.memory_region[ind].shared_cores = ["wkup-r5fss0-0","c75ss0-0","c75ss1-0"];                                                                                                                                        
                      break;
                      
                   case 16:  // DDR_SHARED_MEM (shared with all cores)                                                                                                                                       
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "TIOVX_LOG_RT_MEM";                                                                                                                                
                      regionInst.memory_region[ind].size = 0x01000000;                                                                                                                                      
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xA6000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                                        
                      regionInst.memory_region[ind].isShared = true;
                      regionInst.memory_region[ind].shared_cores = ["wkup-r5fss0-0","c75ss0-0","c75ss1-0"];                                                                                                                                        
                      break;
              }                                                                                                                                                                                             
          }                                                                                                                                                                                                 
   
          // MCU1_0 (wkup-r5fss0-0) - 9 regions (skip shared regions - created by main core)
          else if(core.includes("wkup-r5fss0-0")){
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
                  case 6:  // DDR_MCU1_0_IPC_TRACE                                                                                                                                                          
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_MCU1_0_IPC_TRACE";                                                                                                                          
                      regionInst.memory_region[ind].size = 0xFFC00;                                                                                                                                         
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xA0100400;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                      break;
                  case 7:  // DDR_MCU1_0
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_MCU1_0";
                      regionInst.memory_region[ind].size = 0xE00000;
                      regionInst.memory_region[ind].auto = false;
                      regionInst.memory_region[ind].manualStartAddress = 0xA0200000;
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                      break;
                  case 8:  // DDR_MCU1_0_LOCAL_HEAP                                                                                                                                                        
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_MCU1_0_LOCAL_HEAP";                                                                                                                         
                      regionInst.memory_region[ind].size = 0x800000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB5000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                                                                                                                                                                                                
              }                                                                                                                                                                                             
          }                                                                                                                                                                                                 
                                                                                                                                                                                                            
          // C7X_1 (c75ss0-0) - 18 regions (skip shared regions - created by main core)
          else if(core.includes("c75ss0-0")){
              let coreNames = common.getSysCfgCoreNames();
              switch(ind) {                                                                                                                                                                                 
                  case 0:  // L2RAM_C7x_1_MAIN
                      regionInst.memory_region[ind].type = "L3_C75";                                                                                                                                                              
                      regionInst.memory_region[ind].$name = "L2RAM_C7x_1_MAIN";                                                                                                                             
                      regionInst.memory_region[ind].size = 0x200000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x7E000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 1:  // L2RAM_C7x_1_AUX
                      regionInst.memory_region[ind].type = "L2_C75SS0-0";                                                                                                                                                               
                      regionInst.memory_region[ind].$name = "L2RAM_C7x_1_AUX";                                                                                                                              
                      regionInst.memory_region[ind].size = 0x3C000;                                                                                                                                         
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x7F000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 2:  // L2RAM_C7x_1_AUX_AS_L1
                      regionInst.memory_region[ind].type = "L1_C75SS0-0";                                                                                                                                                         
                      regionInst.memory_region[ind].$name = "L2RAM_C7x_1_AUX_AS_L1";                                                                                                                        
                      regionInst.memory_region[ind].size = 0x4000;                                                                                                                                          
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x7F03C000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break; 
                                                                                                                                                                                                 
                  case 3:  // DDR_C7x_1_IPC                                                                                                                                                                 
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_1_IPC";                                                                                                                                 
                      regionInst.memory_region[ind].size = 0x100000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;
                      regionInst.memory_region[ind].manualStartAddress = 0xAD000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 4:  // DDR_C7x_1_RESOURCE_TABLE                                                                                                                                                      
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_1_RESOURCE_TABLE";                                                                                                                      
                      regionInst.memory_region[ind].size = 0x400;                                                                                                                                           
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xAD100000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 5:  // DDR_C7x_1_IPC_TRACE                                                                                                                                                           
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_1_IPC_TRACE";                                                                                                                           
                      regionInst.memory_region[ind].size = 0xFFC00;                                                                                                                                         
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xAD100400;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 6:  // DDR_C7x_1_BOOT                                                                                                                                                               
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_1_BOOT";                                                                                                                                
                      regionInst.memory_region[ind].size = 0x400;                                                                                                                                           
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xAD200000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 7:  // DDR_C7x_1_VECS                                                                                                                                                               
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_1_VECS";                                                                                                                                
                      regionInst.memory_region[ind].size = 0x4000;                                                                                                                                          
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xAD400000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 8:  // DDR_C7x_1_SECURE_VECS                                                                                                                                                        
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_1_SECURE_VECS";                                                                                                                         
                      regionInst.memory_region[ind].size = 0x4000;                                                                                                                                          
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xAD600000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                      break;                                                                                                                                                                                
                  case 9:  // DDR_C7x_1
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_1";
                      regionInst.memory_region[ind].size = 0x39FC000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xAD604000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 10:  // DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE                                                                                                                                           
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE";                                                                                                            
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x100000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 11:  // DDR_C7X_1_SCRATCH_NON_CACHEABLE                                                                                                                                              
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_1_SCRATCH_NON_CACHEABLE";                                                                                                               
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x104000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 12:  // DDR_C7X_1_LOCAL_HEAP                                                                                                                                                         
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_1_LOCAL_HEAP";                                                                                                                          
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x108000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 13:  // DDR_C7X_1_SCRATCH                                                                                                                                                            
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_1_SCRATCH";                                                                                                                             
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x10C000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 14:  // DDR_C7X_1_2_LOCAL_HEAP_NON_CACHEABLE                                                                                                                                         
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_1_2_LOCAL_HEAP_NON_CACHEABLE";                                                                                                          
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x110000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 15:  // DDR_C7X_1_2_SCRATCH_NON_CACHEABLE
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_1_2_SCRATCH_NON_CACHEABLE";                                                                                                             
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x114000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 16:  // DDR_C7X_1_2_LOCAL_HEAP                                                                                                                                                       
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_1_2_LOCAL_HEAP";                                                                                                                        
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x118000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 17:  // DDR_C7X_1_2_SCRATCH                                                                                                                                                          
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_1_2_SCRATCH";                                                                                                                           
                      regionInst.memory_region[ind].size = 0x4000000;
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x11C000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
              }                                                                                                                                                                                             
          }                                                                                                                                                                                                 

          // C7X_2 (c75ss1-0) - 18 regions (same structure as c7x_1 but with c7x_2 names)
          else if(core.includes("c75ss1-0")){
              let coreNames = common.getSysCfgCoreNames();
              switch(ind) {                                                                                                                                                                                 
                  case 0:  // L2RAM_C7x_2_MAIN
                      regionInst.memory_region[ind].type = "L3_C75";                                                                                                                                                              
                      regionInst.memory_region[ind].$name = "L2RAM_C7x_2_MAIN";                                                                                                                             
                      regionInst.memory_region[ind].size = 0x200000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x7E200000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 1:  // L2RAM_C7x_2_AUX
                      regionInst.memory_region[ind].type = "L2_C75SS1-0";                                                                                                                                                               
                      regionInst.memory_region[ind].$name = "L2RAM_C7x_2_AUX";                                                                                                                              
                      regionInst.memory_region[ind].size = 0x3C000;                                                                                                                                         
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x7F800000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 2:  // L2RAM_C7x_2_AUX_AS_L1
                      regionInst.memory_region[ind].type = "L1_C75SS1-0";
                      regionInst.memory_region[ind].$name = "L2RAM_C7x_2_AUX_AS_L1";
                      regionInst.memory_region[ind].size = 0x4000;                                                                                                                                          
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x7F83C000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;
                                                                                                                                                                                                  
                  case 3:  // DDR_C7x_2_IPC                                                                                                                                                                 
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_2_IPC";                                                                                                                                 
                      regionInst.memory_region[ind].size = 0x100000;                                                                                                                                        
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB1000000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 4:  // DDR_C7x_2_RESOURCE_TABLE                                                                                                                                                      
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_2_RESOURCE_TABLE";                                                                                                                      
                      regionInst.memory_region[ind].size = 0x400;                                                                                                                                           
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB1100000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 5:  // DDR_C7x_2_IPC_TRACE                                                                                                                                                           
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_2_IPC_TRACE";                                                                                                                           
                      regionInst.memory_region[ind].size = 0xFFC00;                                                                                                                                         
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB1100400;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 6:  // DDR_C7x_2_BOOT                                                                                                                                                               
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_2_BOOT";                                                                                                                                
                      regionInst.memory_region[ind].size = 0x400;                                                                                                                                           
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB1200000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;
                  case 7:  // DDR_C7x_2_VECS                                                                                                                                                               
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_2_VECS";                                                                                                                                
                      regionInst.memory_region[ind].size = 0x4000;                                                                                                                                          
                      regionInst.memory_region[ind].auto = false;
                      regionInst.memory_region[ind].manualStartAddress = 0xB1400000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 8:  // DDR_C7x_2_SECURE_VECS                                                                                                                                                        
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_2_SECURE_VECS";                                                                                                                         
                      regionInst.memory_region[ind].size = 0x4000;                                                                                                                                          
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB1600000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 9:  // DDR_C7x_2                                                                                                                                                                    
                      regionInst.memory_region[ind].type = "DDR_LOW_ALL";
                      regionInst.memory_region[ind].$name = "DDR_C7x_2";                                                                                                                                     
                      regionInst.memory_region[ind].size = 0x39FC000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0xB1604000;                                                                                                                        
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 10:  // DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE";
                      regionInst.memory_region[ind].size = 0x4000000;
                      regionInst.memory_region[ind].auto = false;
                      regionInst.memory_region[ind].manualStartAddress = 0x110000000;
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;
                  case 11:  // DDR_C7X_2_SCRATCH_NON_CACHEABLE                                                                                                                                              
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_2_SCRATCH_NON_CACHEABLE";                                                                                                               
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x114000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 12:  // DDR_C7X_2_LOCAL_HEAP                                                                                                                                                         
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_2_LOCAL_HEAP";                                                                                                                          
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x118000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;
                  case 13:  // DDR_C7X_2_SCRATCH                                                                                                                                                            
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_2_SCRATCH";                                                                                                                             
                      regionInst.memory_region[ind].size = 0x4000000;
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x11C000000;
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                      break;
                  case 14:  // DDR_C7X_2_1_LOCAL_HEAP_NON_CACHEABLE                                                                                                                                         
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_2_1_LOCAL_HEAP_NON_CACHEABLE";                                                                                                          
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x100000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 15:  // DDR_C7X_2_1_SCRATCH_NON_CACHEABLE                                                                                                                                            
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_2_1_SCRATCH_NON_CACHEABLE";                                                                                                             
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x104000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
                  case 16:  // DDR_C7X_2_1_LOCAL_HEAP                                                                                                                                                       
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_2_1_LOCAL_HEAP";
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;
                      regionInst.memory_region[ind].manualStartAddress = 0x108000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];
                      break;
                  case 17:  // DDR_C7X_2_1_SCRATCH
                      regionInst.memory_region[ind].type = "DDR_HIGH_ALL";
                      regionInst.memory_region[ind].addressType = "Virtual";
                      regionInst.memory_region[ind].$name = "DDR_C7X_2_1_SCRATCH";
                      regionInst.memory_region[ind].size = 0x4000000;                                                                                                                                       
                      regionInst.memory_region[ind].auto = false;                                                                                                                                           
                      regionInst.memory_region[ind].manualStartAddress = 0x10C000000;                                                                                                                       
                      regionInst.memory_region[ind].attributes = ["R", "W", "X", "I"];                                                                                                                      
                      break;                                                                                                                                                                                
              }                                                                                                                                                                                             
          }                                                                                                                                                                                                 
      }
  }                                                                                                                                                                                                         

  exports.populate_memory_regions = populate_memory_regions; 