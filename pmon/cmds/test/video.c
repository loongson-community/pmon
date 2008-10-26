extern char *vgabh;
extern char *heaptop;
char *color[]={"Blue","Green","Cyan","Red","Magenta","Brown","White","Gray","Light Blue","Light Green","Light cyan","Light red","Light Magenta","yellow","Bright White"};
void cprintf(int y, int x,int width,char color,const char *fmt,...);
void video_cls(void);
static int videotest()
{
int i,j;
video_cls();

for(i=0;i<25;i++)
cprintf(i,0,0,7,"%80c",0x20);

for(i=0;i<15;i++)
{
for(j=0;j<5;j++)
cprintf(i,j*15,0,i+1,"%s",color[i]);
}
#if 0
   pause();
#endif
delay(2000000);

for(i=0;i<25;i++)
cprintf(i,0,0,7,"%80c",0x20);

for(i=0;i<15;i++)
{
cprintf(i,0,0,(i+1)<<4,"%40s%40c",color[i],0x20);
}
#if 0
    pause();
#endif
delay(2000000);

for(i=0;i<25;i++)
cprintf(i,0,0,7,"%80c",0x20);
video_cls();
return 0;
}

