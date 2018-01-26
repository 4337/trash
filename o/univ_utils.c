#include <windows.h>
#include <stdio.h>
#include <lm.h>
#include "univ_utils.h"
#include "univ_macros.h"
#include "univ_mem.h"

unsigned short univ_crc16(char* i_data,size_t i_len)
{
         unsigned char i;
         unsigned int data;
         unsigned int crc = 0xffff;

         if(i_len == 0) return ~crc;

         do {
             for (i=0, data=(unsigned int)0xff & *i_data++;
                  i < 8; 
                  i++, data >>= 1) {
                   if ((crc & 0x0001) ^ (data & 0x0001))
                        crc = (crc >> 1) ^ UNIV_POLY_CRC16;                 
                   else  crc >>= 1;
              }
         }while(--i_len);

         crc = ~crc;
         data = crc;
         crc = (crc << 8) | (data >> 8 & 0xff);
         return crc;
}


unsigned char univ_set_cwd(void)           
{
              char* l_folder;
              char l_fullpath[MAX_PATH + 1];
              
              if(!GetModuleFileName(NULL,l_fullpath,MAX_PATH))    
                 return 0;
              
              l_folder = strrchr(l_fullpath,'\\');
              if(l_folder == NULL) return SetCurrentDirectory("\\");
              *(l_folder + 1) = '\0';
              
              return (unsigned char)SetCurrentDirectory(l_fullpath);           
}
