U-Boot for Enclustra Mars ZX3
=============================

This is a repository with source for the U-Boot bootloader for the Mars ZX3 module from Enclustra based on Xilinx Zynq. This repo was forked from `Xilinx' U-Boot GitHub <https://github.com/Xilinx/u-boot-xlnx>`_

Compile with:

.. code-block::

   export PATH=/path/to/toolchain:$PATH

   ARCH=arm CROSS_COMPILE=toolchain-prefix make zx3_ant_config 

   ARCH=arm CROSS_COMPILE=toolchain-prefix make
