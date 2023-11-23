#include <idc.idc>

class Musk {
        
      Musk(mask) {

           msg("Musk::constructor\r\n");

           this.mask = mask;
    
           this.values = object();
           
           this.values[0] = object();
           this.values[0].name  = "DELETE";
           this.values[0].value = 0x00010000L;

           this.values[1] = object();
           this.values[1].name = "READ_CONTROL (Standard Access Rights)";
           this.values[1].value = 0x00020000L;

           this.values[2] = object();
           this.values[2].name = "WRITE_DAC (Standard Access Rights)";
           this.values[2].value = 0x00040000L;
   
           this.values[3] = object();
           this.values[3].name = "WRITE_OWNER (Standard Access Rights)";
           this.values[3].value = 0x00080000L;

           this.values[4] = object();
           this.values[4].name = "SYNCHRONIZE (Standard Access Rights)";
           this.values[4].value = 0x00100000L;

           this.values[5] = object();
           this.values[5].name = "STANDARD_RIGHTS_REQUIRED (DELETE, READ_CONTROL, WRITE_DAC, and WRITE_OWNER)";
           this.values[5].value = 0x000F0000L;

           this.values[6] = object();
           this.values[6].name = "STANDARD_RIGHTS_READ (READ_CONTROL)";
           this.values[6].value = 0x00020000L;

           this.values[7] = object();
           this.values[7].name = "STANDARD_RIGHTS_WRITE (READ_CONTROL)";
           this.values[7].value = 0x00020000L;
            
           this.values[8] = object();
           this.values[8].name = "STANDARD_RIGHTS_EXECUTE (READ_CONTROL)";
           this.values[8].value = 0x00020000L;

           this.values[9] = object();
           this.values[9].name = "STANDARD_RIGHTS_ALL (DELETE, READ_CONTROL, WRITE_DAC, WRITE_OWNER, and SYNCHRONIZE)";
           this.values[9].value = 0x001F0000L;
           
           this.values[10] = object();
           this.values[10].name = "SPECIFIC_RIGHTS_ALL";
           this.values[10].value = 0x0000FFFFL;
     
           this.values[11] = object();
           this.values[11].name = "GENERIC_READ";
           this.values[11].value = 0x80000000L;

           this.values[12] = object();
           this.values[12].name = "GENERIC_WRITE";
           this.values[12].value = 0x40000000L;

           this.values[13] = object();
           this.values[13].name = "GENERIC_EXECUTE";
           this.values[13].value = 0x20000000L;
 
           this.values[14] = object(); 
           this.values[14].name = "GENERIC_ALL";
           this.values[14].value = 0x10000000L;

           this.values[15] = object(); 
           this.values[15].name = "ACCESS_SYSTEM_SECURITY";
           this.values[15].value = 0x01000000L; 

           this.values[16] = object(); 
           this.values[16].name = "MAXIMUM_ALLOWED";
           this.values[16].value = 0x02000000L;

           this.values_size = 16;

      }

      chck() {
           auto i;
           auto whole = "ACCESS_MASK: ";
           for(i=0;i<this.values_size; i++) {
               if(this.mask & this.values[i].value) {
                  msg("ACCESS_MASK value (0x%x) contains %s privilege\r\n",this.mask,this.values[i].name);
                  whole = whole + this.values[i].name + "|";  
               } 
           }
           
           whole[strlen(whole) - 1] = "";
           msg(whole);
    }

      ~Musk() {
            Msg("Musk::destructor");
      }

};

static the_musk() {

       auto val = ask_long(0x60FEED3F, "Enter your ACCESS_MASK value Elon");
       auto m = Musk(val);
    
       m.chck();

}


static main() {

       the_musk();
 
}