<?php

require_once("js_grammar.php");
require_once("utils.php");
require_once("payload.php");

$context = new context();

$context->set_var( new jsvar( $context->random_main_class( VARNAME() )  ));
$context->set_var( new jsvar( $context->random_main_class( VARNAME() ) ));
$context->set_var( new jsvar( $context->random_main_class( ) ));
$context->set_var( new jsvar( $context->random_main_class( VARNAME() ) ));
$context->set_var( new jsvar( $context->random_main_class( VARNAME() ) ));

$context->immutable_init();

_DUMPVARS($context);
//$context->set_var( new jsvar( new _prototype() ));

$objects = $context->objects_with_name( true );

for($i=0;$i<sizeof($objects);$i++) {
	
	$ins  = $context->random_instance_method( $objects[$i]['var']->object );
	$args = payload::args_by_types_array( (array)$ins->argv->type, (int)$ins->min_argc, (int)$ins->max_argc, (int)$ins->required_argc );
    //js_let/var/const
	$assign_types = ['js_const']; //['js_let','js_var','js_const'];
	$ass_type = $assign_types[ R(0, sizeof($assign_types) - 1) ];
	echo utils::line( $context->{$ass_type}( $objects[$i]['var']->object->{(string)$ins->name}( implode(",",$args) ),
	                  null,
					  $objects[$i]['var'] )
					);
	
}

$context->immutable_end();

echo "var ".$context->get_vars()[0]->object." type is : ".get_class($context->get_vars()[0]->object)."\r\n";

echo utils::line( $context->js_assign( $context->get_vars()[0]->object->__proto__, $context->get_vars()[1]->object ) );

echo "var ".$context->get_vars()[0]->object." type is : ".get_class($context->get_vars()[0]->object)."\r\n";

try {

 //$context->js_var(  $context->get_vars()[0]->object, 1122333 );

}catch(Exception $e) {
  echo("i skip them all \r\n");
}
////////////////// zmina instacji //////////////////////////////

//echo "var ".$context->get_vars()[0]->object." type is : ".get_class($context->get_vars()[0]->object)."\r\n";

//echo utils::line( $context->js_assign( $context->get_vars()[0]->object, $context->get_vars()[1]->object ) );

//echo "var ".$context->get_vars()[0]->object." type is : ".get_class($context->get_vars()[0]->object)."\r\n";

/////////////////////////////////////////

//echo utils::line( $context->js_assign( $context->get_vars()[0]->object, 1337 ) );

//echo "var ".$context->get_vars()[0]->object." type is : ".get_class($context->get_vars()[0]->object)."\r\n";

//echo utils::line( $context->js_assign( $context->get_vars()[0]->object, $context->random_property_ex( $context->get_vars()[1]->object ) ) );

//echo "var ".$context->get_vars()[0]->object." type is : ".get_class($context->get_vars()[0]->object)."\r\n";

//echo utils::line( $context->js_assign( $context->get_vars()[0]->object->__proto__->__proto__->__proto__, $context->random_property_ex( $context->get_vars()[1]->object ) ) ); //<-- Exception - ok


//echo utils::line( $context->js_assign( $context->get_vars()[0]->object->__proto__->constructor->__proto__->constructor->__proto__->constructor->__proto__->constructor->__proto__, $context->random_property_ex( $context->get_vars()[1]->object ) ) );

//echo "var ".$context->get_vars()[0]->object." type is : ".get_class($context->get_vars()[0]->object)."\r\n";

//echo utils::line( $context->js_assign( $context->get_vars()[0]->object->length->constructor->__proto__, utils::js_str("ALAMAKOTA_TO_BARDZODOBRARZECZ") ) );

//echo "var ".$context->get_vars()[0]->object." type is : ".get_class($context->get_vars()[0]->object)."\r\n";

_DUMPVARS($context);

?>