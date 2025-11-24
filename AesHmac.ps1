<#
 Bo kryptografia .NET jest głupia ;]
#>

Add-Type -Assembly System

<#
 Helpers 
 form: https://www.powershellgallery.com/packages/AnsibleVault/0.2.0/Content/Private%5CConvert-HexToByte.ps1
       https://www.powershellgallery.com/packages/ConvertStrings/0.0.5/Content/Private%5CConvert-ByteArrayToHex.ps1
#>

function Convert-HexStrToByteArray {
    [CmdletBinding()]
    param(
        [String]$Value
    )
    $bytes = New-Object -TypeName byte[] -ArgumentList ($Value.Length / 2)
    for ($i = 0; $i -lt $Value.Length; $i += 2) {
        $bytes[$i / 2] = [Convert]::ToByte($Value.Substring($i, 2), 16)
    }
    return [byte[]]$bytes
}

function Convert-ByteArrayToHexStr {
    [CmdletBinding()]
    param (
         [Byte[]]$ByteArray
    )
    Process {
        $stringBuilder = [System.Text.StringBuilder]::new($ByteArray.Length * 2)
        $ByteArray | ForEach-Object {
            [void]$stringBuilder.Append($_.ToString("x2"))
        }
        return $stringBuilder.ToString()
    }
}
<# end from :D #>
<#
  Format helpers
#>

enum _HmacType {
	 SHA1 = 0
	 SHA256 = 1
	 SHA384 = 2
	 SHA512 = 3
}

function _Get-HmacSize {
	  [CmdletBinding()] 
	  param(
	       [_HmacType] $hmac_type = [_HmacType]::SHA256
	  )
	  switch($hmac_type) {
			 SHA1 { return 20;}
			 SHA256 { return 32; }
	         SHA384 { return 48; }
		     SHA512 { return 64; }
      }
      return 0;	  
}

function _Aes-Params-Cookie {
	     [CmdletBinding()]
          param (
                [string]$Cookie,
				[_HmacType] $HmacType
          )
		$hmac_size = _Get-HmacSize $HmacType;
	    $params = @{};
		
		$array = Convert-HexStrToByteArray $Cookie;
		
		$params.IV = [byte[]]::new(16);
		[byte[]]::Copy($array,$params.IV,16);
		$params.IV_STR = (Convert-ByteArrayToHexStr $params.IV).ToUpper();
		
		$params.MSG = [byte[]]::new($array.length - 16 - $hmac_size);
		[byte[]]::Copy($array, 16, $params.MSG, 0, $array.length - $hmac_size - 16);
		$params.MSG_STR = (Convert-ByteArrayToHexStr $params.MSG).ToUpper();
		
		if($hmac_size -ne 0) {
		   $params.HMAC = [byte[]]::new($hmac_size);
		   [byte[]]::Copy($array, $array.length - $hmac_size, $params.HMAC, 0, $hmac_size);
		   $params.HMAC_STR = (Convert-ByteArrayToHexStr $params.HMAC).ToUpper();
		} else {
		  $params.HMAC = $null;
          $params.HMAC_STR = $null;		  
		}
		
		return $params;
}

<#
 Aes class
#>

class _Aes {
	
	   [System.Security.Cryptography.Aes]$csp;
	   
	   <#
	     constructors
	   #>
	   
	    _Aes () {
		     $this.csp = [System.Security.Cryptography.Aes]::Create("System.Security.Cryptography.AesManaged");
	    }
	    _Aes(
	         [byte[]]$key, 
		     [byte[]]$IV, 
		     [System.Security.Cryptography.CipherMode]$mode,
		     [int]$block_size, 
		     [int]$key_size, 
		     [System.Security.Cryptography.PaddingMode]$padding 
	   ) {
		    $this.csp = [System.Security.Cryptography.Aes]::Create("System.Security.Cryptography.AesManaged");
			$this.csp.key = $key;
			$this.csp.IV = $IV;
			$this.csp.mode = $mode;
			$this.csp.blockSize = $block_size;
			$this.csp.keySize  = $key_size;
			$this.csp.padding  = $padding;   
	   }
	   
	   _Aes(
	         [byte[]]$key, 
		     [byte[]]$IV
	   ) {
		    $this.csp = [System.Security.Cryptography.Aes]::Create("System.Security.Cryptography.AesManaged");
			$this.csp.key = $key;
			$this.csp.IV = $IV;
			$this.csp.mode = [System.Security.Cryptography.CipherMode]::CBC;
			$this.csp.blockSize = 128;
			$this.csp.keySize  = 256;
			$this.csp.padding  = [System.Security.Cryptography.PaddingMode]::PKCS7;   
	   }
	   
	   <# -- methods -- #>
	   
	   <# set vector IV #>
	   [byte[]] SetIV( [string]$IV ) {
			  $this.csp.IV = [system.Text.Encoding]::ASCII.GetBytes($IV);
	 		  return $this.csp.IV;
	   }
	   
	   [byte[]] SetIV( [byte[]]$IV ) {
		   $this.csp.IV  = $IV;
		   return $this.csp.IV;
	   }
	   
	   <# set key #>
	   [byte[]] SetKey( [byte[]]$key ) { 
		   $this.csp.key = $key;
		   return $this.csp.key;
	   }
	   
	   [byte[]] SetKey( [string]$key ) { 
		   $this.csp.key =  Convert-HexStrToByteArray $key;
		   return $this.csp.key;
	   }
	   
	   <# set blockSize #>
	   [int] SetBlockSize( [int]$blockSize ) {
		     $this.csp.blockSize = $blockSize;
			 return $this.csp.blockSize;
	   }
	   
	   <# set keySize #>
	   [int] SetKeySize( [int]$keySize ) {
		     $this.csp.keySize = $keySize;
			 return $this.csp.keySize;
	   }
	   
	   <# set padding #>
	   [System.Security.Cryptography.PaddingMode] SetPadding( [System.Security.Cryptography.PaddingMode]$padding ) {
		   $this.csp.padding = $padding;
		   return $this.csp.padding;
	   }
	   
	   <# set mode #>
	   [System.Security.Cryptography.CipherMode] SetMode(   [System.Security.Cryptography.CipherMode]$mode ) {
		   $this.csp.mode = $mode;
		   return  $this.csp.mode;
	   }
	   
	   [string] Encrypt( [string]$msg ) {
		        $encryptor = $this.csp.CreateEncryptor();  
				$msg_arr = [system.Text.Encoding]::UTF8.GetBytes($msg);
				$enc_msg = $encryptor.TransformFinalBlock($msg_arr, 0, $msg_arr.length);
			    [byte[]] $ivenc = $this.csp.IV + $enc_msg;
			    $ret = (Convert-ByteArrayToHexStr $ivenc).ToUpper();
				return $ret;
	   }
	   
	   [string] Encrypt( [byte[]]$msg ) {
		        $encryptor = $this.csp.CreateEncryptor();  
				$enc_msg = $encryptor.TransformFinalBlock($msg, 0, $msg.length);
			    [byte[]] $ivenc = $this.csp.IV + $enc_msg;
			    $ret = (Convert-ByteArrayToHexStr $ivenc).ToUpper();
				return $ret;
	   }
	   
	   [byte[]] Decrypt( [string]$msg ) {
		   $decryptor = $this.csp.CreateDecryptor(); 
           $msg_arr = Convert-HexStrToByteArray $msg;	
           $ret = $decryptor.TransformFinalBlock($msg_arr, 0, $msg_arr.length);
           return $ret; 	
	   }
	   
	   
}

function _Copmpute-Hmac {
	      [CmdletBinding()] 
	      param ( 
		         [string]$enc_msg,  #hex string
				 [string]$key,      #hex string
				 [_HmacType]$hmac_type =  [_HmacType]::SHA256
		  )
		  $hmac_size = _Get-HmacSize $hmac_type;
		  $enc_arr = Convert-HexStrToByteArray $enc_msg;
		  $key_arr = Convert-HexStrToByteArray $key;
		  switch($hmac_type) {
			     SHA1{ $hmac = [System.Security.Cryptography.HMACSHA1]::new($key_arr); }
				 SHA384{ $hmac = [System.Security.Cryptography.HMACSHA384]::new($key_arr); }
				 SHA512{ $hmac = [System.Security.Cryptography.HMACSHA512]::new($key_arr); }
				 SHA256{ $hmac = [System.Security.Cryptography.HMACSHA256]::new($key_arr); }
		  }
		  $hmac_hex = (Convert-ByteArrayToHexStr $hmac.ComputeHash($enc_arr)).ToUpper();
		  return $enc_msg + $hmac_hex;
		  
}

<#
  TESST
#>

$cookie = "3E5D27FCB245A9EA280A67CAFF5D274E4F09797D26B238746ED249206597F8EFA54C03FE1068BD0471658D13B303F7E02196A1C8D3129382F711B5C3BB126F0A78D5D559B50AF0C2B82DE5D82D8D683C8C57CECD89FE190CCBB3A58A9EA8851BDA2B336C3499FA0614EB7ED793FBEDF8FE41D48BBB271E92DDDB80B5325D3017ACD5173FF326F75DF691B736170A756A7F1D91B56B9ED8AE7FDD633A2A95BFC0";
$params1 = _Aes-Params-Cookie $cookie SHA256
$aes = [_Aes]::new()
#$iv = $aes.SetIV("0123456789123456");
$mode = $aes.SetMode( [System.Security.Cryptography.CipherMode]::CBC );
$key = $aes.SetKey("B26C371EA0A71FA5C3C9AB53A343E9B962CD947CD3EB5861EDAE4CCC6B019581");
$block_size = $aes.SetBlockSize(128);
$padding = $aes.SetPadding([System.Security.Cryptography.PaddingMode]::PKCS7);

#$encmsg = $aes.Encrypt("ALA MA KOTA");
#$encmsghmac = _Copmpute-Hmac $encmsg "EBF9076B4E3026BE6E3AD58FB72FF9FAD5F7134B42AC73822C5F3EE159F20214B73A80016F9DDB56BD194C268870845F7A60B39DEF96B553A022F1BA56A18B80"  SHA256
#write-output "Encrypted text           : $encmsg"
#write-output "Encrypted text with hmac : $encmsghmac"
#$params = _Aes-Params-Cookie $encmsghmac SHA256
$iv = $aes.SetIV($params1.IV);
$decrypted = $aes.Decrypt($params1.MSG_STR);
#$decrypted.findAll(
$dec_str = [system.Text.Encoding]::UTF8.GetString($decrypted)
write-output "Decrypted text : $dec_str"
Set-Content cookie.bin -Value $decrypted -Encoding Byte

#$decrypted =  [System.IO.File]::ReadAllBytes("fixedcookie.bin")# 
$enced = $aes.Encrypt($decrypted);
$encedhmac = _Copmpute-Hmac $enced "EBF9076B4E3026BE6E3AD58FB72FF9FAD5F7134B42AC73822C5F3EE159F20214B73A80016F9DDB56BD194C268870845F7A60B39DEF96B553A022F1BA56A18B80"  SHA256
#$decbytes = Convert-ByteArrayToHexStr $decrypted;
write-output "Encrypted text and hmac : $encedhmac";
if($cookie.Contains($encedhmac)) {
	write-output "Cryptomsg are the same !";
}