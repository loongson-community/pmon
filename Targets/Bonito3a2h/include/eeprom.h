void i2c_init();
int eeprom_read_cur(unsigned int base, unsigned char *buf);
int eeprom_read_rand(unsigned int base, int data_addr, unsigned char *buf);
int eeprom_read_seq(unsigned int base, int data_addr, unsigned char *buf, int count);
int eeprom_write_byte(unsigned int base, int data_addr, unsigned char *buf);
int eeprom_write_page(unsigned int base, int data_addr, unsigned char *buf, int count);
int mac_read(int data_addr, unsigned char *buf, int count);
int mac_write(int data_addr, unsigned char *buf, int count);

