#!/bin/bash

NFS_TIMEO=5;

ADDR_LIST=();

MOUNTPOINT_PREFIX="/tmp/mount"

function nfsShowMount {
         local ARGC=$#;
         if [ $ARGC -lt 1 ] ; then
            echo "(*) Podaj adres serwera !";
            exit;
         else
            local RET=0;
            local ADDR=$1;
            RET=( $(showmount -a $ADDR 2>/dev/null) ); 
            if [ -z "$RET" ] ; then 
               echo "(!) Error  "$ADDR" !";
               RET=0;
            else 
               local cnt=5;
               local size=${#RET[@]};
               echo "(!) +++++++++++++++++++++++++++++++++++++++"
               echo "(!) Dostepne zasoby:                       "
               while [ $cnt -lt $size ] ; do
                     local share=${RET[$cnt]};
                     share=${share#*:};
                     echo "    "$share
                     local MP=$MOUNTPOINT_PREFIX$ADDR"_"$cnt;
                     mkdir $MP;
                     echo "    (*) mount -t nfs "$ADDR":"$share" "$MP;
                     mount -t nfs $ADDR":"$share $MP 2>/dev/null; # timeo=$NFS_TIMEO); # 2>/dev/null);
                     if [ ! $? -eq 0  ] ; then
                         echo "      (!) Nie moge zamontowac "$ADDR$share" !";
                     else 
                         echo "p0wnp0wn20wn0wn" > $MP"/pown.txt" 2>/dev/null;
                         if [  $? -eq 0 ] ; then
			    echo $ADDR$share >> ./NFS.log.txt
                            ls -la $MP >> ./NFS.log.txt #tod: tree & privs !
                            rm -v $MP"/pown.txt" > /dev/null;
                            umount $MP > /dev/null;
                         fi
                     fi
                     #rm -rv $MP > /dev/null; 
                     let cnt=cnt+1;           
               done
            fi
         fi
}


function nfsMainCheck {
         local counter=0;
         local size=${#ADDR_LIST[@]};
         while [ $counter -lt $size ] ; do 
                 local ADDR=${ADDR_LIST[$counter]};
                 echo "(*) Sprawdzam adres "${ADDR_LIST[$counter]}" !";
                 nfsShowMount $ADDR;
                 let counter=counter+1;
         done
}


function nfsGetAddrList {
         local ARGC=$#;
         if [ $ARGC -lt 1 ] ; then 
            echo "(*) Podaj sciezke do pliku !";
         else 
            local FILE=$1
            if [ ! -e $FILE ] ; then 
               echo "(*) Plik "$FILE"nie istnieje !";
            else
               if [ ! -s $FILE ] ; then
                  echo "(*) Plik "$FILE" jest pusty !";
               else
                 ADDR_LIST=( $(cat $FILE) );
                 echo "(*) Znaleziono "${#ADDR_LIST[@]}" adresy IP/DNS !";
               fi
            fi
         fi
}

if ! type $(showmount --help > /dev/null)  ; then 
    echo "(*) Musisz zainstalowac pakiet nfs-common (sudo apt-get install nfs-common) !";
    exit;
fi 

nfsGetAddrList $1;
nfsMainCheck;
    
