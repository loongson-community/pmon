static int cputest()
{
	static volatile float x=1.2342374;
	static volatile float y=9.153784;
	static volatile float z;
	//int i=0;
	static  int tmp[32];
	int status = 0;
	memset(tmp, 0, sizeof(tmp));
	printf("cp0 register read test\n");
	//T0_CU0|ST0_KX|ST0_SX|ST0_FR
#define ST0_CU0         0x10000000
#define ST0_KX          0x00000080
#define ST0_SX          0x00000040
#define ST0_FR          0x04000000

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
			mfc0 $2,$12
			li   $3,(1<<29)
			or   $2,$3
			mtc0 $2,$12
			"
			:::"$2","$3"
			);

	printf("cp0 register read test ok\n");
	printf("cpu float calculation test\n");

#if 1
	__asm__ __volatile__(
			"ctc1 $8,$31\n":"=m"(status)
			);
#endif

#if 0
	asm(
			"swc1  $f0, %0\n"
			"swc1  $f1, %1\n"
			"swc1  $f2, %2\n"
			"swc1  $f3, %3\n"
			"swc1  $f4, %4\n"
			"swc1  $f5, %5\n"
			:"=m"(tmp[0]), 
			"=m"(tmp[1]), 
			"=m"(tmp[2]), 
			"=m"(tmp[3]), 
			"=m"(tmp[4]), 
			"=m"(tmp[5]) 
	   );
	for (;i<6;i++ )
		printf("float point aaaaaaaaaaaaaaa== %x\n",tmp[i]);
#endif

#if 0
	z=x*y;
	if(z==11.297943) printf("cpu float calculation test ok\n");
	else printf("cpu float calculation test error\n");
#endif

#if 1
	z=x*y;
	if( (0<11.297943-z) && (11.297943-z < 0.0000009) )
		printf("cpu float calculation test OK !!\n");
	else 
	{
		printf("cpu float calculation test Error !!\n");
		return 1; //add
	}
#endif
	return 0;
}



