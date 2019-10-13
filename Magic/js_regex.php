<?php

require_once("utils.php");

class regex {
	  
	  private static $parts      = [];
	  
	  private static $l_chars    = [ '*','+','$','?','.' ];
	  private static $m_chars    = [ '*','+','?','.','|' ];
	  private static $f_chars    = [ '^','.' ];
      private static $ranges     = [ 'a-z','0-9','A-Z','[A-Z]','[0-9]','[a-z]' ];
	  private static $g_open     = [ '(','[','(?:','(?=','(?!','(x?<=','(y?<!','[^','(^' ];
	  private static $g_close    = [ ')',']',')'  ,  ')',')'  ,')'    ,')'    ,']' , ')' ];
	  private static $separators = [ '+','|' ];
	  private static $flags      = [ 'i','g','m' ];
	  private static $metas      = [ '\w','\W','\d','\D','\b','\0','\n','\f','\r','\t','\v' ];
	  
	  private static $info       = [
	                                'group_index'=>[],
									'f_chars'=>0,
									'm_chars'=>0,
									'ranges'=>0,
									'values'=>0,
									'metas'=>0,
									'l_chars'=>0
	                               ];
	
	  private static function group_start( ) {
		      
			  $r = '';
			  $c = R(0, 3);
			  do {
				  $i = R(0, sizeof( self::$g_open ) - 1);
				  if( self::$g_open[$i] == '[' || self::$g_open[$i] == '[^' ) {
					  if( preg_match("/\[(.*)\((.*)/", $r ) ) {
						  continue;  
					  }
				  }	  
				  self::$info['group_index'][] = $i;
				  $r .= self::$g_open[$i];
				  if( R(0, 1) > 0 ) {
					  $r .= utils::wstring( R(1, 3) );  
				  }
			  } while(--$c > 0);
			  return $r;
			  
	  }
	  
	  private static function group_end( ) {
		  
		      $r = '';
			  $s = sizeof( self::$info['group_index'] );
			  for($i=$s-1;$i>=0;$i--) {
				  $j = self::$info['group_index'][$i];
				  $r .= self::$g_close[$j];
			  }
			  return $r;
		  
	  }
	  
	  private static function group( ) {
		  
		      $t = self::group_start();
			  $need_val = 1;
			  
			  self::$info['f_chars'] = R(0, 1);
			  if(  self::$info['f_chars'] > 0 ) {
				   $t .= self::$f_chars[ R(0, sizeof(self::$f_chars) - 1) ];
			  }
			  
			  self::$info['m_chars'] = R(0, 1);
			  if( self::$info['m_chars']  > 0 ) {
				  $t .= utils::wstring( R(1, 16) );
				  $t .= self::$m_chars[ R(0, sizeof(self::$m_chars) - 1) ];
				  $t .= utils::wstring( R(1, 16) );
				  $need_val = 0;
			  }
			  
			  self::$info['ranges'] = R(0, 1);
			  if( ( self::$info['ranges'] > 0 ) && ( !preg_match("/\[(.*)\((.*)/", $t ) ) ){
				    $t .= self::$ranges[ R(0, sizeof(self::$ranges) - 1) ];
					$need_val = 0;
			  }
			  
			  self::$info['values'] = R(0, 1);
			  if( self::$info['values'] > 0 ) {
				  $t .= utils::wstring( R(1, 16) );
				  $need_val = 0;
			  }
			  
			  self::$info['metas'] = R(0, 1);
			  if( self::$info['metas'] > 0 ) {
				  $t .= self::$metas[ R(0, sizeof(self::$metas) - 1)];
				  $need_val = 0;
			  }
			  
			  if( $need_val == 1 ) {
				  if( R(0, 1) > 0 ) {
					  $t .= self::$ranges[ R(0, sizeof(self::$ranges) - 1) ];
				  } else {
					$t .= utils::wstring( R(1, 16) );
				  }
			  }
			  
			  self::$info['l_chars'] = R(0, 1);
			  if( self::$info['l_chars'] > 0 ) {
				  $t .= self::$l_chars[ R(0, sizeof(self::$l_chars) - 1 ) ];
			  }
			  
			  $t .= self::group_end();
			  
			  self::$parts[] = $t;
			  
			  self::$info['group_index'] = [];
			  self::$info['f_chars'] = 0;
			  self::$info['m_chars'] = 0;
			  self::$info['ranges']  = 0;
			  self::$info['values']  = 0;
			  self::$info['metas']   = 0;
			  self::$info['l_chars'] = 0;

	  }
	  
	  private static function values( ) {
		  
		      $t = '';
			  $need_val = 1;
			  
			  self::$info['f_chars'] = R(0, 1);
			  if(  self::$info['f_chars'] > 0 ) {
				   $t .= self::$f_chars[ R(0, sizeof(self::$f_chars) - 1) ];
			  }
			  
			  self::$info['m_chars'] = R(0, 1);
			  if( self::$info['m_chars']  > 0 ) {
				  $t .= utils::wstring( R(1, 16) );
				  $t .= self::$m_chars[ R(0, sizeof(self::$m_chars) - 1) ];
				  $t .= utils::wstring( R(1, 16) );
				  $need_val = 0;
			  }
			  
			  self::$info['ranges'] = R(0, 1);
			  if( ( self::$info['ranges'] > 0 ) && ( !preg_match("/\[(.*)\((.*)/", $t ) ) ){
				    $t .= self::$ranges[ R(0, sizeof(self::$ranges) - 1) ];
					$need_val = 0;
			  }
			  
			  self::$info['values'] = R(0, 1);
			  if( self::$info['values'] > 0 ) {
				  $t .= utils::wstring( R(1, 16) );
				  $need_val = 0;
			  }
			  
			  self::$info['metas'] = R(0, 1);
			  if( self::$info['metas'] > 0 ) {
				  $t .= self::$metas[ R(0, sizeof(self::$metas) - 1)];
				  $need_val = 0;
			  }
			  
			  if( $need_val == 1 ) {
				  if( R(0, 1) > 0 ) {
					  $t .= self::$ranges[ R(0, sizeof(self::$ranges) - 1) ];
				  } else {
					$t .= utils::wstring( R(1, 16) );
				  }
			  }
			  
			  /*
			  self::$info['l_chars'] = R(0, 1);
			  if( self::$info['l_chars'] > 0 ) {
				  $t .= self::$l_chars[ R(0, sizeof(self::$l_chars) - 1 ) ];
			  }
			  */
			  
			  self::$parts[] = $t;
			  
			  self::$info['group_index'] = [];
			  self::$info['f_chars'] = 0;
			  self::$info['m_chars'] = 0;
			  self::$info['ranges']  = 0;
			  self::$info['values']  = 0;
			  self::$info['metas']   = 0;
			  self::$info['l_chars'] = 0;
			  
	  }
	  
	  private static function parts( ) {
		      
			  $gcnt = R(0,2);
	          for($i=0;$i<$gcnt;$i++){
			      self::group();                  
			  }
			 
			  $vcnt = R(0,2);
			  for($i=0;$i<$vcnt;$i++){
			      self::values();
			  }
			  
	  }
	  
	  public static function create( ) {
		  
		     $r = '/';
			 
			 self::parts( );
			 
			 $s = sizeof( self::$parts );
		
		     if( $s > 0 ) {
			    
 				 shuffle( self::$parts );
			 
			     for($i=0;$i<$s;$i++) {
				     $r .= self::$parts[ $i ];
				     if( $i < $s - 1 ) { 
				         $r .= self::$separators[ R(0, sizeof(self::$separators) - 1) ];
				     }
			     }
			 
			 }
			 
			 $r .= '/';
			 
			 if( R(0, 1) > 0 ) {
				 $r .= self::$flags[ R(0, sizeof(self::$flags) - 1) ];
		     }
			 
			 self::$parts = [];
			 
			 return $r;
			 
	  }
	  
};

?>