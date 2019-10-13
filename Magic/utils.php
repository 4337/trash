<?php

require_once("env.php");

function R($min, $max) {
	return random_int($min, $max);
}

function VARNAME(){
	 $chars = '0123456789abcdefghijklmnopqrstuvwxyz';
	 return 'jsv_'.substr(str_shuffle($chars),0,4).substr(str_shuffle($chars),0,3);
}

function _DBG( $msg ) {
       echo("[DBG] ".$msg."\r\n");	
}

function _DUMPVARS( &$context ) {
	  
	     $v = $context->get_vars( );
	     $s = sizeof( $v );
		 for($i=0;$i<$s;$i++) {
			 printf(
			        "%d].                \r\n".
			        "   var-name     : %s\r\n".
			        "   var-type     : %s\r\n".
					"   has-instance : %d\r\n",
					$i,
					$v[$i]->object,
					utils::unnormalize(get_class($v[$i]->object)),
					$v[$i]->object->__has_instance__
					);
		 }
		 
}


class utils {
	
	  /*
	    : konwertuje tablice właściwości w formacie json do opisów właściwości xml :
        IN: inline_props = tablica właściwości eg. json_decode( '{"a":2,"b":{},"c":...} )
        OUT: null=error || tablica którą można przekazać do vjs->set_properties		
		NOTE: nie obsługuje zagnieżdżonch obiektów eg. a={a:1,b:{x:sss}},... : 
		      b jest dostępny jako obiekt ale bez właściwości x
	  */
	  public static function to_properties( &$inline_props ) {
		     $r = [];
			 if( !empty( $inline_props ) ) {
			     foreach( $inline_props as $key => $val ) {
					      $type = utils::value_type_string( $val );
					      $tpl = [
						          'name'=>$key,
								  'visibility'=>"JS_VISIBLE_IN_INSTANCE",
								  'type'=>$type,
								  'writeable'=>'true',
				                  'readable'=>'true',
								  'callable'=>($type == "JS_FUNCTION") ? 'true' : 'false',
								  'input'=>$type,
					              'output'=>$type
						  ];
				          $r[] = $tpl;
			     }
				 return $r;
			 }
			 return null;
	  }
	
	  public static function unnormalize( $class_name ) {
		     return preg_replace('/_/','',$class_name,1);  
	  }
	  
	  public static function normalize( $class_name ) {
		     return '_'.(string)$class_name;  
	  }
	  
	  public static function js_str( $str ) {
		     return '"'.$str.'"';
	  }
	  
	  public static function frmt_argv( $args ) {
	       $ret = '';
           for($i=0;$i<sizeof($args);$i++) {
			   $ret .= $args[$i];
			   if( $i < (sizeof( $args ) - 1) ) {
				   $ret .= ',';  
		       }
		   }
           return $ret; 		   
	  }
	  
	  public static function line( $js ) {
		     if( !empty($js) ) {
                 return env::_try().$js.";".env::_catch()."\n";
             }				 
	  }
	  
	  public static function value_type_string( $val ) {
			 
			 switch( self::value_type( $val ) ) {
				     case JS_NUMBER:
                          return "JS_NUMBER";
                     case JS_STRING:
                          return "JS_STRING";
                     case JS_VOID: 
                          return "JS_VOID";
                     case JS_CONSTRUCTOR:
                          return "JS_CONSTRUCTOR";
                     case JS_PROTOTYPE:
                          return "JS_PROTOTYPE";
                     case JS_PROTO:
                          return "JS_PROTO";
                     case JS_SYMBOL:
                          return "JS_SYMBOL";
                     case JS_BOOLEAN:
                          return "JS_BOOLEAN";
                     case JS_NUMBER:	
                          return "JS_NUMBER";
                     case JS_STRING:		
                          return "JS_STRING";
                     case JS_OBJECT:	
                          return "JS_OBJECT";
                     case JS_FUNCTION:
					      return "JS_FUNCTION";
					 case JS_ARRAY:
					      return "JS_ARRAY";
					 case JS_UNSUPPORTED:
					      return "JS_UNSUPPORTED";
			 }			 
	  }
	  
	  public static function value_type( $val ) {  // to musi być z xml-a przecież 
		     if( is_numeric( $val ) || $val == "NaN" ) {
				 return JS_NUMBER; 
			 }
			 if ( is_null( $val ) || $val == 'null' ) {
			      return JS_VOID;  
			 }
			 if ( is_string( $val )) {
                  return JS_STRING;
             }
             if ( is_array( $val ) ) { //todo: || val = [...] || val = new Array || val == Array
				   return ( ( sizeof($val) > 0 ) && ( array_keys( $val ) !== range(0, sizeof($val) - 1) ) ) ? JS_OBJECT : JS_ARRAY;
			 }	
			 if( $val instanceof _Array ) {
				 return JS_ARRAY; 
			 }
			 if( $val instanceof unsupported ) {
					  return JS_UNSUPPORTED;  
		     }
             if( $val instanceof _constructor){
			     return JS_CONSTRUCTOR;
			 }
             if( $val instanceof _prototype){
	  		     return JS_PROTOTYPE;
			 }
			 if( $val instanceof ___proto__){
				 return JS_PROTO;
			 }
			 if( $val instanceof _Symbol){
		  	     return JS_SYMBOL;
			 }
			 if( $val instanceof _Boolean){
				 return JS_BOOLEAN;
			 }
			 if( $val instanceof _Number){
				 return JS_NUMBER;
			 }
			 if( $val instanceof _String){
				 return JS_STRING;
			 }
			 if( $val instanceof _Object){
				 return JS_OBJECT;
			 }
			 if( $val instanceof _Function){
				 return JS_FUNCTION;
			 }			 
	  }
	  
	  public static function wstring( $len ) {
		     $r = '';
		     $c = ["Ą","ó","縗","A","B","C","D","E","F",
			       "G","H","a","b","c","d","e","f","g",
				   "h","1","2","3","4"," ","5","6","7","8",
				   "9","0","_","<",">","֍","ओ",
				   "௵","ᑂ"
			 ];
			 $s = sizeof($c);
			 for($i=0;$i<$len;$i++) {
				 $r .= $c[R(0, $s - 1)];
		     }
			 return $r;
	  }
	  
};

?>