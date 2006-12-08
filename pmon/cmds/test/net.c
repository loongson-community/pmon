static int transmitted,received,loss;
static int pingfilter(char *s)
{
if(strstr(s,"transmitted"))
{
sscanf(s,"%d packets transmitted,%d packets received,%d%%",&transmitted,&received,&loss);
}
return 0;
}

static int myping()
{
char cmd[100];
	filterstdout(pingfilter);
	strcpy(cmd,"ping -c 3 10.0.0.3");
	do_cmd(cmd);
	filterstdout(0);
	printf("%d %d %d%%\n",transmitted,received,loss);
return 0;
}
