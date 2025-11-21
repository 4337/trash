Add-Type -Assembly System

class _Aes {
	
	  [System.Security.Cryptography.Aes]$aes_csp;
	  
      _Aes( 
		   [byte[]]$key, 
		   [byte[]]$IV, 
		   [System.Security.Cryptography.CipherMode]$mode = [System.Security.Cryptography.CipherMode]::CBC,
		   [int]$block_size = 128, 
		   [int]$key_size = 128, 
		   [System.Security.Cryptography.PaddingMode]$padding = [System.Security.Cryptography.PaddingMode]::PKCS7 
		  ) { 
		     $this.aes_csp = [System.Security.Cryptography.Aes]::Create("System.Security.Cryptography.AesManaged");
			 $this.aes_csp.key  = $key;
			 $this.aes_csp.IV   = $IV;
			 $this.aes_csp.mode = $mode;
			 $this.aes_csp.bloc_size = $block_size;
			 $this.aes_csp.key_size  = $key_size;
			 $this.aes_csp.padding   = $padding;
	  }
	  
	  #dokonczyc!!!!!
		
	  
}

enum _HmacType {
	 SHA1 = 0
	 SHA256 = 1
	 SHA384 = 2
	 SHA512 = 3
}


function _Get-Aes-Params-From-Cryptogram {
	      [CmdletBinding()]
	      param ( 
		         [string]$enc_msg,
				 [_HmacType] $hmac_type = [_HmacType]::SHA256
		  )
		  $bytes =  Convert-HexToByte($enc_msg)
		  $ret = @{};
		  
		  #chck length !!!
          $ret.IV = [byte[]]::new(16);
		  [byte[]]::Copy($bytes,$ret.IV,15);	 #16
		  #$ret.IV.length;
		  
		  $ret.ENC = [byte[]]::new($bytes.length - 16);
		  [byte[]]::Copy($bytes,16,$ret.ENC,0, $bytes.length - 16);	
          
		  $ret.ALL = [byte[]]::new($bytes.length);
		  [byte[]]::Copy($bytes,0,$ret.ALL,0, $bytes.length);	
		  
          switch($hmac_type) {   #chck! length
                 SHA1 {
					   $ret.HMAC = [byte[]]::new(20);
					   [byte[]]::Copy($bytes,$bytes.length - 20,$ret.HMAC,0, 20);
				 }
				 SHA256{
					   $ret.HMAC = [byte[]]::new(32);
					   [byte[]]::Copy($bytes,$bytes.length - 32,$ret.HMAC,0, 32);
				 }
				 SHA384 {
					   $ret.HMAC = [byte[]]::new(48);
					   [byte[]]::Copy($bytes,$bytes.length - 48,$ret.HMAC,0, 48);
				 }
		         SHA512{
					   $ret.HMAC = [byte[]]::new(64);
					   [byte[]]::Copy($bytes,$bytes.length - 64,$ret.HMAC,0, 64);
		         }
          }			  
				 
		  return $ret;		 
}

function Convert-ByteArrayToHex {
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

Function Convert-HexToByte {
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

function _HmacCompute {
	      [CmdletBinding()] 
	      param ( 
		         [byte[]]$enc_msg,
				 [byte[]]$key,
				 [_HmacType] $hmac_type = [_HmacType]::SHA256
		  )
		  $hmac_size = 32;
		  switch($hmac_type) {
			 SHA1 {
			      $hmac_size = 20;
			 }
	         SHA384 {
			      $hmac_size = 48;
		     }
		     SHA512 {
			       $hmac_size = 64;
		     }
		  }
		  $enc = [byte[]]::new($enc_msg.length - $hmac_size);
		  [byte[]]::Copy($enc_msg,0,$enc,0, $enc_msg.length - $hmac_size);	
		  $hmc = [System.Security.Cryptography.HMACSHA256]::new($key);
		  return (Convert-ByteArrayToHex $hmc.ComputeHash($enc)).ToUpper();
		  
}

$cryptogram = "504879FEC73695421F279100317F4F177EEF160EA0CE855AB6177016EC7181DF0AF16E4BCD395A0A1A19E2F089C29FEEB832D46956ACBE9B64BC97E32DAF0A7192875D2C5A4EA63036F386B19725AFEB871067F62BF1531FC0D0DBE0DEF413D62597054F3A8E0CD4B4CDBFE7436E4A429EAE441564D8D214722C3FBCA41F14702C84B63A45CC37258439B8233DD0A8A2F50095AA58DCAF1A0256E7EC20F9CB21";
$aes_params = _Get-Aes-Params-From-Cryptogram($cryptogram);
$aes_params.IV.GetType();
$aes_params.IV.length;
$aes_params.ENC.GetType();
$aes_params.ENC.length;
$aes_params.ALL.GetType();
$aes_params.ALL.length;
$aes_params.HMAC.GetType();
$aes_params.HMAC.length;
$valid = Convert-HexToByte("EBF9076B4E3026BE6E3AD58FB72FF9FAD5F7134B42AC73822C5F3EE159F20214B73A80016F9DDB56BD194C268870845F7A60B39DEF96B553A022F1BA56A18B80");
$hmac_code = _HmacCompute $aes_params.ALL $valid;
write-output "HmacSha256 : $hmac_code";
if($cryptogram.Contains($hmac_code)) {
   Write-output "HmacSha256 is correct !";
}

