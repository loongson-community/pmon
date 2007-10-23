static int cputest()
{
static volatile float x=1.2342374;
static volatile float y=9.153784;
static volatile float z;
printf("cp0 register read test\n");

asm("
mfc0 $0,$0
mfc0 $0,$1
mfc0 $0,$2
mfc0 $0,$3
mfc0 $0,$4
mfc0 $0,$5
mfc0 $0,$6
mfc0 $0,$7
mfc0 $0,$8
mfc0 $0,$9
mfc0 $0,$10
mfc0 $0,$11
mfc0 $0,$12
mfc0 $0,$13
mfc0 $0,$14
mfc0 $0,$15
mfc0 $0,$16
mfc0 $0,$17
mfc0 $0,$18
mfc0 $0,$19
mfc0 $0,$20
mfc0 $0,$21
mfc0 $0,$22
mfc0 $0,$23
mfc0 $0,$24
mfc0 $0,$25
mfc0 $0,$26
mfc0 $0,$27
mfc0 $0,$28
mfc0 $0,$29
mfc0 $0,$30
mfc0 $0,$31
"
:::"$2","$3"
	);

printf("cp0 register read test ok\n");
printf("cpu float calculation test\n");
z=x*y*10000;
if((int)z==112979) printf("cpu float calculation test ok\n");
else printf("cpu float calculation test error\n");
return 0;
}
