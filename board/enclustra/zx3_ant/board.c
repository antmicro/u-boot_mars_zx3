/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <zynqpl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

#include <nand.h>

DECLARE_GLOBAL_DATA_PTR;

/* Bootmode setting values */
#define BOOT_MODES_MASK		0x0000000F
#define QSPI_MODE		0x00000001
#define NOR_FLASH_MODE		0x00000002
#define NAND_FLASH_MODE		0x00000004
#define SD_MODE			0x00000005
#define JTAG_MODE		0x00000000

#ifdef CONFIG_FPGA
Xilinx_desc fpga;

/* It can be done differently */
Xilinx_desc fpga010 = XILINX_XC7Z010_DESC(0x10);
Xilinx_desc fpga020 = XILINX_XC7Z020_DESC(0x20);
Xilinx_desc fpga030 = XILINX_XC7Z030_DESC(0x30);
Xilinx_desc fpga045 = XILINX_XC7Z045_DESC(0x45);
Xilinx_desc fpga100 = XILINX_XC7Z100_DESC(0x100);
#endif

int board_init(void)
{
#ifdef CONFIG_FPGA
	u32 idcode;

	idcode = zynq_slcr_get_idcode();

	switch (idcode) {
	case XILINX_ZYNQ_7010:
		fpga = fpga010;
		break;
	case XILINX_ZYNQ_7020:
		fpga = fpga020;
		break;
	case XILINX_ZYNQ_7030:
		fpga = fpga030;
		break;
	case XILINX_ZYNQ_7045:
		fpga = fpga045;
		break;
	case XILINX_ZYNQ_7100:
		fpga = fpga100;
		break;
	}
#endif

	/* temporary hack to clear pending irqs before Linux as it
	 * will hang Linux
	 */
	writel(0x26d, 0xe0001014);

#ifdef CONFIG_FPGA
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif
	return 0;
}

int board_late_init(void)
{
	switch ((zynq_slcr_get_boot_mode()) & BOOT_MODES_MASK) {
	case QSPI_MODE:
		setenv("modeboot", "qspiboot");
		break;
	case NAND_FLASH_MODE:
		setenv("modeboot", "nandboot");
		break;
	case NOR_FLASH_MODE:
		setenv("modeboot", "norboot");
		break;
	case SD_MODE:
		setenv("modeboot", "sdboot");
		break;
	case JTAG_MODE:
		setenv("modeboot", "jtagboot");
		break;
	default:
		setenv("modeboot", "");
		break;
	}

	return 0;
}

/**
 * Set pin muxing for NAND access
 */
static void set_mio_mux_nand( void ){
#define NANDMUX0 0x0000001610
#define NANDMUX  0x0000000610

	writel(0xDF0D, &slcr_base->slcr_unlock);	//unlock slcr

	/* Define MuxIO for NAND */
	/* Caution: overwrite some QSPI muxing !!! */
	writel(NANDMUX0, &slcr_base->mio_pin[0]);	/* Pin 0, NAND Flash Chip Select */
	writel(NANDMUX,  &slcr_base->mio_pin[2]);	/* Pin 2, NAND Flash ALEn */
	writel(NANDMUX,  &slcr_base->mio_pin[3]);	/* Pin 3, NAND WE_B */
	writel(NANDMUX,  &slcr_base->mio_pin[4]);	/* Pin 4, NAND Flash IO Bit 2 */
	writel(NANDMUX,  &slcr_base->mio_pin[5]);	/* Pin 5, NAND Flash IO Bit 0 */
	writel(NANDMUX,  &slcr_base->mio_pin[6]);	/* Pin 6, NAND Flash IO Bit 1 */
	writel(NANDMUX,  &slcr_base->mio_pin[7]);	/* Pin 7, NAND Flash CLE_B */
	writel(NANDMUX,  &slcr_base->mio_pin[8]);	/* Pin 8, NAND Flash RD_B */
	writel(NANDMUX,  &slcr_base->mio_pin[9]);	/* Pin 9, NAND Flash IO Bit 4 */
	writel(NANDMUX,  &slcr_base->mio_pin[10]);	/* Pin 10, NAND Flash IO Bit 5 */
	writel(NANDMUX,  &slcr_base->mio_pin[11]);	/* Pin 11, NAND Flash IO Bit 6 */
	writel(NANDMUX,  &slcr_base->mio_pin[12]);	/* Pin 12, NAND Flash IO Bit 7 */
	writel(NANDMUX,  &slcr_base->mio_pin[13]);	/* Pin 13, NAND Flash IO Bit 3 */
	writel(NANDMUX,  &slcr_base->mio_pin[14]);	/* Pin 14, NAND Flash Busy */

	writel(0x767B, &slcr_base->slcr_lock);		//lock slcr
}


/**
 * Set the pin muxing for QSPI NOR access
 */
static void set_mio_mux_qspi( void ){
#define QSPIMUX 0x0000000602

	writel(0xDF0D, &slcr_base->slcr_unlock);		//unlock slcr

	/* Define MuxIO for QSPI */
	/* Caution: overwrite some NAND muxing !!! */
	writel(0x00001600, &slcr_base->mio_pin[0]);		/* Pin 0, Level 3 Mux */
	writel(0x00001602, &slcr_base->mio_pin[1]);		/* Pin 1, Quad SPI 0 Chip Select */
	writel(QSPIMUX,    &slcr_base->mio_pin[2]);		/* Pin 2, Quad SPI 0 IO Bit 0 */
	writel(QSPIMUX,    &slcr_base->mio_pin[3]);		/* Pin 3, Quad SPI 0 IO Bit 1 */
	writel(QSPIMUX,    &slcr_base->mio_pin[4]);		/* Pin 4, Quad SPI 0 IO Bit 2 */
	writel(QSPIMUX,    &slcr_base->mio_pin[5]);		/* Pin 5, Quad SPI 0 IO Bit 3 */
	writel(QSPIMUX,    &slcr_base->mio_pin[6]);		/* Pin 6, Quad SPI 0 Clock */
	writel(QSPIMUX,    &slcr_base->mio_pin[7]);		/* Pin 7, Reserved*/
	writel(QSPIMUX,    &slcr_base->mio_pin[8]);		/* Pin 8, Quad SPI Feedback Clock */
	writel(0x00001600, &slcr_base->mio_pin[9]);		/* Pin 9, Level mux -> disable */
	writel(0x00001600, &slcr_base->mio_pin[10]);	/* Pin 10, Level mux -> disable */
	writel(0x00001600, &slcr_base->mio_pin[11]);	/* Pin 11, Level mux -> disable */
	writel(0x00001600, &slcr_base->mio_pin[12]);	/* Pin 12, Level mux -> disable */
	writel(0x00001600, &slcr_base->mio_pin[13]);	/* Pin 13, Level mux -> disable */
	writel(0x00001600, &slcr_base->mio_pin[14]);	/* Pin 14, Level mux -> disable */

	writel(0x767B, &slcr_base->slcr_lock);			//lock slcr
}

static int zx3_current_storage = ZX3_NONE;

void zx3_set_storage (int store) {
	debug("zx3_set_storage: from %u to %u\n", zx3_current_storage, store);

	if (store == zx3_current_storage)
		return;

	switch (store)
	{
		case ZX3_NAND:
			set_mio_mux_nand ();
			zx3_current_storage = ZX3_NAND;
			break;
		case ZX3_QSPI:
			set_mio_mux_qspi();
			zx3_current_storage = ZX3_QSPI;
			break;
		default:
			zx3_current_storage = ZX3_NONE;
			break;
	}
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	u32 ret = 0;

#ifdef CONFIG_XILINX_AXIEMAC
	ret |= xilinx_axiemac_initialize(bis, XILINX_AXIEMAC_BASEADDR,
						XILINX_AXIDMA_BASEADDR);
#endif
#ifdef CONFIG_XILINX_EMACLITE
	u32 txpp = 0;
	u32 rxpp = 0;
# ifdef CONFIG_XILINX_EMACLITE_TX_PING_PONG
	txpp = 1;
# endif
# ifdef CONFIG_XILINX_EMACLITE_RX_PING_PONG
	rxpp = 1;
# endif
	ret |= xilinx_emaclite_initialize(bis, XILINX_EMACLITE_BASEADDR,
			txpp, rxpp);
#endif

#if defined(CONFIG_ZYNQ_GEM)
# if defined(CONFIG_ZYNQ_GEM0)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR0,
						CONFIG_ZYNQ_GEM_PHY_ADDR0, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM1)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR1,
						CONFIG_ZYNQ_GEM_PHY_ADDR1, 0);
# endif
#endif
	return ret;
}
#endif

/**
 * Set the pin muxing for QSPI NOR access
 */
#ifdef CONFIG_ZYNQ_SDHCI0
static void set_mio_mux_sdio0( void ){
//#define MUXPINVALUE 0x00000680
#define MUXPINVALUE 0x00001780

    writel(0xDF0D, &slcr_base->slcr_unlock);        //unlock slcr

    writel(MUXPINVALUE, &slcr_base->mio_pin[40]);     /* Pin 40, SD0.CLK */
    writel(MUXPINVALUE, &slcr_base->mio_pin[41]);     /* Pin 41, SD0.CMD */
    writel(MUXPINVALUE, &slcr_base->mio_pin[42]);     /* Pin 42, SD0.D0 */
    writel(MUXPINVALUE, &slcr_base->mio_pin[43]);     /* Pin 43, SD0.D1 */
    writel(MUXPINVALUE, &slcr_base->mio_pin[44]);     /* Pin 44, SD0.D2 */
    writel(MUXPINVALUE, &slcr_base->mio_pin[45]);     /* Pin 45, SD0.D3 */

    writel(0x2001, &slcr_base->reserved1[2]);         /* Set SDHCI clock to 32 MHz */

    writel(0x767B, &slcr_base->slcr_lock);          //lock slcr
}
#endif

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
    int ret = 0;

#if defined(CONFIG_ZYNQ_SDHCI)
# if defined(CONFIG_ZYNQ_SDHCI0)
    set_mio_mux_sdio0();
    ret = zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR0);
# endif
# if defined(CONFIG_ZYNQ_SDHCI1)
	ret |= zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR1);
# endif
#endif
	return ret;
}
#endif

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	zynq_ddrc_init();

	return 0;
}
