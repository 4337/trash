<?php

  define('JS_IMMUTABLE_VAR',-1);
  define('JS_VAR',1);

  define('_EMPTY',          0x00000000);

  define('JS_FUNCTION',   '_Function');
  define('JS_OBJECT',     '_Object');
  define('JS_SYMBOL',     '_Symbol');
  define('JS_BOOLEAN',    '_Boolean');
  /*define('JS_BIGINT',     '_BigInt');*//* unimplemeneted */
  define('JS_NUMBER',     '_Number');
  define('JS_STRING',     '_String');
  define('JS_PROTOTYPE',  '_prototype');
  define('JS_PROTO',      '___proto__');
  define('JS_CONSTRUCTOR','_constructor');

/* define('JS_INT',          0x00000001);//17.07.2019 
                                         //Symbol.toSource("NFKD").lastIndexOf(2.3023e-319).isInteger("mong").hasOwnProperty().__lookupGetter__() = error
                                         //Symbol.toSource("NFKD").lastIndexOf(2.3023e-319) = -1 
                                         //Symbol.toSource("NFKD").lastIndexOf(2.3023e-319).isInteger = undefined	
										 //18.07.2019 - problem wynikał z widoczności isInteger w Number

*/										 
  define('JS_VOID',         0x00000010);
  define('JS_PROPERTIE',    0x00000100);    /* width, length, size, heigh, ... */
  define('JS_INPUT_TYPE',   0x00001000);     
  define('JS_ANY',          0x00010000);
  define('JS_FUNCTION_BODY',0x00100000);
  
  define('JS_ARRAY',        '_Array');
  
  define('JS_DATA_DESCRIPTOR', 0x10000000);  /* {configurable: true, writeable: true, value: true} || {whatever:666,...} *//* payload->data_desc_generator() */
  define('JS_ITERABLE',        0x00000011);  /* array of array || map */
  
  define('JS_LOCALES',         0x00000111);  /* co, kn, kf */
  define('JS_REGEX',           0x00001111);  /* payload->regexp_generator() */
  define('JS_ITERATOR',        0x00011111);  /* payload->iterator_generator() */
  
  define('JS_FORM_NORMALIZATION', 0x00111111); /* "NFC", "NFD", "NFKC", "NFKD" */
 
  define('JS_PARENT_TYPE',        0x11111111); /* Object.__proto__ = Object, Function.__proto__ = Function */
  
  define('JS_RADIX',              0x00000002); /* range 2 - 36 */
  define('JS_SMALL_INT',          0x00000020); /* liczby w granicach dopuszczalnych zakresów fix : repeat count must be less than infinity and not overflow maximum string size */
  
  define('JS_UNSUPPORTED',        0x00000200);
  define('JS_F_ARGS',             0x00002000); /* argumenty funkcji w formie pojedyńczego sgtringa : "1,{},2.2,null" itd */
  define('JS_NULL',               JS_VOID);
  define('JS_OBJECT_BODY',        0x00020000);
  
  define('JS_VISIBLE_IN_CLASS',             0x00000001);  /* klasa: eg. Object.values nie jest widoczny w var a = new Object; a.values <-- error */
  define('JS_VISIBLE_IN_INSTANCE_AND_CLASS',0x00000010);  /* instancja : new | klasa : a = String */
  define('JS_VISIBLE_IN_INSTANCE',          0x00000100);  /* tylko instancja */
  
  define('JS_UNSUPPORTED_TYPE_EXCEPTION',   0x00001000);
  define('JS_SYNTAX_ERROR_EXCEPTION',       0x00010000);
  define('JS_TO_MUCH_PROTO_EXCEPTION',      0x00100000); 
  define('JS_INVALID_PARENT_TYPE_EXCEPTION',0x01000000);
  define('JS_INVALID_GETTER_EXCEPTION',     0x10000000);
  define('JS_INVALID_SETTER_EXCEPTION',     0x11000000);
  define('JS_INVALID_OBJECT_BODY_EXCEPTION',0x01100000);
  define('JS_INVALID_ARGUMENT_EXCEPTION',   0x00110000);
  define('JS_INVALID_CALL_EXCEPTION',       0x00011000);
  


?>