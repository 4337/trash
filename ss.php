<?php
/*
* Created by Maciek z klanu (R) 2010
* 
* W ten sposób mozna wlasciwie stworzyc "absolutnie skonczony" skaner podatnosci aplikacji 
* Trzeba tylko byc na poziomie tworcow parserow/kompilatorow 
* i zaimplementowac logike ktora rozumie instrukcje warunkowe aby pokryc jak najwiekszy obszar kodu
* Mowiac prostym jezykiem trzeba napisac tak zmodyfikowany parser/kompilator 
* ktory pobiera identyfikatory zmiennych uwzgledniajac instrukcje logiczne/warunkowe 
* mapuje je w odpowiedni sposob i przekazuje na wejscie programu z odpowiednimi payloadami ;]
* -------------
* Aplikacja wycina identyfikatory zmiennych mapuje je na URL-e i przekazuje do nich BUG_MAKER 
* nastepnie sprawdza czy na wyjsciu pojwawil sie komunikat o blêdach (wymaga wlaczonego raportowania blêdów)
*/
define('HOST','localhost');

define('GUARD','CrashCrashCrash');
                                                                                                  
$crn = dirname(__FILE__).'/';
$crn = str_replace('\\','/',$crn);

if((getcwd() != $crn) && (chdir($crn))) define('PATH',$crn);
else define('PATH','/');

if(!extension_loaded('curl'))
{
    if((!dl('php_curl.dll')) &&
       (!dl('php_curl.so'))) die('die: I cant load curl !'); 
}
if(!file_exists(PATH.'tmp'))
    if(!mkdir(PATH.'tmp')) die("Can\'t create tmp\r\n");
if(!file_exists(PATH.'safeme'))
    if(!mkdir(PATH.'safeme')) die("Can\'t create safeme\r\n");    
if(!file_exists(PATH.'logs'))
    if(!mkdir(PATH.'logs')) die("Can\'t create logs\r\n");    

define('BUG_MAKER','99999999999999999999999999999999999999.9999999999999\'-1,null,id,`../../../../../../../ABDBSBEBSUHUWHFUHFUHshjsbhbhdcbhdbc9999befb');

class config
{
      private static $env = null;
      
      public 
      static function chckConf()
      {
                      if(self::$env != null) return self::$env;
                      $tmp = getenv("CLIENTNAME");
                      if($tmp == "Console")
                      {
                         if(!file_exists(PATH.'logs/root.cfg'))
                         {
                           if(!($fp = fopen("php://stdin",r)))
                                throw new Exception('Fopen(STDIN) error');
                            echo("SET UP YOUR WEB ROOT PATH\r\n".
                                 "EX: C:\\usr\\Apache\\htdocs\r\n".
                                 "    /usr/var/www\r\n");     
                            self::$env = fgets($fp,256); 
                          fclose($fp); 
                         }
                        else self::$env = file_get_contents(PATH.'logs/root.cfg');     
                      }                      
                      else self::$env = $_SERVER['DOCUMENT_ROOT']; 
                      self::$env = trim(self::$env);
                      if(!is_dir(self::$env))
                         die("ETO nie jest katalog\r\n");  
                      file_put_contents(PATH.'logs/root.cfg',self::$env);    
                      echo("Well done !!\r\n");  
                      return trim(self::$env);
      }
      public
      static function getClientType()
      {
             return (($tmp = getenv("CLIENTNAME")) == 'Console') ? 'CONSOLE' 
                                                                 : 'WEB';                         
      }
};

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
 
class ioCreator           
{
      private $cSocket;
      private $storage = array('vars'=>array(),      
                               'method'=>'GET',        
                               'interator'=>null);     
      private $logPath;
       
      public 
      function __get($var)        
      { 
               return $this->storage[$var];  
      } 
      
      public 
      function getSocks()
      {
               return $this->cSocket;
      }
      
      public 
      function __set($var,$val)
      {
               $this->storage['interator'] = sizeof($val) -1;
               (!is_array($val)) ? $this->storage[$var][$this->storage['interator']] = $val  
                                 : $this->storage[$var] = $val;
      }  
         
      public 
      function setMethod($metod)
      {
               $this->storage['method'] = (strtolower($metod) == 'post') ? 'POST'
                                                                         : 'GET';
      }                      
      
      public
      function __construct()
      {
              try
              {
                $logFname = 'o['.date("d.m.Y h.i").'].log';
                if(!@file_put_contents(PATH.'logs/'.$logFname,"[".date("d.m.Y H:i:s")."]\r\n"))
                        throw new Exception('I can\'t create log file'); 
                $this->logPath = PATH.'logs/'.$logFname; 
                $this->cSocket = curl_init();
              }
              catch(Exception $err)
              {
                    //$this->__destruct(); //auto
                    die('Constructor error '.$err->getFile().' '.
                                             $err->getLine().' '.
                                             $err->getMessage());
              }     
      }
      
      public 
      function __destruct()
      {
              if(file_exists($this->logPath)) 
               if(filesize($this->logPath) <= 23)
                  unlink($this->logPath);
               if(@get_resource_type($this->cSocket) == 'curl')
                  @curl_close($this->cSocket);                    
      }
      
      public 
      function makeLog($data)
      {
               $data = trim($data);
               file_put_contents($this->logPath,$data."\n\n",FILE_APPEND); 
      }
      
      public 
      function __STDOUT($data)
      { 
               echo($data);
      }
      
      public 
      function cGetData($data)            
      {
               $pLoad = $this->storage['vars'][$this->storage['interator']].'='.BUG_MAKER;
               $method = strtolower($this->storage['method']);
               curl_setopt($this->cSocket,CURLOPT_URL,$data.'?'.$pLoad);
               curl_setopt($this->cSocket, CURLOPT_RETURNTRANSFER, 1);
               if($method == 'post')
               {
                  curl_setopt($this->cSocket,CURLOPT_POST,1);
                  curl_setopt($this->cSocket,CURLOPT_POSTFIELDS,$pLoad);
               }
               ($this->storage['interator'] > 0) ? $this->storage['interator']--
                                                 : $this->storage['interator'] = sizeof($this->storage['vars']) - 1;
               $ret = curl_exec($this->cSocket);
               return $ret;
      }     
};

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/



class eFilter
{
      private $regExp= array();
      
      public 
      function setExpresion($expresion)
      { 
               (is_array($expresion)) ? $this->regExp = $expresion
                                      : $this->regExp[0] = $expresion;
      }
      
      public
      function getExpresion($data)
      {
             $size = sizeof($this->regExp);
             for($i=0; $i<$size; $i++) 
             {
                    if(($rets = explode($this->regExp[$i],$data)))
                    {   
                        $trus = explode("[</b> on line <b>]+\d+[</b><br />]",$rets[1]);
                        if($trus[0]) return $trus[0];
                    }    
             }    
             return -1;
      }
};


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

class v2
{                         
      private $meta = array('IO'=>null,                   
                            'EXPRESION'=>null,
                            'fCount'=>0,
                            'uri'=>array(),
                            'fPath'=>array());           
                            
      public 
      function __construct($dir,$ext)
      {
               $dRoot = config::chckConf();
               if(!ereg($dRoot,$dir)) $dir = $dRoot.$dir; 
               if(!is_dir($dir))
               {
                  //$this->__destruct();        // auto
                  die("Bad directory ".$dir."\r\n");
               }
               $this->meta['IO'] = new ioCreator();
               $this->meta['EXPRESION'] = new eFilter();
               $this->doList($dir,$ext);
      }                      
      
      public 
      function __destruct()
      {

      }
      
      private 
      function doList($dir,$ext)                           
      { 
                      $data = @file_get_contents(PATH.'safeme/last.scn');
                      $data = str_replace("\\",'/',$data);
                      $dir = str_replace("\\",'/',$dir);
                      $dir = str_replace("//",'/',$dir);
                      if(strstr(trim($data),$dir)) 
                      {
                         if((($this->meta['fPath'] = @file(PATH.'safeme/filepath.sfw',FILE_SKIP_EMPTY_LINES | FILE_IGNORE_NEW_LINES))) &&
                            (($this->meta['uri'] = @file(PATH.'safeme/urlpath.sfw',FILE_SKIP_EMPTY_LINES | FILE_IGNORE_NEW_LINES)))) 
                            $this->meta['fCount'] = sizeof($this->meta['fPath']);
                            if($this->meta['fCount'] > 0) return;
                      }
                      file_put_contents(PATH.'safeme/last.scn',$dir);
                      $folders = array();
                      $folders[0] = $dir;
                      for($i=0; $i<sizeof($folders); $i++)
                      {
                           if(!($DIRHND = @opendir($folders[$i]))) continue;
                                while($stuff = readdir($DIRHND))
                                {
                                      if(($stuff == '.') || ($stuff == '..'))
                                          continue;
                                      else if(is_dir($folders[$i].'/'.$stuff))
                                                     $folders[] = $folders[$i].'/'.$stuff;
                                      else if(strtolower(substr($stuff,strlen($stuff) - 3,3)) == $ext)
                                      {
                                              $this->meta['fPath'][] = $folders[$i].'/'.$stuff; 
                                              $this->meta['uri'][] = $this->createURLt($folders[$i],$stuff);
                                              $this->meta['fCount']++;    /* licznik zewnÄ&#8482;trzenej pÄ&#8482;tli */
                                      }                         
                                } 
                           closedir($DIRHND);     
                      }                 
      }
      
      private 
      function createURLt($dPath,$fName)
      {
                          $SERVER_ADDR = 'http://'.HOST;
                          $ROOT_PATH = config::chckConf();
                          $endDayOfRoot = strlen(trim($ROOT_PATH));
                          if(($cmpresult = substr($dPath,$endDayOfRoot,strlen($dPath)-$endDayOfRoot)))
                              $SERVER_ADDR .=  ereg_replace("[\x5c]","/",$cmpresult); 
                          return $SERVER_ADDR .= '/'.$fName;         
      }
      
      protected
      function getVars($data) 
      {
               $tmpVars = array();
               if(preg_match_all('/\${1}[A-Za-z\_]+[A-Za-z0-9\_]/',$data,$ret))
               {
                        $unikaty = implode("\n",array_unique($ret[0]));
                        $unikaty = str_replace("$",null,$unikaty);
                        $unikaty = str_replace("_GET",null,$unikaty); 
                        $unikaty = str_replace("_POST",null,$unikaty);   
                        $unikaty = str_replace("_REQUEST",null,$unikaty); 
                        $unikaty = str_replace("_SESSION",null,$unikaty);
                        $unikaty = str_replace("_COOKIE",null,$unikaty); 
                        $unikaty = str_replace("_SERVER",null,$unikaty); 
                        $unikaty = ereg_replace("[\x20\'\"]",null,$unikaty);
                        file_put_contents(PATH.'tmp/tmp.tmp',$unikaty."\n",FILE_APPEND);
               }
               if(preg_match_all('/define\(+\x20|[\'\"]+[\x20A-Za-z\_]+[A-Za-z0-9\_\x20]+[\'\"]{1}/',$data,$retsi)) //blad
               {
                   $unikaty_const = implode("\n",array_unique($retsi[0]));
                   $unikaty_const = preg_replace("/define\(/",null,$unikaty_const);           
                   $unikaty_const = ereg_replace("[\x20\'\"]",null,$unikaty_const);
                   file_put_contents(PATH.'tmp/tmp.tmp',$unikaty_const."\n",FILE_APPEND);
               }
               if(preg_match_all('/\${1}[A-Za-z\_]+[A-Za-z0-9\_]+\[+[\x20A-Za-z0-9\"\']+\]{1}/',$data,$restis))     //
               {         
              
                  $size = sizeof($restis[0]);
                  for($i=0; $i<$size; $i++) 
                  {
                     if((strstr($restis[0][$i],'$_GET') || (strstr($restis[0][$i],'$_POST') ||
                         strstr($restis[0][$i],'$_COOKIE') || (strstr($restis[0][$i],'$_REQUEST') ||
                         strstr($restis[0][$i],'$_SERVER') || /* (strstr($restis[0][$i],'$GLOBALS') ||*/
                         strstr($restis[0][$i],'$_FILES') || (strstr($restis[0][$i],'$_SESSION')))))) 
                     {
                        $restis[0][$i] = preg_replace('/\${1}+\_{1}[A-Za-z]+[A-Za-z]+\[{1}+[\'\"]{1}/',null,$restis[0][$i]);
                        $restis[0][$i] = preg_replace('/[\x20\'\"\]]/',null,$restis[0][$i]);
                     }  
                  } 
                  $unikaty_get = implode("\n",$restis[0]);
                  $unikaty_get = preg_replace('/[\x20\"\'\$]/',null,$unikaty_get);
                  file_put_contents(PATH.'tmp/tmp.tmp',$unikaty_get."\n",FILE_APPEND);  
              }
              if(file_exists(PATH.'tmp/tmp.tmp'))
              {
                  $tmpVars = file(PATH.'tmp/tmp.tmp',FILE_SKIP_EMPTY_LINES | FILE_IGNORE_NEW_LINES);
                  $tmpVars = array_unique($tmpVars);
                  file_put_contents(PATH.'tmp/tmp.tmp',implode("\n",$tmpVars));
                  $tmpVars = file(PATH.'tmp/tmp.tmp',FILE_SKIP_EMPTY_LINES | FILE_IGNORE_NEW_LINES);
                  $tmpVars = array_unique($tmpVars);
                  unlink(PATH.'tmp/tmp.tmp');
                  return $tmpVars;
              }    
              return array('_SESSION[]','GLOBALS[]','0','_REQUEST[]','_COOKIE[]','_FILES[]',
                           'GLOBALS[0]','_REQUEST[0]','_COOKIE[0]');        
      }
      
      public 
      function getVarsByName($fName)
      {
               if(!($gData = @file_get_contents($fName)))
                    return -1;
               $this->meta['IO']->vars = $this->getVars($gData);
               return $this->meta['IO']->vars;                        //...                   
      }
      
      public 
      function getVarsById($index)          
      {
               if(!($gData = @file_get_contents($this->meta['fPath'][$index])))
                    return -1;
               $this->meta['IO']->vars = $this->getVars($gData);
               return $this->meta['IO']->vars;      
      }
      
      public 
      function __get($var)
      {
                     return $this->meta[$var];
      } 
      
      public 
      function safe()                       
      {
               $a = $this->meta['uri'];
               $b = $this->meta['fPath'];
               array_shift($a);
               array_shift($b);    
               $filePaths = join("\n",$b);
               $urlPaths = join("\n",$a);
               if((!file_put_contents(PATH.'safeme/filepath.sfw',$filePaths) ||
                  (!file_put_contents(PATH.'safeme/urlpath.sfw',$urlPaths))))  throw new Exception('Cant\'t write in memories dir');             
      }
      
      /* Authenticate user */
      public 
      function auth($u,$p,$up,$op = false)                /* For some rediection bypass */
      {                                                   /* moglby byc jeden argument ex: user=a&pass=b&zon=c ale jak to mowiÄ&#8230; w kupie sila */
               $uCs = explode(':',$u);
               $userVar = $uCs[0].'='.$uCs[1];
               $pCs = explode(':',$p);
               $passVar = $pCs[0].'='.$pCs[1];
               $allVars = $userVar.'&'.$passVar;
               if($op)
               {
                     $op = explode(':',$op);
                     $opVar = $op[0].'='.$op[1];
                     $allVars .= '&'.$opVars;
               }
               curl_setopt($this->IO->getSocks(),CURLOPT_URL,'http://'.HOST.'/'.$up);
               curl_setopt($this->IO->getSocks(),CURLOPT_RETURNTRANSFER, 1);
               curl_setopt($this->IO->getSocks(),CURLOPT_POST,1);
               curl_setopt($this->IO->getSocks(),CURLOPT_POSTFIELDS,$allVars);
               $___R = curl_exec($this->IO->getSocks());                              //<-- generalnie ssie ale juÅ¥ mi siÄ&#8482; nie chce 
               if(preg_match_all("/ERROR:|<strong>ERROR|Error|Failed|>Error|error|error:|ERROR|Error+[\x20]+[:\[\(]/",$___R,$o)) 
                  die("~~~~======Ä&#8222;UTH FAILED======~~~~");
      }
};

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
      
class Menu                 /* W wersji finlnej osbjekt skanera jest wkomponowany w MEnu*/
{                          /* wywolanie wygloda mniej wiÄ&#8482;cej tak new Menu(args) m->var = x,m->var = y; m->create() - i tyle*/ 
      private $type;
      private $mVars = array();
      private $mArgv = array();
      //private $mObject;              /*skun object */ 
      
      public
      function __construct($arg,$arv/*,$ob*/)
      {
               $this->mArgv[0] = $arg;
               $this->mArgv[1] = $arv;
               $this->type = config::getClientType();
            //   $this->mObject = new $ob();
               $this->create();
      }
      
      private 
      function create()
      {
               if($this->type == 'CONSOLE')
                  require_once('iface/console.php'); 
               else if($this->type == 'WEB')
                       require_once('iface/web.php');
               else throw new Exception('Fatal unexcepted un...');        
      }
      
      public 
      function __set($var,$val)
      {
               $this->mVars[$var] = $val;  
      }
      
      public 
      function __get($var)
      {
               return $this->mVars[$var];
      }
      
};

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

try       //test
{
  $m = new Menu($argc,$argv);      
  $v = new v2($m->cms,"php");
  $forFor = $v->fCount;
  $v->IO->setMethod($m->mHttp);
  $v->EXPRESION->setExpresion("<b>Warning</b>:");
  if($auth == 'TRUE') $v->auth($m->user,$m->pass,$m->up,$m->op);
  $iThis = 0;
  $v->getVarsById($iThis);
  $FLAG = 0;
  for(;$iThis<$forFor;)        
  {
    $RECV = $v->IO->cGetData($v->uri[$iThis]);
    echo("Url: ".$v->uri[$iThis]." var: ".$v->IO->vars[$v->IO->interator]." ");
    if(($RES = $v->EXPRESION->getExpresion($RECV)) != -1)
    {
       echo("[stats: file maybe evil]         \r");
       if($FLAG == 0) $v->IO->makeLog($RES);
       $FLAG = 1;
    }   
    else echo("[stats: file ok]            \r");   
    if($v->IO->interator == 0)
    { 
         $v->safe();
         $iThis++;
         $v->getVarsById($iThis);
         $FLAG = 0;
    }   
  }
}
catch(Exception $ERR)
{
    die("JEAPH !!! ".$ERR->getMessage());
}
 
//~echo 
?>
