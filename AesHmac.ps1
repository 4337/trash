<#
 Ponieważ kryptografia .NET jest głupia :)
 Jak być może wiecie domyślne ustawienia crypto różnią się z wersji do wesrji
 Na przykład .NET 4.8 może wykorzystywać domyślnie rozmiar blokó 128 bitów, rozmiar klucza 256
 a nowe .NET jeszcze zupełnie innych ustawień.
 Sytuacja komplikuje sie kiedy chcecie wykorzystać gotową biblioteke i okazuje się że 
 ona z jakiś powodów korzysta z domyślnych ustawień i nie pozwala tego zmienić.
 Jest milion możliwośći! 
 Możecie na przykład naiwnie poświecać czas na szukanie sposóbu na zmianę ustawień domyślnych na poziomie aplikacji 
 (tak app.config to jest myśl) lub próbować zmienić nieco biblioteke którą chcecie wykorzystać lub 
 w ogóle zmienić ustawienia OS-a - pewenie troche rzeczy się zepsuje, ale w końcu chcemy użyć AES 128 CBC prawda ?
 Wydawać by się mogło że przecież musi istenieć taka klasa która umożliwia powiedzenie aplikacji 
 "hej jeśli używasz gdzieś AES-a to tu są parametry bo ja tak chcę", to było by logiczne i przydatne.
 Niestety nie ma takiej klasy.
 Nawet sztuczna inteligencja mówi "dej se spokój".
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
		  $params.HMAC = null;
          $params.HMAC_STR = null;		  
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
	   
	   [string] Encrypt( [string]$msg ) {
		        $encryptor = $this.csp.CreateEncryptor();  
				$msg_arr = [system.Text.Encoding]::UTF8.GetBytes($msg);
				$enc_msg = $encryptor.TransformFinalBlock($msg_arr, 0, $msg_arr.length);
			    [byte[]] $ivenc = $this.csp.IV + $enc_msg;
			    $ret = (Convert-ByteArrayToHexStr $ivenc).ToUpper();
				return $ret;
	   }
	   
	   [string] Decrypt( [string]$msg ) {  #$msg = hex string
		   		$decryptor = $this.csp.CreateDecryptor(); 
                $msg_arr = Convert-HexStrToByteArray $msg;	
                $dec_msg = $decryptor.TransformFinalBlock($msg_arr, 0, $msg_arr.length);
                $ret = [system.Text.Encoding]::UTF8.GetString($dec_msg);
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

#$params = _Aes-Params-Cookie "48F59EE979C4CB7F9BBC90C15D9D3024EA4A2D974B2E484D166A88DCF98C0A737F448093ECCA6B7393F6992639C576CFF6FE8426A41493DA0292F4DFCE5C7D1B860610E4961059A207D0195153297C4DAD83197C455AF79355F258F410CBAB45966F577FAA4E955817F0E12F1575838CE2270F115C1E178C569AC7EA8E0D9651C91F7877AC719C0055B1B3B526CE7A7CBBCFF04328F1F37C70ACAD7AC4C42640" SHA256;

$aes = [_Aes]::new()
$iv = $aes.SetIV("0123456789123456");
$key = $aes.SetKey("B26C371EA0A71FA5C3C9AB53A343E9B962CD947CD3EB5861EDAE4CCC6B019581");
$key_size = $aes.SetKeySize(256);
$block_size = $aes.SetBlockSize(128);
$padding = $aes.SetPadding([System.Security.Cryptography.PaddingMode]::PKCS7);

$encmsg = $aes.Encrypt("ALA MA KOTA");
$encmsghmac = _Copmpute-Hmac $encmsg "EBF9076B4E3026BE6E3AD58FB72FF9FAD5F7134B42AC73822C5F3EE159F20214B73A80016F9DDB56BD194C268870845F7A60B39DEF96B553A022F1BA56A18B80"  SHA256
write-output "Encrypted text           : $encmsg"
write-output "Encrypted text with hmac : $encmsghmac"
$params = _Aes-Params-Cookie $encmsghmac SHA256
$iv = $aes.SetIV($params.IV);
$decrypted = $aes.Decrypt($encmsg);
write-output "Decrypted text (first bytes are IV vector) : $decrypted" 
