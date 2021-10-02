<?php

require_once("utils.php");
require_once("js_types.php");
require_once("js_primitives.php");

define('JSCRIPT_DICTIONARY',getcwd().'\jscript.xml');

define('JS_MAX_PROTO', 3);
define('JS_MAX_NESTED_CALL', 3);

class unsupported {
	
	  private $__var_name__;
	  private $__value__;
	  private $__id__;
	  private $__type__; 
	  private $__has_varname__;
	  
	  public function &__get( $name ) {
		     return $this->$name;
	  }
	  
	  public function __set( $name, $value ) {
		     $this->$name = $value;  
	  }
	  
	  public function __toString() {
		    return (string)$this->__var_name__; 
	  }
	  
	  public function __call($name, $arguments) {
		     throw new Exception("JS_GRAMMAR::unsupported::__call : you try call method on unsupported type !!", JS_UNSUPPORTED_TYPE_EXCEPTION  );  
	  }
	  
	  public function __construct( $type, $value = null, $vn = null ) {
		     $this->__var_name__ = strval($vn);
			 $this->__value__ = $value;
			 $this->__type__ = $type;
			 $this->__has_varname__ = false;
	  }	  
	  
 };

class jsvar {
	
	  private $object;     /* object : eg. _String, _Number, .... */
	  
	  private $proto_count = 0;
	  
	  public $constans;    
	  public $declared;
	  
	  public function __toString() {
		     return $this->object->__var_name__;
	  }
	  
	  public function &__get( $pname ) {
		     return $this->$pname;
	  }
	  
	  public function __set($name, $val) {
		       $this->$name = $val;  
	  }
	  
	  public function __construct(&$object, $vn = null) {
		  
		     if(!class_exists( get_class($object) )) {
				 throw new Exception("JS_GRAMMAR::jsvar::__constructor : javascript unsupported type");				
			 }
			 
			 if(!empty($vn)) {
				 $object->__var_name__ = strval($vn); 
				 $object->__has_varname__ = true;
			 }
			 
		     $this->object = $object;
			 $this->constans = false;
	         $this->declared = false;
			
	  } 
	
};

class context {
	
	  private $__vars__;    /* tablica "zmiennych" [jsvar] */
	  private $__dic__;     /* słownik js/xml *///todo: trzeba zmienić na static
	  
	  private $__immutable__;  /* zmienna dla genereratora funkcji (nie zmienia typu w czasie wykonania) */
	  
	  /*
	   *
	   * ********* ********************** privates ********************** *********
	   *
	   */
	   
	   /*
	    : zaktualizuj informacje o właściwości w tablicy Object->__props__ :
		IN : js_object - obiekt, prop_name - nazwa właściwości, value - wartość 
	    OUT : void
	   */
	   private function update_prop_info( &$js_object, $prop_name, $value = null ) {
               
			   $t = ($value != null) ? utils::value_type_string($value)   
								     : "JS_ANY";
									 
			   if($js_object->__has_instance__ == true) {
				  $v = "JS_VISIBLE_IN_INSTANCE";  
				  $insta = JS_INSTANCE;
			   } else {
				  $v = "JS_VISIBLE_IN_CLASS";
				  $insta = JS_CLASS;
			   }
			   
			   $t = [   
				     'name'=>$prop_name,
					 'visibility'=>$v,
					 'type'=>$t,
					 'writeable'=>'true',
				     'readable'=>'true',
					 'callable'=>($t == "JS_FUNCTION") ? 'true' : 'false',
					 'input'=>$t,
					 'output'=>$t    
			   ];
			   
			   if( ($index = $js_object->get_property_index( $js_object->__props__[$insta], $prop_name )) == -1 ) {
				    $js_object->__props__[$insta][] = (object)$t;
					_DBG("JS_GRAMMAR::context::update_prop_info : object : ".$js_object." : prop_name : ".$prop_name." created - 2 type : ".$t['type']);
			   } else {
				  $v = $js_object->__props__[$insta][$index]->visibility;
				  $t['visibility'] = (string)$v;
				  $js_object->__props__[$insta][$index] = (object)$t;  
				  _DBG("JS_GRAMMAR::context::update_prop_info : object : ".$js_object." : prop_name : ".$prop_name." updated - 2 type : ".$t['type']);
			   }
			   
       }
	   
      /*
        : zmiana widoczności - (przypadek specjalny : X.constructor.__proto__ = null ) :
		IN : js_object = obiekt ^^
        OUT : void
      */	  
	  private function visibility_update_via_null_constructor_proto( &$js_object ) {  

              $o = clone $js_object;
              $top = $this->top_object( $o );
			  $t = get_class( $top );
			  
			  for( $i=0;$i<sizeof($this->__vars__); $i++ ){
				   if( get_class( $this->__vars__[$i]->object ) == $t ) {
                       if( $this->__vars__[$i]->object->__has_varname__ == false ) {
						   for($j=0;$j<sizeof( $this->__vars__[$i]->object->__methods__[JS_CLASS] );$j++){
							   /* $insta = ($this->__vars__[$i]->object->__has_instance__) ? JS_INSTANCE : JS_CLASS; */
                               if(constant($this->__vars__[$i]->object->__methods__[JS_CLASS][$j]->visibility) == JS_VISIBLE_IN_INSTANCE_AND_CLASS) {
							      $this->__vars__[$i]->object->__methods__[JS_CLASS][$j]->visibility = "JS_VISIBLE_IN_INSTANCE";
							   }
                           } 
					   }
                   }						  
			  }
      }
	  
	  /*
	    : pobierz najbardziej lewy obiekt lol (top obiekt) :
		IN : js_object - obiekt
	    OUT : top obiekt
	  */
	  private function top_object( &$js_object ) {
		  
		      if( !is_object( $js_object )) {
				   return $js_object;  
			  }
			  
			  $o = $js_object;
			  
			  do {
					
				  $t = $o;
				  
				  $insta = ($t->__has_instance__ == true) ? JS_INSTANCE
					                                      : JS_CLASS;
														  
				  if( empty( $t->__props__[$insta] ) ) {  
				      $this->get_properties_by_object( $t );
			      } 
			 
			      if( empty( $t->__methods__[$insta] ) ) {  
				      $this->get_methods_by_object( $t );
			      }
					
				  if($o->__prev__ == null) {  
					 break;
				  }

                  $o = $t->__prev__;					
					
			  } while ( true );

              return $o;
				
	  }
	  
	  /*
	    : pobiera informacje o właściwości z xml-a lub z __props__ :
	    IN : value - js_object lub wartość
		OUT : obiekt - informacje o właściwości
	  */
	  private function property_info( $value  ) {
		      
			  if( is_object( $value ) ) {
				  if( $value->__prev__ != null ) {
					  $s = 0;
				      $insta = ($value->__prev__->__has_instance__ == true) ? JS_INSTANCE
					                                                        : JS_CLASS;
					  if( empty($value->__prev__->__props__[$insta] ) ) {
					      $s = $this->get_properties_by_object( $value->__prev__ );
					  }
					  if( $s > 0 ) {
					      $index = $value->__prev__->get_property_index( $value->__prev__->__props__[$insta], $value );
						  return $value->__prev__->__props__[$insta][$index];
					  }
				  }
			  }
			  
			  $n = explode('.', $value);
			  $n = ( !empty( $n ) ) ? $n[ sizeof($n) - 1] : $value;
			  $c = utils::value_type( $value );  //TODO: FIX FIX FIX !!!!!!
			  $p = @$this->__dic__->xpath("//class[name=\"".$c."\"/propertie[name=\"".$n."\"]"); 
			  
			  return $p;
	  }
	  
	  /*
	    : utworz instancje obiektu na podstawie wartośći :
		IN : l_object - , r_value - 
	    OUT : instancja obiektu o określonm typie
	  */
	  private function object_by_val( &$l_object, $r_value ) {
		  
		      if( is_object( $r_value ) ){
				  return $r_value;  
			  }
			  
			  $t = $this->property_info( $r_value );
			  if( empty( $t ) ) {
			      $t = utils::value_type( $r_value );
			  } else {
				$t = constant( $t->type );  
			  }
			  
			  switch( $t ) {
				      case JS_UNSUPPORTED:
						   $r = $r_value;
					  break;
					  case JS_STRING:
					  default: 
                              $r = new $t();	
							  $r->__has_instance__ = true;//$l_object->__has_instance__;  //?? tak czy nie tak 
                              $r->__prev__ = $l_object->__prev__;
			  }
			  
			  return $r;
			  
      }		  
	  /*
	    : pobierz pierwszy obiekt od prawej (pomin prototype i __proto) w łańcuchu :
		IN : [&] js_object - obiekt eg. _Number
	    OUT : pierwszy obiekt od prawej po pominięciu proto i prototype
	  */
	  private function first_top( &$js_object ) {
		      if( !is_object( $js_object )) {
				  return $js_object;
			  }
			  
			  $o = clone $js_object;
			  $r = $js_object;
			  
			  do {
				  
				  $t = $o;
				 
			      if($r->__prev__ != null) {
					  
					  $insta = ($r->__prev__->__has_instance__ == true) ? JS_INSTANCE
					                                                    : JS_CLASS;
					  if( empty( $r->__prev__->__props__[$insta] ) ) {  
				            $this->get_properties_by_object( $r->__prev__ );
			          } 
			 
			          if( empty( $r->__prev__->__methods__[$insta] ) ) {  
				          $this->get_methods_by_object( $r->__prev__ );
			          }  
					  
				  }
				  
				  if($o->__prev__ == null) {  
				     break;
				  } 
				   
				  if( !($o instanceof _prototype ) && !($o instanceof ___proto__) ) {
				        break;   
				  }
					
                  $r = $r->__prev__;					
				  $o = $t->__prev__;
				  
			  } while( true );
			  
			  return $o;
	  }
	  
	  /*
	    : czy lewy i prawy są tego samego typu :
		: weryfikacja typów oraz instancji :
		IN : left, right
	    OUT: true || false
	  */
	  private function the_same_ex( &$left, &$right ) {
		      if( is_object( $right ) ) {
				  $l = $this->first_top( $left );
				  $r = $this->first_top( $right );
				  if( get_class($l) == get_class($r) && 
   				      $l->__has_instance__ == $r->__has_instance__ ){
					  return true;	  
				  }
			  }
			  return false;
	  }
	  
 	  /*
	    : czy lewy i prawy są tego samego typu :
		IN : left = lewy obiekt , right = prawy obiekt
		OUT : true = są tego samego typu | false = nie są 
	  */
	  private function the_same( &$left, &$right ) {
              if( is_object( $right ) ){
				  $l = $this->first_top( $left );
				  $r = $this->first_top( $right );
				  if( get_class($l) == get_class($r) ){
					  return true;  
				  }
			  }
			  return false;
      }		  
	  
	  /*
	    : pobierz index obiektu z __vars__ na podstawie jsvar->object :
		IN : js_object - obiekt :) eg. _String
		OUT: -1 = nie ma obiektu w vars albo index z vars
	  */
	  private function var_index_1( &$js_object ) {
		      $s = sizeof($this->__vars__);
              for($i=0;$i<$s;$i++) {
				  _DBG("context->var_index_1 :: vars[".$i."] : ".$this->__vars__[$i]->object->__var_name__." js_object : ".$js_object->__var_name__);
                  if(($this->__vars__[$i]->object->__var_name__ == strval($js_object->__var_name__)) && 
				     ($this->__vars__[$i]->object->__id__ == $js_object->__id__) ){ 
				      return $i;	
			      }				
              }				
              return -1;	
	  }
	  /* 
	    : pobierz nazwę klasy bazowej :
	    IN : [&] js_object = jsvar->object eg. _String, _Number, _Symbol, ... 
		OUT: nazwa klasy bazowej ; (obsługa JS_PARENT_TYPE)
	  */
	  private function get_base_class_name( &$js_object ) {
		   
		      $c_o = clone $js_object;
			  $n = utils::unnormalize( get_class($c_o) );
			  
			  _DBG("get_base_class_name :: js_object : ".$js_object." type ".get_class( $c_o )." n : ".$n);
			  
			  do {
				  
				  $t = $this->__dic__->xpath("//class[name='".$n."']/base");
				  $s = sizeof( $t );
				  
				  if($s > 0) {
					 if( @constant($t[0]) != JS_PARENT_TYPE ) {
						 if( !empty($t[0]) ) { 
						    $n = (string)$this->__dic__->xpath("//class[name='".$t[0]."']")[0]->name; 
							_DBG("get_base_class_name :: (type != JS_PARENT_TYPE) n : ".$n);
						 }
					 } else { /* klasa bazowa jest typu rodzica */
					    /* 0. Jeśli __prev__ != null */
						/* 1. sprawdź czy __prev__->__has_varname__ == true, jeśli tak użyj get_class, jeśli nie 
                              użyj __prev__ */	
                        /* 2. Jeśli __prev__ == null (to prawdopodobnie musimy rzucić wyjątek, bo coś się zepsuło */
                        if( $c_o->__prev__ == null ) {
                            throw new Exception("JS_GRAMMAR::get_base_class_name exception : invalid js_object !\r\n",JS_INVALID_PARENT_TYPE_EXCEPTION);
                        } 
                        if( $c_o->__prev__->__has_varname__ == true ) {
                            $n = utils::unnormalize( get_class( $c_o->__prev__ ) );
							_DBG("get_base_class_name :: (type == JS_PARENT_TYPE) : __prev__->has_varname == TRUE : n : ".$n);
                        } else {
                           $n = (string)$c_o->__prev__;
						   _DBG("get_base_class_name :: (type == JS_PARENT_TYPE) : __prev__->has_varname == FALSE : n : ".$n);
                        }							
				     }		  
				  } 
				  
				  $c_o = $c_o->__prev__;
				  
			  }while($c_o != null);
		      
			  return $n;
	  }
	  
	  /*
	    : pobierz metody dla obiektu :
		IN : [&] js_object = jsvar->object eg. _String, ...
		         base_name = STRING = obiekt bazowy (nazwa) | null = pobierz nazwę obiektu bazowego
	    OUT : ilość pobranych metod
	  */
	  private function get_methods_by_object( &$js_object, $base_name = null ) {
		      $p = explode('.',$js_object);
			  $p = $p[sizeof($p) - 1];
			  
			  $v = ( ($js_object->__has_instance__ == true) || ($p == "prototype") ) ? "and (visibility='JS_VISIBLE_IN_INSTANCE_AND_CLASS' or visibility='JS_VISIBLE_IN_INSTANCE')" 
			                                                                         : "and (visibility='JS_VISIBLE_IN_INSTANCE_AND_CLASS' or visibility='JS_VISIBLE_IN_CLASS')";
			 
			  $n = ( $base_name == null ) ? $js_object->__base_class_name__ = $this->get_base_class_name( $js_object ) : $base_name;
			  $m = $this->__dic__->xpath("//class[name='".$n."']/method[not(./instance) ".$v."]");
			  $s = sizeof($m);
			  _DBG("get_methods_by_object :: js_object : ".$js_object." class[name='".$n."'] size of methods : ".$s);
			  if($s > 0) {
				 $js_object->set_methods( $m );
			  }
			  return $s;
	  }
	  
	  /*
	    : pobierz składowe dla obiektu :
		IN : [&] js_object = jsvar->object eg. _String, _Number, ...
		         base_name = STRING = obiekt bazowy (nazwa) | null = pobierz nazwę obiektu bazowego
	    OUT : ilość pobranych właściwośći
	  */
	  private function get_properties_by_object( &$js_object, $base_name = null ) {
		  	  $p = explode('.', $js_object);
              $p = $p[sizeof($p) - 1];
              
			  $v = ( ($js_object->__has_instance__ == true) || ($p == "prototype") ) ? "visibility='JS_VISIBLE_IN_INSTANCE_AND_CLASS' or visibility='JS_VISIBLE_IN_INSTANCE'" 
			                                                                         : "visibility='JS_VISIBLE_IN_INSTANCE_AND_CLASS' or visibility='JS_VISIBLE_IN_CLASS'";
			 
			  $n = ( $base_name == null ) ?  $js_object->__base_class_name__ =  $this->get_base_class_name( $js_object )  : $base_name;
			  $p = $this->__dic__->xpath("//class[name='".$n."']/propertie[".$v."]");
			  $s = sizeof($p);
			  _DBG("get_properties_by_object :: js_object : ".$js_object." class[name='".$n."'] size of properties : ".$s);
			  if($s > 0) {
				 $js_object->set_properties( $p );   
			  }
              return $s;		  
	  }
	  
	  /*
	   *
	   * ********* ********************** publics ********************** *********
	   *
	   */
	   
	  public function __construct( ) {
			   if( !file_exists( JSCRIPT_DICTIONARY ) ||
                  ( ($this->__dic__ = simplexml_load_file(JSCRIPT_DICTIONARY)) == FALSE) ) {
					 throw new Exceptions( "simplexml_load_file(".JSCRIPT_DICTIONARY.") in js\context::__construct" );
			   }
			   $this->__vars__  = [];		
               $this->__cache__ = [];	
               $this->__immutable__ = [];			   
	  }
	  
	  /*
	    : zwraca losową metodę obiektu przekazanego jako argument (wyszukiwanie po obiekcie) :
	    IN : js_object = obiekt 
		OUT : m = definicja metody ze słownika
	  */
	  public function random_method_by_o( &$js_object ) {
		  
		     $insta = ($js_object->__has_instance__ == true) ? JS_INSTANCE : JS_CLASS;
			 //if(empty(js_object->methods[insta])){
			 //   
		     //   $this->get_methods_by_object( js_object );
			 //}
			 //$m = $js_object->methods[insta][ R(0, sizeof($js_object->methods[insta]) - 1) ];
			 //return $m;
	  }
	  /*
	    : wywolanie metody bez przypisania eg. object->metoda(argumenty) lub object->metodaA()->metodaB()->wlasnosc->metodaX() itd :
		: ogólnie może tworzyć łańcuch wywołań :
		IN :
		OUT :
	  */
	  //l->funkcjaA()->propA->propB|funckjaB()->funkcjaC() ...
	  //dokończysz ?
	  public function js_call(&$l_object, $method = null, $arguments = []) {
		  
		     payload::add_context( $this );
			 
			 //...
			 $nested_deep = R(0, JS_MAX_NESTED_CALL);
			 //...
			 //...
	  }
	  
	  /*
	    : przypisanie eg. a = b
		IN : l_object = lewy obiekt :) eg. _Symbol
		     r_value = prawa wartość (obiekt lub cokolwiek)
	    OUT : string (przypisanie eg. a = b)
	  */
	  public function js_assign( &$l_object, $r_value = null ) {
			 
			 payload::add_context( $this );
			 
			 if( $r_value == null ) {
				 throw new Exception("JS_GRAMMAR::context::js_assign : can not asign right to left when right not exsits",JS_SYNTAX_ERROR_EXCEPTION);
		     }

			 if( $this->the_same_ex( $l_object, $r_value ) ) {
				 return $l_object.' = '.$r_value;
			 }

			 $r = $this->first_top( $r_value );
			 if( is_object( $r ) ) {
			     $r = $this->get_base_class_name( $r );
			     $r = utils::normalize( $r );
				 $r = new $r( );
			 }
			 
			 $r = $this->object_by_val( $l_object, $r );
			 
			 $this->get_properties_by_object( $r );
			 $this->get_methods_by_object( $r );
			 
			 if( $l_object instanceof ___proto__ ) {
				 if ( ( ($l_object->__prev__ instanceof _constructor ) ) && ( utils::value_type( $r_value ) == JS_NULL ) ) { 
				     /* case : X.constructor.__proto__ = null; */
					 $this->visibility_update_via_null_constructor_proto( $l_object );
					 return $l_object .' '.$r_value;
				 }
			 }
			 
			 $o_left = clone $l_object;
			 
			 do {
				 
				 $t = $l_object;
				 
				 if( $l_object->__prev__ == null ) {
					 break; 
				 }
				 
				 if(!($t instanceof ___proto__)) {
					   break;
			     } else {
				   if( ++$this->proto_count == JS_MAX_PROTO ) { /* CHCK !!! */          
                         throw new Exception("JS_GRAMMAR::context::js_assign : __proto__ is null",JS_TO_MUCH_PROTO_EXCEPTION);
                   }						 	
				 }
				  
				 $l_object = $t->__prev__;    
				 
		     } while( true );
			 
			 $this->proto_count = 0;
			 
			 if($o_left instanceof ___proto__)  {
				if( $this->the_same_ex( $l_object, $r ) ) {
					throw new Exception("JS_GRAMMAR::context::js_assign : can't set prototype: it would cause a prototype chain cycle",666);
			    }
			 }
			 /* FIX : Błąd logiczny tu jest !!!!!*/
			 if ( !$l_object->is_writeable( ) ) {          //no-effect : todo: poprawienie atrybutów dostępu w słowniku xml
  			      return ($o_left." = ".$r_value);
			 }
			 
			 if( $l_object->__prev__ != null ) {   
				 
				 $p = explode('.', $l_object);
				 $p = $p[ sizeof( $p ) - 1 ];
				 
				 $vn =  strval( $o_left->__var_name__ );
				 $rid = $o_left->__id__;	
				 $hvn = $o_left->__has_varname__;
				 
				 $l_object->__prev__->{ $p } = $r;
				 
				 $l_object->__prev__->{ $p }->__var_name__ = strval( $vn );
				 $l_object->__prev__->{ $p }->__id__       = $rid;
			     $l_object->__prev__->{ $p }->__has_varname__ = $hvn;

				 $this->update_prop_info( $l_object->__prev__, $p , $r );
				 //...
				 
			 } else {
				
			   $index = $this->var_index_1( $l_object );
			   
			   if( $this->__vars__[$index]->constans == true ) {  //const a = 1 <- ok, const b.c = 1 <- error
				  throw new Exception("JS_GRAMMAR::context::js_assign : redeclaration of const ".$l_object, JS_SYNTAX_ERROR_EXCEPTION);
			   }
			   
			   $vn = $this->__vars__[$index]->object->__var_name__;
			   $rid = $this->__vars__[$index]->object->__id__;
			   $hvn = $this->__vars__[$index]->object->__has_varname__;
			   
			   $this->__vars__[$index]->object = $r;
			   
			   $this->__vars__[$index]->object->__var_name__ = strval($vn);	
			   $this->__vars__[$index]->object->__id__ = $rid;
			   $this->__vars__[$index]->object->__has_varname__ = $hvn;
			   
		     }
			 
			 return ($o_left." = ".$r_value);
			 
	  }
	  
	  /*
	    : const a = b :
		IN : l_object = obiekt lub inicjalizacja obiektu
		     r_value = obiekt lub cokolwiek 
			 js_var = gdy l_object jest inicjalizacją obiektu to js_var musi być równe inicjalizowanemu obiektowi - wszystko jasne :)
	    OUT : (string) const var = cokowliek
	  */
	  public function js_const( &$l_object, $r_value = null, $js_var = null ) {
		   
		     payload::add_context( $this );
			 
			 $o = ($js_var != null) ? $js_var->object
			                        : $l_object;
				
             if(strpos($o,".") == true) {  
                throw new Exception("JS_GRAMMAR::js_const : unexpected . in left ",JS_SYNTAX_ERROR_EXCEPTION);
             }	
			 
			 $index = $this->var_index_1( $o );
			 
			 _DBG("JS_GRAMMAR::context::js_const : index : ".$index." r_value : ".$r_value." o-l : ".$o);
			 
			 
             if( $this->__vars__[$index]->declared == true ) { 
			     throw new Exception("JS_GRAMMAR::js_const : redeclaration of left ",JS_SYNTAX_ERROR_EXCEPTION);
		     }		

             if( $this->__vars__[$index]->constans == true ) {  
				 throw new Exception("JS_GRAMMAR::js_const redeclaration of const ".$this->__vars__[$index]->object,JS_SYNTAX_ERROR_EXCEPTION);
		     }	
			 
			 $this->__vars__[$index]->constans = true;
			 
			 if($r_value == null) {
				if ($js_var == null) {
			        throw new Exception("JS_GRAMMAR::js_const missing = in const declaration",JS_SYNTAX_ERROR_EXCEPTION);
				} else {
				  return "const ".$l_object;
				}
			 }		
			 
			 if( $this->the_same_ex( $l_object, $r_value ) ) {
				 return 'const '.$l_object.' = '.$r_value; 
			 }
			 
			 $r = $this->first_top( $r_value );
			 if( is_object( $r ) ) {
			     $r = $this->get_base_class_name( $r );
			     $r = utils::normalize( $r );
				 $r = new $r( );
			 }
			 $r = $this->object_by_val( $l_object, $r );
			 
			 $this->get_properties_by_object( $r );
			 $this->get_methods_by_object( $r );
			 
			 $vn = $this->__vars__[$index]->object->__var_name__;
		     $rid = $this->__vars__[$index]->object->__id__;
		     $hvn = $this->__vars__[$index]->object->__has_varname__;

			 $this->__vars__[$index]->object = $r; 
			 	 
		     $this->__vars__[$index]->object->__var_name__ = strval($vn);	
			 $this->__vars__[$index]->object->__id__ = $rid;
			 $this->__vars__[$index]->object->__has_varname__ = $hvn;
			  
             _DBG("JS_GRAMMAR::context::js_var : l_object : ".$l_object." r : ".$r);
			  
		     return "const ".$l_object." = ".$r_value;	

	  }
	  
	  /*
	    : var a = b :
		IN :
		OUT :
	  */
	  public function js_var( &$l_object, $r_value = null, $js_var = null ) {
		  
		     payload::add_context( $this );
			 
			 $o = ($js_var != null) ? $js_var->object
			                        : $l_object;
				
             if(strpos($o,".") == true) {  
                throw new Exception("JS_GRAMMAR::js_var : unexpected . in left ",JS_SYNTAX_ERROR_EXCEPTION);
             }	

             $index = $this->var_index_1( $o );
			 
			 _DBG("JS_GRAMMAR::context::js_var : index : ".$index." r_value : ".$r_value." o-l : ".$o);
			 
			 
             if( $this->__vars__[$index]->declared == true ) { 
			     throw new Exception("JS_GRAMMAR::js_var : redeclaration of left ",JS_SYNTAX_ERROR_EXCEPTION);
		     }		

             if( $this->__vars__[$index]->constans == true ) {  
				 throw new Exception("JS_GRAMMAR::js_var redeclaration of const ".$this->__vars__[$index]->object,JS_SYNTAX_ERROR_EXCEPTION);
		     }	
			 
			 $this->__vars__[$index]->declared = true;
			 
			 if($r_value == null) {
			    return "var ".$l_object;
			 }		
			 
			 if( $this->the_same_ex( $l_object, $r_value ) ) {
				 return 'var '.$l_object.' = '.$r_value; 
			 }
			 
			 $r = $this->first_top( $r_value );
			 if( is_object( $r ) ) {
			     $r = $this->get_base_class_name( $r );
			     $r = utils::normalize( $r );
				 $r = new $r( );
			 }
			 $r = $this->object_by_val( $l_object, $r );
			 
			 $this->get_properties_by_object( $r );
			 $this->get_methods_by_object( $r );
			 
			 $vn = $this->__vars__[$index]->object->__var_name__;
		     $rid = $this->__vars__[$index]->object->__id__;
		     $hvn = $this->__vars__[$index]->object->__has_varname__;

			 $this->__vars__[$index]->object = $r; 
			 	 
		     $this->__vars__[$index]->object->__var_name__ = strval($vn);	
			 $this->__vars__[$index]->object->__id__ = $rid;
			 $this->__vars__[$index]->object->__has_varname__ = $hvn;
			  
             _DBG("JS_GRAMMAR::context::js_var : l_object : ".$l_object." r : ".$r);
			  
		     return "var ".$l_object." = ".$r_value;	
			 
	  }
	  
	  /*
	    : let a = b :
		IN : l_object - lewy obiekt eg. _String 
		     r_value - prawa wartość lub null
			 js_var - jsvar lub null :
	    OUT: string - kod js eg. let x = blablabla
	  */
	  public function js_let( &$l_object, $r_value = null, $js_var = null ) {
		     
			 payload::add_context( $this );
			 
		     $o = ($js_var != null) ? $js_var->object 
			                        : $l_object;
									
			 if(strpos($o,".") == true) {  
                throw new Exception("JS_GRAMMAR::js_let : unexpected . in left ",JS_SYNTAX_ERROR_EXCEPTION);
             }
				
             $index = $this->var_index_1( $o );
			 
			 _DBG("JS_GRAMMAR::context::js_let : index : ".$index." r_value : ".$r_value." o-l : ".$o);

             if( $this->__vars__[$index]->declared == true ) { 
			     throw new Exception("JS_GRAMMAR::js_let : redeclaration of left ",JS_SYNTAX_ERROR_EXCEPTION);
		     }		

             if( $this->__vars__[$index]->constans == true ) {  
				 throw new Exception("JS_GRAMMAR::js_let redeclaration of const ".$this->__vars__[$index]->object,JS_SYNTAX_ERROR_EXCEPTION);
		     }				 
			 
			 $this->__vars__[$index]->declared = true;
			 
			 if($r_value == null) {
			    return "let ".$l_object;
			 }		
			 
			 if( $this->the_same_ex( $l_object, $r_value ) ) {
				 return 'let '.$l_object.' = '.$r_value; 
			 }
			 
			 $r = $this->first_top( $r_value );
			 if( is_object( $r ) ) {
			     $r = $this->get_base_class_name( $r );
			     $r = utils::normalize( $r );
				 $r = new $r( );
			 }
			 $r = $this->object_by_val( $l_object, $r );
			 
			 $this->get_properties_by_object( $r );
			 $this->get_methods_by_object( $r );
			 
			 $vn = $this->__vars__[$index]->object->__var_name__;
		     $rid = $this->__vars__[$index]->object->__id__;
		     $hvn = $this->__vars__[$index]->object->__has_varname__;

			 $this->__vars__[$index]->object = $r; 
			 	 
		     $this->__vars__[$index]->object->__var_name__ = strval($vn);	
			 $this->__vars__[$index]->object->__id__ = $rid;
			 $this->__vars__[$index]->object->__has_varname__ = $hvn;
			  
             _DBG("JS_GRAMMAR::context::js_let : l_object : ".$l_object." r : ".$r);
			  
		     return "let ".$l_object." = ".$r_value;						
			 
	  }
	  
	  /*
	    : wywołaj zmienną jak funkcje + uwzględni __immutable__ jeśli parametr poprawny :
	    IN : type 
		OUT: string wywołanie funkcji 
	  */
	  public function call_var_ex( $type = JS_VAR ) {
		     
             if( $type != JS_VAR ) {
                 $s = sizeof($this->__immutable__);
                 if( $s > 0 ) {
					 $t = [];
					 for($i=0;$i<$s;$i++) {
						 if( $this->__immutable__[$i]->object->__callable__ == true ) {
							 array_push( $t, $this->__immutable__[$i]->object );
						 }
				     }
					 if( !empty( $t ) ) {
						 shuffle( $t ); 
						 $args = payload::dumb_args( $t[0]->__callable__argc__ );
				         return $t[0]."(".implode(",",$args).")";
					 }
			     }
				 return '';
             }
			 
             return $this->call_var();			 
	  } 
	  
	  /*
	    : wywołaj zmienną jak funckje jesli callable = true :
		IN: void
	    OUT: string wywołanie funckji
	  */
	  public function call_var( ) {
		  
		     $t = [];
			 
			 for($i=0;$i<sizeof($this->__vars__);$i++) {
                 if( $this->__vars__[$i]->object->__callable__ == true ) {
			         array_push( $t, $this->__vars__[$i]->object );
				 }
             }

			 if( !empty( $t )) {
			      shuffle( $t );
				  $args = payload::dumb_args( $t[0]->__callable__argc__ );
				  return $t[0]."(".implode(",",$args).")";
			 }
			 
			 return '';
	  }
	  
	  /*
	    : zwraca ref __vars__ :
		IN : void
		OUT : tablica obiektów jsvar
	  */
	  public function &get_vars( ) {
		     return $this->__vars__;  
	  }
	  
	  /*
	    : zwróć losową właściwość obiektu w formacie obiekt.właściwość :
		  :: metoda uzupełnia info o metodach i właściwościach ::
		  :: obiektów jeśli istnieje taka potrzeba             ::
	    IN : js_object = jsvar->object eg. _String, _Symbol, ...
		OUT: właściwość eg. String.length
	  */
	  public function random_property_ex( &$js_object ) {
		     
			 $insta = ($js_object->__has_instance__ == true) ? JS_INSTANCE : JS_CLASS;
			 
			 if( empty( $js_object->__props__[$insta] ) ) {
				 $this->get_properties_by_object( $js_object );
			 }
			 
			 $p = $js_object->__props__[$insta][ R(0, sizeof( $js_object->__props__[$insta] ) - 1) ];
			 
			 _DBG("JS_GRAMMAR::random_property : js_object : ".$js_object." p : ".$p->name);
			 
			 $t = &$js_object->{$p->name};  //call __get - dla pewności
			 
			 if( is_object( $t ) ) {
			
 			     if( empty( $t->__props__[JS_INSTANCE] ) ) {  //czy zawsze dla instancji ??
				     $this->get_properties_by_object( $js_object->{$p->name} );
			     } 
			 
			     if( empty( $t->__methods__[JS_INSTANCE] ) ) {  //czy zawsze dla instancji ??
				     $this->get_methods_by_object( $js_object->{$p->name} );
			     }
				 
			 }

			 return $js_object->{$p->name};
			 
	  }
	  
	  /*
	    : zwraca losową zmienną jsvar z __vars__ :c
		IN : void
		OUT : losowa instancja jsvar z this->__vars__
	  */
	  public function &random_var( ) {
		     return $this->__vars__[ R(0, sizeof( $this->__vars__ ) - 1) ];   
	  }
	  
	  /*
	    : zwraca losową zmienną jsvar z __immutable__ :c
		IN : void
		OUT : losowa instancja jsvar z this->__immutable__
	  */
	  public function &random_immutable( ) {
		     return (!empty( $this->__immutable__ )) ? $this->__immutable__[ R(0, sizeof( $this->__immutable__ ) - 1) ]
                                                     : null;			 
	  }
	  
	  
	  /*
	    : pobierz obiekty z instancją lub bez :
		IN : instance = true = obiekty mające instancje || false = obiekty niemające instancji 
		OUT : tablica obiektów w formacie [var=>jsvar, index=offest w tablicy __vars__
	  */
	  public function objects_with_instance( $instance = true ) {
		     $o = [];
			 for($i=0;$i<sizeof($this->__vars__);$i++) {
				 if( $this->__vars__[$i]->object->__has_instance__ == $instance ) {
					 $o[] = ['var'=>&$this->__vars__[$i],'index'=>$i];
				 }
			 }
			 return $o;
	  }
	  
	  /*
	    : pobierz obiekty z __vars__ posiadające nazwę zmiennej lub nie :
		IN : $status = true = obiekt ma nazwę zminnej || false = obiekt nie posiada nazwy zmiennej
		OUT : tablica obiektów w formacie [var=>jsvar,index=offest w tablicy __vars__]
	  */
	  public function objects_with_name( $status = true ) {
		     $o = [];
			 for($i=0;$i<sizeof($this->__vars__);$i++) {
				 if( ($this->__vars__[$i]->object->__has_varname__ == $status) && 
					 ($this->__vars__[$i]->declared == false) && 
					 ($this->__vars__[$i]->constans == false) ) {
					  
					  $o[] = ['var'=>&$this->__vars__[$i],'index'=>$i];						
					 
					 }
		     }
			 return $o;
	  }
	  
	  /*
	    : pobierz losową metodę do tworzenia instancji (_new||_instance) :
	    IN : [&] js_object
		OUT : (XMLObject) metoda ze słownika :)
	  */
	  public function random_instance_method( &$js_object ) {
		     $n = utils::unnormalize( get_class( $js_object ) );
			 $m = $this->__dic__->xpath("//class[name='".$n."']/method[instance='true']");
			 $m = $m[ R(0, sizeof($m) - 1) ];
			 return $m;
	  }
	  
	  /*
	    : utwórz i zwróć losową instancje głównego obiektu (taki ktory nie jest komponentem :)) :
		IN : ...$var_name = zmienna ilość argumentów zgodnych z definicją konstruktora poszczególnego obiektu 
		                    z reguły jest tu nazwa zmiennej lub nie ma nic 
		OUT : instancja obiektu 
	  */
	  public function random_main_class( ...$var_name ) {
		     $main_clss = $this->__dic__->xpath("//class[component='false']"); 
			 $c_name = utils::normalize( $main_clss[ R(0, sizeof($main_clss) -1 ) ]->name );
			 _DBG("random_main_class :: c_name : ".$c_name);
			 return new $c_name( ...$var_name );
      }
	  
	  /*
	    : ustaw zmienne dla generatora funckji :
		: metoda kopiuje dwie ostatnie zmienne z __vars__ do __immutable__ :
		: musi zostać wywoałana po bloku set_var :
		IN : void
		OUT : void 
	  */
	  public function immutable_init(  ) {

             $this->__immutable__ = array_slice( $this->__vars__, -2 );		 
			 
	  }
	  
	  /*
	    : usuniń orginały przeniesione do __immutable__ z __vars__ :
		: metoda musi być wywołana po bloku inicjalizacji zmiennych :
		IN : void 
		OUT : void 
	  */
	  public function immutable_end( ) {
		   
             array_pop( $this->__vars__ );
			 array_pop( $this->__vars__ );
		   
	  }
	  
	  /*
	    : dodaj zmienną "js" jsvar do kontextu :
		IN  : js_var = instancja jsvar 
	    OUT : void
	  */
	  public function set_var( $js_var ) {
		  
		     if( $js_var->object->__has_varname__ == true ) {				 
				 $js_var->object->__has_instance__ = true;   
			 }
			 
			 $index = array_push($this->__vars__, $js_var); 
			 
			 $this->__vars__[$index - 1]->object->__base_class_name__ = $this->get_base_class_name( $js_var->object );
			 
			 $this->get_methods_by_object( $this->__vars__[$index - 1]->object, $this->__vars__[$index - 1]->object->__base_class_name__ );
			 $this->get_properties_by_object( $this->__vars__[$index - 1]->object, $this->__vars__[$index - 1]->object->__base_class_name__ );
			 
	  }
	
};


?>