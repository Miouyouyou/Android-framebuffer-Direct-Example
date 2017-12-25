If you appreciate this project, support me on Patreon or Liberapay !

[![Patreon !](https://raw.githubusercontent.com/Miouyouyou/RockMyy/master/.img/button-patreon.png)](https://www.patreon.com/Miouyouyou) 
[![Liberapay !](https://raw.githubusercontent.com/Miouyouyou/RockMyy/master/.img/button-liberapay.png)](https://liberapay.com/Myy/donate) 
[![Tip with Altcoins](https://raw.githubusercontent.com/Miouyouyou/Shapeshift-Tip-button/9e13666e9d0ecc68982fdfdf3625cd24dd2fb789/Tip-with-altcoin.png)](https://shapeshift.io/shifty.html?destination=16zwQUkG29D49G6C7pzch18HjfJqMXFNrW&output=BTC)

About
-----

This project demonstrates how to access and write into a "window"
framebuffer in Android.

As it stands, it just lit every line of pixel of the window with
a color randomly selected in a palette of 4 colors.

The useful part of the example is inside
**[fill_pixels.c](./fill_pixels.c)**.  
**[android_native_app_glue.c](./android_native_app_glue.c) is, as the
name implies, just a way to connect Android's NativeActivity code to
to this native code.

Testing
-------

If you want to test this code, you'll need :
* An Android SDK with the right build tools (27.0.0);
* An Android NDK (used by the Android build system to get the right
  compilers for each architecture);
* An Android terminal (Phone, tablet, emulator, ...).

Then you will need to execute, from a terminal workig in this folder :
```bash
git submodule init
git submodule update
cd apk
./gradlew installDebug
```
