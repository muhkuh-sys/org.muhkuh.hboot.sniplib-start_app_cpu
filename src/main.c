#include "netx_io_areas.h"
#include "rdy_run.h"
#include "sha384.h"


#define NETX90_APP_CPU_BOOTBLOCK_MAGIC        0xf3beaf00U
#define NETX90_APP_CPU_BOOTBLOCK_SIGNATURE    0x41505041U


/* Define a structure for all CM4 vectors. */
typedef struct NETX90_APP_CPU_BOOTBLOCK_STRUCT
{
	unsigned long  ulMagic;               /* The magic value NETX90_APP_CPU_BOOTBLOCK_MAGIC. */
	unsigned long aulReserved01[3];       /* Reserved, should be 0. */
	unsigned long  ulImageSizeInDwords;   /* The size of the image starting after this header, counted in DWORDS. */
	unsigned long  ulReserved05;          /* Reserved, should be 0. */
	unsigned long  ulSignature;           /* The signature NETX90_APP_CPU_BOOTBLOCK_SIGNATURE. */
	unsigned long  ulParameter;           /* A pointer to the parameter block. */
	unsigned long aulHash[7];             /* A SHA384 hash over the CM4 header and the complete image. */
	unsigned long  ulChecksum;            /* A simple checksum over this header. */
} NETX90_APP_CPU_BOOTBLOCK_T;

typedef union NETX90_APP_CPU_BOOTBLOCK_UNION
{
	NETX90_APP_CPU_BOOTBLOCK_T s;
	unsigned long aul[sizeof(NETX90_APP_CPU_BOOTBLOCK_T)/sizeof(unsigned long)];
} NETX90_APP_CPU_BOOTBLOCK_UN_T;


#define NETX90_APP_CPU_IFLASH_MAX_APPLICATION_SIZE_DWORDS ((0x80000 - 0x200) / sizeof(unsigned long))

typedef struct NETX90_APP_CPU_IFLASH_IMAGE_STRUCT
{
	unsigned long aulCM4Vectors[112];
	NETX90_APP_CPU_BOOTBLOCK_UN_T tBootHeader;
	unsigned long aulApplication[NETX90_APP_CPU_IFLASH_MAX_APPLICATION_SIZE_DWORDS];
} NETX90_APP_CPU_IFLASH_IMAGE_T;

extern const NETX90_APP_CPU_IFLASH_IMAGE_T tAppCpuIntflashImage;


void start(void);
void __attribute__ ((section (".init_code"))) start(void)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptHashArea);
	int iResult;
	unsigned long ulValue;
	unsigned long ulChecksum;
	unsigned int uiCnt;
	unsigned long ulApplicationSizeInDwords;
	const unsigned long *pulCnt;
	const unsigned long *pulEnd;
	BLINKI_HANDLE_T tBlinkiHandle;
	unsigned long aulHash[7];


	/* Be pessimistic... */
	iResult = -1;

	/* Can the APP CPU be enabled? */
	ulValue  = ptAsicCtrlArea->asClock_enable[0].ulMask;
	ulValue &= HOSTMSK(clock_enable0_mask_arm_app);
	if( ulValue!=0 )
	{
		/* Is the APP CPU already running? */
		ulValue  = ptAsicCtrlArea->asClock_enable[0].ulEnable;
		ulValue &= HOSTMSK(clock_enable0_arm_app);
		if( ulValue!=0 )
		{
			/* The APP CPU is already running. */
			iResult = 0;
		}
		else
		{
			/* Is the magic value present? */
			if( tAppCpuIntflashImage.tBootHeader.s.ulMagic==NETX90_APP_CPU_BOOTBLOCK_MAGIC )
			{
				/* Is the signature correct? */
				if( tAppCpuIntflashImage.tBootHeader.s.ulSignature==NETX90_APP_CPU_BOOTBLOCK_SIGNATURE )
				{
					/* Calculate the checksum. */
					ulChecksum = 0;
					for(uiCnt=0; uiCnt<(sizeof(NETX90_APP_CPU_BOOTBLOCK_UN_T)/sizeof(unsigned long)); ++uiCnt)
					{
						ulChecksum += tAppCpuIntflashImage.tBootHeader.aul[uiCnt];
					}
					if( ulChecksum==0 )
					{
						/* Test the application size. */
						ulApplicationSizeInDwords = tAppCpuIntflashImage.tBootHeader.s.ulImageSizeInDwords;
						if( ulApplicationSizeInDwords<=NETX90_APP_CPU_IFLASH_MAX_APPLICATION_SIZE_DWORDS )
						{
							/* Build a hash over the CM4 header and the application. */
							sha384_initialize();

							pulCnt = tAppCpuIntflashImage.aulCM4Vectors;
							pulEnd = pulCnt + (sizeof(tAppCpuIntflashImage.aulCM4Vectors)/sizeof(unsigned long));
							while( pulCnt<pulEnd )
							{
								sha384_update_ul(*(pulCnt++));
							}

							pulCnt = tAppCpuIntflashImage.aulApplication;
							pulEnd = pulCnt + ulApplicationSizeInDwords;
							while( pulCnt<pulEnd )
							{
								sha384_update_ul(*(pulCnt++));
							}

							ulValue  = ulApplicationSizeInDwords;
							ulValue += sizeof(tAppCpuIntflashImage.aulCM4Vectors) / sizeof(unsigned long);
							sha384_finalize(aulHash, sizeof(aulHash)/sizeof(unsigned long), ulValue);

							/* Compare the hash. */
							ulValue = 0;
							for(uiCnt=0; uiCnt<sizeof(aulHash)/sizeof(unsigned long); ++uiCnt)
							{
								ulValue = aulHash[uiCnt] ^ tAppCpuIntflashImage.tBootHeader.s.aulHash[uiCnt];
								if( ulValue!=0 )
								{
									break;
								}
							}

							if( ulValue==0 )
							{
								/* All OK! Start the APP CPU. */
								ulValue  = ptAsicCtrlArea->asClock_enable[0].ulEnable;
								ulValue |= HOSTMSK(clock_enable0_arm_app);
								ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
								ptAsicCtrlArea->asClock_enable[0].ulEnable = ulValue;

								iResult = 0;
							}
						}
					}
				}
			}
		}
	}

	if( iResult!=0 )
	{
		/* Stop and blink fast yellow. */
		rdy_run_blinki_init(&tBlinkiHandle, 0x00000005, 0x00000015);
		while(1)
		{
			rdy_run_blinki(&tBlinkiHandle);
		};
	}
}
