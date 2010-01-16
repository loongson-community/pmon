#define ATTINY_PID_S       0x53    // 'S'
#define ATTINY_PID_E       0x45    // 'E'
#define ATTINY_HID_N       0x4E    // 'N'
#define ATTINY_HID_O       0x4F    // 'O'
#define ATTINY_HID_R       0x52    // 'R'
#define ATTINY_HID_M       0x4D    // 'M'
#define ATTINY_HID_A       0x41    // 'A'
#define ATTINY_HID_L       0x4C    // 'L'
#define ATTINY_ACK_START   0xaa    // start ack
#define ATTINY_ACK_DATA    0xdd    // data ack
#define ATTINY_ACK_END     0xee    // end ack
#define ATTINY_ACK_ERROR   0xff    // Error ack with size

#define ATTINY_MAX_SIZE            0x6c0   // 1728 Bytes for max whole chip size
#define ATTINY_PROGRAM_SIZE        0x6c0   // 1728 Bytes for program
#define ATTINY_BLOCK_SIZE          0x40    // 64 Bytes for one block
#define ATTINY_SLAVE_ADDRESS       0xb6
#define ATTINY_ACK_REG             0x41

extern int attiny44_update_rom(void *src, int size);
