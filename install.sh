#!/bin/bash

set -e # Abort if any command fails

echo 'Checking DomoticPi dependencies'

if ls /usr/local/lib | grep wirningPi.so.2.46-ansaya >/dev/null then
	echo 'WiringPi by Ansaya is already installed'
else
	echo 'Installing WiringPi by Ansaya'
	git clone https://github.com/Ansaya/wiringPi.git wiringpi-ansaya
	./wiringpi-ansaya/build
fi

if ldconfig -p | grep libhap >/dev/null then
	echo 'HAP library by Ansaya is already installed'
else
	echo 'Installing HAP library by Ansaya'
	git clone https://github.com/Ansaya/Personal-HomeKit-HAP.git hap-ansaya
	./hap-ansaya/install.sh
fi

echo 'Dependencies ready\n\nCompiling domoticPi library...'
cd LibDomoticPi
mkdir libdomoticPi-release
cd libdomoticPi-release
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make -j4
sudo make install

echo 'Compiling domoticPi service...'
cd ../DomoticPi
mkdir domoticPi-release
cd domoticPi-release
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make -j4
sudo make install

ehco '\n\nDomoticPi service succesfully installed!\n\nCreate a new nodeConfig.json under /usr/local/etc/domoticPi directory before starting the service.'