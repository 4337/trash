#ifndef UNIV_MACROS_H
#define UNIV_MACROS_H

#ifdef MSVC
 #define univ_inline __forceinline 
#else 
 #define univ_inline __inline__ 
#endif


#endif
