uboot-enclustra
===============

u-boot for the enclustra Zynq based module (mars-zx3). This repo was forked from u-boot-xlnx.

compile with:

export PATH=/path/to/toolchain:$PATH

ARCH=arm CROSS_COMPILE=<toolchain-prefix> make zx3_ant_config 
ARCH=arm CROSS_COMPILE=<toolchain-prefix> make
