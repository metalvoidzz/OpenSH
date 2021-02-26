#!/bin/sh

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo add-apt-repository -y ppa:zoogie/sdl2-snapshots
sudo add-apt-repository -y ppa:mc3man/trusty-media
sudo add-apt-repository -y ppa:jonathonf/ffmpeg-4
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test

sudo apt update -qq
sudo apt install -y -qq iwyu
sudo apt install -y -qq libsdl2-dev
sudo apt install -y -qq ffmpeg
sudo apt install -y -qq libavcodec-dev
sudo apt install -y -qq libavformat-dev
sudo apt install -y -qq libswscale-dev
sudo apt install -y -qq libopenal-dev
sudo apt install -y -qq g++-7
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 90
