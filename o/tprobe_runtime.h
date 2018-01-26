#ifndef TPROBE_RUNTIME_H
#define TPROBE_RUNTIME_H

#include "univ_utils.h"
#include "univ_macros.h"

#define TPROBE_DEBUG 0

#define T_PROBE_LOCALHOST_NO_SUPPORT

#define NO_RESPONSE_ERROR_CNT 3

#define TPROBE_SIG_FILENAME ".\\signatures.deb"

#if TPROBE_DEBUG == 1
 #include "tp_ndebug.h"
#endif

/* 
 RETURN 1=SUCCESS , 0=ERROR ,-1=INTERNAL
*/
static univ_inline
char tprobe_runtime(void) 
{
      return (univ_set_cwd() == 0) ?  0 : 1;                
}


#endif
