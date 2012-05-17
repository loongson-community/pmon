/*************************
 * Small modified by cxk.
*************************/
#define BONITO_HTIO_BASE_VA 0x90000efdfc000000

#ifdef  USE_SB_I2C
#ifndef MULTI_I2C_BUS
LEAF(i2cread)
/***************
use register:
v0, v1
a0, a1
input: a0,a1
   a0: device ID
   a1: register offset
***************/
	    ori     a0, a0, 1

        /* set device address */
        dli     v0, BONITO_HTIO_BASE_VA + SMBUS_HOST_ADDRESS
        sb      a0, 0(v0);

        /* store register offset */
        dli     v0, BONITO_HTIO_BASE_VA + SMBUS_HOST_COMMAND
        sb      a1, 0(v0);

        /* read byte data protocol */
        dli     v0, 0x08
        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_CONTROL
        sb      v0, 0(v1);

        /* make sure SMB host ready to start, important!--zfx */
        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
        lbu     v0, 0(v1)
        andi    v0, v0, 0x1f
        beqz    v0, 1f
        nop
        sb      v0, 0(v1)
        lbu     v0, 0(v1)   #flush the write
1:
        /* start */
        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_CONTROL
        lbu     v0, 0(v1)
        ori     v0, v0, 0x40
        sb      v0, 0(v1);

        /* wait */
        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS

1:
#if 1
        /* delay */
        dli     v0, 0x100
2:
        bnez    v0, 2b
        daddiu  v0, -1
#endif
        lbu     v0, 0(v1)
        andi    v0, SMBUS_HOST_STATUS_BUSY
        bnez    v0, 1b  #IDEL ?
        nop

        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
        lbu     v0, 0(v1)
        andi    v0, v0, 0x1f
        beqz    v0, 1f
        nop
        sb      v0, 0(v1)   #reset
        lbu     v0, 0(v1)   #flush the write
1:

        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_DATA0
        lbu     v0, 0(v1)

        jr      ra
        nop
END(i2cread)
#else
/****************************************************/
/*		support multi i2c bus mode			        */
/****************************************************/
LEAF(i2cread)
/***************
use register:
v0, v1
a0, a1, a2
input: a0,a1,a2
   a0: device ID
   a1: register offset
   a2: chip ID (0,1,2,3)
***************/
	    ori     a0, a0, 1
		/* calculate address according chip ID */
        dsll    a2, a2, 44

        /* set device address */
        dli     v0, BONITO_HTIO_BASE_VA + SMBUS_HOST_ADDRESS
		daddu   v0, v0, a2
        sb      a0, 0(v0);

        /* store register offset */
        dli     v0, BONITO_HTIO_BASE_VA + SMBUS_HOST_COMMAND
		daddu   v0, v0, a2
        sb      a1, 0(v0);

        /* read byte data protocol */
        dli     v0, 0x08
        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_CONTROL
		daddu   v1, v1, a2
        sb      v0, 0(v1);

        /* make sure SMB host ready to start, important!--zfx */
        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
		daddu   v1, v1, a2
        lbu     v0, 0(v1)
        andi    v0, v0, 0x1f
        beqz    v0, 1f
        nop
        sb      v0, 0(v1)
        lbu     v0, 0(v1)   #flush the write
1:
        /* start */
        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_CONTROL
		daddu   v1, v1, a2
        lbu     v0, 0(v1)
        ori     v0, v0, 0x40
        sb      v0, 0(v1);

        /* wait */
        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
		daddu   v1, v1, a2

1:
#if 1
        /* delay */
        dli     v0, 0x100
2:
        bnez    v0, 2b
        daddiu  v0, -1
#endif
        lbu     v0, 0(v1)
        andi    v0, SMBUS_HOST_STATUS_BUSY
        bnez    v0, 1b  #IDEL ?
        nop

        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
		daddu   v1, v1, a2
        lbu     v0, 0(v1)
        andi    v0, v0, 0x1f
        beqz    v0, 1f
        nop
        sb      v0, 0(v1)   #reset
        lbu     v0, 0(v1)   #flush the write
1:

        dli     v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_DATA0
		daddu   v1, v1, a2
        lbu     v0, 0(v1)

        jr      ra
        nop
END(i2cread)
#endif

#else

#ifdef  USE_GPIO_I2C
#include "i2c_firewall.S"
#endif

#endif

