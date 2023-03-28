/*	$NetBSD: elf.h,v 1.2 1995/03/28 18:19:14 jtc Exp $	*/

/*
 * Copyright (c) 1994 Ted Lemon
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef __MACHINE_ELF_H__
#define __MACHINE_ELF_H__

#include <stdint.h>

/* ELF executable header... */
struct ehdr {
  char elf_magic [4];		/* Elf magic number... */
  uint32_t magic [3];		/* Magic number... */
  uint16_t type;		/* Object file type... */
  uint16_t machine;		/* Machine ID... */
  uint32_t version;		/* File format version... */
  uint32_t entry;		/* Entry point... */
  uint32_t phoff;		/* Program header table offset... */
  uint32_t shoff;		/* Section header table offset... */
  uint32_t flags;		/* Processor-specific flags... */
  uint16_t ehsize;		/* Elf header size in bytes... */
  uint16_t phsize;		/* Program header size... */
  uint16_t phcount;		/* Program header count... */
  uint16_t shsize;		/* Section header size... */
  uint16_t shcount;		/* Section header count... */
  uint16_t shstrndx;		/* Section header string table index... */
};

/* Program header... */
struct phdr {
  uint32_t type;		/* Segment type... */
  uint32_t offset;		/* File offset... */
  uint32_t vaddr;		/* Virtual address... */
  uint32_t paddr;		/* Physical address... */
  uint32_t filesz;		/* Size of segment in file... */
  uint32_t memsz;		/* Size of segment in memory... */
  uint32_t flags;		/* Segment flags... */
  uint32_t align;		/* Alighment, file and memory... */
};

/* Section header... */
struct shdr {
  uint32_t name;		/* Offset into string table of section name */
  uint32_t type;		/* Type of section... */
  uint32_t flags;		/* Section flags... */
  uint32_t addr;		/* Section virtual address at execution... */
  uint32_t offset;		/* Section file offset... */
  uint32_t size;		/* Section size... */
  uint32_t link;		/* Link to another section... */
  uint32_t info;		/* Additional section info... */
  uint32_t align;		/* Section alignment... */
  uint32_t esize;		/* Entry size if section holds table... */
};

/* Symbol table entry... */
struct sym {
  uint32_t name;		/* Index into strtab of symbol name. */
  uint32_t value;		/* Section offset, virt addr or common align. */
  uint32_t size;		/* Size of object referenced. */
  unsigned type    : 4;		/* Symbol type (e.g., function, data)... */
  unsigned binding : 4;		/* Symbol binding (e.g., global, local)... */
  unsigned char other;		/* Unused. */
  uint16_t shndx;		/* Section containing symbol. */
};

/* Values for program header type field */

#define PT_NULL         0               /* Program header table entry unused */
#define PT_LOAD         1               /* Loadable program segment */
#define PT_DYNAMIC      2               /* Dynamic linking information */
#define PT_INTERP       3               /* Program interpreter */
#define PT_NOTE         4               /* Auxiliary information */
#define PT_SHLIB        5               /* Reserved, unspecified semantics */
#define PT_PHDR         6               /* Entry for header table itself */
#define PT_LOPROC       0x70000000      /* Processor-specific */
#define PT_HIPROC       0x7FFFFFFF      /* Processor-specific */
#define PT_MIPS_REGINFO	PT_LOPROC	/* Mips reginfo section... */

/* Program segment permissions, in program header flags field */

#define PF_X            (1 << 0)        /* Segment is executable */
#define PF_W            (1 << 1)        /* Segment is writable */
#define PF_R            (1 << 2)        /* Segment is readable */
#define PF_MASKPROC     0xF0000000      /* Processor-specific reserved bits */

/* Reserved section indices... */
#define SHN_UNDEF	0
#define SHN_ABS		0xfff1
#define SHN_COMMON	0xfff2
#define SHN_MIPS_ACOMMON 0xfff0

/* Symbol bindings... */
#define STB_LOCAL	0
#define STB_GLOBAL	1
#define STB_WEAK	2

/* Symbol types... */
#define STT_NOTYPE	0
#define	STT_OBJECT	1
#define STT_FUNC	2
#define	STT_SECTION	3
#define STT_FILE	4

#define ELF_HDR_SIZE	(sizeof (struct ehdr))
#ifdef _KERNEL
int     pmax_elf_makecmds __P((struct proc *, struct exec_package *));
#endif /* _KERNEL */
#endif /* __MACHINE_ELF_H__ */
