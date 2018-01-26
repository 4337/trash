#ifndef UNIV_MEM_H
#define UNIV_MEM_H

#include <stdio.h>
#include <stdlib.h>
#include "univ_macros.h"

#define _ZEROM(m,s) memset(m,0,s)
/*
  Malloc
*/
/*inline*/
static univ_inline
void* univ_malloc(size_t i_size)
{
       return malloc(i_size);
}

static univ_inline
void* univ_calloc(size_t num,size_t size)
{
	 return calloc(num,size);
}

/* Free */
#define  univ_free(i_mem)\
do\
{\
    if(i_mem != NULL) free(i_mem); \
}while(0)

/* Realloc */
static univ_inline
void* univ_realloc(void* i_mem,size_t i_size)
{
      return realloc(i_mem,i_size);       
}

static univ_inline 
void** univ_multi_alloc(size_t c,size_t size)
{    
	  int i = 0;
	  if(( ((c + 1) * size) > c ) && ( size  + 1 > size ) &&
	     ( ((c + 1) * sizeof(void*)))){
	     void** ret = (void*)univ_malloc((c + 1) * sizeof(void*));
		 if(ret != NULL) {
			for(;i<c;i++){
				ret[i] = (void*)univ_malloc(size + 1);
				if(ret[i] == NULL) goto __error__;
				memset(ret[i],0,size);
			}
			return (void*)ret;
		 }
__error__: 
		 while(--i >= 0) free(ret[i]);
	  }
	  return NULL;
}

#endif
