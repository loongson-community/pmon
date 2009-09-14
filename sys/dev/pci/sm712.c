/***************************************************************************
 * Name:
 *     sm712.c
 * License:
 *     2003-2007, Copyright by BLX IC Design Co., Ltd.
 * Description:
 *     sm712 driver
 *
 ***************************************************************************/

#include "sm712.h"

/*******************************SM712 2D Engine*******************************/

#ifdef	SM712_2D_DEBUG
extern void delay(int count);
extern void tgt_display(char *, int);
void smi_2d_wait_for_nonbusy(void)
{
	unsigned long i = 0x100000;

	while(i--){
		if( (smi_seqr(0x16) & 0x18) == 0x10 ){
			break;
		}
	}

	smi_2d_busy = 0;
}

void sm712_2d_init(void)
{
	/* 
	 * crop top boundary
	 * crop disable
	 * crop inhibit outside
	 * crop left boundary
	 */
	smi_2dregw( REG_2D_CROP_LEFTTOP, (0x000 << 16) | (0 << 13) | (0 << 12) | (0x000 << 0) );
	smi_2dregw( REG_2D_CROP_RIGHTBOTTOM, (SM712_FIX_HEIGHT << 16) | (SM712_FIX_WIDTH << 0) );
	/*
	 * pitch src/dst width in pixel 
	 * window src/dst width in pixel
	 */
	smi_2dregw( REG_2D_PITCH_XY, (SM712_FIX_WIDTH << 16) | (SM712_FIX_WIDTH << 0) );
	smi_2dregw( REG_2D_SRC_WINDOW, (SM712_FIX_WIDTH << 16) | (SM712_FIX_WIDTH << 0) );
	/*
	 * normal drawing engine
	 * pattern y
	 * pattern x
	 * 16bpp
	 * xy mode
	 * stretch height Y
	 */
	smi_2dregw( REG_2D_DEDF, (1 << 30) | (0x00 << 27) | (0x00 << 24) | 
					(0x01 << 20) | (0x00 << 16) | (0x03 << 0) );
	/*
	 * byte mask
	 * bit mask
	 */
	smi_2dregw( REG_2D_BB_MASK, (0xffff << 16) | (0xffff << 0) );
	/*
	 * color compare mask
	 */
	smi_2dregw( REG_2D_COLOR_COM_MASK, (0xffffff << 0) );
	/*
	 * color compare
	 */
	smi_2dregw( REG_2D_COLOR_COM, (0xffffff << 0) );
}

void sm712_2d_copy(unsigned long dst_base, unsigned long dst_bpp, 
				unsigned long dst_x, unsigned long dst_y, 
				unsigned long dst_w, unsigned long dst_h, 
				unsigned long src_base, unsigned long src_x, 
				unsigned long src_y, unsigned long rop2)
{
	unsigned long nDirection = 0;

	smi_2d_wait_for_nonbusy();

	smi_2dregw( REG_2D_SRC_BASE, (src_base << 0) );
	smi_2dregw( REG_2D_DST_BASE, (dst_base << 0) );
	if(src_y < dst_y){
		nDirection = 1;
	}else if(src_y > dst_y){
		nDirection = 0;
	}else{
		if(src_x <= dst_x){
			nDirection = 1;
		}else{
			nDirection = 0;
		}
	}
	if( nDirection == 1 ){
		src_x += dst_w - 1;
		dst_x += dst_w - 1;
		src_y += dst_h - 1;
		dst_y += dst_h - 1;
	}

	smi_2dregw( REG_2D_SRC_XY, (0 << 31) | (src_x << 16) | (src_y << 0) );
	smi_2dregw( REG_2D_DST_XY, (0 << 31) | (dst_x << 16) | (dst_y << 0) );
	smi_2dregw( REG_2D_DIM_XY, (dst_w << 16) | (dst_h << 0) );
	/*
	 * bit31 : 2d engine start
	 * bit27 : left to right/right to left
	 * bit20:16 : command bitblt
	 * bit15 : rop2
	 * bit3:0 : rop code(just source)
	 */
	smi_2dregw( REG_2D_DE_CTRL, (1 << 31) | (nDirection << 27) | (0x0 << 16) | (1 << 15) | (0x0c << 0) );

	smi_2d_busy = 1;	
}

void video_hw_bitblt(int bpp, int sx, int sy, int dx, int dy, int w, int h)
{
	sm712_2d_copy(0, bpp, dx, dy, w, h, 0, sx, sy, 0x0c);

	return;
}

void sm712_2d_fill_rect(unsigned long dst_base, unsigned long dst_x, unsigned long dst_y,
			   	unsigned long dst_w, unsigned long dst_h, unsigned long color)
{
	smi_2d_wait_for_nonbusy();
	
	smi_2dregw( REG_2D_DST_BASE, (dst_base << 0) );
	smi_2dregw( REG_2D_COLOR_FG, (color << 0) );
	smi_2dregw( REG_2D_MONO_PATTERN_LOW, 0xffffffff );
	smi_2dregw( REG_2D_MONO_PATTERN_HIGH, 0xffffffff );
	smi_2dregw( REG_2D_PITCH_XY, (SM712_FIX_WIDTH << 16) | (SM712_FIX_WIDTH << 0) );
	smi_2dregw( REG_2D_SRC_WINDOW, (SM712_FIX_WIDTH << 16) | (SM712_FIX_WIDTH << 0) );

	/*
	 * wrap disable 
	 */
	smi_2dregw( REG_2D_DST_XY, (0 << 31) | (dst_x << 16) | (dst_y << 0) );
	smi_2dregw( REG_2D_DIM_XY, (dst_w << 16) | (dst_h << 0) );
	/*
	 * bit31 : 2d engine start
	 * bit27 : left to right
	 * bit21 : draw last pixel
	 * bit20:16 : command rectangle fill
	 * bit15 : rop2
	 * bit3:0 : rop code(just source)
	 */
	smi_2dregw( REG_2D_DE_CTRL, (1 << 31) | (0 << 27) | (0 << 21) 
					| (0x1 << 16) | (1 << 15) | (0x0c << 0) );
#if	1
/*
	if( (*((volatile unsigned char *)(0xbfd00000 | 0x3cc))) & 0x01 ){
		while( (*((volatile unsigned char *)(0xbfd00000 | 0x3da))) & 0x08 );
		while( !((*((volatile unsigned char *)(0xbfd00000 | 0x3da))) & 0x08) );
	}else{
		while( (*((volatile unsigned char *)(0xbfd00000 | 0x3ba))) & 0x08 );
		while( !((*((volatile unsigned char *)(0xbfd00000 | 0x3ba))) & 0x08) );
	}
*/
	if( (*((volatile unsigned char *)(PTR_PAD(0xbfd00000) | 0x3cc))) & 0x01 ){
		while( (*((volatile unsigned char *)(PTR_PAD(0xbfd00000) | 0x3da))) & 0x08 );
		while( !((*((volatile unsigned char *)(PTR_PAD(0xbfd00000) | 0x3da))) & 0x08) );
	}else{
		while( (*((volatile unsigned char *)(PTR_PAD(0xbfd00000) | 0x3ba))) & 0x08 );
		while( !((*((volatile unsigned char *)(PTR_PAD(0xbfd00000) | 0x3ba))) & 0x08) );
	}
#else
	delay(100);
#endif

	smi_2d_busy = 1;	
}

void video_hw_rectfill(unsigned int bpp, unsigned int dst_x, unsigned int dst_y, 
				unsigned int dim_x, unsigned int dim_y, unsigned int color)
{
	sm712_2d_fill_rect(0, dst_x, dst_y, dim_x, dim_y, color);

	return;
}

#endif	/* SM712_2D_DEBUG */

/*****************************************************************************/

static void smi_set_timing(struct par_info *hw)
{
	int i=0,j=0;
	u32 m_nScreenStride;
	
	for (j=0;j < numVGAModes;j++) {
		if (VGAMode[j].mmSizeX == hw->width &&
				VGAMode[j].mmSizeY == hw->height &&
				VGAMode[j].bpp == hw->bits_per_pixel &&
				VGAMode[j].hz == hw->hz)
		{
			smi_mmiowb(0x0,0x3c6);

			smi_seqw(0,0x1);

			smi_mmiowb(VGAMode[j].Init_MISC,0x3c2);

			for (i=0;i<SIZE_SR00_SR04;i++)  /* init SEQ register SR00 - SR04 */
			{
				smi_seqw(i,VGAMode[j].Init_SR00_SR04[i]);
			}

			for (i=0;i<SIZE_SR10_SR24;i++)  /* init SEQ register SR10 - SR24 */
			{
				smi_seqw(i+0x10,VGAMode[j].Init_SR10_SR24[i]);
			}

			for (i=0;i<SIZE_SR30_SR75;i++)  /* init SEQ register SR30 - SR75 */
			{
				if (((i+0x30) != 0x62) && ((i+0x30) != 0x6a) && ((i+0x30) != 0x6b))
					smi_seqw(i+0x30,VGAMode[j].Init_SR30_SR75[i]);
			}
			for (i=0;i<SIZE_SR80_SR93;i++)  /* init SEQ register SR80 - SR93 */
			{
				smi_seqw(i+0x80,VGAMode[j].Init_SR80_SR93[i]);
			}
			for (i=0;i<SIZE_SRA0_SRAF;i++)  /* init SEQ register SRA0 - SRAF */
			{
				smi_seqw(i+0xa0,VGAMode[j].Init_SRA0_SRAF[i]);
			}

			for (i=0;i<SIZE_GR00_GR08;i++)  /* init Graphic register GR00 - GR08 */
			{
				smi_grphw(i,VGAMode[j].Init_GR00_GR08[i]);
			}

			for (i=0;i<SIZE_AR00_AR14;i++)  /* init Attribute register AR00 - AR14 */
			{

				smi_attrw(i,VGAMode[j].Init_AR00_AR14[i]);
			}

			for (i=0;i<SIZE_CR00_CR18;i++)  /* init CRTC register CR00 - CR18 */
			{
				smi_crtcw(i,VGAMode[j].Init_CR00_CR18[i]);
			}

			for (i=0;i<SIZE_CR30_CR4D;i++)  /* init CRTC register CR30 - CR4D */
			{
				smi_crtcw(i+0x30,VGAMode[j].Init_CR30_CR4D[i]);
			}

			for (i=0;i<SIZE_CR90_CRA7;i++)  /* init CRTC register CR90 - CRA7 */
			{
				smi_crtcw(i+0x90,VGAMode[j].Init_CR90_CRA7[i]);
			}
		}
	}
	smi_mmiowb(0x67,0x3c2);

	/* set VPR registers */
	writel(hw->m_pVPR+0x0C, 0x0);
	writel(hw->m_pVPR+0x40, 0x0);
	/* set data width */
	m_nScreenStride = (hw->width * hw->bits_per_pixel) / 64;

	/* case 16: */
	writel(hw->m_pVPR+0x0, 0x00020000);

	writel(hw->m_pVPR+0x10, (u32)(((m_nScreenStride + 2) << 16) | m_nScreenStride));
}



/***************************************************************************
 * We need to wake up the LynxEM+, and make sure its in linear memory mode.
 ***************************************************************************/
static inline void smi_init_hw(void)
{
	linux_outb(0x18, 0x3c4);
	linux_outb(0x11, 0x3c5);
}

int  sm712_init(char * fbaddress,char * ioaddress)
{
	
	u32 smem_size, i;
	
	smi_init_hw();
	
	hw.m_pLFB = SMILFB = fbaddress;
	hw.m_pMMIO = SMIRegs = SMILFB + 0x00700000; /* ioaddress */
	hw.m_pDPR = hw.m_pLFB + 0x00408000;
	hw.m_pVPR = hw.m_pLFB + 0x0040c000;

	/* now we fix the  mode */
	hw.width = SM712_FIX_WIDTH;
	hw.height = SM712_FIX_HEIGHT;
	hw.bits_per_pixel = SM712_FIX_DEPTH;
	hw.hz = SM712_FIX_HZ;

	if (!SMIRegs)
	{
		printf(" unable to map memory mapped IO\n");
		return -1;
	}

	smi_seqw(0x21,0x00); 
	
	smi_seqw(0x62,0x7A);
	smi_seqw(0x6a,0x16);
	smi_seqw(0x6b,0x02);
	
	smem_size = 0x00400000;
#if 1	/* fill sm712 framebuffer as black 2008-10-06 */
	for(i = 0; i  < smem_size / 2; i += 4){
		*((volatile unsigned int*)(fbaddress + i)) = 0x00;
	}
#endif

	/* LynxEM+ memory dection */
	*(u32 *)(SMILFB + 4) = 0xAA551133;
	if (*(u32 *)(SMILFB + 4) != 0xAA551133)
	{
		smem_size = 0x00200000;
		/* Program the MCLK to 130 MHz */
		smi_seqw(0x6a,0x12);
		smi_seqw(0x6b,0x02);
		smi_seqw(0x62,0x3e);
	}

	smi_set_timing(&hw);

#ifdef	SM712_2D_DEBUG
	SMI2DBaseAddress = hw.m_pDPR;
	sm712_2d_init();
#endif

	printf("Silicon Motion, Inc. LynxEM+ Init complete.\n");

	return 0;
}

/* dummy implementation */
void video_set_lut2(int index, int rgb)
{
	return;
}

int GetXRes(void)
{
	return SM712_FIX_WIDTH;
}

int GetYRes(void)
{
	return SM712_FIX_HEIGHT;
}

int GetBytesPP(void)
{
	return (SM712_FIX_DEPTH / 8);
}

/* 
 * for 8 bit depth 
 * FIXME currently not implemented
 * */
void video_set_lut(int index, int r, int g, int b)
{

}
