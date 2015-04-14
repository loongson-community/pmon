/*
 *
 * BRIEF MODULE DESCRIPTION
 *	LS232 EVA BOARD Interrupt Numbering
 *
 * Copyright 2000 MontaVista Software Inc.
 * Author: MontaVista Software, Inc.
 *         	ppopov@mvista.com or source@mvista.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _LS2H_INT_H
#define _LS2H_INT_H

#define LS2H_IRQ_OFF 			64

#define LS2H_IRQ_BASE			LS2H_IRQ_OFF

/* group 0 */
#define LS2H_ACPI_IRQ			(0 + LS2H_IRQ_OFF)
#define LS2H_HPET_IRQ			(1 + LS2H_IRQ_OFF)
#define LS2H_UART0_IRQ			(2 + LS2H_IRQ_OFF)
#define LS2H_UART1_IRQ			(3 + LS2H_IRQ_OFF)
#define LS2H_UART2_IRQ			(4 + LS2H_IRQ_OFF)
#define LS2H_UART3_IRQ			(5 + LS2H_IRQ_OFF)
#define LS2H_SPI_IRQ			(6 + LS2H_IRQ_OFF)
#define LS2H_I2C0_IRQ			(7 + LS2H_IRQ_OFF)
#define LS2H_I2C1_IRQ			(8 + LS2H_IRQ_OFF)
#define LS2H_AC97_IRQ			(9 + LS2H_IRQ_OFF)
#define LS2H_DMA0_IRQ			(10 + LS2H_IRQ_OFF)
#define LS2H_DMA1_IRQ			(11 + LS2H_IRQ_OFF)
#define LS2H_DMA2_IRQ			(12 + LS2H_IRQ_OFF)
#define LS2H_LPC_IRQ			(13 + LS2H_IRQ_OFF)
#define LS2H_RTC_INT0_IRQ		(14 + LS2H_IRQ_OFF)
#define LS2H_RTC_INT1_IRQ		(15 + LS2H_IRQ_OFF)
#define LS2H_RTC_INT2_IRQ		(16 + LS2H_IRQ_OFF)
#define LS2H_TOY_INT0_IRQ		(17 + LS2H_IRQ_OFF)
#define LS2H_TOY_INT1_IRQ		(18 + LS2H_IRQ_OFF)
#define LS2H_TOY_INT2_IRQ		(19 + LS2H_IRQ_OFF)
#define LS2H_RTC_TICK_IRQ		(20 + LS2H_IRQ_OFF)
#define LS2H_TOY_TICK_IRQ		(21 + LS2H_IRQ_OFF)
#define LS2H_NAND_IRQ			(22 + LS2H_IRQ_OFF)
#define LS2H_SYS_INTN_IRQ		(23 + LS2H_IRQ_OFF)

/* group 1 */
#define LS2H_EHCI_IRQ			(32 + LS2H_IRQ_OFF)
#define LS2H_OHCI_IRQ			(33 + LS2H_IRQ_OFF)
#define LS2H_OTG_IRQ			(34 + LS2H_IRQ_OFF)
#define LS2H_GMAC0_IRQ			(35 + LS2H_IRQ_OFF)
#define LS2H_GMAC1_IRQ			(36 + LS2H_IRQ_OFF)
#define LS2H_SATA_IRQ			(37 + LS2H_IRQ_OFF)
#define LS2H_GPU_IRQ			(38 + LS2H_IRQ_OFF)
#define LS2H_DC_IRQ			(39 + LS2H_IRQ_OFF)
#define LS2H_PWM0_IRQ			(40 + LS2H_IRQ_OFF)
#define LS2H_PWM1_IRQ			(41 + LS2H_IRQ_OFF)
#define LS2H_PWM2_IRQ			(42 + LS2H_IRQ_OFF)
#define LS2H_PWM3_IRQ			(43 + LS2H_IRQ_OFF)
#define LS2H_HT0_IRQ			(44 + LS2H_IRQ_OFF)
#define LS2H_HT1_IRQ			(45 + LS2H_IRQ_OFF)
#define LS2H_HT2_IRQ			(46 + LS2H_IRQ_OFF)
#define LS2H_HT3_IRQ			(47 + LS2H_IRQ_OFF)
#define LS2H_HT4_IRQ			(48 + LS2H_IRQ_OFF)
#define LS2H_HT5_IRQ			(49 + LS2H_IRQ_OFF)
#define LS2H_HT6_IRQ			(50 + LS2H_IRQ_OFF)
#define LS2H_HT7_IRQ			(51 + LS2H_IRQ_OFF)
#define LS2H_PCIE_PORT0_INTA_IRQ	(52 + LS2H_IRQ_OFF)
#define LS2H_PCIE_PORT1_INTA_IRQ	(53 + LS2H_IRQ_OFF)
#define LS2H_PCIE_PORT2_INTA_IRQ	(54 + LS2H_IRQ_OFF)
#define LS2H_PCIE_PORT3_INTA_IRQ	(55 + LS2H_IRQ_OFF)
#define LS2H_SATA_PHY_IRQ		(56 + LS2H_IRQ_OFF)
#define LS2H_HDA_IRQ			(57 + LS2H_IRQ_OFF)

#define LS2H_PCIE_PORT0_INTA_BIT	(1 << 20)
#define LS2H_PCIE_PORT1_INTA_BIT	(1 << 21)
#define LS2H_PCIE_PORT2_INTA_BIT	(1 << 22)
#define LS2H_PCIE_PORT3_INTA_BIT	(1 << 23)

/* group 2 */
#define LS2H_GPIO0_IRQ			(64 + LS2H_IRQ_OFF)
#define LS2H_GPIO1_IRQ			(65 + LS2H_IRQ_OFF)
#define LS2H_GPIO2_IRQ			(66 + LS2H_IRQ_OFF)
#define LS2H_GPIO3_IRQ			(67 + LS2H_IRQ_OFF)
#define LS2H_GPIO4_IRQ			(68 + LS2H_IRQ_OFF)
#define LS2H_GPIO5_IRQ			(69 + LS2H_IRQ_OFF)
#define LS2H_GPIO6_IRQ			(70 + LS2H_IRQ_OFF)
#define LS2H_GPIO7_IRQ			(71 + LS2H_IRQ_OFF)
#define LS2H_GPIO8_IRQ			(72 + LS2H_IRQ_OFF)
#define LS2H_GPIO9_IRQ			(73 + LS2H_IRQ_OFF)
#define LS2H_GPIO10_IRQ			(74 + LS2H_IRQ_OFF)
#define LS2H_GPIO11_IRQ			(75 + LS2H_IRQ_OFF)
#define LS2H_GPIO12_IRQ			(76 + LS2H_IRQ_OFF)
#define LS2H_GPIO13_IRQ			(77 + LS2H_IRQ_OFF)
#define LS2H_GPIO14_IRQ			(78 + LS2H_IRQ_OFF)
#define LS2H_GPIO15_IRQ			(79 + LS2H_IRQ_OFF)

#define LS2H_GPIO_IRQ			64+LS2H_IRQ_OFF
#define LS2H_GPIO_FIRST_IRQ		64+LS2H_IRQ_OFF
#define LS2H_GPIO_IRQ_COUNT		96
#define LS2H_GPIO_LAST_IRQ		(LS2H_GPIO_FIRST_IRQ + LS2H_GPIO_IRQ_COUNT-1)

#define LS2H_LAST_IRQ		(160 + LS2H_IRQ_OFF)

#define LS2H_KBD_IRQ			1
#define LS2H_AUX_IRQ			12

struct ls2h_int_ctrl_regs
{
	volatile unsigned int int_isr;
	volatile unsigned int int_en;
	volatile unsigned int int_set;
	volatile unsigned int int_clr;		/* offset 0x10*/
	volatile unsigned int int_pol;
	volatile unsigned int int_edge;		/* offset 0 */
};

struct ls2h_cop_global_registers
{
	volatile unsigned int control;
	volatile unsigned int rd_inten;
	volatile unsigned int wr_inten;
	volatile unsigned int rd_intisr;		/* offset 0x10*/
	volatile unsigned int wr_intisr;
	unsigned int unused[11];
};

struct ls2h_cop_channel_regs
{
	volatile unsigned int rd_control;
	volatile unsigned int rd_src;
	volatile unsigned int rd_cnt;
	volatile unsigned int rd_status;		/* offset 0x10*/
	volatile unsigned int wr_control;
	volatile unsigned int wr_src;
	volatile unsigned int wr_cnt;
	volatile unsigned int wr_status;		/* offset 0x10*/
};

struct ls2h_cop_regs
{
	struct ls2h_cop_global_registers global;
	struct ls2h_cop_channel_regs chan[8][2];
};

#endif
