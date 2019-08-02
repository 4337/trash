<?php
//echo 2010
if(!defined('GUARD')) die("...death...");

function help()
{
         die ("{-----------------------------------------}\r\n".
              " SkUn v2.1 created by Maciek z Klanu (R)  \r\n".
              " use skv2.php [options]\r\n".
              "{-----------------------------------------}\r\n".
              " Options:\r\n".
              "              -cms NazwaKatalogu/TwojegoCms_a || / jesli jest to katalog glowny\r\n".
              "   (optional) -method GET || POST\r\n".
              "   (bonus)    -bonus \r\n".
              "   (bonus)    -bugs  \r\n".
              "   (optional) -auth var:user var:pass Path/ulogin.php optionaVar:lol\r\n".
              "(i n33d help) -help  \r\n"); 
}

function info($opt1,$opt2,$opt3,$opt4)
{
         echo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\r\n".
              "@  Argumments  @                                @\r\n".
              "@@@@@@@@@@@@@@@@              @@ @@@@@@@@@@@@@@@@\r\n".
              "@ -cmPath : ".$opt1."                             \r\n".
              "@ -mHttp  : ".$opt2."                             \r\n".
              "@ -auth : ".$opt3."                               \r\n");
         if($opt3 == 'TRUE') echo("@ -uLogin : ".$opt4."         \r\n");  
            
         echo("@                            @ echo 2o0po01o (r) @\r\n".
              "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\r\n");
}

function bugs()
{
         die("************** \r\n".
             "PrestaShop 1.3.1 0day exploit [269a5fcc1b6d09413a45f880fe436caf] 18.06.2010\r\n".
             "osCommerce 3 alfa 0day exploit [922259a6c3b873115fb0f8c4711434820f94012c] 20.06.2010\r\n".
             "net2ftp  0.98 0day exploit  [17d7055859d99a0d606cfaf17ae38638] 27.09.2010\r\n");
}

if($this->mArgv[0] < 2) help();
else if($this->mArgv[1][1] == '-bonus') bonus();
else if($this->mArgv[1][1] == '-bugs') bugs();
else if($this->mArgv[1][1] == '-help') help();
else if(!in_array('-cms',$this->mArgv[1])) die("Bad option \r\n");
$argc = $this->mArgv[0];
$argv = $this->mArgv[1];
global $auth;
$auth = 'FALSE';
$this->mHttp = 'GET';
for($i=1; $i<$argc; $i+=2)
{
    $cArgv = strtolower($argv[$i]);
    if($cArgv == '-method') $this->mHttp = ($argv[$i+1]) ? strtoupper($argv[$i+1])
                                                         : 'GET';
    else if($cArgv == '-cms')
    {
       $cms = $argv[$i+1];
       if($cms[0] != '/') 
          $this->cms = '/'.$cms;
       else $this->cms = $cms;    
    }   
    else if($cArgv == '-auth')
    {
         if((!$argv[$i + 1]) || (!$argv[$i + 2]) || (!$argv[$i + 3])) help();
         if(($argv[$i + 4]) && ($argv[$i + 4][0] != '-'))
         {
            if(strpos($argv[$i + 4],':') === false) die("OptionaVar is mailformed\r\n");
            $this->op = $argv[$i + 4];
            $i++;
            $_omg_ = 1;
         } 
         else{ $this->op = false; $_omg_ = 2; }
         if((strpos($argv[$i + $_omg_],':') === false) ||
            (strpos($argv[$i+1],':') === false)) die("".$argv[$i + $_omg_]." ".$argv[$i+1]."\r\n");//help();
         $this->user = $argv[$i + 1];
         $this->pass = $argv[($i++) + 1];
         $this->up = $argv[($i++) + 2];
         $auth = 'TRUE';
    }
    else help(); 
}

$upCop = $this->cms.'/'.$this->up;
$this->up = $upCop;
info($this->cms,$this->mHttp,$auth,$this->up);

function bonus()
{
         die("no fucking bonus\r\n");
}
?>