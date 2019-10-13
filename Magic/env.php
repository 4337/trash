<?php

set_error_handler("__PHP_WARNING", E_WARNING);

function __PHP_WARNING($errno, $errstr) { 
}

class env {
	  
	  private static $use_try = false;  /*from config file || command line */
	  
	  public static function _try() {
		     return (self::$use_try == true) ? "try{ " : "";
	  }
	
	  public static function _catch() {
	         return (self::$use_try == true) ? " }catch(e){}" : "";
	  }
	  
};

?>