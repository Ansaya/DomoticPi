#!/bin/bash

set -e # Abort if any command fails

if [ $1 -eq "clean" ]; then
	sudo rm -r wiringpi-ansaya
	sudo rm -r hap-ansaya
	sudo rm -r LibDomoticPi/release
	sudo rm -r DomoticPi/release
	exit 0
fi

echo 'Checking DomoticPi dependencies'
git submodule update --init --recursive

if ldconfig -p | grep libmosquitto >/dev/null; then
	echo '  - Mosquitto MQTT library ... installed'
else
	echo '  - Mosquitto MQTT library ... installation needed'
	sudo apt-get install -y libmosquitto-dev
fi

if ls /usr/local/lib | grep wiringPi.so.2.47-ansaya >/dev/null; then
	echo '  - WiringPi by Ansaya ... installed'
else
	echo '  - WiringPi by Ansaya ... installation needed'
	cd wiringpi
	chmod +x build
	./build
	cd ..
fi

if ls /usr/local/lib | grep libhap >/dev/null; then
	echo '  - HAP library by Ansaya ... installed'
else
	echo '  - HAP library by Ansaya ... installation needed'
	cd libhap
	chmod +x install.sh
	./install.sh
	cd ..
fi

echo 'Dependencies ready'
echo ''
echo 'Compiling domoticPi library...'
mkdir -p LibDomoticPi/release
cd LibDomoticPi/release
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make -j4
sudo make install
sudo ldconfig		#Register new library

echo 'Compiling domoticPi service...'
cd ../../DomoticPi
mkdir -p release
cd release
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make -j4
sudo make install
sudo systemctl daemon-reload

echo ''
echo 'DomoticPi service succesfully installed!'
echo ''
echo 'Create a new nodeConfig.json under /usr/local/etc/domoticPi directory before starting the service.'
