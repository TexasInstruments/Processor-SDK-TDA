
#ifndef APP_FC_CFG_H_
#define APP_FC_CFG_H_

Vhwa_M2mFcGraphPathInfo gPathInfo[] =
{
    /* Start Path DDR->VISS->DDR YUV420 */
    /* 0 */
    {
        3,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
        }
    },
    /* 1 */
    /* Start Path DDR->MSC->DDR YUV420 */
    {
        4,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_MSC0_IN_1},            
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
        }
    },
    /* Start Path DDR->VISS->MSC->DDR Luma 8 */
    /* 2 */
    {
        3,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
        }
    },
    /* 3 */
    /* Start Path DDR->VISS->MSC->DDR Chroma 8 */
    {
        3,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
        }
    },
    /* 4 */
    /* Start Path DDR->VISS->MSC0/1->DDR YUV420 8 */
    {
        5,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC1_IN_0},
            {VHWA_FC_PORT_MSC1_OUT_1, VHWA_FC_PORT_DDR},
        }
    },
    /* 5 */
    /* Start Path DDR->VISS->MSC0->DDR YUV420 8 */
    {
        5,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC0_IN_1},            
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
        }
    },
    /* 6 */
    /* Y8 -> MSC0_IN_0 and Y12 -> DDR */
    {
        5,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
        }
    },
    /* 7 */
    /* UV8 -> MSC0_IN_0 and UV12 -> DDR */    
    {
        5,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC0_IN_0},          
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_DDR},            
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
        }
    },
    /* 8 */
    /* Y8,UV8 -> MSC0/1_IN_0 and Y12,UV12 -> DDR */    
    {
        9,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_DDR},                  
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC1_IN_0},            
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_3, VHWA_FC_PORT_DDR},            
        }
    },
    /* 9 */
    /* Y12,UV12 -> MSC0/1_IN_0 and Y8,UV8 -> DDR */        
    {
        9,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},                  
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_MSC1_IN_0},            
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_3, VHWA_FC_PORT_DDR},         
        }
    },
    /* 10 */
    /* Y8,UV8 -> MSC0_IN_0/1 and Y12,UV12 -> DDR */        
    {
        9,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_DDR},                  
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC0_IN_1},            
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_3, VHWA_FC_PORT_DDR},   
        }
    },
    /* 11 */
    /* Y12,UV12 -> MSC0_IN_0/1 and Y8,UV8 -> DDR */        
    {
        9,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},                  
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_MSC0_IN_1},            
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_3, VHWA_FC_PORT_DDR},   
        }
    },      
    /* 12 */
    /* Y8,UV8 -> MSC1_IN_0/1 and Y12,UV12 -> DDR */        
    {
        9,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_MSC1_IN_0},
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_DDR},                  
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC1_IN_1},            
            {VHWA_FC_PORT_MSC1_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_3, VHWA_FC_PORT_DDR},   
        }
    },
    /* 13 */
    /* Y12,UV8 -> MSC1_IN_0/1 and Y8,UV12 -> DDR */        
    {
        9,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_MSC1_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_DDR},            
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC1_IN_1},            
            {VHWA_FC_PORT_MSC1_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_3, VHWA_FC_PORT_DDR},   
        }
    },  
    /* 14 */
    /* Y12,UV8,Y8,UV12 -> MSC0/1_IN_0/1 */        
    {
        13,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_MSC1_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_MSC1_IN_1},            
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC0_IN_1},            
            {VHWA_FC_PORT_MSC1_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_3, VHWA_FC_PORT_DDR},   
            {VHWA_FC_PORT_MSC0_OUT_4, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_5, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_6, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_7, VHWA_FC_PORT_DDR},               
        }
    },  
    /* 15 */
    /* Y12,UV12 -> MSC0/1_IN_0 10 outputs */        
    {
        15,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_MSC1_IN_0},            
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
            {VHWA_FC_PORT_MSC1_OUT_5, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_6, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_7, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_8, VHWA_FC_PORT_DDR},   
            {VHWA_FC_PORT_MSC1_OUT_9, VHWA_FC_PORT_DDR},   
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_3, VHWA_FC_PORT_DDR},               
            {VHWA_FC_PORT_MSC0_OUT_4, VHWA_FC_PORT_DDR},               
        }
    },  
    /* 16 */
    /* Y12,UV12 -> MSC0_IN_0/1 10 outputs and Y8,UV8 -> DDR */        
    {
        15,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_MSC0_IN_1},            
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_3, VHWA_FC_PORT_DDR},   
            {VHWA_FC_PORT_MSC0_OUT_4, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_5, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_6, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_7, VHWA_FC_PORT_DDR},     
            {VHWA_FC_PORT_MSC0_OUT_8, VHWA_FC_PORT_DDR},     
            {VHWA_FC_PORT_MSC0_OUT_9, VHWA_FC_PORT_DDR},                                       
        }
    },
    /* 17 */
    /* Y12,UV8,Y8,UV12 -> MSC1_IN_0/1 10 outputs and Y8,UV8 -> DDR */        
    {
        15,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_MSC1_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_MSC1_IN_1},            
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
            {VHWA_FC_PORT_MSC1_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_3, VHWA_FC_PORT_DDR},   
            {VHWA_FC_PORT_MSC1_OUT_4, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_5, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_6, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_7, VHWA_FC_PORT_DDR},     
            {VHWA_FC_PORT_MSC1_OUT_8, VHWA_FC_PORT_DDR},     
            {VHWA_FC_PORT_MSC1_OUT_9, VHWA_FC_PORT_DDR},                  
        }
    },
    /* 18 */
    /* Y8,UV8 -> MSC0/1_IN_0 10 outputs */        
    {
        15,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC1_IN_0},            
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_DDR},            
            {VHWA_FC_PORT_MSC1_OUT_5, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_6, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_7, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_8, VHWA_FC_PORT_DDR},   
            {VHWA_FC_PORT_MSC1_OUT_9, VHWA_FC_PORT_DDR},   
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_3, VHWA_FC_PORT_DDR},               
            {VHWA_FC_PORT_MSC0_OUT_4, VHWA_FC_PORT_DDR},               
        }
    },
    /* 19 */
    /* Y12,UV12 -> MSC0_IN_0/1 6 outputs and Y8,UV8 -> DDR */        
    {
        11,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_MSC0_IN_1},            
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_3, VHWA_FC_PORT_DDR},   
            {VHWA_FC_PORT_MSC0_OUT_4, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_5, VHWA_FC_PORT_DDR},                                  
        }
    },
    /* 20 */
    /* Y12,UV12 -> MSC0_IN_0/1 6 outputs*/        
    {
        9,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_MSC0_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_MSC0_IN_1},       
            {VHWA_FC_PORT_MSC0_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_3, VHWA_FC_PORT_DDR},   
            {VHWA_FC_PORT_MSC0_OUT_4, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC0_OUT_5, VHWA_FC_PORT_DDR},                                  
        }
    },
    /* 21 */
    /* Y12,UV12 -> MSC1_IN_0/1 6 outputs and Y8,UV8 -> DDR */        
    {
        11,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_MSC1_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_MSC1_IN_1},            
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
            {VHWA_FC_PORT_MSC1_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_1, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_2, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_3, VHWA_FC_PORT_DDR},   
            {VHWA_FC_PORT_MSC1_OUT_4, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_5, VHWA_FC_PORT_DDR},                                  
        }
    },                 
    /* 22 */
    /* Y12,UV12 -> MSC1_IN_0/1 2 outputs and Y8,UV8 -> DDR */        
    {
        7,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y12, VHWA_FC_PORT_MSC1_IN_0},
            {VHWA_FC_PORT_VISS_OUT_UV12, VHWA_FC_PORT_MSC1_IN_1},            
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
            {VHWA_FC_PORT_MSC1_OUT_0, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_MSC1_OUT_1, VHWA_FC_PORT_DDR},                              
        }
    },  
    /* phase 3 - negative test case - startport = 8 , To cover Vhwa_getSrcNode() else*/
    /* 23 */
    {
        3,
        {
            {8, VHWA_FC_PORT_VISS_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
        }
    }, 
    /* endport = 8, To cover else part of Vhwa_getDesNode()*/
    /* 24 */
    {
        3,
        {
            {8, 8},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
        }
    },
    /* 25 firstNode = VHWA_FC_NODE_MSC1*/  
    {
        3,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_MSC1_IN_0},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
        }
    },  
    /* 26 To cover Vhwa_getDesNode(), endPort = VHWA_FC_PORT_VISS_IN_1*/
    {
        3,
        {
            {VHWA_FC_PORT_DDR, VHWA_FC_PORT_VISS_IN_1},           
            {VHWA_FC_PORT_VISS_OUT_Y8, VHWA_FC_PORT_DDR},
            {VHWA_FC_PORT_VISS_OUT_UV8, VHWA_FC_PORT_DDR},            
        }
    },         

};


AppFc_TestParams gAppFcTestPrms[] =
{
    /* 0: DDR->VISS->DDR start YUV420 */
    {
        .testName = "TC_000",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UFALSE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                 /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
                &gPathInfo[0],
                &gAppVissFcTestConfig[3],
                NULL,
                NULL
            }, {0}, {0}, {0}
        },
        .isEnableTest = UTRUE,                   /* isEnableTest */
        .vissIsSwitchGlbceCtx = UFALSE,          /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,           /* vissChCfgOnEachIter  */
    },
    /* 1: DDR->MSC->DDR start YUV420 */
    {
        .testName = "TC_001",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UFALSE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                 /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
                &gPathInfo[1],
                NULL,
                &gAppMscFcTestCfg[4],
                NULL
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,          /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,           /* vissChCfgOnEachIter  */
    },
    /* 2: DDR->VISS->MSC->DDR start Luma Only */
    {
        .testName = "TC_002",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                 /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
                &gPathInfo[2],
                &gAppVissFcTestConfig[0],
                &gAppMscFcTestCfg[0],
                NULL
            }, {0}, {0}, {0}
        },
        .isEnableTest = UTRUE,                   /* isEnableTest */
        .vissIsSwitchGlbceCtx = UFALSE,          /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,           /* vissChCfgOnEachIter  */
    },
    /* 3: DDR->VISS->MSC0->DDR Chroma only start */
    {
        .testName = "TC_003",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[3],
            &gAppVissFcTestConfig[1],
            &gAppMscFcTestCfg[1],       
            NULL,
            }, {0}, {0}, {0}
        },
        .isEnableTest = true,            /* isEnableTest */
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },
    /* 4: DDR->VISS->MSC0/MSC1->DDR YUV420 start */
    {
        .testName = "TC_004",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UTRUE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[4],
            &gAppVissFcTestConfig[2],
            &gAppMscFcTestCfg[2],
            &gAppMscFcTestCfg[3],
            }, {0}, {0}, {0}
        },
        .isEnableTest = true,            /* isEnableTest */
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },
    /* 5: DDR->VISS->MSC0->DDR YUV420 start */
    {
        .testName = "TC_005",                   /* Test Name */
        .numHandles = 2,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[5],
            &gAppVissFcTestConfig[3],
            &gAppMscFcTestCfg[4],
            NULL,
            }, 
            {
            &gPathInfo[5],
            &gAppVissFcTestConfig[3],
            &gAppMscFcTestCfg[4],
            NULL,
            }, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },
    /* 6: */
    {
        .testName = "TC_006",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[6],
            &gAppVissFcTestConfig[4],
            &gAppMscFcTestCfg[8],
            NULL,
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },    
    /* 7: */
    {
        .testName = "TC_007",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[7],
            &gAppVissFcTestConfig[5],
            &gAppMscFcTestCfg[9],
            NULL,
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },   
    /* 8: */
    {
        .testName = "TC_008",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UTRUE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[8],
            &gAppVissFcTestConfig[6],
            &gAppMscFcTestCfg[5],
            &gAppMscFcTestCfg[6],
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },
    /* 9: */
    {
        .testName = "TC_009",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UTRUE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[9],
            &gAppVissFcTestConfig[6],
            &gAppMscFcTestCfg[5],
            &gAppMscFcTestCfg[6],
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },
    /* 10: */
    {
        .testName = "TC_0010",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[10],
            &gAppVissFcTestConfig[7],
            &gAppMscFcTestCfg[7],
            NULL,
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },
    /* 11: */
    {
        .testName = "TC_0011",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[11],
            &gAppVissFcTestConfig[7],
            &gAppMscFcTestCfg[7],
            NULL,
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },   
    /* 12: */
    {
        .testName = "TC_0012",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UFALSE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UTRUE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[12],
            &gAppVissFcTestConfig[7],
            NULL,
            &gAppMscFcTestCfg[10],
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },   
    /* 13: */
    {
        .testName = "TC_0013",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UFALSE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UTRUE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[13],
            &gAppVissFcTestConfig[7],
            NULL,
            &gAppMscFcTestCfg[10],
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },   
    /* 14: */
    {
        .testName = "TC_0014",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UTRUE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[14],
            &gAppVissFcTestConfig[7],
            &gAppMscFcTestCfg[11],
            &gAppMscFcTestCfg[10],
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },   
    /* 15: */
    {
        .testName = "TC_0015",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UTRUE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[15],
            &gAppVissFcTestConfig[8],
            &gAppMscFcTestCfg[12],
            &gAppMscFcTestCfg[13],
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },  
    /* 16: */
    {
        .testName = "TC_0016",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[16],
            &gAppVissFcTestConfig[9],
            &gAppMscFcTestCfg[14],
            NULL,
            }, {0}, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },  
    /* 17: */
    {
        .testName = "TC_0017",                   /* Test Name */
        .numHandles = 2,                        /* Num Handles */
        .repeatCnt = 10,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UFALSE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UTRUE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[17],
            &gAppVissFcTestConfig[9],
            NULL,
            &gAppMscFcTestCfg[15],
            }, 
            {
            &gPathInfo[17],
            &gAppVissFcTestConfig[9],
            NULL,
            &gAppMscFcTestCfg[15],                
            }, {0}, {0}
        },
        /* Disabled for J721E */
        #if defined (VHWA_VPAC_IP_REV_VPAC)
        .isEnableTest = false,            /* isEnableTest */
        #else
        .isEnableTest = true,            /* isEnableTest */
        #endif
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },  
    /* 18: */
    {
        .testName = "TC_0018",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UTRUE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UTRUE,                  /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
            &gPathInfo[18],
            &gAppVissFcTestConfig[10],
            &gAppMscFcTestCfg[12],
            &gAppMscFcTestCfg[13],
            }, {0}, {0}, {0}
        },
        .isEnableTest = true,            /* isEnableTest */
        .vissIsSwitchGlbceCtx = UFALSE,   /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,    /* vissChCfgOnEachIter  */
    },
    /* 19: srcNode = VHWA_FC_NODE_NONE */
    {
        .testName = "TC_019",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UFALSE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                 /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
                &gPathInfo[23],
                &gAppVissFcTestConfig[3],
                NULL,
                NULL
            }, {0}, {0}, {0}
        },
        .isEnableTest = UTRUE,                   /* isEnableTest */
        .vissIsSwitchGlbceCtx = UFALSE,          /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,           /* vissChCfgOnEachIter  */
    },
    /* 20: desNode = VHWA_FC_NODE_NONE */
    {
        .testName = "TC_020",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UFALSE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                 /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
                &gPathInfo[24],
                &gAppVissFcTestConfig[3],
                NULL,
                NULL
            }, {0}, {0}, {0}
        },
        .isEnableTest = UTRUE,                   /* isEnableTest */
        .vissIsSwitchGlbceCtx = UFALSE,          /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,           /* vissChCfgOnEachIter  */
    },
    /* 21: VHWA_FC_NODE_MSC1, copy of TC 0*/
     {
        .testName = "TC_021",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UFALSE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UTRUE,                 /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
                &gPathInfo[25],
                &gAppVissFcTestConfig[3],
                NULL,
                NULL
            }, {0}, {0}, {0}
        },
        .isEnableTest = UTRUE,                   /* isEnableTest */
        .vissIsSwitchGlbceCtx = UFALSE,          /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,           /* vissChCfgOnEachIter  */
    },
    /* 22 Positive, copy of 0 */
    {
        .testName = "TC_022",                   /* Test Name */
        .numHandles = 1,                        /* Num Handles */
        .repeatCnt = 3,                         /* Repeate Count */
        .isPerformanceTest = UFALSE,             /* Is Performance */
        .isVissEnabled = UTRUE,                  /* isVissEnabled */
        .isMsc0Enabled = UFALSE,                  /* Is Msc0Enabled */
        .isMsc1Enabled = UFALSE,                 /* isMsc1Enabled */
        .testCfg =                              /* Test Config */
        {
            {
                &gPathInfo[26],
                &gAppVissFcTestConfig[3],
                NULL,
                NULL
            }, {0}, {0}, {0}
        },
        .isEnableTest = UTRUE,                   /* isEnableTest */
        .vissIsSwitchGlbceCtx = UFALSE,          /* vissIsSwitchGlbceCtx */
        .vissChCfgOnEachIter = UFALSE,           /* vissChCfgOnEachIter  */
    },

};
#endif
