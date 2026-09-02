/*
    (c)Texas Instruments 2015
*/

/**
    \file exif.c
    \brief EXIF 2.2 Creator/Parser APIs
*/

#include "util.h"

#define MAX_BUFFER_SIZE (1*KB*KB)

Uint8 buffer[MAX_BUFFER_SIZE];

char *strlwr(char *str)
{
  Uint8 *p = (Uint8 *)str;

  while (*p) {
     *p = tolower((Uint8)*p);
      p++;
  }

  return str;
}

char *strupr(char *str)
{
  Uint8 *p = (Uint8 *)str;

  while (*p) {
     *p = toupper((Uint8)*p);
      p++;
  }

  return str;
}

STATUS bin2c(Uint8 *inName, Uint8 *outName, Uint8 *sensorName)
{
    STATUS status=E_PASS;
    Uint32 bytes=1, size, i, csize, sz;
    FILE* fin, *fout;
    Uint32  chunkSize = MAX_BUFFER_SIZE;
    char    str[1024];

    fprintf(stderr, "\r\n Converting binary file [%s] to C array ", inName );

    fin = fopen( inName, "rb");
    if(fin==NULL) {
        fprintf(stderr, "\r\n ERROR: Input file [%s] not found", inName);
        status = E_DEVICE;
        goto error_stop;
    }
    fout = fopen( outName, "wb");
    if(fout ==NULL) {
        fprintf(stderr, "\r\n ERROR: Output file [%s] ", outName);
        status = E_DEVICE;
        goto error_stop;
    }

    fseek(fin, 0L, SEEK_END);
    sz = ftell(fin);
    fseek(fin, 0L, SEEK_SET);

    fprintf(fout, "/******************************************************************************\n");
    fprintf(fout, "Copyright (c) 2012-2025 Texas Instruments Incorporated\n");
    fprintf(fout, "\n");
    fprintf(fout, "All rights reserved not granted herein.\n");
    fprintf(fout, "\n");
    fprintf(fout, "Limited License.\n");
    fprintf(fout, "\n");
    fprintf(fout, " Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive\n");
    fprintf(fout, " license under copyrights and patents it now or hereafter owns or controls to\n");
    fprintf(fout, " make,  have made, use, import, offer to sell and sell (\"Utilize\") this software\n");
    fprintf(fout, " subject to the terms herein.  With respect to the foregoing patent license,\n");
    fprintf(fout, " such license is granted  solely to the extent that any such patent is necessary\n");
    fprintf(fout, " to Utilize the software alone.  The patent license shall not apply to any\n");
    fprintf(fout, " combinations which include this software, other than combinations with devices\n");
    fprintf(fout, " manufactured by or for TI (\"TI Devices\").  No hardware patent is licensed\n");
    fprintf(fout, " hereunder.\n");
    fprintf(fout, "\n");
    fprintf(fout, " Redistributions must preserve existing copyright notices and reproduce this\n");
    fprintf(fout, " license (including the above copyright notice and the disclaimer and\n");
    fprintf(fout, " (if applicable) source code license limitations below) in the documentation\n");
    fprintf(fout, " and/or other materials provided with the distribution\n");
    fprintf(fout, "\n");
    fprintf(fout, " Redistribution and use in binary form, without modification, are permitted\n");
    fprintf(fout, " provided that the following conditions are met:\n");
    fprintf(fout, "\n");
    fprintf(fout, " * No reverse engineering, decompilation, or disassembly of this software\n");
    fprintf(fout, "   is permitted with respect to any software provided in binary form.\n");
    fprintf(fout, "\n");
    fprintf(fout, " * Any redistribution and use are licensed by TI for use only with TI Devices.\n");
    fprintf(fout, "\n");
    fprintf(fout, " * Nothing shall obligate TI to provide you with source code for the software\n");
    fprintf(fout, "   licensed and provided to you in object code.\n");
    fprintf(fout, "\n");
    fprintf(fout, " If software source code is provided to you, modification and redistribution of\n");
    fprintf(fout, " the source code are permitted provided that the following conditions are met:\n");
    fprintf(fout, "\n");
    fprintf(fout, " * Any redistribution and use of the source code, including any resulting\n");
    fprintf(fout, "   derivative works, are licensed by TI for use only with TI Devices.\n");
    fprintf(fout, "\n");
    fprintf(fout, " * Any redistribution and use of any object code compiled from the source code\n");
    fprintf(fout, "   and any resulting derivative works, are licensed by TI for use only with TI\n");
    fprintf(fout, "   Devices.\n");
    fprintf(fout, "\n");
    fprintf(fout, " Neither the name of Texas Instruments Incorporated nor the names of its\n");
    fprintf(fout, " suppliers may be used to endorse or promote products derived from this software\n");
    fprintf(fout, " without specific prior written permission.\n");
    fprintf(fout, "\n");
    fprintf(fout, " DISCLAIMER.\n");
    fprintf(fout, "\n");
    fprintf(fout, " THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS \"AS IS\" AND ANY EXPRESS OR\n");
    fprintf(fout, " IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n");
    fprintf(fout, " MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.\n");
    fprintf(fout, " IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,\n");
    fprintf(fout, " INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n");
    fprintf(fout, " LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n");
    fprintf(fout, " PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n");
    fprintf(fout, " LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE\n");
    fprintf(fout, " OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF\n");
    fprintf(fout, " ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n");
    fprintf(fout, "******************************************************************************/\n");

    strlwr(sensorName);
    sprintf(str,"%s_dcc.h",sensorName);
    strupr(sensorName);
    fprintf(fout, "/**\n");
    fprintf(fout, " *  \\file %s\n", str);
    fprintf(fout, " *\n");
    fprintf(fout, " *  \\brief Sensor DCC Cfg file.\n");
    fprintf(fout, " *\n");
    fprintf(fout, " */\n");
    fprintf(fout, "\n");
    sprintf(str,  "%s_DCC_H_",sensorName);
    fprintf(fout, "#ifndef %s\n",str);
    fprintf(fout, "#define %s\n", str);
    fprintf(fout, "\n");

    sprintf(str, "%s_DCC_CFG_NUM_ELEM", sensorName);
    fprintf(fout, "#define %s       (%du)\n", str, sz);
    fprintf(fout, "\n");
    sprintf(str,"%sDCC_CFG",sensorName);
    fprintf(fout, "#define %s \\\n", str);
    fprintf(fout, "    { \\\n");

    size=0;
    csize=0;
    while(bytes) {
        bytes = fread(buffer, 1, chunkSize, fin );
        fprintf(stderr, ".");
        for(i=0;i<bytes;i++) {
            if ((i%(16)==0) && (0 != i))
                fprintf(fout, "\\\n");
            fprintf(fout, "0x%02x,", buffer[i]);
            csize++;
        }
        size +=bytes;
    }
    fprintf(fout, "   \\\n}\n");
    fprintf(fout, "\n\n");
    sprintf(str,"BSPDRV_%s_DCC_H_",sensorName);
    fprintf(fout, "#endif /* End of #ifndef %s */\n", str);

    fprintf(stderr, " Done. (%ld bytes)", size);

    if(csize!=size) {
        fprintf(stderr, "\n ERROR: Check output file (byte diff %ld)", size-csize);
    }
error_stop:
    fclose(fout);
    fclose(fin);

    return status;
}

void main(int argc, char **argv)
{
    if(argc!=4) {
        printf("\r\n USAGE: bin2c <Input File>  <Output File> <SensorName>\n" );
        return;
    }

    printf ("%s %s %s\n", argv[1], argv[2], argv[3]);

    bin2c(argv[1], argv[2], argv[3]);
}
