#define ST7_PID_S       0x53    // 'S'
#define ST7_PID_E       0x45    // 'E'
#define ST7_HID_N       0x4E    // 'N'
#define ST7_HID_O       0x4F    // 'O'
#define ST7_HID_R       0x52    // 'R'
#define ST7_HID_M       0x4D    // 'M'
#define ST7_HID_A       0x41    // 'A'
#define ST7_HID_L       0x4C    // 'L'
#define ST7_ACK_START   0xaa    // start ack
#define ST7_ACK_DATA    0xdd    // data ack
#define ST7_ACK_END     0xee    // end ack
#define ST7_ACK_ERROR   0xff    // Error ack with size

#define ST7_MAX_SIZE            0x700   // 1792Bytes for max whole chip size
#define ST7_PROGRAM_SIZE        0x700   //  1792Bytes for program
#define ST7_BLOCK_SIZE          0x40    // 32bytes for one block
#define ST7_SLAVE_ADDRESS       0xb6
#define ST7_ACK_REG             0x41

extern int attiny44_update_rom(void *src, int size);
