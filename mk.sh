arm-linux-androideabi-gcc --sysroot=/opt/android-ndk/platforms/android-14/arch-arm/ -static extract-ramdisk.c -o extract-ramdisk -Wall
arm-linux-androideabi-strip extract-ramdisk

gcc -static extract-ramdisk.c -o extract-ramdisk_pc -Wall

cd rom2sd1
cp ~/android/recovery/update-binary META-INF/com/google/android/
zip -r temp.zip *
signapk temp.zip
rm temp.zip.unsigned
mv temp.zip ../rom2sd1.zip
cd ..

cd rom2sd2
cp ../extract-ramdisk bin/
cp ~/android/recovery/update-binary META-INF/com/google/android/
zip -r temp.zip *
signapk temp.zip
rm temp.zip.unsigned
mv temp.zip ../rom2sd2.zip
cd ..
