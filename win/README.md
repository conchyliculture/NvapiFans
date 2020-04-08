# Installation

## Windows

### Requirements

You need some recent Visual C++ libs from [here](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads).

### Build

In folder `win` open `NvapiFans.sln` with VisualStudio 2019, run `Build`.

### Run

`NvapiFansCli.exe` should be generated and you can list your graphic card parameters:

```
PS C:\Users\renzokuken\source\repos\NvapiFans\win> .\x64\Debug\NvapiFansCli.exe
Found 1 NVidia GPUs.
- GeForce RTX 2070 SUPER
 * External fan speed was set at 57%
 * Actual External fan1 Speed: 1380 RPM
 * Actual External fan2 Speed: 1380 RPM
 * GPU temp: 62C
 * Memory temp: 62C
```

You can then set the required speed of your External/Fanconnect fans to, for example, 55%:

```
PS C:\Users\renzokuken\source\repos\NvapiFans\win> .\x64\Debug\NvapiFansCli.exe -e 55
PS C:\Users\renzokuken\source\repos\NvapiFans\win> .\x64\Debug\NvapiFansCli.exe
Found 1 NVidia GPUs.
- GeForce RTX 2070 SUPER
  * External fan speed was set at 55%
  * Actual External fan1 Speed: 1350 RPM
  * Actual External fan2 Speed: 1350 RPM
  * GPU temp: 61C
  * Memory temp: 61C
```

### Install the service

I also provide a Windows Service you can keep running which will try to set the appropriate 
fan speed to keep your GPU under a target temperature.

Install it like so:

  * If needed, copy & edit `config.json` and store it in `C:\ProgramData\NvapFansSvc\`
  * Consider copying `NvapiFansSvc.exe` to a safe place
  * From an elevated command prompt, install the service with `sc.exe create "NvapiFans Service" -binPath= <ABSOLUTE_PATH_TO_YOUR_BUILD>\NvFansSvc.exe start= boot DisplayName= "NvapiFans Service"`
  * Make sure it is started `sc.exe start "NvapiFans Service"` 

