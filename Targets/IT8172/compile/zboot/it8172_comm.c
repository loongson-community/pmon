void
led_display (int code)
{
	    int i;

	    *((volatile unsigned char *)0xb80000f0) = code&0x0ff;
}

void
flash_gpio (int code)
{
	int i = 50000;
        *((volatile unsigned short *)0xb4013802) = 0x0550;
	*((volatile unsigned char *)0xb4013800) = (code & 0xf) << 2;
}
