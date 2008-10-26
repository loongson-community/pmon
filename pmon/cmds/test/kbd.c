//extern int kbd_test,kbd_test_hit; //erase line
extern int kbd_initialize();
void pckbd_leds(unsigned char leds);
int kbdtest()
{
int kbd_test,kbd_test_hit; 
int i;
printf("keyboard test\n");
printf("check keyboard...");
if(kbd_initialize())
{
	printf("failed\n");
	return 1;//add
 }
else
{
	printf("ok\n");
	printf("test kbd leds\n");
	
	printf("num Lock off");
	for(i=0;i<2;i++)
	{
	printf("\b\b\b on");
	pckbd_leds(2);
	delay1(1000);
	pckbd_leds(0);
	printf("\b\b\boff");
	delay1(1000);
	}
	printf("\ncaps Lock off");
	for(i=0;i<2;i++)
	{
	printf("\b\b\b on");
	pckbd_leds(4);
	delay1(1000);
	pckbd_leds(0);
	printf("\b\b\boff");
	delay1(1000);
	}
	printf("\nscroll Lock off");
	for(i=0;i<2;i++)
	{
	printf("\b\b\b on");
	pckbd_leds(1);
	delay1(1000);
	pckbd_leds(0);
	printf("\b\b\boff");
	delay1(1000);
	}
	printf("\n");
	printf("press key test,wait 2s to quit\n");
	while(1)
	{
	kbd_test=1;
	kbd_test_hit=0;
//	delay1(10000);
	delay1(2000);
	if(!kbd_test_hit){
		kbd_test=0;
		break;
	}
	}
    return 0; //add	
}
//return 0; //erease
}
