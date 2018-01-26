@echo off
echo "kompilacja t-probe 1.1.0 ..."

rem gcc -c univ_mem.c -o univ_mem.o -ansi -fexpensive-optimizations -O3 -Wall
gcc -c univ_random.c -o univ_random.o -ansi -fexpensive-optimizations -O3 -Wall
gcc -c univ_net_utils.c -o univ_net_utils.o -ansi -fexpensive-optimizations -O3 -Wall
gcc -c univ_utils.c -o univ_utils.o  -ansi -fexpensive-optimizations -O3 -Wall
gcc -c tprobe_ui.c -o tprobe_ui.o -ansi -fexpensive-optimizations -O3 -Wall       
gcc -c tprobe_signatures.c -o tprobe_signatures.o -ansi -fexpensive-optimizations -O3 
gcc -c tprobe_osf.c -o tprobe_osf.o -ansi -fexpensive-optimizations -O3  
gcc -c wcallback.c -o wcallback.o -ansi -fexpensive-optimizations -O3 -Wall
gcc -c wpcap.c -o wpcap.o -ansi -fexpensive-optimizations -O3 -Wall
gcc -c univ_tcp_options.c -o univ_tcp_options.o -ansi -fexpensive-optimizations -O3 -Wall
gcc -c tprobe_uptime.c -o tprobe_uptime.o -ansi -fexpensive-optimizations -O3 -Wall
gcc -c univ_packet.c -o univ_packet.o -ansi -fexpensive-optimizations -O3 
gcc -c tprobe_net_ports.c -o tprobe_net_ports.o -ansi -fexpensive-optimizations -O3 -Wall
gcc -c tracert.c -o tracert.o -ansi -fexpensive-optimizations -O3 -Wall
gcc -c main.c -o main.o -ansi -fexpensive-optimizations -O3  

windres.exe -i version.rc --input-format=rc -o version.res -O coff
gcc univ_random.o univ_net_utils.o tprobe_ui.o univ_utils.o univ_tcp_options.o univ_packet.o tprobe_uptime.o tprobe_net_ports.o wcallback.o wpcap.o  tracert.o tprobe_signatures.o tprobe_osf.o version.res main.o -o tprobe_1.1.0.exe -lws2_32 -liphlpapi -lnetapi32 -lwpcap

del *.o

PAUSE