cp -f  ./RealBattery /data/adb/modules/pandora_kernel
cp -f  ./RealBattery /data/adb/modules/RealBattery
#killall -15 RealBattery
/system/bin/sh /data/adb/modules/RealBattery/service.sh
