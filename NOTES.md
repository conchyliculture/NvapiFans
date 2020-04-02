# Notes 

## Goal

Graphic cards made by ASUS have their own [PWM fan headers](https://rog.asus.com/articles/gaming-graphics-cards/strix-gtx-10801070-what-is-asus-fancontrol/) that can be controlled via their specific app, for example to trigger extra air intake when you GPU is getting toast.

The problem is I couldn't find any other tool to control it but [ASUS provided software](https://www.asus.com/us/site/graphics-cards/gpu-tweak-ii/).
It also turns out that their app is [kind](https://www.reddit.com/r/buildapc/comments/4cbb1y/discussion_bad_experience_with_asus_gpu_tweak_ii/) of [lame](https://www.reddit.com/r/nvidia/comments/99r9vo/asus_gpu_tweak_ii_killed_my_gtx_970/) and bloated and [might inject ads](https://www.guru3d.com/news_story/asus_gpu_tweak_ii_injects_ads_into_your_games.html) and install [terrible drivers](https://syscall.eu/blog/2020/03/30/asus_gio/), so it’d be cool to have something else.

In my case, I decided to "[deshroud](https://old.reddit.com/r/sffpc/comments/exncu9/deshrouded_asus_strix_rtx_2070_super_with_guide/)" (ie: remove the provided fans & plastic covering the whole thing) my new Asus Rog Strix RTX 2070s, so I could use slightly bigger and hopefull more silent fan, that I could orient as "exhaust". 

So basically I want to be able to control these fan headers.

##  The GPUTweakII package

### GPUTweakII.exe

It is a 32b exe, that you run as Admin. You can download [here](https://dlcdnets.asus.com/pub/ASUS/Graphic%20Card/Unique_Applications/GPUTweakII-Version2171.zip) the version I used (2.1.7.1).

```
497482e1dbaccb01e6d523246120c81d  GPUTweakII.exe
```

The way you can change the "External" fans speed is to:
 * Set the speed mode to "Manual"
 * Change the speed via the terrible slider
 * Click a big ‘Apply’ button that seems to commit everything to the graphic card

![Image of GPUTweakII.exe](docs/gputweak2.jpg)

### Loaded DLLs

So how do I do things?

The whole package comes with a bunch of binaries, DLLs, installed drivers and services and it's pretty tought figuring out where to start.

I was hinted towards looking at DLL being loaded by GPUTweakII with [ProcessExplorer](https://docs.microsoft.com/en-us/sysinternals/downloads/process-explorer).

![Process Explorer](docs/procexp.jpg)

Out of these, some of them come from the ASUS folder:

 * `AURA_DLL.dll`, which might be related to [blinky lights](https://www.asus.com/campaign/aura/global/)
 * `EIO.dll`  ooooh IO is good I guess?
 * `Exeio.dll` executive IO? even more interesing
 * `Vender.dll` looks lame
 * ̀ VGA_Extra.dll` sure why not

I then used [CFF Explorer](https://ntcore.com/?page_id=388) to see if any of these DLL has any interesting exported functions.

 * `EIO.dll`  has things like `ReadI2C`, nice 
 * `Exeio.dll` has things like `GetFanDuty_ByReg`, interesting
 * `Vender.dll` also has a `ReadI2C` function, weird 
 * ̀ VGA_Extra.dll` has functions like `GetDeviceBus`

All of theses looked kind of interesting.

At this point I start loading some of these dll in [IDA](https://www.hex-rays.com/products/ida/support/download_freeware/) and click around with not much success.

## WinDBG

My knowledge of WinDBG was basically getting PTSD from one insane "Windows internals" training week with Alex Ionescu, fortunately I had some friends to hold my hand and learn a couple tricks.

One of these is Breakpoints! Not knowing which DLL is interesting, one solution is just to break on every function from every of these interesting DLLs:

```
bm EIO!*
bm Exeio!*
bm Vender!*
bm VGA_Extra!*
```

and then manually set fan speed.

When doing this, a bunch of functions from Vender.dll kept being called, like `GetThermalInfor`, `VGA_GetMemoryUsage`, etc. even when I wasn't changing fan speed, so I would just use `bc` to remove these breakpoints that were not related to me changing fan speed.

I was left with calls to `Vender!ReadI2C` and `Vender!WriteI2C` everytime I would change fan speed in the app, as well as other stuff, but the fan speed would actually change on a call to `Vender!WriteI2C`.

So Vender.dll should be the next point of focus.

