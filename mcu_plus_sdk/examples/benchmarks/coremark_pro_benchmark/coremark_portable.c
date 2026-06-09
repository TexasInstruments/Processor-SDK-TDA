/*
 *  Copyright (C) 2025 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "th_cfg.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
/* since we are using stdio, FILE is defined */
#define FILE_TYPE_DEFINED
#include "th_file.h"
#include "th_types.h"
#include "th_lib.h" /* for th_log */
#include "th_al.h"
#include "al_smp.h"

#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/CycleCounterP.h>
#include <kernel/nortos/dpl/common/printf.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Define Host specific (POSIX), or target specific global time variables.
 * Define: TIMER_RES_DIVIDER
 *	Divider to trade off timer resolution and total time that can be measured.
 *
 *	Use lower values to increase resolution, but make sure that overflow does not occur.
 *	If there are issues with the return value overflowing, increase this value.
 */

#define SAMPLE_TIME_IMPLEMENTATION 1

#define NSECS_PER_SEC 1000000000

#define EE_TIMER_TICKER_RATE 1000

#define ALTIMETYPE uint32_t

#define GETMYTIME(_t) (*_t=(ClockP_getTimeUsec()))

#define MYTIMEDIFF(fin,ini) ((fin)-(ini))

#define TIMER_RES_DIVIDER 1000

ALTIMETYPE initial, final;

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None*/

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None*/

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*------------------------------------------------------------------------------
 * Function: al_signal_start
 *
 * Description:
 *			Adaptation layer implementation of th_signal_start()
 *
 *          This Function is called when the benchmark starts.  It should
 *          start the target timer or record the current value of a free
 *          running timer.
 *
 * PORTING:
 *			If you want to support target based timing, you need to
 *          fill in this Function.
 * ---------------------------------------------------------------------------*/
void al_signal_start( void )
{
	GETMYTIME (&initial );
}

/*------------------------------------------------------------------------------
 * Function: al_signal_finished
 *
 * Description:
 *			Adaptation layer implementation of th_signal_finished()
 *
 *          This Function is called when a benchmark or test finishes.  It
 *          stops the target timer or reads the value of a free running
 *          timer and calculates the number of timer ticks since
 *          al_signal_start() was called.
 *
 * RETURNS:
 *			The number of ticks since al_signal_start() was called.
 *
 * PORTING:
 *			If you want to support target based timing, you need to
 *          fill in this Function.
 *
 * IMPORTANT:
 * 	Make sure that no wraparound is happening when returning a value from this function.
 *	Use the <ticks_per_sec> function to lower resolution if required.
 * ---------------------------------------------------------------------------*/

size_t al_signal_finished( void )
{
	size_t elapsed;
	GETMYTIME( &final );
	elapsed=(size_t)(MYTIMEDIFF(final,initial));
	return elapsed;
}

/* find out current time from beginning of run */
/*------------------------------------------------------------------------------
 * Function: al_signal_now
 *
 * Description:
 *			find out current time from beginning of run
 *
 *          This Function is called when a benchmark or test finishes.  It
 *          reads the value of a free running
 *          timer and calculates the number of timer ticks since
 *          al_signal_start() was called.
 *			It does NOT stop the timer.
 *
 * RETURNS:
 *			The number of ticks since al_signal_start() was called.
 *
 * PORTING:
 *			If you want to support target based timing, you need to
 *          fill in this Function. Make sure this function remains thread safe.
 * ---------------------------------------------------------------------------*/
size_t al_signal_now( void )
{
	size_t elapsed;
	ALTIMETYPE now;
	GETMYTIME( &now );
	elapsed=(size_t)MYTIMEDIFF(now,initial);
	return elapsed;
}

/*------------------------------------------------------------------------------
 * Function: al_ticks_per_sec
 *
 * Description:
 *			Adaptation layer implementation of th_ticks_per_sec()
 *
 *          This Function is used to determine the resolution of the target
 *          timer.  This value is reported to the host computer and is
 *          is used to report test results.
 *
 * RETURNS:
 *			The number of timer ticks per second.
 *
 * PORTING:
 *			If a target timer is supported, then this Function must be
 *          implemented.
 *
 *          If the target timer is NOT supported, then this Function MUST
 *          return zero.
 *
 * NOTES:
 *			ANSI C, POSIX requires that CLOCKS_PER_SEC equals 1000000
 *          independent of the actual resolution.
 *
 *          On Linux and Solaris hosts this results in durations to be large
 *          numbers which always end with three zeros.  This is correct,
 *          because the clock resolution is less than the POSIX required
 *          resolution of 1000000. The resulting calculation to seconds is
 *          correct, and the actual resolution is measured to be 1000, or a
 *          millisecond timer.
 *
 *          Note that the time can wrap around.  On a 32 bit system
 *          where CLOCKS_PER_SEC equals 1000000 this Function will
 *          return the same value approximately every 72 minutes.
 *          ( Excerpt from GNU man page clock )
 *
 * ---------------------------------------------------------------------------*/

size_t al_ticks_per_sec( void )
{
	return (size_t) (NSECS_PER_SEC / TIMER_RES_DIVIDER);
}

/*------------------------------------------------------------------------------
 * Function: al_tick_granularity
 *
 * Description:
 *			used to determine the granularity of timer ticks.
 *
 *          Example 1: the value returned by al_stop_timer() may be
 *          in milliseconds. In this case, al_ticks_pers_sec() would
 *          return 1000.  However, the timer interrupt may only occur
 *          once very 10ms.  So al_tick_granularity() would return 10.
 *
 *          Example 2: on another system, th_ticks_sec() returns 10
 *          and th_tick_granularity() returns 1.  This means that each
 *          increment of the value returned by th_stop_timer() is in 100ms
 *          intervals.
 *
 * RETURNS:
 *			the granularity of the value returned by th_stop_timer()
 *
 * PORTING:
 *			If a target timer is supported, then this Function must be
 *          implemented.
 * ---------------------------------------------------------------------------*/

size_t al_tick_granularity( void )
{
	return EE_TIMER_TICKER_RATE;
}

/*------------------------------------------------------------------------------
 *                       >>> SUPPORT FunctionS <<<
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * These support Functions are part of the Adaptaion Layer but do not
 * generally need to be modified.
 * ---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Function: al_exit
 *
 * Description:
 *			Exits the benchmark.
 *
 *
 * PARAMS:
 *			exit_code - the traditional exit code
 *
 *
 * PORTING:
 *			Standard C in stdlib.h
 * ---------------------------------------------------------------------------*/

void al_exit( int exit_code)

{
	al_hardware_reset(exit_code); /* Notify hardware reset that we came from th_exit */
	exit(exit_code);
}

/*------------------------------------------------------------------------------
 * Function: al_printf
 *
 * Description:
 *			The traditional vprintf, control output from here.
 *
 *
 * PORTING:
 *			Standard C in stdio.h, and stdargs.h
 *
 * NOTE:
 *			To convert this to a string, use vsprintf, NOT sprintf.
 * ---------------------------------------------------------------------------*/
int	  al_printf(const char *fmt, va_list	args)
{
    printf_("\r"); /* add carriage return */
	return	vprintf_(fmt,args);
}

/*-----------------------------------------------------------------------------
 * Function: al_sprintf
 *
 * Description:
 *			The traditional vsprintf, control output from here.
 *
 *
 * PORTING:
 *			Standard C in stdio.h, and stdargs.h
 *
 * NOTE:
 *			To convert this to a string, use vsprintf, NOT sprintf.
 * ---------------------------------------------------------------------------*/
int  al_sprintf(char *str, const char *fmt, va_list   args)
{
	return  vsprintf(str,fmt,args);
}

/*------------------------------------------------------------------------------
 * Function: al_report_results
 *
 * Description:
 *			Print additional messages below harness output.
 *
 * PORTING:
 *			Use harness th_printf.
 * ---------------------------------------------------------------------------*/
void  al_report_results( void )
{
	/* Add any debug printing here, outside the normal log */
}

/* scanf input format conversion family <stdargs.h> */

/** Function: al_vsscanf
 Simple vsscanf implementation, using scanf

*/
int	al_vsscanf(const char *str, const char *format, va_list ap)
{
#if 	USE_TH_FILEIO || !HAVE_VSSCANF
#define VSSF_MAX 7
	/* simple implementation of up to 7 args using sscanf */
  void      *arg[VSSF_MAX];
  /* first find out how many args there are from fmt? */
  int numargs=0,i;
  const char *pfind=format;
  while (*pfind) {
	  if (pfind[0]=='%') {
		  if (pfind[1]=='*' || pfind[1]=='%')
			  pfind++; /* ignore %* and %% */
		  else
			  numargs++; /*otherwise there should be another arg */
	  }
	  pfind++;
  }
  if (numargs>VSSF_MAX) {
	  th_log(TH_ERROR,"ERROR: too many args to vsscanf");
	  return 0; /* error here */
  }
  /* copy the args to a local array to make sure we are thread safe */
  {
    for ( i=0; i<numargs; i++ )
      arg[i] = va_arg( ap, void * );
  }
  /* then call sscanf */
  switch (numargs) {
	  case 1: return sscanf(str,format,arg[0]);
	  case 2: return sscanf(str,format,arg[0],arg[1]);
	  case 3: return sscanf(str,format,arg[0],arg[1],arg[2]);
	  case 4: return sscanf(str,format,arg[0],arg[1],arg[2],arg[3]);
	  case 5: return sscanf(str,format,arg[0],arg[1],arg[2],arg[3],arg[4]);
	  case 6: return sscanf(str,format,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5]);
	  case 7: return sscanf(str,format,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6]);
	  default: return 0;
  }
#elif	HAVE_FILEIO || HAVE_VSSCANF
	return vsscanf(str,format,ap);
#else
	return NULL;
#endif
}

/** Function: al_vsscanf
 Simple vsscanf implementation, using scanf

*/
int	al_vfscanf(ee_FILE *stream, const char *format, va_list ap)
{
#if 	USE_TH_FILEIO || !HAVE_VFSCANF
 #if FAKE_FILEIO
 #warning "No implementation for vfscanf, please make sure no workloads are using this function or provide an implementation for the adaptation layer"
 return NULL;
 #endif
 #define VSSF_MAX 7
	/* simple implementation of up to 7 args using fscanf */
  void      *arg[VSSF_MAX];
  /* first find out how many args there are from fmt? */
  int numargs=0,i;
  const char *pfind=format;
  while (*pfind) {
	  if (pfind[0]=='%') {
		  if (pfind[1]=='*' || pfind[1]=='%')
			  pfind++; /* ignore %* and %% */
		  else
			  numargs++; /*otherwise there should be another arg */
	  }
	  pfind++;
  }
  if (numargs>VSSF_MAX) {
	  th_log(TH_ERROR,"ERROR: too many args to vfscanf");
	  return 0; /* error here */
  }
  /* copy the args to a local array to make sure we are thread safe */
  {
    for ( i=0; i<numargs; i++ )
      arg[i] = va_arg( ap, void * );
  }
  /* then call sscanf */
  switch (numargs) {
	  case 1: return fscanf(stream,format,arg[0]);
	  case 2: return fscanf(stream,format,arg[0],arg[1]);
	  case 3: return fscanf(stream,format,arg[0],arg[1],arg[2]);
	  case 4: return fscanf(stream,format,arg[0],arg[1],arg[2],arg[3]);
	  case 5: return fscanf(stream,format,arg[0],arg[1],arg[2],arg[3],arg[4]);
	  case 6: return fscanf(stream,format,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5]);
	  case 7: return fscanf(stream,format,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6]);
	  default: return 0;
  }
#elif	HAVE_FILEIO
	return vfscanf(stream,format,ap);
#else
	return NULL;
#endif
}

/* NON Standard routines */
/* Function: al_filecmp
	Compare two files.

	Note:
		Not supported in initial version of MITH
*/
int	al_filecmp (const char *file1, const char *file2)
{
	return file1==file2;
}
/* Function: al_fcreate
	create a file with predefined content
*/
void  *al_fcreate(const char *filename, const char *mode, char *data, size_t size) {
	ee_FILE *f;
	/* A writable file - just create */
	if (strchr(mode, 'w') || size==0)
		return th_fopen(filename,mode);
	/* Otherwise, create a new file with data, and then open it */
	f=th_fopen(filename,"w");
	th_fwrite(data,size,1,f);
	th_fclose(f);
	return th_fopen(filename,mode);
}


/** Function: al_fsize
	Non Standard routine to return file size.

Note:
	If the toolchain has the stat Function, a POSIX standard
	way is available to return file size.
 */
size_t	al_fsize(const char *filename)
{
#if	USE_TH_FILEIO
	return 0; /* not implemented */
#elif	HAVE_STAT_H || HAVE_SYS_STAT_H
/** Get file size using stat Function */
    size_t	length;
    struct	stat st;

    if (!filename || !*filename)
		length=0;
	else {
        if ((stat(filename,&st))==-1)
			length=0;
        else
			length = st.st_size;
	}
    return length;
#elif	HAVE_FILEIO
	size_t total;
	FILE *fp=fopen(filename, "rb");
	if (fp==NULL) return 0;
	fseek(fp, 0, SEEK_END);
	total=ftell(fp);
	fclose(fp);
	return total;
#else
	th_log(0,"ERROR: fsize called but no implementation");
	return 0;
#endif
}

/** Function: al_getenv
Get Environment Variable (libc)

This routine is used for host based benchmark which use
environment variables. You can set up pre-defined environment
variables here, or return NULL for not found.
Params:
	key - Environment variable name
Returns:
	Environment variable value, or NULL if not found.
*/
char  *al_getenv( const char *key )
{
	return getenv(key);
}


/*------------------------------------------------------------------------------
 * Function: al_main
 *
 * Description:
 *			Target Specific Initialization for all Benchmarks
 *
 * PORTING:
 *			Use bmark_lite.c: main for benchmark specific init..
 * ---------------------------------------------------------------------------*/

void redirect_std_files(void);
void	al_main( int argc, char* argv[]  )
{
	argc=argc; /*avoid compiler warning */
	argv=argv; /*avoid compiler warning */
	redirect_std_files();
}

/* Function: al_hardware_reset
 * Description:
 *			This is the last step called by main in all the benchmarks.
 *
 * NOTE:
 *			ev from th_exit is passed, or 0 if not from an exit.
 */

void al_hardware_reset(int ev)
{
    /* Do Nothing */
}

#include "fp_shape.h"

int store_dp(e_f64 *value, intparts *asint)
{
   e_f64 v64;
   e_u32 iexp;
   e_s32 exp=asint->exp;
   e_u32 manthigh=asint->mant_high32;

   if (manthigh >= ((e_u32)1<<(52-32 + 1)))
   {
      return 0;
   }
   if (!(manthigh & ((e_u32)1<<(52-32))))
   {
      /* special casing signed zero */
      if (exp ==0 && asint->mant_low32 == 0 && manthigh == 0)
      {
		INSERT_WORDS(v64, (e_u32)(asint->sign) << 31, 0);
        *value = v64;
        return 1;
      }
      return 0;
   }

   manthigh &= ((e_u32)1 << (52-32)) - 1;

   exp += 1023;
   if (exp <= 0 || exp >= 2047)
   {
      return 0;
   }
   iexp = exp << (52-32);
   if (asint->sign)
   {
      iexp |= 0x80000000;
   }
   INSERT_WORDS(v64, (manthigh | iexp), asint->mant_low32);
   *value = v64;
   return 1;
}

/* supports denorm, no inf/nan */
int load_dp(e_f64 *value, intparts *asint)
{
   e_u32 iValue0, iValue1;

   if (!value || !asint)
      return 0;

   EXTRACT_WORDS(iValue1, iValue0, *value);

   asint->mant_low32 = iValue0;
   asint->mant_high32 = (iValue1 & (((e_u32)1 << (52 - 32)) - 1));
   asint->exp = ((iValue1 >> (52-32)) & 2047);
   asint->sign = iValue1 >> 31;

   if (asint->exp == 2047)
      return 0;

   if (asint->exp != 0)
   {
      asint->mant_high32 |=  ((e_u32)1 << (52-32));
      asint->exp -= 1023;
   }
   else
   {
      if (asint->mant_high32 || asint->mant_low32)
         return 0;
   }
   return 1;
}

/* no denormal/inf/nan support */
int store_sp(e_f32 *value, intparts *asint)
{
   e_u32 iValue;
   e_f32 v32;
   e_u32 iexp;
   e_s32 exp=asint->exp;
   e_u32 mant=asint->mant_low32;

   if (asint->mant_high32)
      return 0;

   if (mant >= ((e_u32)1<<24))
   {
      return 0;
   }
   if (!(mant & ((e_u32)1<<23)))
   {
      /* special casing signed zero */
      if (exp == 0 && mant == 0)
      {
         iValue = (e_u32)(asint->sign) << 31;
         SET_FLOAT_WORD(v32, iValue);
         *value = v32;
         return 1;
      }
      return 0;
   }

   mant &= ((e_u32)1 << 23) - 1;

   exp += 127;
   if (exp <= 0 || exp >= 255)
   {
      return 0;
   }
   iexp = exp << 23;
   if (asint->sign)
   {
      iexp |= 0x80000000;
   }
   iValue = mant | iexp;
   SET_FLOAT_WORD(v32, iValue);
   *value = v32;
   return 1;
}

int load_sp(e_f32 *value, intparts *asint)
{
   e_u32 iValue;

   if (!value || !asint)
      return 0;

   GET_FLOAT_WORD(iValue, *value);

   asint->mant_high32 = 0;
   asint->mant_low32 = (iValue & (((e_u32)1 << 23) - 1));
   asint->exp = ((iValue >> 23) & 255);

   if (asint->exp == 255)
      return 0;

   if (asint->exp != 0)
   {
      asint->mant_low32 |=  ((e_u32)1 << 23);
      asint->exp -= 127;
   }
   else
   {
      if (asint->mant_low32)
         return 0;
   }
   asint->sign = iValue >> 31;
   return 1;
}
