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

## WinDBG 101

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


## Vender.dll

Firing up IDA, this is the listing for `WriteI2C`:
```
Vender!WriteI2C
.text:10014E90                 public WriteI2C
.text:10014E90 WriteI2C        proc near               ; DATA XREF: .rdata:off_10180E18↓o
.text:10014E90
.text:10014E90 var_30          = byte ptr -30h
.text:10014E90 var_10          = dword ptr -10h
.text:10014E90 var_4           = dword ptr -4
.text:10014E90 arg_0           = dword ptr  8
.text:10014E90 arg_4           = dword ptr  0Ch
.text:10014E90
.text:10014E90                 push    ebp
.text:10014E91                 mov     ebp, esp
.text:10014E93                 sub     esp, 30h
.text:10014E96                 mov     eax, ___security_cookie
.text:10014E9B                 xor     eax, ebp
.text:10014E9D                 mov     [ebp+var_4], eax
.text:10014EA0                 push    ebx
.text:10014EA1                 mov     ebx, [ebp+arg_4]
.text:10014EA4                 push    esi
.text:10014EA5                 mov     esi, [ebp+arg_0]
.text:10014EA8                 push    edi
.text:10014EA9                 mov     ecx, 0Bh
.text:10014EAE                 lea     edi, [ebp+var_30]
.text:10014EB1                 rep movsd
.text:10014EB3                 mov     eax, [ebp+var_10]
.text:10014EB6                 push    eax
.text:10014EB7                 call    sub_10014650 ; doesn't really go anywhere, maybe error handling
.text:10014EBC                 mov     ecx, dword_101C6EE8
.text:10014EC2                 add     esp, 4
.text:10014EC5                 mov     [ebp+var_10], eax
.text:10014EC8                 test    ecx, ecx
.text:10014ECA                 jnz     short loc_10014EDF
.text:10014ECC                 pop     edi
.text:10014ECD                 pop     esi
.text:10014ECE                 xor     al, al
.text:10014ED0                 pop     ebx
.text:10014ED1                 mov     ecx, [ebp+var_4]
.text:10014ED4                 xor     ecx, ebp
.text:10014ED6                 call    @__security_check_cookie@4 ; __security_check_cookie(x)
.text:10014EDB                 mov     esp, ebp
.text:10014EDD                 pop     ebp
.text:10014EDE                 retn
.text:10014EDF ; ---------------------------------------------------------------------------
.text:10014EDF
.text:10014EDF loc_10014EDF:                           ; CODE XREF: WriteI2C+3A↑j
.text:10014EDF                 mov     edx, [ecx]
.text:10014EE1                 mov     edx, [edx+48h]
.text:10014EE4                 push    ebx
.text:10014EE5                 lea     eax, [ebp+var_30]
.text:10014EE8                 push    eax
.text:10014EE9                 call    edx
.text:10014EEB                 mov     ecx, [ebp+var_4]
.text:10014EEE                 pop     edi
.text:10014EEF                 pop     esi
.text:10014EF0                 xor     ecx, ebp
.text:10014EF2                 pop     ebx
.text:10014EF3                 call    @__security_check_cookie@4 ; __security_check_cookie(x)
.text:10014EF8                 mov     esp, ebp
.text:10014EFA                 pop     ebp
.text:10014EFB                 retn
.text:10014EFB WriteI2C        endp
```

So what is edx at 0x10014EE9? Breaking there shows edx always pointing at the same thing, for example:
`72444ee9 ffd2            call    edx {Vender+0xeb90 (7243eb90)}`
So let's look at EB90:

```
.text:1000EB90 sub_1000EB90    proc near               ; DATA XREF: .rdata:1013DB34↓o
.text:1000EB90
.text:1000EB90 var_34          = dword ptr -34h
.text:1000EB90 var_30          = dword ptr -30h
.text:1000EB90 var_2C          = byte ptr -2Ch
.text:1000EB90 var_2B          = byte ptr -2Bh
.text:1000EB90 var_28          = dword ptr -28h
.text:1000EB90 var_24          = dword ptr -24h
.text:1000EB90 var_20          = dword ptr -20h
.text:1000EB90 var_1C          = dword ptr -1Ch
.text:1000EB90 var_18          = dword ptr -18h
.text:1000EB90 var_14          = dword ptr -14h
.text:1000EB90 var_10          = byte ptr -10h
.text:1000EB90 var_C           = dword ptr -0Ch
.text:1000EB90 var_8           = dword ptr -8
.text:1000EB90 var_4           = dword ptr -4
.text:1000EB90 arg_0           = dword ptr  8
.text:1000EB90 arg_4           = dword ptr  0Ch
.text:1000EB90
.text:1000EB90                 push    ebp
.text:1000EB91                 mov     ebp, esp
.text:1000EB93                 sub     esp, 34h
.text:1000EB96                 mov     eax, [ebp+arg_0]
.text:1000EB99                 push    esi
.text:1000EB9A                 push    edi
.text:1000EB9B                 mov     edi, [eax+20h]
.text:1000EB9E                 mov     eax, [ebp+arg_4]
.text:1000EBA1                 mov     edx, [eax+24h]
.text:1000EBA4                 mov     esi, ecx
.text:1000EBA6                 mov     cl, [eax+14h]
.text:1000EBA9                 mov     [ebp+var_2B], cl
.text:1000EBAC                 mov     ecx, [eax+18h]
.text:1000EBAF                 mov     [ebp+var_28], edx
.text:1000EBB2                 mov     edx, [eax+1Ch]
.text:1000EBB5                 mov     [ebp+var_24], ecx
.text:1000EBB8                 mov     ecx, 1
.text:1000EBBD                 mov     [ebp+var_1C], edx
.text:1000EBC0                 mov     dl, [eax+4]
.text:1000EBC3                 mov     eax, [eax+28h]
.text:1000EBC6                 mov     [ebp+var_C], ecx
.text:1000EBC9                 mov     [ebp+var_8], ecx
.text:1000EBCC                 lea     ecx, [ebp+var_8]
.text:1000EBCF                 mov     [ebp+var_10], dl
.text:1000EBD2                 push    ecx
.text:1000EBD3                 lea     edx, [ebp+var_34]
.text:1000EBD6                 mov     [ebp+var_20], eax
.text:1000EBD9                 mov     eax, [esi+edi*4+3E8h]
.text:1000EBE0                 push    edx
.text:1000EBE1                 push    eax
.text:1000EBE2                 mov     [ebp+var_34], 3002Ch
.text:1000EBE9                 mov     [ebp+var_18], 0FFFFh
.text:1000EBF0                 mov     [ebp+var_14], 6
.text:1000EBF7                 mov     [ebp+var_2C], 0
.text:1000EBFB                 mov     [ebp+var_30], 0
.text:1000EC02                 mov     [ebp+var_4], 0
.text:1000EC09                 call    sub_1001B250
.text:1000EC0E                 add     esp, 0Ch
.text:1000EC11                 cmp     eax, 0FFFFFFF7h
.text:1000EC14                 jnz     short loc_1000EC35
.text:1000EC16                 mov     eax, [esi+edi*4+3E8h]
.text:1000EC1D                 lea     ecx, [ebp+var_8]
.text:1000EC20                 push    ecx
.text:1000EC21                 lea     edx, [ebp+var_34]
.text:1000EC24                 push    edx
.text:1000EC25                 push    eax
.text:1000EC26                 mov     [ebp+var_34], 20024h
.text:1000EC2D                 call    sub_1001B250
.text:1000EC32                 add     esp, 0Ch
.text:1000EC35
.text:1000EC35 loc_1000EC35:                           ; CODE XREF: sub_1000EB90+84↑j
.text:1000EC35                 test    eax, eax
.text:1000EC37                 pop     edi
.text:1000EC38                 setz    al
.text:1000EC3B                 pop     esi
.text:1000EC3C                 mov     esp, ebp
.text:1000EC3E                 pop     ebp
.text:1000EC3F                 retn    8
.text:1000EC3F sub_1000EB90    endp
```

Interesting thing is the double call to `sub_1001B250`, which start like this:

```
.text:1001B250 sub_1001B250    proc near               ; CODE XREF: sub_1000EB90+79↑p
.text:1001B250                                         ; sub_1000EB90+9D↑p
.text:1001B250
.text:1001B250 var_8           = dword ptr -8
.text:1001B250 var_4           = dword ptr -4
.text:1001B250 arg_0           = dword ptr  4
.text:1001B250 arg_4           = dword ptr  8
.text:1001B250 arg_8           = dword ptr  0Ch
.text:1001B250
.text:1001B250                 sub     esp, 8
.text:1001B253                 push    esi
.text:1001B254                 push    offset Addend   ; lpAddend
.text:1001B259                 call    ds:InterlockedIncrement
.text:1001B25F                 push    0
.text:1001B261                 push    0
.text:1001B263                 call    sub_10019BF0
.text:1001B268                 mov     esi, eax
.text:1001B26A                 add     esp, 8
.text:1001B26D                 test    esi, esi
.text:1001B26F                 jnz     loc_1001B30D
.text:1001B275                 mov     eax, dword_101C7470
.text:1001B27A                 test    eax, eax
.text:1001B27C                 jnz     short loc_1001B2AF
.text:1001B27E                 mov     eax, dword_101C7268
.text:1001B283                 test    eax, eax
.text:1001B285                 jz      short loc_1001B29A
.text:1001B287                 push    283AC65Ah
.text:1001B28C                 call    eax ; dword_101C7268
.text:1001B28E                 add     esp, 4
.text:1001B291                 test    eax, eax
.text:1001B293                 mov     dword_101C7470, eax
.text:1001B298                 jnz     short loc_1001B2AF
```

The constant here is cool and googling for 283AC65A [tells you](https://github.com/tokkenno/nvapi.net/wiki/NvAPI-Functions) this is a constant used to access `NvAPI_I2CWriteEx` in `nvapi.dll`

This function has seen some interest, from the [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB/-/issues/7) project, which makes the blinky lights go blink, and other things.

This is also where things get dark and obscure. Nvidia offers [some documentation](https://docs.nvidia.com/gameworks/content/gameworkslibrary/coresdk/nvapi/group__i2capi.html#gaf7e90150d628f012642c4b61f9781d87) around its NVAPI framework, but it only lists `NvAPI_I2CWrite` and not `NvAPI_I2CWriteEx`.

One [Rust package](https://arcnmx.github.io/nvapi-rs/nvapi_hi/sys/i2c/private/fn.NvAPI_I2CWriteEx.html) seems to provide some documentation about that function.

## NVAPI

Friends helped me understand how the argument massaging & passing to the actual call to `NvAPI_I2CWriteEx` was happening, and that the structure that contains info is actually set in `sub_1000EB90`. So what is that structure like?

The [OpenRGB](https://github.com/CalcProgrammer1/OpenRGB/blob/7b120515d802204ff5cc04df0c059d6eb1bfbc5e/dependencies/NVFC/nvapi.h#L455) project did quite a bunch of reverse engineering around that function, and we learn there that the struct is of type [NV_I2C_INFO_V3 which is actually documented by Nvidia](https://docs.nvidia.com/gameworks/content/gameworkslibrary/coresdk/nvapi/structNV__I2C__INFO__V3.html)

So back to IDA to set this as the type of our struct, being taught at the same time about the importance of aligning your fields, and how much of a nightmare can IDA be for doing what looks like simple things:
```
NV_I2C_INFO_V3  struc ; (sizeof=0x2C, mappedto_255)
; XREF: nv_writei2c_wrapper/r
00000000 version         dd ?
00000000
00000004 display_mask    dd ?
00000008 is_ddc_port     db ?
00000009 i2c_dev_address db ?   // Address of device on the I2C bus
0000000A                 db ? ; undefined
0000000B                 db ? ; undefined
0000000C i2c_reg_address dd ?   // pointer to the Address of the register we're accessing on the i2c device
00000010 reg_addr_size   dd ?
00000014 data            dd ?   // pointer to byte array
00000018 size            dd ?   // size of data
0000001C i2c_speed       dd ?   // deprecated and should always be always 0xFFFF
00000020 i2c_speed_khz   dd ?   // Enum of values
00000024 portId          db ?
00000025                 db ? ; undefined
00000026                 db ? ; undefined
00000027                 db ? ; undefined
00000028 is_port_id_set  dd ?
0000002C NV_I2C_INFO_V3  ends
```
In Ida, doubleclick on var_34, and from the stack view, apply the new type you just created.

This lets us confirm that some values statically set in the code in IDA match expectations:

 * `version` is a constant
 * `i2c_speed` is correctly set to `0xffff`
 * `i2c_speed_khz` is set to `6` which means `NVAPI_I2C_SPEED_400KHZ`

## WinDBG 102

Since we have a nice struct, let's ask WinDBG to show us the contents of it. It would be super nice to be able to trace
all calls to this function and display the values of the NV_I2C_INFO_V3 struct fields.

We know the call to the wrapper to the NvAPI_I2CWriteEx function happens at
1000EC09 which is (0x10014E90 - 0x1000EC09) = 0x6287 from Vender!WriteI2C.

This is where the WinDbg scripting part gets super annoying, but let's do this!

```
bp Vender!WriteI2C - 0x6287 "r$t0=poi(poi(@esp+4)+4);r$t1=by(poi(@esp+4)+8);r$t2=by(poi(@esp+4)+9);r$t3=poi(poi(poi(@esp+4)+c));r$t4=poi(poi(@esp+4)+10);r$t5=by(poi(poi(@esp+4)+14));r$t6=poi(poi(@esp+4)+18);r$t7=by(poi(@esp+4)+24);r$t8=poi(poi(@esp+4)+28);.echotime;.printf\"[%08x] WRITE DispMask=%08x,IsDDCPort=%02x,DevAddress=%02x, RegAddress=%02x, RegAddressSize=%08x, Data=%02x, Size=%02x, PortID=%02x, IsPortIDSet=%08x\\n\",@$tpid,@$t0,@$t1,@$t2,@$t3,@$t4,@$t5,@$t6,@$t7,@$t8;gc"
```

Let's split that. Offsets from @esp+4 are read from the struct display in IDA.
```
bp Vender!WriteI2C - 0x6287  \\ BP address
"r$t0=poi(poi(@esp+4)+4);  \\ display_mask is a dword
r$t1=by(poi(@esp+4)+8);    \\ is_ddc_port is a byte
r$t2=by(poi(@esp+4)+9);    \\ i2c_dev_address is a byte
r$t3=poi(poi(poi(@esp+4)+c));   \\ i2c_reg_address is a pointer to a dword
r$t4=poi(poi(@esp+4)+10);       \\ reg_addr_size is a dword (always 1, as we'll see after tracing)
r$t5=by(poi(poi(@esp+4)+14))    \\ data is a pointer to a byte (always only one as size is always 1)
r$t6=poi(poi(@esp+4)+18);       \\ size is a dword
r$t7=by(poi(@esp+4)+24);        \\ portId is a byte (we ignore i2c_speed & i2c_speed_khz as both as hardcoded
r$t8=poi(poi(@esp+4)+28);       \\ is_ddc_port  is a dword
.echotime;       \\ display a timestamp
.printf\"[%08x] WRITE DispMask=%08x,IsDDCPort=%02x,DevAddress=%02x, RegAddress=%02x, RegAddressSize=%08x, Data=%02x, Size=%02x, PortID=%02x, IsPortIDSet=%08x\\n\",@$tpid,@$t0,@$t1,@$t2,@$t3,@$t4,@$t5,@$t6,@$t7,@$t8;
gc"  // continue execution
```

Now fireup GPUTweakII.exe, attach to it in WinDbg, set your magic breakpoint, and start changing fan speed in the GUI, and we start seeing stuff like:

<snip>
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=07, RegAddressSize=00000001, Data=01, Size=01, PortID=01, IsPortIDSet=00000001
Debugger (not debuggee) time: Wed Apr  8 15:48:48.950 2020 (UTC + 2:00)
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=04, RegAddressSize=00000001, Data=22, Size=01, PortID=01, IsPortIDSet=00000001
Debugger (not debuggee) time: Wed Apr  8 15:48:48.955 2020 (UTC + 2:00)
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=05, RegAddressSize=00000001, Data=22, Size=01, PortID=01, IsPortIDSet=00000001
Debugger (not debuggee) time: Wed Apr  8 15:48:48.960 2020 (UTC + 2:00)
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=06, RegAddressSize=00000001, Data=22, Size=01, PortID=01, IsPortIDSet=00000001
Debugger (not debuggee) time: Wed Apr  8 15:48:48.982 2020 (UTC + 2:00)
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=04, RegAddressSize=00000001, Data=22, Size=01, PortID=01, IsPortIDSet=00000001
Debugger (not debuggee) time: Wed Apr  8 15:48:48.987 2020 (UTC + 2:00)
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=05, RegAddressSize=00000001, Data=22, Size=01, PortID=01, IsPortIDSet=00000001
Debugger (not debuggee) time: Wed Apr  8 15:48:48.989 2020 (UTC + 2:00)
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=06, RegAddressSize=00000001, Data=22, Size=01, PortID=01, IsPortIDSet=00000001
Debugger (not debuggee) time: Wed Apr  8 15:48:49.012 2020 (UTC + 2:00)
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=04, RegAddressSize=00000001, Data=22, Size=01, PortID=01, IsPortIDSet=00000001
Debugger (not debuggee) time: Wed Apr  8 15:48:49.022 2020 (UTC + 2:00)
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=05, RegAddressSize=00000001, Data=22, Size=01, PortID=01, IsPortIDSet=00000001
Debugger (not debuggee) time: Wed Apr  8 15:48:49.024 2020 (UTC + 2:00)
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=06, RegAddressSize=00000001, Data=22, Size=01, PortID=01, IsPortIDSet=00000001
Debugger (not debuggee) time: Wed Apr  8 15:48:49.032 2020 (UTC + 2:00)
<snip>
```
Nice! This shows all writes are indeed 1 byte long, which makes the WinDbg unpacking easier.

This is a LOT of calls for what should be just a single command sent to the fans controler. But it turns out the RGB LED on the GPU will also
do some show for you as you commit the new speed values.

I decided to remove `gc` at the end of the BP macros, so I could see which call actually made the fan turn.
```
WRITE DispMask=00000000,IsDDCPort=00,DevAddress=54, RegAddress=41, RegAddressSize=00000001, Data=ff, Size=01, PortID=01, IsPortIDSet=00000001
```
Got it! Setting Fans to 100% speed means I get a 0xFF there, and 60% gives me 0x99.


## Linux

It's time to see if we're super lucky.

Linux will scan for all interesting SMBus/I2C devices connected, and can expose them in a very simple to use way using the module `i2c-dev`.

Let's see if I can see a device with address 0x54.
