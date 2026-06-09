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

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <network_api.h>
char *inet_ntoa(struct in_addr in);

int32_t Network_read(const Network_SockObj *pObj, uint8_t *dataBuf, uint32_t *dataSize)
{
    int32_t status = 0;
    uint32_t tmpDataSize;
    int32_t actDataSize = 0;
    uint8_t *pDataBuf;

    tmpDataSize = *dataSize;
    pDataBuf = dataBuf;

    while(tmpDataSize > 0U)
    {
        actDataSize = (int32_t)recv(pObj->connectedSockFd, (void *)pDataBuf, tmpDataSize, 0);
        if(actDataSize <= 0)
        {
            *dataSize = 0U;
            status = -1;
            appLogPrintf("%s: tmpDataSize = 0x%x recv returned 0x%x \n", __FUNCTION__, tmpDataSize, actDataSize);
            break;
        }
        else
        {
            pDataBuf = &pDataBuf[actDataSize];
            tmpDataSize -= (uint32_t)actDataSize;
        }
    }

    return status;
}


int32_t Network_write(const Network_SockObj *pObj, uint8_t *dataBuf, uint32_t dataSize)
{
    int32_t status = 0;
    int32_t actDataSize=0;
    uint8_t *pDataBuf;
    uint32_t remainingSize;

    pDataBuf = dataBuf;
    remainingSize = dataSize;

    while (remainingSize > 0U) {
        actDataSize = (int32_t)send(pObj->connectedSockFd, pDataBuf,
            (size_t)remainingSize, 0);

        if (actDataSize <= 0)
        {
            break;
        }
        else
        {
            pDataBuf = &pDataBuf[actDataSize];
            remainingSize -= (uint32_t)actDataSize;
        }
    }

    if (remainingSize > 0U) {
        status = -1;
    }

    return status;
}

int32_t Network_open(Network_SockObj *pObj, uint32_t port)
{
    int32_t status = 0;
    struct sockaddr_in   sin1;
    int32_t option = 1;
    struct ifreq ifr;
    int32_t fd;
    struct sockaddr *pSockAddr;
    struct sockaddr_in *pSockAddrIn;

    pObj->connectedSockFd = -1;
    pObj->port = port;
    pObj->sockFd = (int32_t)socket(AF_INET, (int32_t)SOCK_STREAM, 0);
    if( pObj->sockFd < 0)
    {
        appLogPrintf(" NETWORK: Unable to open socket (port=%d)!!!\n", port);
        status = -1;
    }
    else
    {
        /* Bind to the specified Server port */
        bzero( &sin1, sizeof(struct sockaddr_in) );
        sin1.sin_family      = AF_INET;
        sin1.sin_addr.s_addr = INADDR_ANY;
        sin1.sin_port        = (uint16_t)(htons((uint16_t)(pObj->port)));

        (void)setsockopt(pObj->sockFd, SOL_SOCKET, SO_REUSEADDR, &option,
            (socklen_t)sizeof(option));

        pSockAddr = (struct sockaddr *)(void *)&sin1;
        if (bind(pObj->sockFd, pSockAddr, (socklen_t)sizeof(sin1)) < 0)
        {
            appLogPrintf(" NETWORK: Unable to bind() (port=%d) !!!\n", port);
            (void)close(pObj->sockFd);
            pObj->sockFd = -1;
            status = -1;
        }
        else
        {
            if(listen(pObj->sockFd, 5) < 0)
            {
                (void)close(pObj->sockFd);
                pObj->sockFd = -1;
                status = -1;
            }
        }
    }

    (ifr.ifr_addr).sa_family = AF_INET;
    (void)strncpy(ifr.ifr_name, "eth0", (IFNAMSIZ - 1));

    fd = (int32_t)socket(AF_INET, (int32_t)SOCK_DGRAM, 0);
    if (fd >= 0)
    {
        if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
        {
            pSockAddrIn = (struct sockaddr_in *)(void *)&(ifr.ifr_addr);
            appLogPrintf(" NETWORK: Opened at IP Addr = %s, socket port=%d!!!\n", inet_ntoa(pSockAddrIn->sin_addr), port);
        }
        else
        {
            appLogPrintf(" NETWORK ERROR: ioctl SIOCGIFADDR failed!!!\n");
        }

        (void)close(fd);
    }
    else
    {
        appLogPrintf(" NETWORK ERROR: Failed to create socket!!!\n");
    }

    return status;
}

int32_t Network_close(Network_SockObj *pObj, Bool closeServerSock)
{
    if(pObj->connectedSockFd != -1)
    {
        (void)close(pObj->connectedSockFd);
        pObj->connectedSockFd = -1;
    }

    if(closeServerSock != 0)
    {
        if (pObj->sockFd != -1)
        {
            (void)close(pObj->sockFd);
            pObj->sockFd = -1;
        }
    }

    return 0;
}

int32_t Network_waitConnect(Network_SockObj *pObj, uint32_t timeout)
{
    int32_t         status;
    struct timeval  timeout1;
    fd_set          master_set;

    /* LDRA_JUSTIFY_START
    <justification start>
    Rationale: FD_ZERO and FD_SET are system macros from <sys/select.h> that initialize and manipulate fd_set structures.
    The following MISRA violations are inherent to the POSIX API implementation and cannot be modified:
    - MISRA.STMT.COND.NOT_BOOLEAN.2012 (#3199): Controlling expression has type 'signed char' instead of Boolean
    - MISRA.STDLIB.WRONGNAME.UNDERSCORE (#3201, #3202): Reserved names '__i' and '__arr' used in macro expansion
    - MISRA.EXPR.PARENS.2012 (#3203): Operator precedence not made explicit in macro expressions
    - MISRA.STMT.NO_COMPOUND (#3204): Loop body is not a compound statement in macro expansion
    - MISRA.ETYPE.COMP.CAST.EXPL.DIFFERENT.2012 (#3205): Cast from 'unsigned long int' to 'signed' type
    - MISRA.ETYPE.INAPPR.OPERAND.BINOP.2012 (#3207, #3208): Binary operators with inappropriate essential types
    - MISRA.ASSIGN.SUBEXPR.2012 (#3211): Assignment operator result used as subexpression
    Effect on this unit: These macros have been tested extensively as part of the POSIX standard and function correctly
    across all supported platforms. The violations exist within the macro implementation and do not affect code correctness.
    <justification end> */
    FD_ZERO(&master_set);
    FD_SET((pObj->sockFd), (&master_set));
    /* LDRA_JUSTIFY_END */

    timeout1.tv_sec  = (time_t)timeout;
    timeout1.tv_usec = 0;

    status = select(pObj->sockFd + 1, &master_set, NULL, NULL, &timeout1);
    if (status < 0)
    {
        appLogPrintf(" NETWORK: Select Failed\n");
        status = -1;
    }
    else if (status == 0)
    {
        appLogPrintf(" NETWORK: Timeout\n");
        status = 0;
    }
    else
    {
        /* LDRA_JUSTIFY_START
        <justification start>
        Rationale: FD_ISSET is a system macro from <sys/select.h> that performs bitwise operations on fd_set.
        The following MISRA violations are inherent to the POSIX API implementation and cannot be avoided:
        - MISRA.ETYPE.CATEGORY.DIFFERENT.2012 (#447211): Operands of '!=' have different essential type categories (Boolean vs signed)
        - MISRA.ETYPE.COMP.CAST.EXPL.DIFFERENT.2012 (#447212): Cast from 'unsigned long int' to 'signed' type category
        - MISRA.ETYPE.INAPPR.OPERAND.BINOP.2012 (#447213, #447214): Binary operators with inappropriate essential types
        Effect on this unit: The macro has been tested extensively and functions correctly across all supported platforms.
        The violations exist within the macro implementation and do not affect code correctness.
        <justification end> */
        if (FD_ISSET((pObj->sockFd), (&master_set)))
        /* LDRA_JUSTIFY_END */
        {
            pObj->connectedSockFd = accept(pObj->sockFd, NULL, NULL);

            if (pObj->connectedSockFd < 0)
            {
                status = 1;
            }
        }
    }

    /* NO connection, retry */
    return status;
}

int32_t Network_sessionOpen(const void *handle)
{
    (void)handle;
    return 0;
}

int32_t Network_sessionClose(const void *handle)
{
    (void)handle;
    return 0;
}

