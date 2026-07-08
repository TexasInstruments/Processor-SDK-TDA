#ifndef APP_VISS_FC_CFG_H_
#define APP_VISS_FC_CFG_H_

AppViss_TestConfig gAppVissFcTestConfig[] =
{
    /* 0, Used for Single Frame Input 720p StartColor B Luma only 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1280, 720, {1280*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS12_UNPACKED16
            },
            /* Output Parameters */
            {
                {
                    /* Output Enabled */
                    UFALSE,
                    /* Output Format */
                    {
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {UFALSE},
                        FVID2_DF_LUMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },
                /* H3A Output */
                {
                    UFALSE
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },
    /* 1, Used for Single Frame Input 720p StartColor B Chroma only 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1280, 720, {1280*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS12_UNPACKED16
            },
            /* Output Parameters */
            {
                {
                    /* Output Enabled */
                    UFALSE,
                    /* Output Format */
                    {
                    }
                },              
                {
                    UFALSE,
                    {
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },     
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720/2, {1280,0,0}, {UFALSE}, //TODO Check the height
                        FVID2_DF_CHROMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },                                           
                /* H3A Output */
                {
                    UFALSE
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },
    /* 2, Used for Single Frame Input 720p StartColor B Luma and Chroma 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1280, 720, {1280*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS12_UNPACKED16
            },
            /* Output Parameters */
            {  
                {
                    UFALSE,
                    {
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },       
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {UFALSE},
                        FVID2_DF_LUMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },                                
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720/2, {1280, 0, 0}, {UFALSE},
                        FVID2_DF_CHROMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },                              
                {
                    UFALSE,
                    {
                    }
                },                              
                /* H3A Output */
                {
                    UFALSE
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },    
    /* 3, Used for Single Frame Input 720p StartColor B YUV420 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1280, 720, {1280*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS12_UNPACKED16
            },
            /* Output Parameters */
            {
                {
                    /* Output Enabled */
                    UFALSE,
                    /* Output Format */
                    {
                    }
                },              
                {
                    UFALSE,
                    {
                    }
                },
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {UFALSE},
                        FVID2_DF_YUV420SP_UV, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },                                  
                {
                    UFALSE,
                    {
                    }
                },                              
                /* H3A Output */
                {
                    UTRUE,
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {FALSE},
                        FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },     
    /* 4, Used for Single Frame Input 720p StartColor B Luma only Y12 and Y8 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1280, 720, {1280*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS12_UNPACKED16
            },
            /* Output Parameters */
            {
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {UFALSE},
                        FVID2_DF_LUMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {UFALSE},
                        FVID2_DF_LUMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },
                /* H3A Output */
                {
                    UFALSE
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },
    /* 5, Used for Single Frame Input 720p StartColor B Chroma only UV12 and UV8 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1280, 720, {1280*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS12_UNPACKED16
            },
            /* Output Parameters */
            {
                {
                    /* Output Enabled */
                    UFALSE,
                    /* Output Format */
                    {
                    }
                },              
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720/2, {1280,0,0}, {UFALSE}, //TODO Check the height
                        FVID2_DF_CHROMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },     
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720/2, {1280,0,0}, {UFALSE}, //TODO Check the height
                        FVID2_DF_CHROMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },                                           
                /* H3A Output */
                {
                    UFALSE
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },
    /* 6, Used for Single Frame Input 720p StartColor B 2 Luma and 2 Chroma outputs 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1280, 720, {1280*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS12_UNPACKED16
            },
            /* Output Parameters */
            {  
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {UFALSE},
                        FVID2_DF_LUMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720/2, {1280, 0, 0}, {UFALSE},
                        FVID2_DF_CHROMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },       
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {UFALSE},
                        FVID2_DF_LUMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },                                
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720/2, {1280, 0, 0}, {UFALSE},
                        FVID2_DF_CHROMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },                              
                {
                    UFALSE,
                    {
                    }
                },                              
                /* H3A Output */
                {
                    UFALSE
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },    
    /* 7, Used for Single Frame Input 720p StartColor B 2-YUV420 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1280, 720, {1280*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS12_UNPACKED16
            },
            /* Output Parameters */
            {
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {UFALSE},
                        FVID2_DF_YUV420SP_UV, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },              
                {
                    UFALSE,
                    {
                    }
                },
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {UFALSE},
                        FVID2_DF_YUV420SP_UV, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },                                  
                {
                    UFALSE,
                    {
                    }
                },                              
                /* H3A Output */
                {
                    UTRUE,
                    {
                        0, 1280, 720, {1280, 1280, 1280}, {FALSE},
                        FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },     
    /* 8, Used for Single Frame Input 1920p StartColor B 2 Luma and 2 Chroma outputs 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1920, 1080, {1920*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS16_PACKED
            },
            /* Output Parameters */
            {  
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080, {1920, 1920, 1920}, {UFALSE},
                        FVID2_DF_LUMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080/2, {1920, 0, 0}, {UFALSE},
                        FVID2_DF_CHROMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },       
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080, {1920, 1920, 1920}, {UFALSE},
                        FVID2_DF_LUMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },                                
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080/2, {1920, 0, 0}, {UFALSE},
                        FVID2_DF_CHROMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },                              
                {
                    UFALSE,
                    {
                    }
                },                              
                /* H3A Output */
                {
                    UFALSE
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },    
    /* 9, Used for Single Frame Input 1920p StartColor B 2-YUV420 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1920, 1080, {1920*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS16_PACKED
            },
            /* Output Parameters */
            {
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080, {1920, 1920, 1920}, {UFALSE},
                        FVID2_DF_YUV420SP_UV, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },              
                {
                    UFALSE,
                    {
                    }
                },
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080, {1920, 1920, 1920}, {UFALSE},
                        FVID2_DF_YUV420SP_UV, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },                                  
                {
                    UFALSE,
                    {
                    }
                },                              
                /* H3A Output */
                {
                    UFALSE,
                },
            },
            UFALSE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },     
    /* 10, Used for Single Frame Input 1920p StartColor B 2 Luma and 2 Chroma outputs 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1920, 1080, {1920*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS16_PACKED
            },
            /* Output Parameters */
            {  
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080, {1920*2, 1920*2, 1920*2}, {UFALSE},
                        FVID2_DF_LUMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS12_UNPACKED16
                    }
                },
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080/2, {1920*2, 0, 0}, {UFALSE},
                        FVID2_DF_CHROMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS12_UNPACKED16
                    }
                },       
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080, {1920, 1920, 1920}, {UFALSE},
                        FVID2_DF_LUMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },                                
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080/2, {1920, 0, 0}, {UFALSE},
                        FVID2_DF_CHROMA_ONLY, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },                              
                {
                    UFALSE,
                    {
                    }
                },                              
                /* H3A Output */
                {
                    UTRUE,
                    {
                        0, 1920, 1080, {1920, 1920, 1920}, {FALSE},
                        FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },
    /* 11, Used for Single Frame Input 1920p StartColor B 2-YUV420 8 bit*/
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT,
            /* In Format */
            {
                0, 1920, 1080, {1920*2, 0, 0}, {UFALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS16_PACKED
            },
            /* Output Parameters */
            {
                {
                    UFALSE,
                    {
                    }
                },              
                {
                    UFALSE,
                    {
                    }
                },
                {
                    /* Output Enabled */
                    UTRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080, {1920, 1920, 1920}, {UFALSE},
                        FVID2_DF_YUV420SP_UV, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    UFALSE,
                    {
                    }
                },                                  
                {
                    UFALSE,
                    {
                    }
                },                              
                /* H3A Output */
                {
                    UFALSE,
                },
            },
            UTRUE,                           /* enable GLBCE */
            UTRUE,                           /* Enable NSF4 */
            VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            UTRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[0]
    },
    /* 12, Three Frame Merge wth two YUV420, saturation and AEWB output (copied from ../vhwa_viss_test/vhwa_viss_cfg.h)  */
    {
        /* VISS Parameters */
        {
            /* Input Mode */
            VHWA_M2M_VISS_MODE_THREE_FRAME_MERGE,
            /* In Format */
            {
                0, 1920, 1080, {1920*2, 1920*2, 1920*2}, {FALSE},
                FVID2_DF_RAW, FVID2_SF_PROGRESSIVE,
                FVID2_CCSF_BITS16_PACKED
            },
            /* Output Parameters */
            {
                {
                    /* Output Enabled */
                    TRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080, {1920*2, 1920*2, 1920*2}, {FALSE},
                        FVID2_DF_YUV420SP_UV, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS12_UNPACKED16
                    }
                },
                {
                    FALSE,
                    {
                    }
                },
                {
                    /* Output Enabled */
                    TRUE,
                    /* Output Format */
                    {
                        0, 1920, 1080, {1920, 1920, 1920}, {FALSE},
                        FVID2_DF_YUV420SP_UV, FVID2_SF_PROGRESSIVE,
                        FVID2_CCSF_BITS8_PACKED
                    }
                },
                {
                    FALSE,
                    {
                    }
                },
                {
                    FALSE,
                    {
                    }
                },
                /* H3A Output */
                {
                    FALSE,
                    {
                    }
                },
            },
            .enableGlbce = TRUE,                           /* enable GLBCE */
            .enableNsf4 = TRUE,                           /* Enable NSF4 */
            .edgeEnhancerMode = VHWA_M2M_VISS_EE_DISABLE,       /* EE Mode */
            .enableDpc = TRUE                            /* Enable DPC */
        },
        /* VISS config */
        &gVissCfg[2]
    },
};

#endif
