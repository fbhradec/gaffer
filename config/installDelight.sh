#! /bin/bash

set -e

version=1.1.12
directory=free/beta/2018-11-01-oIDoJTpO

if [[ $(uname -a | grep MINGW) != "" ]] ; then
	# Github CI - Windows
	package=3DelightNSI-$version-setup.exe
	mkdir -p elevate ; cd elevate
	curl -O http://code.kliu.org/misc/elevate/elevate-1.3.0-redist.7z
	7z x elevate-1.3.0-redist.7z
	cd ../
	curl -O https://3delight-downloads.s3-us-east-2.amazonaws.com/$directory/$package
	./elevate/bin.x86-64/elevate.exe $package //SP- //VERYSILENT //DIR=3delight
	rm -rf ./elevate
elif [[ $(uname -a | grep Linux) != "" ]] ; then
	package=3DelightNSI-$version-Linux-x86_64
	curl https://3delight-downloads.s3-us-east-2.amazonaws.com/$directory/$package.tar.xz > $package.tar.xz
	tar -xf $package.tar.xz
	mv ./$package/3delight/Linux-x86_64 ./3delight
else
	package=3DelightNSI-$version-Darwin-Universal
	curl https://3delight-downloads.s3-us-east-2.amazonaws.com/$directory/$package.dmg > $package.dmg
	sudo hdiutil mount $package.dmg
	sudo installer -pkg /Volumes/3Delight\ NSI\ $version/3DelightNSI-$version-Darwin-x86_64.pkg -target /
	sudo mv /Applications/3Delight ./3delight
fi
