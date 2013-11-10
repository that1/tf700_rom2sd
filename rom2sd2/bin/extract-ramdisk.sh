#!/sbin/busybox sh

# get file descriptor for output
# thanks to Chainfire - http://forum.xda-developers.com/showthread.php?t=1023150
OUTFD=$(ps | grep -v "grep" | grep -o -E "update_binary(.*)" | cut -d " " -f 3)
# TWRP names the extracted update-binary "updater"
[ $OUTFD != "" ] || OUTFD=$(ps | grep -v "grep" | grep -o -E "updater(.*)" | cut -d " " -f 3)

# same as ui_print command in updater_script, for example:
#
# ui_print "hello world!"
#
# will output "hello world!" to recovery, while
#
# ui_print
#
# outputs an empty line

ui_print() {
  if [ $OUTFD != "" ]; then
    echo "ui_print ${1} " >&$OUTFD
    echo "ui_print " >&$OUTFD
  else
    echo "${1}"
  fi;
}


ui_redirect() {
  while read -r line
  do
    echo "ui_print $line" >&$OUTFD
    echo "ui_print " >&$OUTFD
  done
}


# extract the ramdisk from the kernel in the staging partition
/tmp/extract-ramdisk /dev/block/mmcblk0p4 > /tmp/ramdisk.gz

# extract the files inside
mkdir /system/boot
mkdir /system/boot/ramdisk
cd /system/boot/ramdisk
gzip -d -c /tmp/ramdisk.gz | cpio -i || cat /tmp/ramdisk.gz | lzma -d -c | cpio -i

# kill the blob header so that the new kernel isn't flashed by the bootloader
dd if=/dev/zero of=/dev/block/mmcblk0p4 size=32 count=1

# now change the ramdisk for ROM2SD:

# mountpoints
for i in *.rc fstab.*; do
<------>sed -i 's/mmcblk0p1\([^0-9]\)/mmcblk1p3\1/g' $i
<------>sed -i 's/mmcblk0p8\([^0-9]\)/mmcblk1p2\1/g' $i
done

# load wifi modules
grep cfg80211.ko init.cardhu.rc >/dev/null || sed -i '/^on boot$/a \
    insmod /system/lib/modules/cfg80211.ko\
    insmod /system/lib/modules/bcmdhd.ko' init.cardhu.rc

ui_print "----- done! -----"
