#!/bin/bash
arm-rockchip-linux-gnueabihf-gcc  -o latency latency.c -I/home/chao/work/build/gui/lib_rk3308/alsalib/include  -L/home/chao/work/build/gui/lib_rk3308/alsalib/lib -lasound -lm
