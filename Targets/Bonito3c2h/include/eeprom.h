int eeprom_read_cur(unsigned char *buf);
int eeprom_read_rand(int data_addr, unsigned char *buf);
int eeprom_read_seq(int data_addr, unsigned char *buf, int count);
int eeprom_write_byte(int data_addr, unsigned char *buf);
int eeprom_write_page(int data_addr, unsigned char *buf, int count);
