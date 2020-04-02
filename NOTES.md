# Notes 

## Goal
Graphic cards made by ASUS have their own [PWM fan headers](https://rog.asus.com/articles/gaming-graphics-cards/strix-gtx-10801070-what-is-asus-fancontrol/) that can be controlled via their specific app, for example to trigger extra air intake when you GPU is getting toast.

The problem is I couldn't find any other tool to control it but [ASUS provided software](https://www.asus.com/us/site/graphics-cards/gpu-tweak-ii/).
It also turns out that their app is [kind](https://www.reddit.com/r/buildapc/comments/4cbb1y/discussion_bad_experience_with_asus_gpu_tweak_ii/) of [lame](https://www.reddit.com/r/nvidia/comments/99r9vo/asus_gpu_tweak_ii_killed_my_gtx_970/) and bloated and [might inject ads](https://www.guru3d.com/news_story/asus_gpu_tweak_ii_injects_ads_into_your_games.html) and install [terrible drivers](https://syscall.eu/blog/2020/03/30/asus_gio/), so it’d be cool to have something else.

In my case, I decided to "[deshroud](https://old.reddit.com/r/sffpc/comments/exncu9/deshrouded_asus_strix_rtx_2070_super_with_guide/)" (ie: remove the provided fans & plastic covering the whole thing) my new Asus Rog Strix RTX 2070s, so I could use slightly bigger and hopefull more silent fan, that I could orient as "exhaust". 

So basically I want to be able to control these fan headers.

## GPUTweakII.exe

It is a 32b exe, that you run as Admin. You can download [here](https://dlcdnets.asus.com/pub/ASUS/Graphic%20Card/Unique_Applications/GPUTweakII-Version2171.zip) the version I used (2.1.7.1).

```
497482e1dbaccb01e6d523246120c81d  GPUTweakII.exe
```

The way you can change the "External" fans speed is to:
 * Change the speed in the app
 * Click a big ‘Apply’ button that seems to commit everything to the graphic card

![Image of GPUTweakII.exe](docs/gputweak2.jpg)

