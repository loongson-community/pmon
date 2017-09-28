void i2c_init();
int ls7a_eeprom_read_cur(unsigned char *buf);
int ls7a_eeprom_read_rand(unsigned int data_addr, unsigned char *buf);
int ls7a_eeprom_read_seq(unsigned int data_addr, unsigned char *buf, int count);
int ls7a_eeprom_write_byte(unsigned int data_addr, unsigned char *buf);
int ls7a_eeprom_write_page(unsigned int data_addr, unsigned char *buf, int count);
int mac_read(unsigned int data_addr, unsigned char *buf, int count);
int mac_write(unsigned int data_addr, unsigned char *buf, int count);

