#include <idc.idc>

//w sanescobar nadal obowiązuje embargo na symbole debugowania i pluginy do Ida-pro, bohaterski lud pracujący musi radzić sobie inaczej :|

static echo_clean( flag ) {
       if( flag == 1 ) {
	      DelStruc( GetStrucIdByName("ECHO_IMAGE_DATA_DIRECTORY"));
		  DelStruc( GetStrucIdByName("ECHO_IMAGE_FILE_HEADER"));
		  DelStruc( GetStrucIdByName("ECHO_IMAGE_OPTIONAL_HEADER"));
		  DelStruc( GetStrucIdByName("ECHO_IMAGE_NT_HEADERS"));
		  DelStruc( GetStrucIdByName("ECHO_SYSTEM_MODULE_ENTRY"));
		  DelStruc( GetStrucIdByName("ECHO_SYSTEM_MODULE_INFORMATION"));
                  DelStruc( GetStrucIdByName("ECHO_DRIVER_OBJECT_x64")); 
                  DelStruc( GetStrucIdByName("ECHO_MDL_x64"));      
          DelStruc( GetStrucIdByName("ECHO_WSPPROC_TABLE_x64") );				  
	   }
}

static echo_error(ret, msg) {

       print("%s",msg);
	   return ret;
	   
}

static echo_add_structures( ) {

       auto p_struct = 0;
       auto ECHO_IMAGE_NUMBEROF_DIRECTORY_ENTRIES = 16;

       p_struct = AddStrucEx(-1,"ECHO_IMAGE_DATA_DIRECTORY",0);
       if( p_struct == -1 ) {
           return echo_error(-1,"AddStrucEx(-1,ECHO_IMAGE_DATA_DIRECTORY,0) FAIL !");
       }

       AddStrucMember(p_struct, "VirtualAddress", 0, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "Size", 4, FF_BYTE, -1, 4);

       p_struct = AddStrucEx(-1,"ECHO_IMAGE_FILE_HEADER",0);
       if( p_struct == -1 ) {
           return echo_error(-1,"AddStrucEx(-1,ECHO_IMAGE_FILE_HEADER,0) FAIL !");
       }

       AddStrucMember(p_struct, "Signature", 0, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "Machine", 4, FF_BYTE, -1, 2);             
       AddStrucMember(p_struct, "NumberOfSections", 6, FF_BYTE, -1, 2);    
       AddStrucMember(p_struct, "TimeDateStamp", 8, FF_BYTE, -1, 4);       
       AddStrucMember(p_struct, "PointerToSymbolTable", 12, FF_BYTE, -1, 4); 
       AddStrucMember(p_struct, "NumberOfSymbols", 16, FF_BYTE, -1, 4); 
       AddStrucMember(p_struct, "SizeOfOptionalHeader", 20, FF_BYTE, -1, 2); 
       AddStrucMember(p_struct, "Characteristics", 20, FF_BYTE, -1, 2); 

       p_struct = AddStrucEx(-1,"ECHO_IMAGE_OPTIONAL_HEADER",0);
       if( p_struct == -1 ) {
           return echo_error(-1,"AddStrucEx(-1,ECHO_IMAGE_OPTIONAL_HEADER,0) FAIL !");
       }
    
       AddStrucMember(p_struct, "Magic", 0, FF_BYTE, -1, 2);
       AddStrucMember(p_struct, "MajorLinkerVersion", 2, FF_BYTE, -1, 1);
       AddStrucMember(p_struct, "MinorLinkerVersion", 3, FF_BYTE, -1, 1);
       AddStrucMember(p_struct, "SizeOfCode", 4, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "SizeOfInitializedData", 8, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "SizeOfUninitializedData", 12, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "AddressOfEntryPoint", 16, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "BaseOfCode", 20, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "BaseOfData", 24, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "ImageBase", 28, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "SectionAlignment", 32, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "FileAlignment", 36, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "MajorOperatingSystemVersion", 40, FF_BYTE, -1, 2);
       AddStrucMember(p_struct, "MinorOperatingSystemVersion", 42, FF_BYTE, -1, 2);
       AddStrucMember(p_struct, "MajorImageVersion", 44, FF_BYTE, -1, 2);
       AddStrucMember(p_struct, "MinorImageVersion", 46, FF_BYTE, -1, 2);
       AddStrucMember(p_struct, "MajorSubsystemVersion", 48, FF_BYTE, -1, 2);
       AddStrucMember(p_struct, "MinorSubsystemVersion", 50, FF_BYTE, -1, 2);
       AddStrucMember(p_struct, "Win32VersionValue", 52, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "SizeOfImage", 56, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "SizeOfHeaders", 60, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "CheckSum", 64, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "Subsystem", 68, FF_BYTE, -1, 2);
       AddStrucMember(p_struct, "DllCharacteristics", 70, FF_BYTE, -1, 2);
       AddStrucMember(p_struct, "SizeOfStackReserve", 72, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "SizeOfStackCommit", 76, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "SizeOfHeapReserve", 80, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "SizeOfHeapCommit", 84, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "LoaderFlags", 88, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "NumberOfRvaAndSizes", 92, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "DataDirectory", 96, FF_BYTE, -1, GetStrucSize(GetStrucIdByName("ECHO_IMAGE_DATA_DIRECTORY")) *  ECHO_IMAGE_NUMBEROF_DIRECTORY_ENTRIES);

       p_struct = AddStrucEx(-1,"ECHO_IMAGE_NT_HEADERS",0); 
       if(p_struct == -1) {
          return echo_error(-1,"AddStrucEx(-1,ECHO_IMAGE_NT_HEADERS,0) FAIL !");
       }
	   
	   AddStrucMember(p_struct, "FileHeader", 0, FF_BYTE, -1, GetStrucSize(GetStrucIdByName("ECHO_IMAGE_FILE_HEADER")));
       AddStrucMember(p_struct, "OptionalHeader", GetStrucSize(GetStrucIdByName("ECHO_IMAGE_FILE_HEADER")), FF_BYTE, -1, GetStrucSize(GetStrucIdByName("ECHO_IMAGE_OPTIONAL_HEADER")));


       p_struct = AddStrucEx(-1,"ECHO_SYSTEM_MODULE_ENTRY",0);  //SYSTEM_MODULE_ENTRY
	   if(p_struct == -1) {
	      return echo_error(-1,"AddStrucEx(-1,ECHO_SYSTEM_MODULE_ENTRY,0) FAIL !");
	   }
   
       AddStrucMember(p_struct, "Section", 0, FF_BYTE, -1, 4);
	   AddStrucMember(p_struct, "MappedBase", 4, FF_BYTE, -1, 4);
	   AddStrucMember(p_struct, "ModuleBaseAddress", 8, FF_BYTE, -1, 4);
	   AddStrucMember(p_struct, "ModuleSize", 12, FF_BYTE, -1, 4);
	   AddStrucMember(p_struct, "Unknow", 16, FF_BYTE, -1, 4);
	   AddStrucMember(p_struct, "ModuleEntryIndex", 20, FF_BYTE, -1, 4);
	   AddStrucMember(p_struct, "ModuleNameLength", 24, FF_BYTE, -1, 2);
	   AddStrucMember(p_struct, "ModuleNameOffset", 26, FF_BYTE, -1, 2);
	   AddStrucMember(p_struct, "ModuleName", 28, FF_BYTE, -1, 256);
	   
	   p_struct = AddStrucEx(-1,"ECHO_SYSTEM_MODULE_INFORMATION",0); 
	   if(p_struct == -1) {
	      return echo_error(-1,"AddStrucEx(-1,ECHO_SYSTEM_MODULE_INFORMATION,0) FAIL !");
	   }
	   
	   AddStrucMember(p_struct, "Count", 0, FF_BYTE, -1, 4);
	   AddStrucMember(p_struct, "Module", 4, FF_BYTE, -1, GetStrucSize(GetStrucIdByName("ECHO_SYSTEM_MODULE_ENTRY")) * 2);
       
       //nowe
       p_struct = AddStrucEx(-1,"ECHO_DRIVER_OBJECT_x64",0);  //_DRIVER_OBJECT (x64)
       if(p_struct == -1) {
	      return echo_error(-1,"AddStrucEx(-1,ECHO_DRIVER_OBJECT_x64,0) FAIL !"); 
       }

       AddStrucMember(p_struct, "Type", 0, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "Size", 4, FF_BYTE, -1, 4);
       AddStrucMember(p_struct, "DeviceObject", 8, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "Flags", 16, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "DriverStart", 24, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "DriverSize", 32, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "DeviceSection", 40, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "DeviceExtension", 48, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "DriverName", 56, FF_BYTE, -1, 16);
       AddStrucMember(p_struct, "HardwareDatabase", 72, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "FastIoDispatch", 80, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "DriverInit", 88, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "DriverStartIo", 96, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "DriverUnload", 104, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "MajorFunction", 112, FF_BYTE, -1, 224);	  

       p_struct = AddStrucEx(-1,"ECHO_MDL_x64",0);  //_MDL (x64)
       if(p_struct == -1) {
	      return echo_error(-1,"AddStrucEx(-1,ECHO_MDL_x64,0) FAIL !"); 
       }

       AddStrucMember(p_struct, "Next", 0, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "Size", 8, FF_BYTE, -1, 2);             //spakowane w 8 bajtow
       AddStrucMember(p_struct, "MdlFlags", 10, FF_BYTE, -1, 6);
       AddStrucMember(p_struct, "Process", 16, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "MappedSystemVa", 24, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "StartVa", 32, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "ByteCount", 40, FF_BYTE, -1, 8); 
       AddStrucMember(p_struct, "ByteOffset", 44, FF_BYTE, -1, 8); 
	   
	   //USER_LAND
	   
	   p_struct = AddStrucEx(-1,"ECHO_WSPPROC_TABLE_x64",0);  //_MDL (x64)
       if(p_struct == -1) {
	      return echo_error(-1,"AddStrucEx(-1,ECHO_WSPPROC_TABLE_x64,0) FAIL !"); 
       }
	   
	   AddStrucMember(p_struct, "lpWSPAccept", 0, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPAddressToString", 8, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPAsyncSelect", 16, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPBind", 24, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPCancelBlockingCall", 32, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPCleanup", 40, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPCloseSocket", 48, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPConnect", 56, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPDuplicateSocket", 64, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPEnumNetworkEvents", 72, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPEventSelect", 80, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "lpWSPGetOverlappedResult", 88, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPGetPeerName", 96, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPGetSockName", 104, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPGetSockOpt", 112, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPGetQOSByName", 120, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPIoctl", 128, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPJoinLeaf", 136, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPListen", 144, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPRecv", 152, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPRecvDisconnect", 160, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPRecvFrom", 168, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPSelect", 176, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPSend", 184, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPSendDisconnect", 192, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPSendTo", 200, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPSetSockOpt", 208, FF_BYTE, -1, 8);
       AddStrucMember(p_struct, "lpWSPShutdown", 216, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPSocket", 224, FF_BYTE, -1, 8);
	   AddStrucMember(p_struct, "lpWSPStringToAddress", 232, FF_BYTE, -1, 8);
	   
       return 1;

}


static main() {
       auto ret;
	   echo_clean( 1 );
	   ret = echo_add_structures( );
	   if( ret != 1 ) return echo_error(-1,"echo_add_structures( ) FAIL !");
	   else {
	         return echo_error(0,"Structure created : echo_add_structures( ) OK !");  
	   }
}