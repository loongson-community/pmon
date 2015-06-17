#ifndef __UBIFS_PMON_H__
#define __UBIFS_PMON_H__


#include <stddef.h>
#include <sys/linux/types.h>
/*ubifs*/
typedef unsigned int    __kernel_size_t;
typedef unsigned short umode_t;
typedef unsigned char __u8;
//typedef unsigned char u8;
typedef unsigned short __u16;
//typedef unsigned short u16;
typedef unsigned int __u32;
//typedef unsigned int u32;

#if (_MIPS_SZLONG == 64)
typedef __signed__ long __s64;
typedef unsigned long __u64;
typedef unsigned long u64;
#else
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
//typedef unsigned long long u64;
#endif
/*ubifs*/
//typedef long long      loff_t;
#if (defined(CONFIG_HIGHMEM) && defined(CONFIG_64BIT_PHYS_ADDR)) \
    || defined(CONFIG_64BIT)

//typedef u64 dma_addr_t;
typedef u64 phys_addr_t;
typedef u64 phys_size_t;

#else
//typedef u32 dma_addr_t;

typedef u32 phys_addr_t;
typedef u32 phys_size_t;

#endif
typedef u64 dma64_addr_t;
                                 /*
 * Don't use phys_t.  You've been warned.
 */
#ifdef CONFIG_64BIT_PHYS_ADDR
typedef unsigned long long phys_t;
#else
typedef unsigned long phys_t;
#endif


/*ubifs*/
#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
#if defined(__GNUC__)
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;
#endif
typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;

typedef unsigned __bitwise__    gfp_t;




//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
/*include/common.h*/
#define DIV_ROUND_UP(n,d)       (((n) + (d) - 1) / (d))

#define ALIGN(x,a)              __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

#ifndef BUG
#define BUG() do { \
        printf("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__); \
        panic("BUG!"); \
} while (0)
#define BUG_ON(condition) do { if (unlikely((condition)!=0)) BUG(); } while(0)
#endif

/*compiler.h*/
#define uninitialized_var(x)            x = x

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define uswap_16(x) \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))
#define uswap_32(x) \
        ((((x) & 0xff000000) >> 24) | \
         (((x) & 0x00ff0000) >>  8) | \
         (((x) & 0x0000ff00) <<  8) | \
         (((x) & 0x000000ff) << 24))
#define _uswap_64(x, sfx) \
        ((((x) & 0xff00000000000000##sfx) >> 56) | \
         (((x) & 0x00ff000000000000##sfx) >> 40) | \
         (((x) & 0x0000ff0000000000##sfx) >> 24) | \
         (((x) & 0x000000ff00000000##sfx) >>  8) | \
         (((x) & 0x00000000ff000000##sfx) <<  8) | \
         (((x) & 0x0000000000ff0000##sfx) << 24) | \
         (((x) & 0x000000000000ff00##sfx) << 40) | \
         (((x) & 0x00000000000000ff##sfx) << 56))
#if defined(__GNUC__)
# define uswap_64(x) _uswap_64(x, ull)
#else
# define uswap_64(x) _uswap_64(x, )
#endif

#endif








