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

### config.json

For the win & linux services, you can use a `config.json` file to set configuration values.
The default values and what they mean are as such:
```
{
    "version": 1,
    "log_level": "quiet",    // Verbosity level for the logfile.
                             // either "quiet", "error",  "info", or "debug". Default is "error". "quiet" will not log anything. "error" will just log errors. "info" will log fan speed changes. "debug" will log debug info.
    "gpu_config": {
        "interval_s": 2,     // The frequency at which the service checks the GPU temperature and adjusts the fan speed.
                            // Seconds, range 1 - 30
        "min_temp_c": 40,    // At this temperature or lower, the service will run the fan at min_fan_speed.
                            // Celsius, range 0 - 100
        "max_temp_c": 80,    // At this temperature or higher, the service will run the fan at max_fan_speed.
                            // Celsius, range 0 - 100
        "min_fan_start_speed": 35,  // When the fan is stopped, this is the minimum speed at which the fan will start spinning.
                                    // Range 0 - 255
        "min_fan_stop_speed": 25,   // When the fan is already spinning, this is the lowest speed at which the fan will continue spinning.
                                    // Range 0 - 255
        "min_fan_speed": 0,         // The speed at which the service will run the fan when the GPU temperature is less than or equal to min_temp_c.
                                    // Range 0 - 255. A value of 0 indicates that the fan will not spin
        "max_fan_speed": 200,       // The speed at which the service will run the fan when the GPU temperature is greater than or equal to max_temp_c.
                                    // Range 0 - 255.
        "average": 1,               // How many of the recent temperature readings are averaged to calculate the "current" temperature, used in fan speed calculations.
                                    // Not implemented.
    }
}
```
