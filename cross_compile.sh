#!/bin/bash
export NDKDIR=/home/benoitgirard/mozilla/tools/android/ndk/android-ndk-r4c
export TOOLDIR=$NDKDIR/build/prebuilt/linux-x86/arm-eabi-4.4.0
export SYSROOT=$NDKDIR/build/platforms/android-5/arch-arm
export DEVICEDIR=/data/local
export CC=$TOOLDIR/bin/arm-eabi-gcc
export CXX=$TOOLDIR/bin/arm-eabi-g++
export CMAKE_C_FLAGS="$CFLAGS -I$SYSROOT/usr/include -D__linux__ -mandroid -msoft-float -fno-short-enums -march=armv5te -mthumb-interwork -fno-strict-aliasing -march=armv7-a -mthumb -Wa, -march=armv7-a -Wa, -mthumb -mfpu=neon"
export CMAKE_CXX_FLAGS="$CFLAGS -I$SYSROOT/usr/include -D__linux__ -mandroid -msoft-float -fno-short-enums -march=armv5te -mthumb-interwork -fno-strict-aliasing -march=armv7-a -mthumb -Wa, -march=armv7-a -Wa, -mthumb -mfpu=neon"
export CFLAGS="$CFLAGS -I$SYSROOT/usr/include -D__linux__ -mandroid -msoft-float -fno-short-enums -march=armv5te -mthumb-interwork -fno-strict-aliasing -march=armv7-a -mthumb -Wa, -march=armv7-a -Wa, -mthumb -mfpu=neon"
export CXXFLAGS="$CFLAGS -I$SYSROOT/usr/include -D__linux__ -mandroid -msoft-float -fno-short-enums -march=armv5te -mthumb-interwork -fno-strict-aliasing -march=armv7-a -mthumb -Wa, -march=armv7-a -Wa, -mthumb -mfpu=neon"
export C_FLAGS="asdfasdfasdf $CFLAGS -I$SYSROOT/usr/include -D__linux__ -mandroid -msoft-float -fno-short-enums -march=armv5te -mthumb-interwork -fno-strict-aliasing -march=armv7-a -mthumb -Wa, -march=armv7-a -Wa, -mthumb -mfpu=neon"
export CXX_FLAGS="sadfasd $CFLAGS -I$SYSROOT/usr/include -D__linux__ -mandroid -msoft-float -fno-short-enums -march=armv5te -mthumb-interwork -fno-strict-aliasing -march=armv7-a -mthumb -Wa, -march=armv7-a -Wa, -mthumb -mfpu=neon"
export ASFLAGS="$ASFLAGS $CFLAGS"
export LDFLAGS="$LDFLAGS -mandroid -L$SYSROOT/usr/lib -Wl,-rpath-link=$SYSROOT/usr/lib --sysroot=$SYSROOT -Wl,--allow-shlib-undefined -Wl,-rpath-link,/usr/local/lib"
