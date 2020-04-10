# NvapiFans

Control external fans (ie: connected via the FanConnect headers) on ASUS ROG Strix 2070 Super.

Might work on other ASUS graphic cards.

![](docs/meme.png)

## Disclaimer

These tools are modifying the physical state of your computer. It might be doing things wrong, and
I want to have nothing to do if you brick your expensive gear.

## Installation / Usage

See relevant documents per platform:
  * [Linux](linux/README.md) (Kind of works, but you'll have to send i2c commands on your own)
  * [Windows](win/README.md) (WiP, almost done)

### config.json

For the win & linux services, you can use a `config.json` file to set configuration values.
The default values and what they mean are as such:
```
{
    "version": 1,
    "gpu_config": {
        "interval_s": 2,// Which interval in seconds between checks
        "min_temp_c": 40,  // Below this, fan switches to its "minimum speed"
        "max_temp_c": 80, // Above this, fan switches to its "maximum speed"
        "min_fan_start_speed": 35, // This is the minimum speed at which the fan begins spinning, between 0 & 255
        "min_fan_stop_speed": 25, // This is the minimum speed at which the fan begins spinning, between 0 & 255
        "min_fan_speed": 0, // Speed to set when fan is below min_temp_c. 0 means "stopped"
        "max_fan_speed": 200, // Speed to set if temp is over max_temp_c. 255 is "max speed"
        "average": 1, // How many last temp readings are used to average the temperature
    }
}
```



## Research notes / Write Up

See [NOTES.md](NOTES.md)

## Acknowledgement

* [Trou](https://twitter.com/_trou_/) for making sense of assembly
* [x0rbl](https://twitter.com/x0rbl) for his knowledge of the dark magics that are c++ & WinDBG
