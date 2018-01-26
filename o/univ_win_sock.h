#ifndef UNIV_WIN_SOCK_H
#define UNIV_WIN_SOCK_H

#include "univ_macros.h"

static univ_inline
int univ_winsock_init(void)
{
    WSADATA i_wsadata;
     
    return WSAStartup(MAKEWORD(2,2),&i_wsadata);   
}

static univ_inline
void univ_winsock_free(void)
{
     WSACleanup();
}

#endif
