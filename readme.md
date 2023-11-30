# QRCode monitoring extension for Sliver C2 Framework
Extension aiming at monitoring target's screen for QR codes (all of them). Particularly usefiul to snag those used for MFA enrollement.
Now you just get to force the target for a re-enrollement!

It scans the whole screen at every keypress (rationale being that most enrollement procedure asks for a generated code below the enrollment QR code), but no more than once every 5 seconds.
This is "configurable" in WinMsgHandler.h.

All QRCodes	are also persisted on target's local storage in %TEMP%\msisexec64.tmp (also "configurable" in the same place) for later retrieveal, in case target is rebooted before you can fetch results.

* Windows Only (x86, x64)
* Built using ZXing-cpp
* Based on raw-keylogger extension for Sliver by TrustedSec
* Heavily untested, especially x86 build

## A few warnings
* as it uses raw inputs mode, it's not compatible with using raw-keylogger extension at the same time in the same implant (only one "window" can get the raw inputs per process on windows)
* the x86 build is crashing as soon as a QRCode is recognized on most tests I made. Didn't manage to find why, seems related to the reflexive loading somehow that breaks exception handling. x64 build is fine.

# Usage
0 to stop monitoring, 1 to start, 2 to fetch results.

![start](https://github.com/smeukinou/QRGrabber/assets/36619449/125515c4-31c1-4ce3-8b00-5b7e77d46cd6)


![fetch](https://github.com/smeukinou/QRGrabber/assets/36619449/d0645d41-a986-4429-8f8a-447a0545696f)

