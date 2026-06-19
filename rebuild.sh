#!/bin/sh
sudo rm -rf /usr/include/mgc/regex
make
sudo make install
sudo ldconfig
make clean