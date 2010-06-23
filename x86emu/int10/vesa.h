/*
 * Modifications to support Loongson Arch:
 * Copyright (c) 2008  Lemote.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * VESA framebuffer support. Author: Lj.Peng <penglj@lemote.com>
 */


/* For vesa mode control */
#define GRAPHIC_MODE_100	0x100	/* 640x480 256*/
#define GRAPHIC_MODE_101	0x101	/* 640x480 256*/
#define GRAPHIC_MODE_102	0x102	/* 800x600 16 */
#define GRAPHIC_MODE_103	0x103	/* 800x600 256*/
#define GRAPHIC_MODE_104	0x104	/* 1024x768 16*/
#define GRAPHIC_MODE_105	0x105	/* 1024x768 256*/
#define GRAPHIC_MODE_106	0x106	/* 1280x1024 16*/
#define GRAPHIC_MODE_107	0x107	/* 1280x1024 256*/
#define GRAPHIC_MODE_10d	0x10d	/* 320x200 32K(1:5:5:5)*/
#define GRAPHIC_MODE_10e	0x10e	/* 320x200 64K(5:6:5)*/
#define GRAPHIC_MODE_10f	0x10f	/* 320x200 16.8M(8:8:8)*/
#define GRAPHIC_MODE_110	0x110	/* 640x480 32K*/
#define GRAPHIC_MODE_111	0x111	/* 640x480 64K*/
#define GRAPHIC_MODE_112	0x112	/* 640x480 16.8M*/
#define GRAPHIC_MODE_113	0x113	/* 800x600 32K*/
#define GRAPHIC_MODE_114	0x114	/* 800x600 64K*/
#define GRAPHIC_MODE_115	0x115	/* 800x600 16.8M*/
#define GRAPHIC_MODE_116	0x116	/* 1024x768 32K*/
#define GRAPHIC_MODE_117	0x117	/* 1024x768 64K*/
#define GRAPHIC_MODE_118	0x118	/* 1024x768 16.8M*/
#define GRAPHIC_MODE_119	0x119	/* 1280x1024 32K*/
#define GRAPHIC_MODE_11a	0x11a	/* 1280x1024 64K*/
#define GRAPHIC_MODE_11b	0x11b	/* 1280x1024 16.8M*/
#define USE_LINEAR_FRAMEBUFFER 0x4000
struct vesamode {
	int mode;
	int width;
	int height;
	int bpp;
};
