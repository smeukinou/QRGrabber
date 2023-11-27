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

# Usage
0 to stop monitoring, 1 to start, 2 to fetch results.



## Start monitoring
rundll


test multiscreen