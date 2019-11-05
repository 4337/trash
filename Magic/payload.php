<?php

require_once("utils.php");
require_once("js_regex.php"); 
require_once("js_grammar.php"); 

function _DUP( $str, $len ) {
         return str_repeat($str, $len);	
}

class payload {
	 
	  private static $jsc; 
	  private static $function_arg_names;
	  
	  private static $types = [JS_FUNCTION, JS_OBJECT, JS_SYMBOL, JS_BOOLEAN,
			                   JS_NUMBER, JS_STRING, JS_PROTOTYPE, JS_PROTO, 
				               JS_CONSTRUCTOR, JS_PROPERTIE, JS_ARRAY, JS_DATA_DESCRIPTOR,
							   JS_ITERABLE, JS_LOCALES, JS_ITERATOR, JS_REGEX,
							   JS_FORM_NORMALIZATION, JS_RADIX,
							   JS_SMALL_INT
							   ]; 
	  
	  /*
	    : dodaj "context" do payloadów
		IN : js_context = instacja context
		OUT : void
	  */
	  public static function add_context( &$js_context ) {
		     self::$jsc = $js_context;
	  }
	  
	  public static $strings;
	  public static $f_strings;
	  public static $numbers;
	  public static $objects;
	  public static $properties;
	  public static $small_ints;
	  public static $locales;
	  
	  public static function simple_arg( $type ) {
		     switch($type) {
					case JS_SMALL_INT:
					     return self::$small_ints[ R(0, sizeof(self::$small_ints) - 1) ];
					break;
					case JS_STRING:
					     if( R(0,1) > 0 ) { 
							 return utils::js_str( self::$strings[ R(0, sizeof(self::$strings) - 1) ] ); 
						  } 
						  return  utils::js_str( self::$f_strings[ R(0, sizeof(self::$f_strings) - 1) ] );
					break;
					case JS_OBJECT:
					     return self::inline_object( );
					break;
					case JS_ARRAY:
					     return '[]';
					break;
					case JS_NUMBER:
					default: return self::$numbers[ R(0, sizeof(self::$numbers) - 1) ];
			 }
	  }
	  
	  private static function locale_options( ) {
			  
			  $localeMatcher = ['lookup','best fit'];
			  $usage = ['sort','search'];
			  $sensitivity = ['accent','base','case','variant'];
			  $caseFirst = ['upper','lower','false'];
			  
			  $r = '{';
			  if( R(0, 1) > 0 ) {
				    $r .= 'localeMatcher: '.utils::js_str( $localeMatcher[ R(0, sizeof($localeMatcher) - 1)] );
			  }
			  if( R(0, 1) > 0 ) {
				    if( $r != '{' ) {
						$r .= ', ';
				    }
					$r .= 'usage: '.utils::js_str( $usage[ R(0, sizeof($usage) - 1) ] );
			  }
			  if( R(0, 1) > 0 ) {
				    if( $r != '{' ) {
						$r .= ', ';
				    }
					$r .= 'sensitivity: '.utils::js_str( $sensitivity[ R(0, sizeof($sensitivity) - 1) ] );
			  }
			  if( R(0, 1) > 0 ) {
					if( $r != '{' ) {
						$r .= ', ';
				    }
					$v  = (R(0,1) > 0) ? 'true' : 'false';
					$r .= 'ignorePunctuation: '.$v;
			  }
			  if( R(0, 1) > 0 ) {
				    if( $r != '{' ) {
						$r .= ', ';
				    }
					$v  = (R(0,1) > 0) ? 'true' : 'false';
					$r .= 'numeric: '.$v;
			  }
			  if( R(0, 1) > 0 ) {
				    if( $r != '{' ) {
						$r .= ', ';
				    }
                    $r .= 'caseFirst: '.utils::js_str( $caseFirst[ R(0, sizeof($caseFirst) - 1) ] );			  
			  }
		      $r .= '}';

			  return $r;
			  
	  }
	  
	  private static function format_function_body( $body ){
	          return str_replace('"','\"',$body);
	  }
	  
	  /*
	    : tworzy ciało boże funckji 
		  z założenia funkcja nie modyfikuje zmiennych z __vars__ ponieważ niechce mi się 
		  implementować w tym miejscu trackingu.
		  Z każdego argumentu tworzona jest kopia eg. f(a,b,c){ a1=a;b1=b;c1=c; } :
		IN : $argv = tablica argumentów 
		OUT: string = ciało funkcji
	  */
	  public static function function_body( $argv ) {
		   
		     $r = '';
			 $f = '';
		     $operations = [];
			 
			 $pre  = ['++','--','~','typeof ','void '];
			 $mid  = ['+','*','-','|','&','^','%','/','==','===','!=','>','<','>=','<=','||'];
			 $ass  = ['=','+=','*=','&=','|=','^=','%=','/='];
			 $post = ['++','--'];
			 
			 shuffle( $argv );
			 
			 $s    = sizeof( $argv );
			 
			 $arg1 = str_replace( '"', "", $argv[R(0, sizeof($argv) - 1 )] );
			 
			 for($i=0;$i<$s;$i++) {
				 $arg  = str_replace( '"', "", $argv[$i] );
				 
				 if( (R(0, 1) > 0) && (self::$jsc != null) ) {  //przypisanie właściwości
					  $f .= $arg.$i.'='.self::$jsc->random_property_ex( self::$jsc->random_immutable()->object ).';';  
			     } else {
				   $r  .= $arg.$i.'='.$arg.';';
				 }
				 
				 $arg = $arg.$i;
				 
				 if( R(0, 1) > 0) {  //artemtyczne i logiczne
					 $t = '';        //w przypadku Symbol operacje artmetyczne skutkują wyjątkiem, ale akceptujemy to narazie 
					 if( R(0, 1) > 0 ) { //pre
						 $t  = $pre[ R(0, sizeof($pre) - 1) ];
						 $t .= $arg;//';';
						 $operations[] = $t;
					 }
					 if( R(0, 1) > 0) {  //mid  a + b | arg + vars[n] | arg + arg1
						 $t  = $arg;
						 $t .= ' '.$mid[ R(0, sizeof($mid) - 1) ].' ';
						 if( R(0, 1) > 0) {
							 if( R(0,1) > 0 ) {
								   $t .= self::simple_arg( JS_NUMBER );
							 } elseif( self::$jsc != null ) {
                                 $t .= self::$jsc->random_var()->object;
                             } else {
							     $t .= $arg1;
							 }
					     } else {
						   $t .= $arg1;
						 }
						 //$t .= ';';
						 $operations[] = $t;
					 }
					 if( R(0, 1) > 0) { //ass
						 $t  = $arg;
                         $t .= ' '.$ass[R(0, sizeof( $ass ) - 1)].' ';
                         $t .= (self::$jsc != null) ? self::$jsc->random_var()->object : $arg1;
						 //$t .= ';';
                         $operations[] = $t;		
					 }
					 if( R(0,1) > 0) {  //post
                         $t  = $arg;
						 $t .= $post[R(0, sizeof( $post ) - 1)];
						 $operations[] = $t;
                     }	
				 }
				 
				 if( (R(0, 1) > 0) ) {
					  $t  = $arg;
					  $t .= ' instanceof '.self::$objects[ R(0, sizeof(self::$objects) - 1) ];
					  $operations[] = $t;
				 } 
				 if( ( R(0, 1) > 0) && (self::$jsc != null) ) {
					  $t  = self::$jsc->random_var()->object;
					  $t .= ' in ';
					  $t .= self::$jsc->random_var()->object;
					  $operations[] = $t;
				 }
				 if( (R(0,1) > 0) && (self::$jsc != null) ) { //call var / todo: change to call_var_ex() -> call immutab 
				      $f .= utils::line( self::$jsc->call_var_ex( JS_IMMUTABLE_VAR ) );
				 }
				 if( (R(0,1) > 0) && (self::$jsc != null) ) {
					  $t  = self::$jsc->random_property_ex( self::$jsc->random_immutable()->object );
					  $t .= ' '.$mid[ R(0, sizeof($mid) - 1) ].' ';
					  $t .= $arg;
					  $operations[] = $t;
			     }
				 //js_call
			 }
			 
			 $s   = sizeof($operations);
			 if( $s > 0 ) {
				 $r  .= 'evil='; 
			 }// else {
			  //	$r .= 'evil='.$arg;  
			  // }
		
		     for( $i=0; $i<$s; $i++) {
				  $sep = $mid[R(0, sizeof($mid) - 1)];
		          $exp  = '('.$operations[$i].')';
				  if( $i < $s - 1 ) {
				      $exp .= $sep; 
				  }
				  $r  .= $exp;
			 }
			 
			 $r .= ';';
				   
		     if( $r != ';' ) {
				 if( self::$jsc == null ) {
				     $r .= (R(0,1) > 0) ? 'return '.$arg1.';' : 'return evil;';
			     } else {
					 $r .= 'return '.self::$jsc->random_var()->object.';';
			     }
			 }
			 
			 $r = $f.$r;
			 
			 $r = preg_replace("/;{2,}/",';',$r);
			 
			 return $r;
			 
	  }
	  
	  public static function dumb_args( $count ) {
		     $t = [];
             for($i=0;$i<$count;$i++) {
				 if( ($a = self::arg( JS_ANY )) != null) {
                      array_push( $t, $a );
			     } else {
				   array_push( $t, R(-0xffe, 4096) ); 
				 }
             }
             return $t;			
	  }
	  
	  private static function object_body( ) {
		      
			  $r = '';
			  $t = [];
              $c = R(0, 16);
			  
			  for($i=0;$i<$c;$i++) {
				  $r .= utils::js_str( chr( 0x61 + $i ) );
				  $r .= ':';
				  $r .= self::simple_arg( self::$types[ R(0, sizeof(self::$types) - 1) ] );
				  if( $i < $c - 1 )  {
					  $r .= ',';
				  }
			  }
			  
			  return $r;
              			  
	  }
	  
	  public static function inline_object( ) {
		     return '{'.self::object_body().'}';  
	  }
	  
	  public static function data_descriptor( ) {
		     
			 $r = '{';
			 
			 $keys = [
			          'configurable','enumerable','writable'
			         ];
					 
			 $setter = 'get'; //funkcja //jesli jest writeable albo value nie moze byc seterów i geterów
			 $getter = 'set'; //funkcja  
			 $value  = 'value'; //wartosc
			 
			 shuffle( $keys );
			 
			 $c = R(0, sizeof( $keys ) );
			 
			 $maybe_setter_getter = true;
			 
			 for( $i=0;$i<$c;$i++ ) {
				  $r .= $keys[$i];
				  if( $keys[$i] == 'writable' ) {
					  $maybe_setter_getter = false;
				  }
                  $r .= ': ';
                  $r .= ( R(0, 1) > 0) ? 'true' : 'false';		
                  if( $i < $c - 1 ) {
                      $r .= ', ';
                  }					  
		     }
			 
			 if( R(0, 1) > 0) {
				 if( $r != '{' ) {
					 $r .= ', '; 
			     }
				 $r .= $value.': '.self::simple_arg( self::$types[ R(0, sizeof( self::$types ) - 1) ] );
				 $maybe_setter_getter = false;
		     }
			 
			 if( $maybe_setter_getter == true ) {
				 
				 //$v = ( self::$jsc != null ) ? self::$jsc->random_var()->object 
				 //                            : self::simple_arg( self::$types[ R(0, sizeof( self::$types ) - 1) ] );
				 
				 if( $r != '{' ) {
					 $r .= ', '; 
			     }
				 if( R(0, 1) > 0) { //setter
				     $v = (self::$jsc != null) ? self::$jsc->random_var()->object : self::$objects[ R(0, sizeof(self::$objects) - 1) ];
                     $r .= 'set(){ '.self::function_body([$v]).'}';  //function body
                 }	
                 if( $r != '{' ) {
					 $r .= ', '; 
			     }				 
                 if( R(0, 1) > 0) { //getter
				     $v = (self::$jsc != null) ? self::$jsc->random_var()->object : self::$objects[ R(0, sizeof(self::$objects) - 1) ];
                     $r .= 'get(){ '.self::function_body([$v]).'}';  //function body
                 }	
				 
			 }
			 
			 $r .= '}';
			 
			 return $r;
			 
	  }
	  
	  public static function arg( $t ) {
		     switch( $t ) {
				     case JS_FUNCTION: /* 0=new Function()||1=function(){}||2=(arg) => body */
					      $k = R(0,2);
						  $f_args = self::arg( JS_F_ARGS );
						  $f_body = self::arg( JS_FUNCTION_BODY );
						  if( $k == 0 ) {
							  return 'new Function('.$f_args.','.$f_body.')';
						  }
						  if( $k == 1 ) {
							  return 'Function('.$f_args.','.$f_body.')';
						  }
						  if( $k == 2 ) {
							 return '('.str_replace('"',"",$f_args).') => {'.str_replace('"',"",$f_body).'}';
						  }
					 break;
					 case JS_OBJECT:
					      $k = R(0, 2);
						  if( $k == 0 ) {
							  if( self::$jsc != null ) {
								  $v = self::$jsc->random_var();
								  if( $v->object->declared == true || $v->object->constans == true ) {
								      return $v->object;
								  }
							  }
						  }
						  if( $k == 1 ) { //create_object
							  return self::inline_object();
						  }
						  return self::$objects[ R(0, sizeof(self::$objects) - 1) ];
					 break;
					 case JS_SYMBOL:
					      $s = [
						        'Symbol.asyncIterator',
								'Symbol.iterator',
								'Symbol()',
								'Symbol(Symbol.evil)',
								'Symbol.unscopables',
								'Symbol.hasInstance',
								'Symbol.isConcatSpreadable',
								'Symbol.match',
								'Symbol.matchAll',
								'Symbol.prototype',
								'Symbol.replace',
								'Symbol.search',
								'Symbol.species',
								'Symbol.split',
								'Symbol.toPrimitive',
								'Symbol.toStringTag'
						  ];
						  return $s[ R(0, sizeof($s) - 1) ];
					 break;
					 case JS_BOOLEAN:
					      if( R(0,1) > 0 ) {
							  return "true";   
						   }
						   return "false";
					 break;
					 case JS_NUMBER:
					      $k = R(0,2);
						  $n = self::$numbers[ R(0, sizeof(self::$numbers) - 1) ];
						  if( $k == 0 ) {
							  return $n;
						  }
						  if( $k == 1 ) {
							  return 'Number('.$n.')';
						  }
						  if( $k == 2 ) {
							  return 'new Number('.$n.')';
						  }
					 break;
				     case JS_STRING:  /* format strings || strings */
					      if( R(0,1) > 0 ) { 
							  return utils::js_str( self::$strings[ R(0, sizeof(self::$strings) - 1) ] ); 
						  } 
						  return  utils::js_str( self::$f_strings[ R(0, sizeof(self::$f_strings) - 1) ] ); 
					 break;
					 case JS_PROTOTYPE:
					      if( R(0,1) > 0) {
							  if( self::$jsc != null ) {
                                  $v = self::$jsc->objects_with_instance( false );
								  $s = sizeof($v);
								  if( $s > 0) {
									  return $v[ R(0, $s - 1) ]->object->prototype;  
								  } else {
									return 'Object.getPrototypeOf('.self::$jsc->random_var()->object.')';
								  }
                              } 
						  }
						  return self::$objects[ R(0, sizeof(self::$objects) - 1) ].'.prototype';
					 break;
					 case JS_PROTO:
					      if( R(0,1) > 0) {
							  if( self::$jsc != null ) {
                                  return self::$jsc->random_var()->object->__proto__;
                              }								  
						  }
						  return self::$objects[ R(0, sizeof(self::$objects) - 1) ].'.__proto__';
					 break;
					 case JS_CONSTRUCTOR:
					      if( R(0,1) > 0) {
							  if( self::$jsc != null ) {
								  return self::$jsc->random_var()->object->constructor;
							  }
						  }
						  return self::$objects[ R(0, sizeof(self::$objects) - 1) ].'.constructor';
					 break;
					 case JS_PROPERTIE:
					      if( R(0,1) > 0) {
							  if( self::$jsc != null ) {
							      return self::$jsc->random_property_ex( self::$jsc->random_var()->object ); 
							  }
						  }
						  return utils::js_str( self::$properties[ R(0, sizeof(self::$properties) - 1) ] );
					 break;
					 case JS_ANY:
						  return self::arg( self::$types[ R(0, sizeof(self::$types) - 1) ] );
					 break;
					 case JS_FUNCTION_BODY:
					      return utils::js_str( self::format_function_body( payload::function_body( self::$function_arg_names ) ) );
					 break;
					 case JS_ARRAY:
					      $k = R(0, 2);
						  if( $k == 0 ) {
							  return '[]';
						  }
						  if( $k == 1 ) {
							  return ( R(0 ,1) > 0) ? 'new Array('.self::$small_ints[ R(0, sizeof(self::$small_ints) - 1 ) ].')' : 'new Array()';
						  }
						  if( $k == 2 ) {
							  return ( R(0 ,1) > 0) ? 'Array('.self::$small_ints[ R(0, sizeof(self::$small_ints) - 1 ) ].')' : 'Array()';
						  }
					 break;
					 case JS_DATA_DESCRIPTOR:
					      return self::data_descriptor( );
					 break;
					 case JS_ITERABLE:
					      return '[ [] ]';
					 break;
					 case JS_LOCALES:
					      $v = utils::js_str( self::$locales[ R(0, sizeof( self::$locales ) - 1) ] );
			              if( R(0, 1) > 0) {
							  $v .= ','.self::locale_options( );
						  }
					      return $v;
					 break;
					 case JS_REGEX:
					      return regex::create();
					 break;
					 case JS_ITERATOR:
					      return '{ next: function() { '.payload::function_body( ['this','Object','Number','String'] ).' } }';
					 break;
					 case JS_FORM_NORMALIZATION:
					      $v = ["NFC", "NFD", "NFKC", "NFKD"];
						  $v = $v[ R(0, sizeof($v) - 1) ];
						  return utils::js_str( $v );
					 break;
					 case JS_RADIX:
					      return R(2, 36);
					 break;
					 case JS_SMALL_INT:
					      $k = R(0, 2);
					      $v = self::$small_ints[ R(0, sizeof(self::$small_ints) - 1) ];
						  if($k == 0) {
							 return $v;  
						  }
						  if($k == 1) {
							 return 'Number('.$v.')';  
						  }
						  return 'new Number('.$v.')';
					 break;
					 case JS_F_ARGS:
						  $c = R(1, 5);
						  self::$function_arg_names = [];
						  for($i=0;$i<$c;$i++) {
							  self::$function_arg_names[] = utils::js_str( chr( 0x61 + $i ) );
						  }
						  return implode(',',self::$function_arg_names);
					 break;
					 case JS_OBJECT_BODY:
					      return self::object_body( );
					 break;
					 case _EMPTY:
					 default: return null; 
			 }
	  }
	  /*
	    : tworzy tablicę wartości na podstawie tablicy typów i informacji o ilości potrzebnych wartośći :
		IN :  $types = tablica typów
		      $min_argc = minimalna ilość argumentów
			  $max_argc = maksymalna ilość argumentów
			  $required_argc = (opcjonalny) wymagana ilość argumentów
		OUT : tablica wartości
	  */
	  public static function args_by_types_array( $types, $min_argc, $max_argc, $required_argc = 0 ) {
		     
			 if(empty($types)) {
				return -1; 
		     }
			 
			 $r = [];
			 $s = sizeof( $types );
			 $last_type = $types[$s - 1];
			 $argc = R($min_argc, $max_argc); 
			 
			 if ( ($argc > 0 ) && ( $argc < $required_argc ) ) {
				   $argc = $required_argc;
			 }
			 
			 for($i=0;$i<$argc;$i++) {
				 if( $i <= $min_argc ) {
					 if( ( $v = self::arg( constant( $types[$i] )) ) != null ) {
						  array_push( $r, $v );
					 }
				 } else {
				   if( ( $v = self::arg( constant( $last_type )) ) != null ) {
					     array_push( $r, $v );
				   }
				 }
			 }
			 
			 return $r;
			 
	  }
	
};

payload::$strings = [

_DUP("A", 128),
_DUP("A", 512),
_DUP("A", 127),
_DUP("A",1024),
_DUP("A",2047),
_DUP("A",4096),
_DUP("a", 128),
_DUP("a", 512),
_DUP("a", 1024),
_DUP("a", 2047),
_DUP("縗", 128),
_DUP("縗", 512),
_DUP("縗", 127),
_DUP("縗", 1024),
_DUP("縗", 2047),
_DUP("縗", 4096),
_DUP("縗", 4095),
_DUP("縗", 4098),
_DUP("6", 128),
_DUP("6", 512),
'null',
_DUP("6", 2047),
_DUP("6", 4096)

];

payload::$f_strings = [

"%x",
"%d",
"%i",
"%u",
"%o",
"%X",
"%f",
"%F",
"%e",
"%g",
"%a",
"%c",
"%s",
"%p",
"%l",
"1337%n",
"%08x",
"%x%d%i%u%o%X%f%F%e%g%a%c%s%p%l",
"31137%x%d%i%u%o%X%f%F%e%g%a%c%s%p%l%n",
"1337%s"

];

payload::$numbers = [

1094795585,
0x7ffffff0,
0x7fffffff,
0x7ffffffe,
0x55555600,
0xffff,
2.3023e-320,
0x7fff,
0xff,
-5.3049894784e-314,
0xfe,
2.3023e-320,
-0,
0xffffffff,
0,
2.3023e-319,
-1,
1,
-5.3049894784e-314,
127,
129,
-128,
2.3023e-321,
65536,
1.337,
31.3370,
5.3049894784e-314,
65535,
0xfffff,
0x7fffff,
0xffffffff,
1.1,
2.2,
0.1,
10000.1,
0x6fff,
0x7ffffe,
-127,
-126,
1024

];

payload::$objects = [

'Object',
'String',
'Number',
'Boolean',
'Array'

];

payload::$properties = [

'width',
'evil',
'size',
'length',
'size',
'id',
'class',
'style',
'a',
'b',
'c',
'd',
'alt',
'title',
'href',
'text'

];

payload::$small_ints = [

0,
1,
64,
65,
127,
128,
129,
511,
512,
513,
1024,
1023,
1025,
2047,
2049,
2048

];

payload::$locales = [

'de',
'en',
'mas',
'fr-CA',
'es-419',
'zh-Hans',
'fr',
'pl',
'sv',
'en-u-kn-true',
'cn'

];

//echo payload::arg(JS_FUNCTION)."\r\n";

?>