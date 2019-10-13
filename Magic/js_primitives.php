<?php

require_once("utils.php");

define('JS_IS_WRITEABLE', "writeable");
define('JS_IS_READABLE',  "readable");
define('JS_IS_CALLABLE',  "callable");

define('JS_INSTANCE', "instance");
define('JS_CLASS',    "class");

abstract class vjs { 
	
	  /******************************************************************************/
	  /* składowe publiczne bo inaczej __get musiałby być                           */
	  /* nieodrzecznie złożony                                                      */
	  
	  public $__id__;
	  public $__var_name__;
	  public $__parent_name__;
	  public $__has_instance__;
	  public $__has_varname__;
	  public $__prev__; 

	  
	  public $__callable__argc__;        /* ilość argumentów w przypadku gdy __callable__ = true */
	  
	  public $__callable__;              /* eg. var a = new Function() then a.__callable__ == true and we can a() | var b = new Object() then b.__callbale__ = false and we can't b() */
	                                      /* THINK : todo : 27.07.2019 problem z __callable__ że musimy zdefiniować dodatkową metodę (funktor) i przechowywać informacje o argumentach inicjalizacyjnytch  */
							              /*                eg. a = new Function("a","b","c") <- obiekt a musi mieć info o ilości argumentów i najlepiej o typie aby wywołać a(1,2.2,3111); */
							
	  public $__base_class_name__;     
	  public $__props__;   
	  public $__methods__; 
	                     
      public $__setters__;
      public $__getters__;	  
	  
	  public $__cache__;
	  
	  /***************************************************************************/
	  /***************************************************************************/
	  
	  protected $__; //meta-amfetamina xD - dzięki __ nie musimy definiować składowych każdego obiektu js eg. Array.length 
	                 //                     definiują się same, a o tym czy są obecne/widoczne w instacji/klasie 
					 //                     decyduje słownik
	  
	  /***************************************************************************/
	  
	  public function __toString() {
		    return $this->__var_name__; 
	  }
	
	  public function __call($name, $arguments) {
			return $this->__var_name__.".".$name."(".implode(',',$arguments).")";
	  }
	 
	  public function __set( $name, $val ) {
		     $this->__[$name] = $val;
	  }
	  
	  public function set_base_name( $tbase ) {
		     $this->__base_class_name__ = $tbase;  
	  }
	  
	  abstract public function _new( $body = null, ...$args );
	  abstract public function _instance( $body = null, ...$args );
	  
	  /* *************** "emulated" part *************** */
	    
	  public function __defineGetter__($property, $fun) {
		  
		     if( (empty($property)) || (empty($fun)) ) {
                 throw new Exception("JS_PRIMITIVES::__defineGetter__ exception : invalid getter usage !",JS_INVALID_GETTER_EXCEPTION);
             }	
			 if( R( 0, 1 ) > 0 ){
				 $p = R(0,5);
			 } else {
			   $p = $property;
			 }
             $this->__getters__[$p] = $fun;
			 return $this->__var_name__.'.__defineGetter__('.$p.','.$fun.')';
			 
	  }
	  
	  public function __lookupGetter__($property = null) {
		  
			 _DBG("__lookupGetter__ :: property : ".$property);
			 
		     if(empty($property)) {
				return $this->__var_name__.'.__lookupGetter__()'; 
			 }
			 
			 $insta = ($this->__has_instance__ == true) ? JS_INSTANCE : JS_CLASS;
			 
			 if( $this->__cache__['__lookupGetterIndex__'] === -1 ) {
			     $this->__cache__['__lookupGetterIndex__'] = $this->get_method_index($this->__methods__[$insta], "__lookupGetter__");
			 }
			 
			 if(array_key_exists($property, $this->__getters__)) {
				_DBG("__lookupGetter__ :: change ret form : ".$this->__methods__[$insta][ $this->__cache__['__lookupGetterIndex__'] ]->ret." to JS_FUNCTION");
                $this->__methods__[$insta][ $this->__cache__['__lookupGetterIndex__'] ]->ret = "JS_FUNCTION";			
			 } else {
				_DBG("__lookupGetter__ :: change ret form : ".$this->__methods__[$insta][ $this->__cache__['__lookupGetterIndex__'] ]->ret." to JS_VOID");
                $this->__methods__[$insta][ $this->__cache__['__lookupGetterIndex__'] ]->ret = "JS_VOID";				
			 }
			 
			 return $this->__var_name__.'.__lookupGetter__('.$property.')';
			 
	  }
	  
	  public function __defineSetter__($property, $fun) {
		  
		     if( (empty($property)) || (empty($fun)) ) {
				 throw new Exception("JS_PRIMITIVES::__defineSetter__ exception : invalid setter usage !",JS_INVALID_SETTER_EXCEPTION);
		     }
			 
			 $this->__setters__[$property] = $fun;
			 return $this->__var_name__.'.__defineSetter__('.$property.','.$fun.')';
			 
	  }
	  
	  public function __lookupSetter__($property = null) {
		  
		     _DBG("__lookupSetter :: property : ".$property);
		  
		     if(empty($property)) {
				return $this->__var_name__.'.__lookupSetter__()'; 
			 }
			 
			 $insta = ($this->__has_instance__ == true) ? JS_INSTANCE : JS_CLASS;
			 
			 if( $this->__cache__['__lookupSetterIndex__'] === -1 ) {
			     $this->__cache__['__lookupSetterIndex__'] = $this->get_method_index($this->__methods__[$insta], "__lookupSetter__");
			 }
			 
			 if(array_key_exists($property, $this->__setters__)) {
				_DBG("__lookupSetter__ :: change ret form : ".$this->__methods__[$insta][ $this->__cache__['__lookupSetterIndex__'] ]->ret." to JS_FUNCTION");
                $this->__methods__[$insta][ $this->__cache__['__lookupSetterIndex__'] ]->ret = "JS_FUNCTION";			
			 } else {
				_DBG("__lookupSetter__ :: change ret form : ".$this->__methods__[$insta][ $this->__cache__['__lookupSetterIndex__'] ]->ret." to JS_VOID");
                $this->__methods__[$insta][ $this->__cache__['__lookupSetterIndex__'] ]->ret = "JS_VOID";				
			 }
			 
		     return $this->__var_name__.'.__lookupSetter__('.$property.')';
			 
	  }
	  
	  /* *********************************************** */
	  
	  
	  private function get_method_index( &$methods, $name ) {
		      
			  $s = sizeof( $methods );
			  for($i=0;$i<$s;$i++) {
				  if( is_object( $methods[$i] ) ) {
                      if( $methods[$i]->name == $name ) {
						  return $i;  
					  }
                  }					  
			  }
			  
			  return -1;
			  
	  }
	  
	  public function get_property_index( &$properties, $name ) {
		      
			  $s = sizeof( $properties );
			  for($i=0;$i<$s;$i++) {
				  if( is_object( $properties[$i] ) ) {
					  if( $properties[$i]->name == $name ) {
					      return $i; 
					  }
				  }
			  }
			  
			  return -1;
			  
	  }
	  
	  /*
        : czy obiekt/obiekt.właściwość można nadpisać :
		IN : void
		OUT : true = można || false = nie można 
	  */
	  public function is_writeable(  ) { //fix 
		  
		     if( $this->__prev__ == null ) {
				 return true;
			 }
			 
			 $insta = ($this->__prev__->__has_instance__ == true) ? JS_INSTANCE : JS_CLASS;
			 
			 $p = explode('.',$this );
			 $p = $p[ sizeof($p) - 1 ];
			 
			 $index = $this->get_property_index( $this->__prev__->__props__[$insta], $p );
			 
			 _DBG("JS_PRIMITIVES::vjs::is_writeable : index : ".$index." p : ".$p." this : ".$this." prev : ".$this->__prev__." writable : ".$this->__prev__->__props__[$insta][$index]->writeable );
			 
			 return ( $this->__prev__->__props__[$insta][$index]->writeable == 'true' ) ? true : false;
		  
	  }
	 
	  public function &__get( $name ) {  /* TODO : optymalizacja | make this code grate again - ogólnie ma być pięknie */
			 
			 $t = null;
			 $insta = ($this->__has_instance__ == true) ? JS_INSTANCE : JS_CLASS;
			 
			 _DBG("JS_PRIMITIVES::__get : name : ".$name);

             $this->__[$name] = $this->__var_name__.".".$name;
			 
			 if( ($index = $this->get_property_index($this->__props__[$insta], $name)) != -1 ) {
		          $c = constant( $this->__props__[$insta][$index]->type );
		          if( class_exists( $c ) ) {
				      $t = $c;
			      } 
			 }

			 if( class_exists( '_'.$name ) ){  
		         $t = utils::normalize($name);
			 }
			 
			 if( $t != null ) {
				 $class_n = $t;
				
				 if( is_object($this->__[$name]) == false ) {
				     /* !! ten obiekt ma puste właściwośći __props__ i __methods__ !! */
					 $this->__[$name] = new $class_n( $this->__var_name__.'.'.$name,  $this->__var_name__ );
				     $this->__[$name]->__has_instance__ = true;
					 $this->__[$name]->__has_varname__ = $this->__has_varname__;
					 $this->__[$name]->__prev__ = &$this;  
				 }
				 
			 } 

	 	     return $this->__[$name];   
	  }
	  
	  public function set_methods( &$methods ) { 
	         if( $this->__has_instance__ == true ) {
		         $this->__methods__[JS_INSTANCE] = $methods + $this->__methods__[JS_INSTANCE]; 
			 } else {
				 $this->__methods__[JS_CLASS] = $methods + $this->__methods__[JS_CLASS];
		     }
	  }
	  
	  public function set_properties( &$props ) { 
	   
	         if( $this->__has_instance__ == true ) {
				 $this->__props__[JS_INSTANCE] = $props + $this->__props__[JS_INSTANCE];
		     } else {
		       $this->__props__[JS_CLASS] = $props + $this->__props__[JS_CLASS]; 
			 }
	  }
	  
	  public function __construct($var_name = "",$parent_name = null, $nested_call = false) {
		  
		     $this->__id__                     = uniqid();
		     $this->__var_name__               = $var_name;
			 $this->__parent_name__            = $parent_name;
			 $this->__has_instance__           = false;
			 $this->__prev__                   = null;
			 
			 $this->__base_class_name__        = null;
			 $this->__props__                  = [
			                                      'instance'=>[],
												  'class'=>[]
			                                     ];
	         $this->__methods__                = [
			                                      'instance'=>[],
			                                      'class'=>[]
												 ];
		    
			 $this->__callable__               = false;
			
             $this->__setters__                = [];
			 $this->__getters__                = [];
			 
			 $this->__cache__                  = [
			                                      '__lookupGetterIndex__'=>-1,
												  '__lookupSetterIndex__'=>-1
												 ];
												 
		     $this->__                         = [];
			 
			 $o_name = utils::unnormalize( get_called_class() );
			 $vn = substr(strrchr($var_name, "."), 1);
			 if($vn === false) {
				$vn = $var_name;
			 }
			 
			 $this->__has_varname__     = ($vn != $o_name) ? true : false;
			 
			 
			 
			 
			 if( $nested_call == true ) {   // CHCK !!! to powinien być _Function nie vjs 
			     $this->__['caller']          = clone $this;                          
			     $this->__['caller']->__parent_name__ = $this->caller->__var_name__;
			     $this->__['caller']->__var_name__ .= ".caller";	
            } else {
                 $this->__['caller'] = $this->__var_name__.".caller"; 
            }	
            
            			
	  }
	
};

class _constructor extends vjs {
	  
	  public function __construct($var_name = 'constructor',$parent_name = null, $nested_call = false) {
		     parent::__construct($var_name, $parent_name, $nested_call);
	  }
	  
	  public function _instance( $arg0 = null, ...$args ) { /* !!!!!! CHCK !!!!!!!!! */
		    $a_and_b = '';
			if( !empty($arg0) ) {
				
		    if( (!preg_match("/^\[+[0-9a-zA-Z\"\'\, ]{0,}+\]/s",$arg0)) &&
			    (!preg_match("/^[ ]{0,}+new+[ ]{0,}+Array+[ ]{0,}/s",$arg0) === false ) ) {
					  throw new Exception("JS_PRIMITIVES::_constructor::_instance : ...constructor() _instance argument is not an array",JS_INVALID_ARGUMENT_EXCEPTION);	
				}
			    $a_and_b = $arg0;	
			}
			$this->__callable__ = true;
		    if((  $this->__var_name__ != "" ) && ( $this->__var_name__ != 'constructor' ) ) { /* var|let zmienna = cos.constructor() */ 
				return $this->__var_name__."(".$a_and_b.")";
			}
			return $this->__var_name__.".constructor(".$a_and_b.")"; /* <-- CHCK !!! */
	  }
	  
	  public function _new( $body = null, ...$args ) { 
	  }
};

class _prototype extends vjs {
	 
	  public function __construct($var_name = 'prototype',$parent_name = null, $nested_call = false) {
		     parent::__construct($var_name, $parent_name, $nested_call);
	  }
	  
	  public function _instance( $arg0 = null, ...$args ) {
		     throw new Exception("JS_PRIMITIVES::_prototype::_instance : prototype you can't create instance of prototype",JS_INVALID_CALL_EXCEPTION);
	  }
	  
	  public function _new( $arg0 = null, ...$args ) { 
	         throw new Exception("JS_PRIMITIVES::_prototype::_new : prototype doesn't have \"new\" operator",JS_INVALID_CALL_EXCEPTION);
	  }
};

class ___proto__ extends vjs {
	  
	 public function __construct($var_name = '__proto__',$parent_name = null, $nested_call = false) {
		    parent::__construct($var_name, $parent_name, $nested_call);
	 }
	 
	 public function _instance( $body = null, ...$args ) {
			throw new Exception("JS_PRIMITIVES::___proto__::_instance : you can't create instance of __proto__",JS_INVALID_CALL_EXCEPTION);
	 }
	  
	 public function _new( $body = null, ...$args ) { 
	        throw new Exception("JS_PRIMITIVES::___proto__::_new : __proto__ doesn't have \"new\" operator",JS_INVALID_CALL_EXCEPTION);
	 }
};

class _Symbol extends vjs {
	  
	  public function __construct($var_name = 'Symbol',$parent_name = null, $nested_call = false) {
			parent::__construct($var_name, $parent_name, $nested_call);
	  }
	  
	  public function _instance( $arg0 = null, ...$args ) {
		    $a_and_b = '';
			$this->__has_instance__ = true;
		    if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			}
			if( !empty($args) ) {
				$cargs = utils::frmt_argv($args);
			    $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			}
		    if((  $this->__var_name__ != "" ) && ( $this->__var_name__ != 'Symbol' ) ) { /* var|let zmienna = Function() */ 
				return $this->__var_name__." = Symbol(".$a_and_b.")";
			}
			return "Symbol(".$a_and_b.")";
	  }
	  
	  public function _new( $body = null, ...$args ) { 
	         throw new Exception("JS_PRIMITIVES::_Symbol::_new : Symbol doesn't have \"new\" operator",JS_INVALID_CALL_EXCEPTION);
	  }
};

class _Boolean extends vjs {
	  
	  public function __construct($var_name = 'Boolean',$parent_name = null, $nested_call = false) {
		    parent::__construct($var_name, $parent_name, $nested_call);
	  }
	  
	  public function _instance( $arg0 = null, ...$args ) {
		     $a_and_b = '';
			 $this->__has_instance__ = true;
			 if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			 }
			 if( !empty($args) ) {
			  	 $cargs = utils::frmt_argv($args);
			     $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			 }
			 if((  $this->__var_name__ != "" ) && ( $this->__var_name__ != 'Boolean' ) ) { /* var|let zmienna = Boolean() */ 
			  	 return $this->__var_name__." = Boolean(".$a_and_b.")";
			 }
			 return "Boolean(".$a_and_b.")"; 
	  }
	  
	  public function _new( $arg0 = null, ...$args ) { 
	        $a_and_b = '';
			$this->__has_instance__ = true;
			if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			}
			if( !empty($args) ) {
				$cargs = utils::frmt_argv($args);
			    $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			}
		    if((  $this->__var_name__  != "" ) && ( $this->__var_name__ != 'String' ) ) { /* var|let zmienna = new Boolean() */
				return $this->__var_name__." = new Boolean(".$a_and_b.")";
			}
			return "new Boolean(".$a_and_b.")";
	  }
};

/*
*
* MDN : BigInt is currently in stage 3 of the ECMAScript specification.
*       When it makes it to stage 4 of the draft, which is the final specification, BigInt will become 
*       the second built-in numeric type in JavaScript.
************************
class _BigInt extends vjs {
	 
	  public function __construct($var_name = 'BigInt',$parent_name = null, $nested_call = false) {
		    parent::__construct($var_name, $parent_name, $nested_call);
	  }
	 
	  public function _instance( $body = null, ...$args ) {
	  }
	  
	  public function _new( $body = null, ...$args ) { 
	  }
}
*/

class _Number extends vjs {
	
	  public function __construct($var_name = 'Number',$parent_name = null, $nested_call = false) {
		    parent::__construct($var_name, $parent_name, $nested_call);
	  }
	
	  public function _instance( $arg0 = null, ...$args ) {
		    $a_and_b = '';
			$this->__has_instance__ = true;
			if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			}
			if( !empty($args) ) {
				$cargs = utils::frmt_argv($args);
			    $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			}
		    if((  $this->__var_name__  != "" ) && ( $this->__var_name__ != 'Number' ) ) { /* var|let zmienna = new Number() */
				return $this->__var_name__." = new Number(".$a_and_b.")";
			}
			return "new Number(".$a_and_b.")";
	  }
	  
	  public function _new( $arg0 = null, ...$args ) { 
	        $a_and_b = '';
			$this->__has_instance__ = true;
			if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			}
			if( !empty($args) ) {
				$cargs = utils::frmt_argv($args);
			    $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			}
		    if((  $this->__var_name__  != "" ) && ( $this->__var_name__ != 'Number' ) ) { /* var|let zmienna = new Number() */
				return $this->__var_name__." = new Number(".$a_and_b.")";
			}
			return "new Number(".$a_and_b.")";
	  }
};

class _String extends vjs {
 
      public function __construct($var_name = 'String',$parent_name = null, $nested_call = false) {
		    parent::__construct($var_name, $parent_name, $nested_call);
	  }
 
      public function _instance( $arg0 = null, ...$args ) {
		    $a_and_b = '';
			$this->__has_instance__ = true;
			if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			}
			if( !empty($args) ) {
				$cargs = utils::frmt_argv($args);
			    $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			}
		    if((  $this->__var_name__ != "" ) && ( $this->__var_name__ != 'String' ) ) { /* var|let zmienna = String() */ 
				return $this->__var_name__." = String(".$a_and_b.")";
			}
			return "String(".$a_and_b.")"; 
	  }
	  
	  public function _imm( $arg ) {   /* wartość natychmiastowa eg. a = "DKWOKWDK" */
		     
			 $this->__has_instance__ = true;
			 
			 if((  $this->__var_name__  != "" ) && ( $this->__var_name__ != 'String' ) ) { /* var|let zmienna = new String() */
				   return $this->__var_name__." = ".$arg;
			 }
			 return $arg;
			 
	  }
	  
	  public function _new( $arg0 = null, ...$args ) { 
	        $a_and_b = '';
			$this->__has_instance__ = true;
			if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			}
			if( !empty($args) ) {
				$cargs = utils::frmt_argv($args);
			    $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			}
		    if((  $this->__var_name__  != "" ) && ( $this->__var_name__ != 'String' ) ) { /* var|let zmienna = new String() */
				return $this->__var_name__." = new String(".$a_and_b.")";
			}
			return "new String(".$a_and_b.")";
	  }
};

class _Object extends vjs {
	
	  public function __construct($var_name = 'Object',$parent_name = null, $nested_call = false) {
		     parent::__construct($var_name, $parent_name, $nested_call);
	  }
	  
	  public function _sym( $symbol, $value = _EMPTY ) {
		     return ( $value == _EMPTY ) ? $this->__var_name__."[".$symbol."]" 
                                         : $this->__var_name__."[".$symbol."]"."=".$value;			 
	  }
	  
	  public function _at( $index, $value = _EMPTY ) {
		     return ( $value == _EMPTY ) ? $this->__var_name__."[".$index."]" 
                                         : $this->__var_name__."[".$index."]"."=".$value;			 
	  }
	  
	  public function _instance( $arg0 = null, ...$args ) {
		    $a_and_b = '';
			$this->__has_instance__ = true;
			if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			}
			if( !empty($args) ) {
				$cargs = utils::frmt_argv($args);
			    $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			}
		    if((  $this->__var_name__ != "" ) && ( $this->__var_name__ != 'Object' ) ) { /* var|let zmienna = Object() */ 
				return $this->__var_name__." = Object(".$a_and_b.")";
			}
			return "Object(".$a_and_b.")";
	  }
	  
	  public function _new( $arg0 = null, ...$args ) { 
	        $a_and_b = '';
			$this->__has_instance__ = true;
			if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			}
			if( !empty($args) ) {
				$cargs = utils::frmt_argv($args);
			    $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			}
		    if((  $this->__var_name__  != "" ) && ( $this->__var_name__ != 'Object' ) ) { /* var|let zmienna = new Object() */
				return $this->__var_name__." = new Object(".$a_and_b.")";
			}
			return "new Object(".$a_and_b.")";
	  }
	  
	  public function _braces( $object_body = null ) { //todo: argumenty eg. a:1, b:2 get l: function(){}, c:2.2 - JS_OBJECT_BODY
		    
			 $this->__has_instance__ = true;
			 
			 if( !empty( $object_body )){
				 $b = '{'.$object_body.'}';
				 $p = json_decode( $b, true );  //nie może być setterów i getterów eg. get length(){}, ...
				 if( $p == null ) {
					 throw new Exception("JS_PRIMITIVES::_Object::_braces : invalid object body !",JS_INVALID_OBJECT_BODY_EXCEPTION); 
			     }
                 $p = utils::to_properties( $p );
				 $this->set_properties( $p );
			 }
			
			 if((  $this->__var_name__  != "" ) && ( $this->__var_name__ != 'Object' ) ) {
				   return $this->__var_name__." = {".$object_body."}";
		     }
			 
			 return "{".$object_body."}";
			 
	  }
	  
};

class _Function extends vjs {
	
	 public function __construct($var_name = 'Function',$parent_name = null, $nested_call = false) {
		     parent::__construct($var_name, $parent_name, $nested_call);
	 }
	  
	 public function _new( $argn = null, ...$args ) {  /* TODO : if(argn != null) then $args[0] <> null # jesli funckja ma argumenty to musi mieć ciało inaczej podczas wywołania ostatni argumemnt traktowany jest jako ciało */

            $a_and_b = '';
			if( !empty($argn) ) {
				$a_and_b = (!empty($args)) ? utils::frmt_argv($args) : '';
			    $a_and_b .= (!empty($a_and_b)) ? ','.$argn : $argn;	
				if( ($cnt = sizeof(explode(",",$argn)) ) > 1 ) {
					 $cnt = $cnt - 1;	
			    }
			} else {
			  $cnt = 0;	
		    }
		    $this->__has_instance__ = true;
		    $this->__callable__argc__ = $cnt;
			$this->__callable__  = true;
		    if((  $this->__var_name__  != "" ) && ( $this->__var_name__ != 'Function' ) ) { /* var|let zmienna = new Function() */
				return $this->__var_name__." = new Function(".$a_and_b.")";
			}
			return "new Function(".$a_and_b.")";
			
	 }
	 
	 public function _instance( $argn = null, ...$args ) {   /* TODO : if(argn != null) then $args[0] <> null # jesli funckja ma argumenty to musi mieć ciało inaczej podczas wywołania ostatni argumemnt traktowany jest jako ciało */

            $a_and_b = '';
			if( !empty($argn) ) {
				$a_and_b = (!empty($args)) ? utils::frmt_argv($args) : '';
			    $a_and_b .= (!empty($a_and_b)) ? ','.$argn : $argn;	
				if( ($cnt = sizeof(explode(",",$argn)) ) > 1 ) {
					 $cnt = $cnt - 1;	
			    }
			} else {
			  $cnt = 0;	
		    }
		    $this->__has_instance__ = true;
		    $this->__callable__argc__ = $cnt;
			$this->__callable__ = true;
		    if((  $this->__var_name__ != "" ) && ( $this->__var_name__ != 'Function' ) ) { /* var|let zmienna = Function() */ 
				return $this->__var_name__." = Function(".$a_and_b.")";
			}
			return "Function(".$a_and_b.")";
			
	 }
	 
};

class _Array extends vjs {
	
	 public function __construct($var_name = 'Array',$parent_name = null, $nested_call = false) {
		     parent::__construct($var_name, $parent_name, $nested_call);
	 }
	 
	 public function _instance( $arg0 = null, ...$args ) {
		    $a_and_b = '';
			$this->__has_instance__ = true;
			if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			}
			if( !empty($args) ) {
				$cargs = utils::frmt_argv($args);
			    $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			}
		    if((  $this->__var_name__ != "" ) && ( $this->__var_name__ != 'Array' ) ) { /* var|let zmienna = Object() */ 
				return $this->__var_name__." = Array(".$a_and_b.")";
			}
			return "Array(".$a_and_b.")";
	  }
	  
	  public function _new( $arg0 = null, ...$args ) { 
	        $a_and_b = '';
			$this->__has_instance__ = true;
			if( !empty($arg0) ) {
			    $a_and_b = $arg0;	
			}
			if( !empty($args) ) {
				$cargs = utils::frmt_argv($args);
			    $a_and_b .= (!empty($a_and_b)) ? ','.$cargs : $cargs; 
			}
		    if((  $this->__var_name__  != "" ) && ( $this->__var_name__ != 'Array' ) ) { /* var|let zmienna = new Object() */
				return $this->__var_name__." = new Array(".$a_and_b.")";
			}
			return "new Array(".$a_and_b.")";
	  }
	  
	  public function _at( $index, $value = _EMPTY ) {
		     return ( $value == _EMPTY ) ? $this->__var_name__."[".$index."]" 
                                         : $this->__var_name__."[".$index."]"."=".$value;			 
	  }
};


?>