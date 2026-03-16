<# 
  Author: echovsky
  Based on: https://www.powershellgallery.com/packages/PSNamedPipe/1.0.0.19/Content/Public%5CNew-PSNamedPipeClient.ps1
  Version: 1.0
#>

function New-PSNamedPipeClient {
    Param (
        [Parameter(Mandatory = $true,
            Position = 0,
            ValueFromPipelineByPropertyName = $true)]
        [string]$Name,
        
        # Param2 help description
        [Parameter(Mandatory = $true,
            Position = 1,
            ValueFromPipelineByPropertyName = $true)]
        [string]$ComputerName = '.',
        
        # Param3 help description
        [Parameter(Mandatory = $true,
            Position = 2,
            ValueFromPipelineByPropertyName = $true)]
        [ValidateSet("In", "Out", "InOut")]
        $Direction,
		
		[Parameter(Mandatory = $false,
            Position = 3,
            ValueFromPipelineByPropertyName = $true)]
		$ipcDataFile = $null,
		
		[Parameter(Mandatory = $false,
            Position = 4,
            ValueFromPipelineByPropertyName = $true)]
		$interactive = $false,
		
		[Parameter(Mandatory = $false,
            Position = 5,
            ValueFromPipelineByPropertyName = $true)]
		$tiemout = 5000
    )
    
    Write-Host '[+]. Creating NamedPipe Client stream'
	Write-Host '[+]. Pipe:'$Name
	Write-Host '[+]. Computer:'$ComputerName
	Write-Host '[+]. Direction:'$Direction
	
    if($ipcDataFile -eq $null) {
		$iPath = Read-Host '[?]. Enter ipc data file path:'
	} else {
		$iPath = $ipcDataFile
	}
	
	try { $iData = Get-Content $iPath -Raw -ErrorAction Stop  }
	catch {
		Write-Host '[!].'$error[0]  
		return
	}

    if($interactive) {
	   $iData = Read-Host '[?]. Enter ipc data:'
	}

    $pipeClient = New-Object -TypeName System.IO.Pipes.NamedPipeClientStream -ArgumentList @(
                  $ComputerName, $Name, $Direction)
    
    Write-Host '[+]. Creating connection'

    try{ $pipeClient.Connect($tiemout) } 
	catch { 
	     Write-Host '[!].'$error[0] 
		 return
	} 
        
    Write-Host '[+]. Connected to the pipe: '$Name	
	
    do {
	    if($pipeClient.CanRead) {
		   $rBuff = New-Object -TypeName System.Byte[] -ArgumentList $pipeClient.InBufferSize
		   $rLen = $pipeClient.Read($rBuff,0, $pipeClient.InBufferSize)
		   Write-Host '[X]. Read inSize: ['$pipeClient.InBufferSize'] '$rLen' bytes'
	    }
		if($pipeClient.CanWrite) {
		   #$pipeClient.Write($iData, 0, $iData.Length);   #trza skonwertowac do byte[]
		   $iBytes = [System.Text.Encoding]::UTF8.GetBytes($iData);
		   try {
		   $pipeClient.Write($iBytes, 0, $iData.Length);
		   $pipeClient.WaitForPipeDrain()
		   } catch {
			  Write-Host '[!].'$error[0]   
		   }
		   Write-Host '[X]. Write inSize: '$iData.Length' bytes';
		   
		}

    } while($pipeClient.IsConnected);	

    $pipeClient.Close()
    $pipeClient.Dispose()	
	
	return;

}

New-PSNamedPipeClient "nord-security\v1\ThreatProtectionService\Server" "." "InOut" 