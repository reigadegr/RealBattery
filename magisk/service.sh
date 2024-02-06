#!/bin/sh
wait_until_login() {
    # in case of /data encryption is disabled
    while [ "$(getprop sys.boot_completed)" != "1" ]; do
        sleep 1
    done

    # in case of the user unlocked the screen
    while [ ! -d "/sdcard/Android" ]; do
        sleep 1
    done
}
wait_until_login

MODDIR=${0%/*}
FileName="RealBattery"

chown 0:0 $MODDIR/$FileName
chmod +x $MODDIR/$FileName
killall -15 $FileName
nohup $MODDIR/$FileName >/dev/null 2>&1 &
