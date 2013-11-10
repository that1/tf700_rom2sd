arm-linux-androideabi-gcc --sysroot=/opt/android-ndk/platforms/android-14/arch-arm/ -static extract-ramdisk.c -o extract-ramdisk -Wall
arm-linux-androideabi-strip extract-ramdisk

gcc -static extract-ramdisk.c -o extract-ramdisk_pc -Wall
