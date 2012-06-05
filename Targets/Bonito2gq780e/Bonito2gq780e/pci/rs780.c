#include "rs780.h"

/*****************************************
* Compliant with CIM_33's ATINB_MiscClockCtrl
*****************************************/
void static rs780_config_misc_clk(device_t nb_dev)
{
	u32 reg;
	u16 word;
	/* u8 byte; */
	//struct bus pbus; /* fake bus for dev0 fun1 */

	reg = pci_read_config32(nb_dev, 0x4c);
	reg |= 1 << 0;
	pci_write_config32(nb_dev, 0x4c, reg);

	word = pci_read_config16(_pci_make_tag(0, 0, 1), 0xf8);
	word &= 0xf00;
	pci_write_config16(_pci_make_tag(0, 0, 1), 0xf8, word);

	word = pci_read_config16(_pci_make_tag(0, 0, 1), 0xe8);
	word &= ~((1 << 12) | (1 << 13) | (1 << 14));
	word |= 1 << 13;
	pci_write_config16(_pci_make_tag(0, 0, 1), 0xe8, word);

	reg =  pci_read_config32(_pci_make_tag(0, 0, 1), 0x94);
	reg &= ~((1 << 16) | (1 << 24) | (1 << 28));
	pci_write_config32(_pci_make_tag(0, 0, 1), 0x94, reg);

	reg = pci_read_config32(_pci_make_tag(0, 0, 1), 0x8c);
	reg &= ~((1 << 13) | (1 << 14) | (1 << 24) | (1 << 25));
	reg |= 1 << 13;
	pci_write_config32(_pci_make_tag(0, 0, 1), 0x8c, reg);

	reg = pci_read_config32(_pci_make_tag(0, 0, 1), 0xcc);
	reg |= 1 << 24;
	pci_write_config32(_pci_make_tag(0, 0, 1), 0xcc, reg);

	reg = nbmc_read_index(nb_dev, 0x7a);
	reg &= ~0x3f;
	reg |= 1 << 2;
	reg &= ~(1 << 6);
	set_htiu_enable_bits(nb_dev, 0x05, 1 << 11, 1 << 11);
	nbmc_write_index(nb_dev, 0x7a, reg);
	/* Powering Down efuse and strap block clocks after boot-up. GFX Mode. */
	reg = pci_read_config32(_pci_make_tag(0, 0, 1), 0xcc);
	reg &= ~(1 << 23);
	reg |= 1 << 24;
	pci_write_config32(_pci_make_tag(0, 0, 1), 0xcc, reg);

	/* BTDC: Programming NB CLK table. */
	{
		u8 temp8;
		//temp8 = pci_cf8_conf1.read8(&pbus, 0, 1, 0xe0);
		temp8 = pci_read_config8(_pci_make_tag(0, 0, 1), 0xe0);
		temp8 |= 0x01;
		//pci_cf8_conf1.write8(&pbus, 0, 1, 0xe0, temp8);
		pci_write_config8(_pci_make_tag(0, 0, 1), 0xe0, temp8);
	}
#if 0
	/* Powerdown reference clock to graphics core PLL in northbridge only mode */
	reg = pci_read_config32(_pci_make_tag(0, 0, 1), 0x8c);
	reg |= 1 << 21;
	pci_write_config32(_pci_make_tag(0, 0, 1), 0x8c, reg);

	/* Powering Down efuse and strap block clocks after boot-up. NB Only Mode. */
	reg = pci_read_config32(_pci_make_tag(0, 0, 1), 0xcc);
	reg |= (1 << 23) | (1 << 24);
	pci_write_config32(_pci_make_tag(0, 0, 1), 0xcc, reg);

	/* Powerdown clock to memory controller in northbridge only mode */
	byte = pci_read_config8(_pci_make_tag(0, 0, 1), 0xe4);
	byte |= 1 << 0;
	pci_write_config8(_pci_make_tag(0,0, 1), 0xe4, reg);

	/* CLKCFG:0xE8 Bit[17] = 0x1 	 Powerdown clock to IOC GFX block in no external graphics mode */
	/* TODO: */
#endif

	reg = pci_read_config32(nb_dev, 0x4c);
	reg &= ~(1 << 0);
	pci_write_config32(nb_dev, 0x4c, reg);

	set_htiu_enable_bits(nb_dev, 0x05, 7 << 8, 7 << 8);
}

