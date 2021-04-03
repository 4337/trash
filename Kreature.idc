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