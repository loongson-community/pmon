#include <linux/types.h>
#include <types.h>
#include "Targets/LS2K/include/bonito.h"
#include "ls2h.h"
#include "ls2h_int.h"
#include "sys/dev/pci/pcireg.h"



typedef u_int32_t pcireg_t;		/* configuration space register XXX */
typedef unsigned long device_t;

#define HT_MAP_TYPE0_CONF_ADDR	0xba000000
#define HT_MAP_TYPE1_CONF_ADDR	0xbb000000

u32 pci_read_type0_config32(u32 dev, u32 func, u32 reg)
{
	u32 addr = HT_MAP_TYPE0_CONF_ADDR;
	addr |= (dev << 11 | func << 8 | reg);
	return *(volatile u32 *)addr;
}

void pci_write_type0_config32(u32 dev, u32 func, u32 reg, u32 val)
{
	u32 addr = HT_MAP_TYPE0_CONF_ADDR;
	addr |= (dev << 11 | func << 8 | reg);
	*(volatile u32 *)addr = val;
}

u32 pci_read_type1_config32(u32 bus, u32 dev, u32 func, u32 reg)
{
	u32 addr = HT_MAP_TYPE1_CONF_ADDR;
	addr |= (bus << 16 | dev << 11 | func << 8 | reg);
	return *(volatile u32 *)addr;
}

void pci_write_type1_config32(u32 bus, u32 dev, u32 func, u32 reg, u32 val)
{
	u32 addr = HT_MAP_TYPE1_CONF_ADDR;
	addr |= (bus << 16 | dev << 11 | func << 8 | reg);
	*(volatile u32 *)addr = val;
}

u32 _pci_conf_readn(device_t tag, int reg, int width)
{
	int bus, device, function;

	if ((width != 4) || (reg & 3) || reg < 0 || reg >= 0x100) {
		printf("_pci_conf_readn: bad reg 0x%x, tag 0x%x, width 0x%x\n", reg, tag, width);
		return ~0;
	}

	_pci_break_tag (tag, &bus, &device, &function); 
	if(bus == 0 && device == 2) return -1;
    //workaround pcie header
    if(bus == 0 && (device >=9 && device <= 14) && reg == 0x8){
        return 0x06040001;
    }

	if (bus == 0) {
		/* Type 0 configuration on onboard PCI bus */
		if (device > 31 || function > 7)
		{	
			printf("_pci_conf_readn: bad device 0x%x, function 0x%x\n", device, function);
			return ~0;		/* device out of range */
		}
		return pci_read_type0_config32(device, function, reg);
	}
	else {
		/* Type 1 configuration on offboard PCI bus */
		if (bus > 255 || device > 0 || function > 7)
		{	
    		//	printf("_pci_conf_readn: bad bus 0x%x, device 0x%x, function 0x%x\n", bus, device, function);
			return ~0;		/* device out of range */
		}
		return pci_read_type1_config32(bus, device, function, reg);
	}

}


u32 _pci_conf_read(device_t tag, int reg)
{
	return _pci_conf_readn(tag, reg, 4);
}

u32 _pci_conf_read32(device_t tag,int reg)
{
	return _pci_conf_readn(tag,reg,4);
}

u8 _pci_conf_read8(device_t tag,int reg)
{
	u32 data;
	u32 offset;
	u32 new_reg;
	
	new_reg = reg & 0xfc;
	data = _pci_conf_readn(tag,new_reg,4);
	offset = reg & 3;
	data = data >> (offset * 8);
	data &= 0xff;
	
	return (u8)data;
}


u16 _pci_conf_read16(device_t tag,int reg)
{
	u32 data;
	u32 offset;
	u32 new_reg;
	
	new_reg = reg & 0xfc;
	data = _pci_conf_readn(tag,new_reg,4);
	offset = reg & 2;
	data = data >> (offset << 3);
	data &= 0xffff;
	
	return (u16)data;
}

void _pci_conf_write(device_t tag, int reg, u32 data)
{
	return _pci_conf_writen(tag, reg, data, 4);
}

void
_pci_conf_write32(device_t tag, int reg, u32 data)
{
	return _pci_conf_writen(tag,reg,data,4);
}

void
_pci_conf_write8(device_t tag, int reg, u8 data)
{
	return _pci_conf_writen(tag,reg,data,1);
}

void
_pci_conf_write16(device_t tag, int reg, u16 data)
{
	return _pci_conf_writen(tag,reg,data,2);
}

void _pci_conf_writen(device_t tag, int reg, u32 data,int width)
{
	int bus, device, function;
	u32 ori;
	u32 mask = 0x0;

	if ((reg & (width -1)) || reg < 0 || reg >= 0x100) {
		printf("_pci_conf_writen: bad reg 0x%x, tag 0x%x, width 0x%x\n", reg, tag, width);
		return;
	}

	_pci_break_tag (tag, &bus, &device, &function);

	if (bus == 0) {
    	/* Type 0 configuration on onboard PCI bus */
    		if (device > 31 || function > 7)
    		{	
			printf("_pci_conf_writen: bad device 0x%x, function 0x%x\n", device, function);
			return;		/* device out of range */
		}
	}
	else {
    	/* Type 1 configuration on offboard PCI bus */
		if (bus > 255 || device > 31 || function > 7)
		{	
			printf("_pci_conf_writen: bad bus 0x%x, device 0x%x, function 0x%x\n", bus, device, function);
			return;		/* device out of range */
		}
	}

	ori = _pci_conf_read(tag, reg & 0xfc);
	if(width == 2){
		if(reg & 2){
			mask = 0xffff;
		}
		else{
			mask = 0xffff0000;
		}
	}
	else if(width == 1){
		if ((reg & 3) == 1) {
			mask = 0xffff00ff;
		}else if ((reg & 3) == 2) {
			mask = 0xff00ffff;
		}else if ((reg & 3) == 3) {
			mask = 0x00ffffff;
		}else{
			mask = 0xffffff00;
		}
	}

	data = data << ((reg & 3) * 8);
	data = (ori & mask) | data;


	if (bus == 0) {
		return pci_write_type0_config32(device, function, reg & 0xfc, data);
	}
	else {
		return pci_write_type1_config32(bus, device, function, reg & 0xfc, data);
	}
}
