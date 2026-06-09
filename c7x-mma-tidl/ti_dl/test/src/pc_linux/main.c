#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>

#include "platform_common.h"

uint8_t *L1Scratch;
uint8_t *L2Scratch;
uint8_t *L3Scratch;
uint8_t *L4Scratch;

TIDL_CreateParams  createParams;
sTIDL_IOBufDesc_t  gIOParams;

int32_t tidlMain(int32_t argc, char **argv);
int32_t tidlNegativeCase(int32_t argc, char **argv);

int32_t debug_printf(const char *format, ...)
{
	va_list ap;
	int32_t ret;

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

void * tidltb_alignMalloc(int32_t size, int32_t alignment)
{
  return memalign(alignment, size);
}

void tidltb_alignFree(void * ptr)
{
  free(ptr);
}


uint64_t tidltb_virtToPhyAddrConversion(const void *virtAddr,
                                      uint32_t chNum,
                                      void *appData)
{
	return (uint64_t)virtAddr;
}

int main(int argc, char **argv)
{
	L1Scratch = (uint8_t*)memalign(L1_TOTAL_MEMORY_SIZE, L1_MEM_SIZE);
	L2Scratch = (uint8_t*)memalign(L2_TOTAL_MEMORY_SIZE, L2_MEM_SIZE);
	L3Scratch = (uint8_t*)memalign(L3_TOTAL_MEMORY_SIZE, L3_MEM_SIZE);
	L4Scratch = (uint8_t*)malloc(L4_MEM_SIZE);

	printf(0, "%6s, %12s, %12s\n" ,"Mem", "StartAdd", "endAdd");

	printf(0, "%6s, %12x, %12x\n" ,"L2", L2Scratch, L2Scratch + L2_MEM_SIZE );
	printf(0, "%6s, %12x, %12x\n" ,"L3", L3Scratch, L3Scratch + L3_MEM_SIZE );

#if HOST_EMULATION
    if((argc != 1) && ((strcmp(argv[1], "NNW") == 0) || (strcmp(argv[1], "NWL") == 0)))
	{
          tidlNegativeCase(argc, argv);
	}
	else
#endif
	{
		  tidlMain(argc, argv);
	}

	free(L1Scratch);
	free(L2Scratch);
	free(L3Scratch);
	free(L4Scratch);

	return 0;
}
