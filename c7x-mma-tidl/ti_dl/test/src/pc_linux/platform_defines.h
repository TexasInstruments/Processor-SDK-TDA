#ifndef __PLATFORM_DEFINES_H__
#define __PLATFORM_DEFINES_H__

#define TI_FILE FILE
#define FOPEN fopen
#define FGETS fgets
#define FCLOSE fclose
#define FWRITE fwrite
#define FSEEK fseek
#define FTELL ftell
#define FREAD fread
#define FPRINTF fprintf

#define EXTRA_MEM_FOR_ALIGN (1024)
#define L4_MEM_SIZE  ((size_t)3*1024*1024*1024)

#endif /*__PLATFORM_DEFINES_H__*/
