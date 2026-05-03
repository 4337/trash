#filemon 1.0 
#author @echovsky
#03/05/2026
#C:\Users\echo\Desktop\filemon\filemon>powershell -ExecutionPolicy Bypass -File ./FileMon2.ps1 -dir C:\ -subdir -Copy C:\ProgramData -mode Created,Deleted,Changed
#C:\Users\echo\Desktop\filemon\filemon>powershell -ExecutionPolicy Bypass -File ./FileMon2.ps1 -dir C:\ -Copy C:\ProgramData
#...
#ogólnie nie wiem czy dobrze działa, kiedyś sprawdzę :D

Param(
    [Parameter(
	Mandatory=$false,
	HelpMessage="Directory path that will be monitored (PATH)"
	)]
    [string] $dir,
	
	[Parameter(
	Mandatory=$false,
	HelpMessage="Subdirectory monitoring (true|false)"
	)]
	[switch]$subdir=$false,
	
	[Parameter(
	 Mandatory=$false,
	 HelpMessage="[System.IO.WatcherChangeTypes]"
	)]
	[System.IO.WatcherChangeTypes[]]$mode=@([System.IO.WatcherChangeTypes]::Created, [System.IO.WatcherChangeTypes]::Deleted),
	
	[Parameter(
	 Mandatory=$false,
	 HelpMessage="[System.IO.WatcherChangeTypes]"
	)]
	[IO.NotifyFilters[]]$filter=@([IO.NotifyFilters]::FileName, [IO.NotifyFilters]::LastWrite),
	
	[Parameter(
	 Mandatory=$false,
	 HelpMessage="[System.IO.WatcherChangeTypes]"
	)]
	[string] $file_filter="*",
	
	[Parameter(
	 Mandatory=$false,
	 HelpMessage="Dopy file to disretory (PATH)"
	)]
	[string]$copy,
	
	[switch]$help

)

function hlp {
	
	write-host "use FileMon2.ps1 (options)"
	write-host "-dir PATH - Directory path that will be monitored (PATH) - mandatory"
	write-host "-subdir - Subdirectory monitoring (true|false) - default false"
	write-host "-mode [System.IO.WatcherChangeTypes] Created,Deleted,... - What kind of change should be monitored - default Created,Deleted"
	write-host "-filter [IO.NotifyFilters] - default, FileName, LastWrite"
	write-host "-file_filter MASK - Mask that defines what kind of file will be monitored - default '*'"
	write-host "-copy PATH - Destination path for file copies"
	
}

function Invoke-SomeAction{
         param
         (
               [Parameter(Mandatory)]
               [System.IO.WaitForChangedResult]
               $ChangeInformation
         )
	     
		 write-host -ForegroundColor DarkYellow "[*]. $($ChangeInformation.Name) [$($ChangeInformation.ChangeType)]" 
		  
	     if($script:PSBoundParameters.ContainsKey('copy')) {
				 $name = Split-Path -Path $ChangeInformation.Name -Leaf;
			     $src = $ChangeInformation.Name;
                 $dst = $copy + "\" + $name;
				 try {
			          Copy-Item -Path $src -Destination $dst -ErrorAction Stop
			          write-host -ForegroundColor White "[*]. File $src was copied to $dst"
				 }
				 catch {
		  	            write-host -ForegroundColor Red "[!]. Houston, we have a problem. $_"	  
		         }	    
	     } 
}

write-host "[!].Use FileMon2.ps1 -help for help"

if($help -Or -not $dir) {
   hlp;
   exit;
}

try
{
	 
  $Timeout = 1000;
  $change_types = $mode;
  Write-Warning "[i]. FileSystemWatcher is monitoring $dir filter($file_filter)"
  
  # create a filesystemwatcher object
  $AttributeFilter = $filter; 
  $watcher = New-Object -TypeName IO.FileSystemWatcher -ArgumentList $dir, $file_filter -Property @{
    IncludeSubdirectories = $subdir
    NotifyFilter = $filter
  }

  #start monitoring manually in a loop:
  do
  {
    $result = $watcher.WaitForChanged($change_types, $Timeout)
    if ($result.TimedOut) { continue }

    Invoke-SomeAction -Change $result 

  } while ($true)
}
finally
{
  # release the watcher and free its memory:
  $watcher.Dispose()
  Write-Warning 'FileSystemWatcher removed.'
  
}


