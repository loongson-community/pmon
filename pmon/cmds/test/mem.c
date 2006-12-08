static int memtest()
{
	char cmd[20];
	printf("memory test\n");
	strcpy(cmd,"mt");
	do_cmd(cmd);
	return 0;
}
