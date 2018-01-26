#ifndef UNIV_UTILS_H
#define UNIV_UTILS_H

#define UNIV_POLY_CRC16 0x8408

#include "univ_macros.h"
/*
 Get current username 
 IN: i_out = pointer to buffer that contains username 
     i_len = size of i_out buffer 
 Return: 0 or 1
*/
static univ_inline
unsigned char  univ_get_username(char* i_out,size_t* i_len)
{
                return (unsigned char)GetUserName(i_out,(LPDWORD)i_len);       
}
/*
  Calculate crc16 for i_data
  IN: i_data = data
      i_len = data length
  Return : Crc 16
*/
unsigned short univ_crc16(char* i_data,size_t i_len);

/*
 Set current working directory 
 to pprobe folder
 In void
 Return 0 || 1
*/
unsigned char univ_set_cwd(void);


#endif
