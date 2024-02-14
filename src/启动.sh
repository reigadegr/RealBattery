killall -15 RealBattery
chmod +x RealBattery
chown 0:0 RealBattery
nohup $(pwd)/RealBattery > $(pwd)/log.log 2>&1 &
