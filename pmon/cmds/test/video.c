extern char *vgabh;
extern char *heaptop;
char *color[]={"Blue","Green","Cyan","Red","Magenta","Brown","White","Gray","Light Blue","Light Green","Light cyan","Light red","Light Magenta","yellow","Bright White"};
void cprintf(int y, int x,int width,char color,const char *fmt,...);
static int videotest()
{
	int i,j;
	for(i=0;i<25;i++)
		cprintf(i,0,0,7,"%80c",0x20);

	for(i=0;i<15;i++)
	{
		for(j=0;j<5;j++)
			cprintf(i,j*15,0,i+1,"%s",color[i]);
	}
	lpause();
	for(i=0;i<25;i++)
		cprintf(i,0,0,7,"%80c",0x20);

	for(i=0;i<15;i++)
	{
		cprintf(i,0,0,(i+1)<<4,"%40s%40c",color[i],0x20);
	}
	lpause();
	for(i=0;i<25;i++)
		cprintf(i,0,0,7,"%80c",0x20);
	return 0;
}

