/*
 *	PMON Graphic Library
 */

#include <termio.h>
#include <pmon.h>
#include <stdio.h>
#include <linux/types.h>
#include "window.h"
#include <sys/ioctl.h>
#include <stdio.h>
#include "mod_framebuffer.h"
#include <linux/io.h>

typedef struct 
{
	char text;
	unsigned char color;	
}scr;
scr background[VIDEO_HEIGHT][VIDEO_WIDTH];
scr foreground[VIDEO_HEIGHT][VIDEO_WIDTH];
#define cr(b,t) ((b<<4)+t)

typedef struct 
{
	int length,speed;
	int x,y16;
}drop;
drop drops[256];
int rpro=-256;
int tpro=0,wpro=0,newpage=0,oldpage=0,xo;
int currentid=0,enterid=0,overid=1,overx1,overy1,overx2,overy2,nextab[4],nextco[4],nextid[4],winit=1;
int theme=2;

extern unsigned int fgx, bgx, eorx;

struct w_color {
	int titlecolor;
	int bodycolor;
	int bodytrans;
	int btnunusedcolor;
	int btnusedcolor;
} w_color={0x0,256,0,0x80,0xf0};

struct w_color w_colortheme[2][3]={{0x0,256,0,0x80,0xf0},{0xf0,256,0,0x80,0xf0}};
extern void video_drawchars (int xx, int yy, unsigned char *s, int count);
extern void video_putchar (int xx, int yy, unsigned char c);
void w_setpage(int i)
{
	newpage=i;
}
int w_getpage()
{
	return oldpage;
}

int w_setpage_safe(int i)
{
	int newpage0=i;
	int oldpage0=oldpage;
	static char password[20]="";

	w_setpage(1);
	//if(!pwd_exist()||!pwd_is_set("admin") || (password[0] && pwd_cmp("admin",password)))
	if(!pwd_exist()||!pwd_is_set("admin"))
	{
		w_setpage(newpage0);
		return 1;
	}

	while(1)
	{
		switch(w_getpage())
		{
			case 1:
				w_window(20,8,50,8,"WARRNING");
				if(w_password(50,10,15, "Please input admin password",password,9))
				{
					if(!strcmp("sroot",password))
					{
						w_setpage(newpage0);
						return 1;
					}
					if(!pwd_cmp("admin",password))
					{
						w_setpage(2);
						break;
					}

					w_setpage(newpage0);
					return 1;
				}
				break;
			case 2:
				w_window(20,8,50,6,"password Error!");
				if(w_button(35,10,20,"Confirm"))
				{
					w_setpage(oldpage0);
					return 0;
				}
			break;

		}

		w_present();
	}
}

int w_focused()
{
	return currentid==overid;
}

int w_entered()
{
	return currentid==enterid;
}

void w_setfocusid(int n)
{
	overid=n;
}

int w_getfocusid()
{
	return overid;
}

static int inside(int x,int y)
{
	return (x<VIDEO_WIDTH && x>=0 && y<VIDEO_HEIGHT && y>=0);
}

static void w_resetdrop(drop *d)
{
	d->x=(rand()&127);
	d->length=(rand()&31)+5;
	d->y16=-(d->length-(rand()&3)+2)<<16;

	d->speed=(rand()& 300)+40;
}

static void w_initbackground(void)
{
	int i,j;
	for(i=0; i<VIDEO_HEIGHT; i++)
		for(j=0; j<VIDEO_WIDTH; j++)
		{
			background[i][j].text=rand()&127;
			background[i][j].color=1;
		}
	for(i=0;i<256;i++)
		w_resetdrop(drops+i);
}

unsigned int pal[16];
scr finalbuf[VIDEO_HEIGHT][VIDEO_WIDTH];
#if NMOD_FRAMEBUFFER
extern void video_disableoutput();
extern void video_enableoutput();
#else
extern unsigned char* vgabh;
#endif

static struct w_config {
	int cls;
	int setcursor;
} w_config={1,1};

void w_setup(int cls,int setcursor)
{
	w_config.cls=cls;
	w_config.setcursor=setcursor;
}

static void w_cls()
{
	int i;
	if(!vga_available)
	{
		memset(finalbuf,0x20,sizeof(foreground));
	}
	else
	{
#if NMOD_FRAMEBUFFER
		if(w_config.cls)
			video_cls();
		memset(finalbuf,0x01,sizeof(foreground));
#else
#ifdef LOONGSON_2G5536
		for (i = 0; i < VIDEO_HEIGHT*VIDEO_WIDTH*2; i += 8)
#else
		for (i = 0; i < VIDEO_HEIGHT*VIDEO_WIDTH; i += 8)
#endif
			*((long long *)(vgabh+i))=(long long)(0x0f000f000f000f00<<32)+0x0f000f000f000f00;
#endif
	}
}

void w_enterconsole()
{
#if  NMOD_FRAMEBUFFER
	if(vga_available)
	{
		video_enableoutput();
		video_set_color(0x8);
	}
#endif
	w_cls();
	tty_flush();
}

void w_leaveconsole()
{
	w_cls();
#if  NMOD_FRAMEBUFFER
	if(vga_available)
		video_disableoutput();
#endif
}

void win_init(void)
{
	int i,j;
	tty_flush();
	w_cls();
	if(vga_available)
	{	
#if NMOD_FRAMEBUFFER
		video_disableoutput();
		for(i=0;i<8;i++)
		{
			pal[i]=(i<<2)+(i<<18);
			pal[i+8]=(i<<13)+(i<<8)+31;
			pal[i+8]+=pal[i+8]<<16;
		}	
#else
		// disable blinking
		linux_inb(0x3da);//http://atrevida.comprenica.com/atrtut06.html
		linux_outb(0x30,0x3c0);//CAS=1,INDEX=0x10
		j=linux_inb(0x3c1);

		linux_inb(0x3da);
		linux_outb(0x30,0x3c0);
		linux_outb(j&0xf7,0x3c0);

		linux_outb(0,0x3c8);
		for(j=0;j<16;j++)
		{
			for(i=0;i<8;i++)
			{
				linux_outb(7,0x3c9);
				linux_outb(0,0x3c9);
				linux_outb(i<<3,0x3c9);
			}

			for(i=0;i<8;i++)	//饱???
			{
				linux_outb(i<<3,0x3c9);
				linux_outb(i<<3,0x3c9);
				linux_outb(7<<3,0x3c9);
			}
		}
#endif
	}
	w_initbackground();
	w_defaultcolor();

}
static void w_copy(void *src,void *dest)//对?????!!
{
	int i;
	for(i=0;i<80*25*2;i+=8)
		*((long long *)(dest+i))=*((long long *)(src+i));
}
static void w_background(void)
{
	int i,j,y,y1;

	if(theme&1)
	{
		for(i=0; i<256; i++)
		{
			y1 = drops[i].y16 >> 16;
			drops[i].y16 += drops[i].speed;
			y = drops[i].y16>>16;
			if(y!=y1 && drops[i].x<VIDEO_WIDTH)
			{
				for(y--,j=0; j<7; y--,j++)
					if(inside(drops[i].x,y))
						background[y][drops[i].x].color=((background[y][drops[i].x].color>1)?background[y][drops[i].x].color-1:1);
				y+=7+drops[i].length;
				if(inside(drops[i].x,y))
				{
					background[y][drops[i].x].text=rand()&127;
					background[y][drops[i].x].color=15;
				}
				for(y--,j=0;j<7;y--,j++)
					if(inside(drops[i].x,y))
						background[y][drops[i].x].color=((background[y][drops[i].x].color>1)?background[y][drops[i].x].color-1:1);
			}

			if(drops[i].y16>39*65536)
				w_resetdrop(drops+i);
		}
		w_copy(background,foreground);
	}
	else
	{
		memset(foreground,0,sizeof(foreground));		

		for(i=2; i< VIDEO_HEIGHT - 2; i++)
			for(j=0; j<VIDEO_WIDTH; j++)
				foreground[i][j].color=0xf0;

		for(j=0; j< VIDEO_WIDTH; j++)
			foreground[0][j].color=0xf0;

		for(i=1; i< VIDEO_WIDTH - 1; i++)
			foreground[2][i].text = foreground[VIDEO_HEIGHT - 3][i].text = (rpro==1024)?196:0x20;

		for(i=3; i< VIDEO_HEIGHT - 3; i++)
			foreground[i][0].text = foreground[i][VIDEO_WIDTH - 1].text = foreground[i][V_COMPART_LINE].text = 179;

		foreground[2][0].text=218;
		foreground[2][VIDEO_WIDTH - 1].text=191;
		foreground[VIDEO_HEIGHT - 3][0].text=192;
		foreground[VIDEO_HEIGHT - 3][VIDEO_WIDTH - 1].text=217;
		foreground[2][V_COMPART_LINE].text=194;
		foreground[VIDEO_HEIGHT - 3][V_COMPART_LINE].text=193;

		w_text(VIDEO_WIDTH/2,0,WA_CENTRE,"LOONGSON BIOS SETUP V2.1");
		w_bigtext(0, VIDEO_HEIGHT-2, VIDEO_WIDTH, 1, "F9:Discard & Quit   F10:Save & Quit   Left Arrow & Right Arrow: Switch Page    Up Arrow & Down Arrow: Select Item ");
		w_bigtext(0, VIDEO_HEIGHT-1, VIDEO_WIDTH, 1, "Enter: Confirm and Switch  Backspace:Remove Last Letter  ESC:Back to Last Menu  ~:Run Command ");
	}

}

int charin=0, cn=0;
int date_index = 0;
char *chgconcolor[]={"\e[0;37;44m","\e[0;7m"};
void w_present(void)
{
	w_present1(0,0,VIDEO_WIDTH,VIDEO_HEIGHT);
	w_background();
}

void w_present1(int x0,int y0,int w0,int h0)
{
	char c,c0,c1;

	int cnt;
	int i,j,x,y;
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	struct termio tbuf;
#endif

	if(theme&2)
		rpro=1024;
	if(vga_available)
	{
#if NMOD_FRAMEBUFFER
		video_enableoutput();
		for(j=y0,y=y0; j<y0+h0; j++,y+=VIDEO_FONT_HEIGHT)
		{
			//here reprint whole line for chinese
			for(i=x0;i<x0+w0;i++)
				if(*(short *)(&(finalbuf[j][i]))!=*(short *)(&(foreground[j][i])))
					break;
			if(i<x0+w0)
				for(i=x0,x=x0; i<x0+w0; i++,x+=VIDEO_FONT_WIDTH)
					if(*(short *)(&(finalbuf[j][i]))!=*(short *)(&(foreground[j][i])) || (unsigned char)finalbuf[j][i].text>0xa0 || (unsigned char)foreground[j][i].text>0xa0)
					{
						video_set_color(foreground[j][i].color);
						//video_putchar (x, y,foreground[j][i].text);
						video_putchar1 (x, y,foreground[j][i].text);
					}
		}
		video_disableoutput();
		w_copy(foreground,finalbuf);
#else
		w_copy(foreground,vgabh);
#endif
	}
	else		//COM output
	{

		rpro=1024;
		x=-2;y=-1;c0=-2;
		for(j=y0+1;j<y0+h0;j++)
		{
			for(i=x0;i<x0+w0;i++)
				if(*(short *)(&(finalbuf[j][i]))!=*(short *)(&(foreground[j][i])))break;
			if(i<x0+w0)
				for(i=x0;i<x0+w0;i++)
					if(*(short *)(&(finalbuf[j][i]))!=*(short *)(&(foreground[j][i])) || (unsigned char)finalbuf[j][i].text>0xa0 || (unsigned char)foreground[j][i].text>0xa0)
					{
						if(foreground[j][i].color<0xf0)c1=0;else c1=1;
						if(c1!=c0)
							printf(chgconcolor[c1]);
						c0=c1;
						if((unsigned)foreground[j][i].text>0xa0 && (unsigned)foreground[j][i+1].text>0xa0)
						{
							if((x==i-1 && y==j) || (i==0 && x==79 && y==j-1))
								printf("%c%c",foreground[j][i].text,foreground[j][i+1].text);
							else
								printf("\e[%d;%dH%c%c",j,i+1,foreground[j][i].text,foreground[j][i+1].text);
							x=i+1;
							y=j;
							i++;

						}
						else
						{
							if(foreground[j][i].text<=0)
								c=' ';
							else 
								c=foreground[j][i].text;
							if((x==i-1 && y==j) || (i==0 && x==79 && y==j-1))
								printf("%c",c);
							else
								printf("\e[%d;%dH%c",j,i+1,c);
							x=i;
							y=j;
						}
					}
		}
		w_copy(foreground,finalbuf);
	}

	winit=0;
	w_background(); //Now add 09.10.28 15:11
	if(newpage != oldpage)
		if(rpro>0 && vga_available && (theme&2)==0)
			rpro-=1;
		else
		{
			overid=1;
			if(vga_available && (theme&2)==0)
				rpro=0;
			oldpage=newpage;
			winit=1;
		}
	else
		if(rpro<1024)
			rpro+=1;
		else
			rpro=1024;
	if(rpro<=512)
		wpro=rpro;
	else wpro=512;
	if(rpro>512)
		tpro=rpro-512;
	else
		tpro=0;
	xo=40*(512-wpro);
	charin=0;

	enterid=-100;

	switch(cn)
	{
		case '\n':
			enterid=overid;
			break;
		case '\t':
			cn='[B';
		case '[A':
		case '[B':
		case '[C':
		case '[D':
			overid=nextid[(cn&255)-'A'];
			date_index = 0;
			break;
		/* The function key don't implement */
		case '[G':  //Delete Key
		case '[H':  //Home Key
		case '[F':  //End Key
			cn = 0;
			break;
		default:
			charin=cn;
	}

	ioctl (STDIN, FIONREAD, &cnt);
	cn=0;
	if(cnt)
	{
		do
		{
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
		ioctl (STDIN, SETNCNE, &tbuf);
#endif
			cn=(cn<<8)+(c=getchar());
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
		ioctl (STDIN, TCSETAW, &tbuf);
#endif
		}while(c=='[');
#if NMOD_FRAMEBUFFER
		if(w_config.setcursor)set_cursor_fb(0,0);
#else
		if(w_config.setcursor)set_cursor(0,0);
#endif		
	}
	currentid=0;
	nextid[0]=nextid[1]=nextid[2]=nextid[3]=overid;
	nextco[0]=nextco[3]=-1000;
	nextab[0]=nextab[1]=nextab[2]=nextab[3]=nextco[1]=nextco[2]=1000;
}

static char password_char=0;
static void w_text2(int x,int y,int xalign,char *ostr,int len)
{
	int i,t;

	if(y<0 || y> (VIDEO_HEIGHT -1))
		return;

	t = len * tpro>>9;
	x -=  t >> xalign ;
	for(i=0; i<t; i++)
		if(ostr[i]!=' ' && ostr[i]!='\n' && inside(x+i,y))
		{
			int oldcolor=foreground[y][x+i].color;

			foreground[y][x+i].text = password_char?password_char:ostr[i];
			foreground[y][x+i].color &= 0xf0;
			foreground[y][x+i].color |= (oldcolor&0xf)?(oldcolor&0xf):(oldcolor&0xe0)?0:7;//dark window color to text color. //((~(foreground[y][x+i].color>>4))&0xf);
		}
}
void w_text(int x,int y,int xalign,char *ostr)
{
	w_text2(x,y,xalign,ostr,strlen(ostr));
}
void w_window(int x,int y,int w,int h,char *text)
{
	int x1,y1,i,lpitch;
	scr *tscr;	

	x=( x * wpro + xo )>> 9;
	y=( y * wpro ) >> 9;
	x1=x+(w*wpro>>9);
	y1=y+(h*wpro>>9);

	if(x1<1 || x>(VIDEO_WIDTH - 1) || y1<1 || y>(VIDEO_HEIGHT - 1))
		return;

	if(x < 0)
		x = 0;
	if(y < 0)
		y = 0;
	if(x1 > VIDEO_WIDTH)
		x1 = VIDEO_WIDTH;
	if(y1 > VIDEO_HEIGHT)
		y1 = VIDEO_HEIGHT;

	tscr = &(foreground[y][x]);
	lpitch = VIDEO_WIDTH - (x1-x);
	for(i=x; i<x1; i++,tscr++)
	{
		tscr->text=' ';
		tscr->color=w_color.titlecolor;
	}
	w_text((x+x1)>>1, y, WA_CENTRE, text);

	for(y++; y<y1; y++)
	{
		tscr += lpitch;
		for(i=x; i<x1; i++,tscr++)
		{
			if(w_color.bodycolor < 256) 
				tscr->color=w_color.bodycolor;
			else if(tscr->color&0x80)
				tscr->color=0x10; //0xf0=>0x10=>0
			else 
				tscr->color=0;
			if(!w_color.bodytrans)
				tscr->text=' '; //transparent
		}
	}
}

inline int dist(int a1,int a2,int b1,int b2)//called by next fun
{
	if(b2>a1)
		if(b1<a2)
			return 0;
		else
			return b1-a2;
	else 
		return a1-b2;
}

void w_window2(int x,int y,int w,int h,char *text) // button
{
	int oldcolor=w_color.titlecolor;
	currentid++;
	if(currentid==overid)
	{
		w_color.titlecolor=w_color.btnusedcolor;
		overx1=x;
		overy1=y;
		overx2=x+w-1;
		overy2=y+h-1;
	}
	else 
	{
		if(y+h-1<overy2 && (nextab[0]>dist(overx1,overx2,x,x+w-1) || (nextab[0]==dist(overx1,overx2,x,x+w-1) && y+h-1>nextco[0])))
		{
			nextab[0]=dist(overx1,overx2,x,x+w-1);
			nextco[0]=y+h-1;
			nextid[0]=currentid;
		}
		if(y>overy1 && (nextab[1]>dist(overx1,overx2,x,x+w-1) || (nextab[1]==dist(overx1,overx2,x,x+w-1) && y<nextco[1])))
		{
			nextab[1]=dist(overx1,overx2,x,x+w-1);
			nextco[1]=y;
			nextid[1]=currentid;
		}
		if(x>overx1 && (nextab[2]>dist(overy1,overy2,y,y+h-1) || (nextab[2]==dist(overy1,overy2,y,y+h-1) && x<nextco[2])))
		{
			nextab[2]=dist(overy1,overy2,y,y+h-1);
			nextco[2]=x;
			nextid[2]=currentid;
		}
		if(x+w-1<overx2 && (nextab[3]>dist(overy1,overy2,y,y+h-1) || (nextab[3]==dist(overy1,overy2,y,y+h-1) && x+w-1>nextco[3])))
		{
			nextab[3]=dist(overy1,overy2,y,y+h-1);
			nextco[3]=x+w-1;
			nextid[3]=currentid;
		}
		w_color.titlecolor=w_color.btnunusedcolor;
	}
	w_window(x,y,w,h,text);
	w_color.titlecolor=oldcolor;
}

int w_button(int x,int y,int w,char *caption)
{
	w_window2(x,y,w,1,caption);
	return currentid==enterid;
}


int w_select(int x,int y,int w,char *caption,char **options,int *current)
{
	w_text(x,y,WA_RIGHT,caption);
	w_window2(x,y,w,1,options[*current]);
	if(currentid==enterid)			//sw: what's this? maybe here is a bug
	{
		(*current)++;
		if(options[*current] == 0)
			*current=0;		//sw: why?
	}
	return currentid==enterid;
}

int w_switch(int x,int y,char *caption,int *base,int mask)
{
	w_window2(x,y,1,1,(((*base)&mask)?"X":" "));
	if(currentid == enterid)	
		(*base)^=mask;
	w_text(x+1,y,WA_LEFT,caption);
	return currentid==enterid;
}
int w_input(int x,int y,int w,char *caption,char * text,int buflen)
{
	int l = strlen(text);
	w_text(x, y, WA_RIGHT, caption);
	w_window2(x, y, w, 1, ((w > l) ? text : text + l - w));
	if(currentid == overid)	{
		if (charin == 127 || charin == '\b') {
			if (date_index)
				text[--date_index] = '\0';
		} else if (charin >= 32 && date_index < buflen-1) {
			text[date_index++] = charin;
			text[date_index] = '\0';
		}
	}
	return currentid == enterid;
}

int w_input1(int x,int y,int w,char *caption,char * text,int buflen)
{
	int l=strlen(text);
	int Mflag = 0;
	int Dflag = 0;
	int Hflag = 0;
	int Yflag = 0;
	w_text(x,y,WA_RIGHT,caption);
	w_window2(x,y,w,1,((w>l)?text:text+l-w));
	if(!strcmp(caption,"Year"))
	{
		Yflag = 1;
	}

	if(!strcmp(caption,"Month"))
	{
		Mflag = 1;
	}
	if(!strcmp(caption,"Day"))
	{
		Dflag = 1;
	}
	if(!strcmp(caption,"Hour") )
	{
		Hflag = 1;
	}
if(!strcmp(caption,"Min"))
	{
		Hflag = 1;
	}
if(!strcmp(caption,"Sec"))
	{
		Hflag = 1;
	}

	if(currentid==overid)	
	{
		if(charin==127 || charin=='\b')
		{
			if(l)
				text[l-1]=0;
		}
		else if((charin >= 0x30) && (l<buflen-1) && (charin <= 0x39))
		{
			if( Mflag && (l < 2))
			{
			if((l == 0))
			{
				text[l]=charin;
				text[l+1]=0;
			}
			else if((l == 1))
			{
				if((text[0] <= 0x31) && (text[0] > 0x30) && (charin <= 0x32) && (charin >= 0x30))
				{
					text[l]=charin;
					text[l+1]=0;
				}
				if(text[0] == 0x30)
				{
					text[l]=charin;
					text[l+1]=0;
				}
			}
			}
			if( Dflag && (l < 2))
			{
			if((l == 0))
			{
				text[l]=charin;
				text[l+1]=0;
			}
			else if((l == 1))
			{
				if(text[0] <= 0x33)
				{
				if((text[0] == 0x33))
				{
					if(charin <= 0x31)
					{
						text[l]=charin;
						text[l+1]=0;
					}
				}
				else
				{
				text[l]=charin;
				text[l+1]=0;
				}
				}
			}
			}
			if( Hflag && (l < 2))
			{
			if((l == 0))
			{
				text[l]=charin;
				text[l+1]=0;
			}
			else if((l == 1))
			{
				if(text[0] <= 0x36)
				{
				if((text[0] == 0x36))
				{
					if(charin <= 0x30)
					{
						text[l]=charin;
						text[l+1]=0;
					}
				}
				else
				{
						text[l]=charin;
						text[l+1]=0;
				}
				}
			}
			}
		if(Yflag && l < 4)
			{
				text[l]=charin;
				text[l+1]=0;
			}
		}
	}
	return currentid==enterid;
}

int w_password(int x,int y,int w,char *caption,char * text,int buflen)
{
	int l=strlen(text);	
	w_text(x,y,WA_RIGHT,caption);
	password_char='*';
	w_window2(x,y,w,1,((w>l)?text:text+l-w));
	password_char=0;
	if(currentid==overid)	
	{
		if(charin==127 || charin== '\b') 
		{
			if(l)
				text[l-1]=0;
		}
		else if(charin>=32 && l<buflen-1)		
		{
			text[l]=charin;
			text[l+1]=0;
		}
	}
	return currentid==enterid;
}

void w_bigtext(int x,int y,int w,int h,char *text)
{
	int i,j,k;
	h+=y;
	for(k=y;k<h;k++)
	{
		for(i=0,j=w;i<w+1;i++)
			switch(text[i])
			{
				case 0:
					j=i;
					w_text(x,k,WA_LEFT,text);
					return;
				case '\n':
					j=i+1;
					goto end;
				case ' ':
					j=i+1;
			}
end:
		w_text2(x,k,WA_LEFT,text,j);
		text+=j;
	}
}
void w_setcolor(int windowcolor,int winbodycolor,int wintrans,int buttonunused,int buttonused)
{
	static struct w_color old;

	if(windowcolor!=-2){
		memcpy(&old,&w_color,sizeof(w_color));

		if(windowcolor>=0) 
			w_color.titlecolor=windowcolor;
		if(winbodycolor>=0) 
			w_color.bodycolor=winbodycolor;
		if(wintrans>=0) 
			w_color.bodytrans=wintrans;
		if(buttonunused>=0) 
			w_color.btnunusedcolor=buttonunused;
		if(buttonused>=0) 
			w_color.btnusedcolor=buttonused;
	}
	else
	{
		memcpy(&w_color,&old,sizeof(w_color));
	}
}

void w_setwindowcolor(int x,int y,int w,int h,int color,int trans)
{
	int x1,y1;
	scr *tscr;	

	for(y1=y; y1<y+h; y1++)
	{
		for(x1=x; x1<x+w; x1++)
		{
			tscr = &(foreground[y1][x1]);
			tscr->color = color;
			if(!trans)
				tscr->text=' '; //transparent
		}
	}
}

void w_defaultcolor()
{
	memcpy(&w_color,w_colortheme[theme&1],sizeof(w_color));
}

int w_keydown(int kin)
{
	if(kin==cn)
	{
		cn=0;
		return 1;
	}
	return 0;
}

int w_selectinput(int x,int y,int w,char *caption,char **options,char *current,int buflen)
{
	//current[buflen-1] is current sel index
	if(winit)
	{
		for(x=0;options[x];x++);

		if((unsigned char)(current[buflen-1])>=x)
			current[buflen-1]=0;
		for(x=0;x<buflen-1 && current[x];x++);
		if(x==0 || x==buflen-1)
			strcpy(current,options[current[buflen-1]]);
		return 0;
	}
	if(w_input(x,y,w,caption,current,buflen-1))
	{
		current[buflen-1]++;
		if(options[current[buflen-1]] == 0)
			current[buflen-1]=0;
		strcpy(current,options[current[buflen-1]]);
	}
	return currentid==enterid;
}


int w_selectinput1(int x,int y,int w,char *caption,char **options,char *current,int buflen,int * dispon)
{
//current[buflen-1] is current sel index

	if(winit)
	{
		for(x=0; options[x]; x++);
		if((unsigned char)(current[buflen-1])>=x)
			current[buflen-1]=0;
		for(x=0; x<buflen-1&&current[x]; x++);
		if(x==0 || x==buflen-1){
			strcpy(current, options[current[buflen-1]]);
		}
		return 0;
	}

	if(w_input(x,y,w,caption,current,buflen-1))
	{
		current[buflen-1]++;
		if(options[current[buflen-1]] == 0)
			current[buflen-1]=0;
		if(*dispon == 0 && !(currentid==enterid)){
			strcpy(current, options[current[buflen-1]]);
		}
	}

	return currentid==enterid;

}

//sw: single button
int w_sigw(int x,int y,int w,int h,char *test)
{
	w_window2(x, y, w, h,test);
	if(currentid == enterid)
		return 1;
	return 0;

}

//sw: muti windows. return the selected value
int w_window3(int x,int y,int w,int h,char *test[],int n)
{
	int i,retval;
	retval = -1;

	for (i = 1; i <= n; i++)
		if(w_sigw(x, y+i-1, w, h,test[i-1]) != 0)
			retval = i-1;

	return retval;
}

int w_window4(int x,int y,int w,int h,char *test[],int n)
{
	int i;
	int retval = 0;

	for (i = 1; i <= n; i++)
	{
		w_window2(x, y+i-1, w, h,test[i-1]);
		if(currentid == overid)
			retval = (retval & 0xff) | ((i << 8) & ~0xff);
		if(currentid == enterid)
			retval = (retval & 0xff00)| i ;
	}

	return retval;
}

int w_biginput(int x,int y,int w,int h,char *caption,char * text,int buflen)
{
	int l=strlen(text);	
	w_window2(x,y,w,h,caption);
	if(currentid==overid)	
	{
		text[l]=219;
		text[l+1]=0;
		w_bigtext(x,y+1,w,h-1,text);
		text[l]=0;

		if(charin==127) 
		{
			if(l)
				text[l-1]=0;
		}
		else if(charin>=32 && l<buflen-2)
		{
			text[l]=charin;
			text[l+1]=0;
		}
	}
	else 
		w_bigtext(x,y+1,w,h-1,text);
	return currentid==enterid;
}
int w_hitanykey()
{
	int cnt;
	while(cnt==0)
		ioctl (STDIN, FIONREAD, &cnt);
	return getchar();
}
