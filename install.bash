#!/bin/bash

DIR=$(dirname -- "$0")

pushd "$DIR" || exit

mkdir build 
pushd build || exit
cmake ..
sudo make
popd || exit
sudo rm -rf build

sudo rm -rf "/etc/systemd/system/news-watcher.service"
sudo cp "news-watcher.service" "/etc/systemd/system/news-watcher.service"

sudo rm -rf "/etc/systemd/system/testwatch-activator.service"
sudo cp "testwatch-activator.service" "/etc/systemd/system/testwatch-activator.service"

sudo rm -rf "/etc/systemd/system/testwatch.service"
sudo cp "testwatch.service" "/etc/systemd/system/testwatch.service"
