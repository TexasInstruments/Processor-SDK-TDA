/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <itt_priv.h>
#include <itt_server.h>
#include <network_api.h>
#include <network_ctrl_api.h>

#define MAX_CMD_NETWORK_SIZE 1024U

static NetworkCtrl_Obj gNetworkCtrl_obj;

static void IttCtrl_cmdHandlerUnsupportedCmd(char *cmd, uint32_t prmSize);
static int32_t IttCtrl_registerHandler(const char *cmd, NetworkCtrl_Handler handler);
int32_t IttCtrl_unregisterHandler(const char *cmd);
static void* IttCtrl_tskMain(void *arg);

static void IttCtrl_cmdHandlerUnsupportedCmd(char *cmd, uint32_t prmSize)
{
    (void)cmd;
    static uint8_t pBuf[MAX_CMD_NETWORK_SIZE];
    uint8_t *pVirtBuf;

    /* alloc tmp buffer for parameters */
    if(prmSize != 0U)
    {
        if(prmSize > MAX_CMD_NETWORK_SIZE)
        {
            appLogPrintf("%s Error: Parameter size %u exceeds maximum %u\n", __FUNCTION__, prmSize, (uint32_t)MAX_CMD_NETWORK_SIZE);
        }
        else
        {
            pVirtBuf = pBuf;

            /* read parameters */
            (void)IttCtrl_readParams(pVirtBuf, prmSize);

            appLogPrintf(" ITT_SERVER Error: UNSUPPORTED CMD (prmSize=%d) !!!\n", prmSize);
        }
    }

    /* send response */
    (void)IttCtrl_writeParams(NULL, 0, (uint32_t)-1);
}

static int32_t IttCtrl_registerHandler(const char *cmd, NetworkCtrl_Handler handler)
{
    NetworkCtrl_Obj *pObj = &gNetworkCtrl_obj;
    int32_t i;
    int32_t firstFreeIdx;

    int32_t status = -2;

    firstFreeIdx = -1;
    if(NULL == cmd)
    {
        status = -1;
    }
    else
    {
        /* check if command is already registered */
        for(i=0; i<(int32_t)NETWORK_CTRL_MAX_CMDS; i++)
        {
            if(pObj->cmdHandler[i].handler != NULL)
            {
                if(strncmp(
                    pObj->cmdHandler[i].cmd,
                    cmd,
                    NETWORK_CTRL_CMD_STRLEN_MAX)
                    ==0)
                {
                    /* command already registered, exit with error */
                    appLogPrintf("%s: command %s already registered \n", __FUNCTION__, cmd);
                    status = -1;
                    break;
                }
            }
            else
            {
                firstFreeIdx = i;
                status = 0;
                break;
            }
        }

        if(status == -2)
        {
            appLogPrintf("%s, no space to register command! \n", __FUNCTION__);
            status = -1;
        }

        if(status == 0)
        {
            /* command not registered, register it */
            pObj->cmdHandler[firstFreeIdx].handler = handler;
            (void)strcpy(pObj->cmdHandler[firstFreeIdx].cmd, cmd);
            appLogPrintf("%s: command %s registered at location %d \n", __FUNCTION__, cmd, firstFreeIdx);
        }
    }

    return status;
}

int32_t IttCtrl_unregisterHandler(const char *cmd)
{
    NetworkCtrl_Obj *pObj = &gNetworkCtrl_obj;
    int32_t i;

    /* check if command is already registered */
    for(i=0; i<(int32_t)NETWORK_CTRL_MAX_CMDS; i++)
    {
        if(pObj->cmdHandler[i].handler != NULL)
        {
            if(strncmp(
                pObj->cmdHandler[i].cmd,
                cmd,
                NETWORK_CTRL_CMD_STRLEN_MAX)
                ==0)
            {
                /* command found, unregister it */
                pObj->cmdHandler[i].handler = NULL;
                pObj->cmdHandler[i].cmd[0] = '\0';
            }
        }
    }

    return 0;
}

int32_t IttCtrl_readParams(uint8_t *pPrm, uint32_t prmSize)
{
    NetworkCtrl_Obj *pObj = &gNetworkCtrl_obj;
    int32_t status = 0;
    uint32_t localPrmSize = prmSize;

    if(prmSize != 0U)
    {
        status = Network_read(&pObj->sockObj, pPrm, &localPrmSize);

        if(status<0)
        {
            appLogPrintf(" ITT_SERVER Error: Network_read() failed to read parameters (port=%d) !!!\n", pObj->serverPort);
        }
    }

    return status;
}

int32_t IttCtrl_writeParams(uint8_t *pPrm, uint32_t prmSize, uint32_t returnStatus)
{
    NetworkCtrl_Obj *pObj = &gNetworkCtrl_obj;
    int32_t status;

    pObj->cmdBuf.prmSize = prmSize;
    pObj->cmdBuf.returnValue = returnStatus;
    pObj->cmdBuf.flags = NETWORK_CTRL_FLAG_ACK;

    status = Network_write(&pObj->sockObj, (uint8_t*)&pObj->cmdBuf,
        (uint32_t)sizeof(pObj->cmdBuf));

    if(status<0)
    {
        appLogPrintf(
            " ITT_SERVER Error: Network_write() failed to write response header (port=%d) !!!\n",
                pObj->serverPort);
    }
    else if(prmSize != 0U)
    {
        status = Network_write(&pObj->sockObj, pPrm, prmSize);

        if(status<0)
        {
            appLogPrintf(
                " ITT_SERVER Error: Network_write() failed to write parameters (port=%d) !!!\n",
                    pObj->serverPort);
        }
    }
    else
    {
        /* No parameters to write */
    }

    return status;
}

static void* IttCtrl_tskMain(void *arg)
{

    NetworkCtrl_Obj *pObj = (NetworkCtrl_Obj*)arg;
    int32_t status;
    uint32_t tempSize;
    volatile uint32_t dataSize;

    status = Network_open(&pObj->sockObj, pObj->serverPort);
	if(0 != status)
    {
    	appLogPrintf("Error : Network_open returned %d\n", status    );
    }
	else
    {
    	while((Bool)FALSE == gNetworkCtrl_obj.tskExit)
        {
        	status = Network_waitConnect(&pObj->sockObj, 10000);
        	if(status<0)
            {
            	break;
            }

        	if(status==0)
            {
            	continue;
            }

        	dataSize = (uint32_t)sizeof(pObj->cmdBuf);

            /* read command header */
        	tempSize = (uint32_t)dataSize;
            status = Network_read(&pObj->sockObj, 
                                (uint8_t *)&pObj->cmdBuf, 
                                &tempSize);
            dataSize = (int32_t)tempSize;

        	if(0 != status)
            {
            	appLogPrintf("Error : Network_read returned %d \n", status);
            }

            uint32_t expectedSize = (uint32_t)sizeof(pObj->cmdBuf);
            bool isSizeCorrect = (dataSize == expectedSize);
        	if((status==0) && isSizeCorrect)
            {
                /* handle command */
            	if(pObj->cmdBuf.header != NETWORK_CTRL_HEADER)
                {
                	appLogPrintf(
                        " ITT_SERVER Error: Invalid header received (port=%d) header = 0x%x!!!\n",
                    	pObj->serverPort, pObj->cmdBuf.header);
                }
            	else
                {
                	Bool isCmdHandled;
                	int32_t i;

                	isCmdHandled = FALSE;

                    #ifdef ENABLE_DEBUG_ITTSERVER
                	appLogPrintf("command header = 0x%x \n", pObj->cmdBuf.header);
                    #endif

                    /* valid header received */
                	for(i=0; i<(int32_t)NETWORK_CTRL_MAX_CMDS; i++)
                    {
                    	if(strncmp(
                        	pObj->cmdHandler[i].cmd,
                        	pObj->cmdBuf.cmd,
                        	NETWORK_CTRL_CMD_STRLEN_MAX)
                            ==0)
                        {
                            /* matched a register command */

                        	if(pObj->cmdHandler[i].handler != NULL)
                            {
                                #ifdef ENABLE_DEBUG_ITTSERVER
                            	appLogPrintf(
                                    " ITT_SERVER: Received command [%s], with %d bytes of parameters\n",
                                	pObj->cmdBuf.cmd,
                                	pObj->cmdBuf.prmSize
                                    );
                                #endif

                            	pObj->cmdHandler[i].handler(
                                	pObj->cmdBuf.cmd,
                                	pObj->cmdBuf.prmSize
                                    );

                                #ifdef ENABLE_DEBUG_ITTSERVER
                            	appLogPrintf(
                                    " ITT_SERVER: Sent response for command [%s], with %d bytes of parameters\n",
                                	pObj->cmdBuf.cmd,
                                	pObj->cmdBuf.prmSize
                                    );
                                #endif

                            	isCmdHandled = TRUE;
                            	break;
                            }
                        }
                    }

                	if(isCmdHandled == FALSE)
                    {
                        /* if command is not handled, then read the params and
                           ACK it with error */
                    	IttCtrl_cmdHandlerUnsupportedCmd(
                        	pObj->cmdBuf.cmd,
                        	pObj->cmdBuf.prmSize
                            );
                    }
                }
            }
        	else
            {
            	appLogPrintf(" ITT_SERVER Error: recv() failed (port=%d) !!!\n",
                	pObj->serverPort);
            }

            /* close socket */
        	(void)Network_close(&pObj->sockObj, FALSE);
        }
    }

    return NULL;
}
void * g_appObj;
void * g_appFileSaveCallback;
void * g_appDccUpdateCallback;

int32_t itt_server_init(void * appObj, void * appFileSaveCallback, void * appDccUpdateCallback)
{
    int32_t err = 0;

    (void)memset(&gNetworkCtrl_obj, 0, sizeof(gNetworkCtrl_obj));

    gNetworkCtrl_obj.serverPort = NETWORK_CTRL_SERVER_PORT;

    err = IttCtrl_registerHandler("echo", itt_ctrl_cmdHandlerEcho);
    if (err != 0)
    {
        appLogPrintf("\n %s Error : failed to register handler for command :[echo] \n", __FUNCTION__);
    }

    err = IttCtrl_registerHandler("iss_read_2a_params", itt_ctrl_cmdHandlerIssRead2AParams);
    if (err != 0)
    {
        appLogPrintf("\n %s Error : failed to register handler for command :[iss_read_2a_params] \n", __FUNCTION__);
    }
    
    err = IttCtrl_registerHandler("iss_write_2a_params", itt_ctrl_cmdHandlerIssWrite2AParams);
    if (err != 0)
    {
        appLogPrintf("\n %s Error : failed to register handler for command :[iss_write_2a_params] \n", __FUNCTION__);
    }

    if((NULL != appObj) && (NULL != appFileSaveCallback))
    {
        g_appObj = appObj;
        g_appFileSaveCallback = appFileSaveCallback;

        err = IttCtrl_registerHandler("iss_raw_save", itt_ctrl_cmdHandlerIssFileSave);
        if (err != 0)
        {
            appLogPrintf("\n %s Error : failed to register handler for command :[iss_raw_save] \n", __FUNCTION__);
        }

        err = IttCtrl_registerHandler("iss_yuv_save", itt_ctrl_cmdHandlerIssFileSave);
        if (err != 0)
        {
            appLogPrintf("\n %s Error : failed to register handler for command :[iss_yuv_save] \n", __FUNCTION__);
        }
    }
    else
    {
        appLogPrintf("\n %s Error : failed to register handler for command :[iss_raw_save] \n", __FUNCTION__);
        appLogPrintf("\n %s Error : failed to register handler for command :[iss_yuv_save] \n", __FUNCTION__);
        appLogPrintf("\n %s Error : Application must provide a valid callback \n", __FUNCTION__);
    }

    err = IttCtrl_registerHandler("iss_read_sensor_reg", itt_ctrl_cmdHandlerIssReadSensorReg);
    if (err != 0)
    {
        appLogPrintf("\n %s Error : failed to register handler for command :[iss_read_sensor_reg] \n", __FUNCTION__);
    }

    err = IttCtrl_registerHandler("iss_write_sensor_reg", itt_ctrl_cmdHandlerIssWriteSensorReg);
    if (err != 0)
    {
        appLogPrintf("\n %s Error : failed to register handler for command :[iss_write_sensor_reg] \n", __FUNCTION__);
    }

    err = IttCtrl_registerHandler("dev_ctrl", itt_ctrl_cmdHandlerIssDevCtrl);
    if (err != 0)
    {
        appLogPrintf("\n %s Error : failed to register handler for command :[dev_ctrl] \n", __FUNCTION__);
    }

    err = IttCtrl_registerHandler("iss_send_dcc_file", itt_ctrl_cmdHandlerIssDccSendFile);
    if (err != 0)
    {
        appLogPrintf("\n %s Error : failed to register handler for command :[iss_send_dcc_file] \n", __FUNCTION__);
    }else
    {
        g_appDccUpdateCallback = appDccUpdateCallback;
    }

    err = pthread_create(&gNetworkCtrl_obj.thrHndl, NULL, &IttCtrl_tskMain, (void *)&gNetworkCtrl_obj);

    if (err != 0)
    {
        appLogPrintf("\n %s Error : can't create thread :[%s] \n", __FUNCTION__, strerror(err));
    }
    else
    {
        #ifdef ENABLE_DEBUG_ITTSERVER
        appLogPrintf("\n Thread created successfully\n");
        #endif
    }

    return err;
}
