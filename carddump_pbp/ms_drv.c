// табул€ци€ по версии no-no-notepad++
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "ms_drv.h"
#include "libpspexploit.h"

/****************************************************************************
****************************************************************************/
static int ms_wait_ready(void)
{
	int status;

	//Kprintf("ms_wait_ready\n");
	do {
		status = IO_MEM_STICK_STATUS;
	} while(!(status & MS_RDY));

	if (status & (MS_CRC_ERROR | MS_TIME_OUT))
	{
		#if SHOW_ERR_MSG
		Kprintf("err:ms_wait_ready %08X\n", status);
		#endif

		return -1;
	}

	return 0;
}

/****************************************************************************
****************************************************************************/
static int send_data_and_sync(int arg1, int arg2)
{
	int ret;
	IO_MEM_STICK_DATA = arg1;
	IO_MEM_STICK_DATA = arg2;

	ret = ms_wait_ready();

	return ret;
}

/****************************************************************************
****************************************************************************/
static int ms_get_reg_int(void)
{
	int ret, dummy, status;

	IO_MEM_STICK_CMD = GET_INT | 0x1;

	do {
		status = IO_MEM_STICK_STATUS;
		if (status & MS_TIME_OUT)
		{
			#if SHOW_ERR_MSG
			Kprintf("err:get_reg_int timeout\n");
			#endif
			return -1;
		}
	} while(!(status & MS_FIFO_RW));

	ret = IO_MEM_STICK_DATA;
	dummy = IO_MEM_STICK_DATA;

	do {
		status = IO_MEM_STICK_STATUS;
		if (status & MS_TIME_OUT)
		{
			#if SHOW_ERR_MSG
			Kprintf("err:get_reg_int timeout\n");
			#endif
			return -1;
		}
	} while(!(status & MS_RDY));

	return ret & 0xff;
}

/****************************************************************************
****************************************************************************/
static int read_data(void *addr, int count)
{
	int i;
	int status;

	for(i = 0; i < count; i += 4)
	{
		do {
			status = IO_MEM_STICK_STATUS;
			if (status & MS_TIME_OUT)
				return -1;
		} while(!(status & MS_FIFO_RW));

	*((volatile int*)(addr + i)) = IO_MEM_STICK_DATA;

	#if SHOW_READ_DATA
	Kprintf("%08X ", *((volatile int*)(addr + i)));
	if ((i%0x20) == 0x1c)
		Kprintf("\n");
	#endif
	}

	return 0;
}

static int write_data(void *addr, int count)
{
	int i;
	int status;

	for(i = 0; i < count; i += 4)
	{
		do {
			status = IO_MEM_STICK_STATUS;
			if (status & MS_TIME_OUT)
				return -1;
		} while(!(status & MS_FIFO_RW));

		IO_MEM_STICK_DATA = *((volatile int*)(addr + i));

		#if SHOW_READ_DATA
		Kprintf("%08X ", *((volatile int*)(addr + i)));
		if ((i%0x20) == 0x1c)
			Kprintf("\n");
		#endif
	}

	return 0;
}

/****************************************************************************
****************************************************************************/
static int ms_get_reg(void *buffer, int reg)
{
	int ret;

	//Kprintf("READ_REG\n");
	IO_MEM_STICK_CMD = READ_REG | reg;
	ret = read_data(buffer, reg);

	return ret;
}

/****************************************************************************
****************************************************************************/
static void ms_wait_unk1(void)
{
	while(!(IO_MEM_STICK_STATUS & 0x2000));
}

/****************************************************************************
****************************************************************************/
static void ms_wait_ced(void)
{
	int result;

	//Kprintf("wait CED\n");
	do {
		result = ms_get_reg_int();
	} while((result < 0) || ((result & INT_REG_CED) == 0));
}

/****************************************************************************
	setup & check status register
****************************************************************************/
static int ms_check_status(void)
{
	int ret;
	u8 sts[8];

	//set rw reg addrs of type reg (?)
	IO_MEM_STICK_CMD = SET_RW_REG_ADRS | 0x4;
	#if 1
	ret = send_data_and_sync(0x06100800, 0x00000000);
	#else
	IO_MEM_STICK_DATA = 0x06100800;
	IO_MEM_STICK_DATA = 0x00000000;
	ret = ms_wait_ready();
	#endif

	if (ret != 0)
		return -1;

	ms_get_reg(sts, 8);
	#if SHOW_STATUS_REG
	Kprintf("STATUS %02X CHK[%02X] %02X %02X PRO[%02X] %02X %02X %02X \n", sts[0], sts[1], sts[2], sts[3], sts[4], sts[5], sts[6], sts[7]);
	#endif

	#if 1
	if(sts[4] != 0x01)
	{
		#if SHOW_ERR_MSG
		Kprintf("PRE-IPL Supported MS Pro only!\n");
		#endif

		return -1;
	}
	if ((sts[2] & 0x15) != 0)
		return -1;
	#else
	// status 0 to 7
	u32 val_a = *((volatile int*)(&sts[0]));
	u32 val_b = *((volatile int*)(&sts[4]));

	// STS[02]
	if ((val_a >> 16) & 0x15 != 0)
		return -1;
	#endif

	return 0;
}

/****************************************************************************
****************************************************************************/
int pspMsInit(void)
{
	//Kprintf("_ms_init\n");

	//initialize the hardware
	*((volatile int*)(0xBC100054)) |= 0x00000100;
	*((volatile int*)(0xBC100050)) |= 0x00000400;
	*((volatile int*)(0xBC100078)) |= 0x00000010;
	*((volatile int*)(0xBC10004C)) &= 0xFFFFFEFF;

	printf("reset\n");

	//reset the controller
	IO_MEM_STICK_SYS = MSRST;
	while(IO_MEM_STICK_SYS & MSRST);

	printf("check status\n");
	ms_check_status();
	printf("wait ready status\n");
	ms_wait_ready();
	printf("wait ced\n");
	//ms_wait_ced();

	return 0;
}

/**************************************************************************** 
****************************************************************************/ 
int pspMsReadSector(int sector, void *addr)
{
	int ret;

	#if SHOW_SECTOR_ACCESS
	Kprintf("ms_read_sector(%08X,%08X)\n", sector, addr);
	#endif

	/*
	MS format
	SYS_PARAM_REG		(0x10)
	BLOCK_ADD_REG2		(0x11)
	BLOCK_ADD_REG1		(0x12)
	BLOCK_ADD_REG0		(0x13)
	CMD_PARAM_REG		(0x14)
	PAGE_ADD_REG		(0x15)
	OVER_WR_FLAG_REG	(0x16)

	MSPro format
	//size = 1;
	buf[0] = mode; // 0x10
	buf[1] = (size & 0xFF00) >> 8;
	buf[2] = (size & 0xFF);
	buf[3] = (address & 0xFF000000) >> 24;
 	[4] = (address & 0x00FF0000) >> 16;
	buf[5] = (address & 0x0000FF00) >> 8;
	buf[6] = (address & 0x000000FF);
	buf[7] = 0x00;
	*/

	//send a command with 8 bytes of params, reverse endian. (0x200001XX 0xYYYYYY00) => READ_DATA
	IO_MEM_STICK_CMD = EX_SET_CMD | 0x7;
	ret = send_data_and_sync(0x010020 | (sector>>24)<<24, ((sector>>16)&0xff) | (sector&0xff00) | ((sector<<16)&0xff0000) );
	if (ret < 0)
		return -1;

	ms_wait_unk1();

	//Kprintf("wait BREQ\n");
	do {
	ret = ms_get_reg_int();
	if (ret < 0)
		return -1;
	} while((ret & INT_REG_BREQ) == 0);

	if (ret & INT_REG_ERR)
	{
		#if SHOW_ERR_MSG
		Kprintf("err:ms wait int\n");
		#endif
		return -1;
	}

	//Kprintf("READ_PAGE_DATA\n");

	//send command to read data and get the data.
	IO_MEM_STICK_CMD = READ_PAGE_DATA | 512;
	ret = read_data(addr, 512);
	if (ret < 0)
		return -1;

	if (ms_wait_ready() < 0)
		return -1;
	ms_wait_unk1();
	ms_wait_ced();

	return 0;
} 

int pspMsReadAttrB(int attr, void *addr)
{
	int ret;

	IO_MEM_STICK_CMD = EX_SET_CMD | 0x7;
	ret = send_data_and_sync(0x010024 | (attr>>24)<<24, ((attr>>16)&0xff) | (attr&0xff00) | ((attr<<16)&0xff0000) );

	if (ret < 0)
		return -1;

	ms_wait_unk1();

	//Kprintf("wait BREQ\n");
	do {
		ret = ms_get_reg_int();
		if (ret < 0)
			return -1;
	} while((ret & INT_REG_BREQ) == 0);

	if (ret & INT_REG_ERR)
	{
		#if SHOW_ERR_MSG
		Kprintf("err:ms wait int\n");
		#endif
		return -1;
	}

	IO_MEM_STICK_CMD = READ_PAGE_DATA | 512;
	ret = read_data(addr, 512);
	if (ret < 0)
		return -1;

	if (ms_wait_ready() < 0)
		return -1;
	ms_wait_unk1();
	ms_wait_ced();

	return 0;
}

int pspMsWriteSector(int sector, void *addr)
{
	int ret;

	#if SHOW_SECTOR_ACCESS
	Kprintf("ms_read_sector(%08X,%08X)\n", sector, addr);
	#endif

	/*
	MS format
	SYS_PARAM_REG		(0x10)
	BLOCK_ADD_REG2		(0x11)
	BLOCK_ADD_REG1		(0x12)
	BLOCK_ADD_REG0		(0x13)
	CMD_PARAM_REG		(0x14)
	PAGE_ADD_REG		(0x15)
	OVER_WR_FLAG_REG	(0x16)

	MSPro format
	//size = 1;
	buf[0] = mode; // 0x10
	buf[1] = (size & 0xFF00) >> 8;
	buf[2] = (size & 0xFF);
	buf[3] = (address & 0xFF000000) >> 24;
	buf[4] = (address & 0x00FF0000) >> 16;
	buf[5] = (address & 0x0000FF00) >> 8;
	buf[6] = (address & 0x000000FF);
	buf[7] = 0x00;
	*/

	//send a command with 8 bytes of params, reverse endian
	//(0x200001XX 0xYYYYYY00) => READ_DATA
	IO_MEM_STICK_CMD = EX_SET_CMD | 0x7;
	ret = send_data_and_sync(0x010021 | (sector>>24)<<24, ((sector>>16)&0xff) | (sector&0xff00) | ((sector<<16)&0xff0000));
	if (ret < 0)
		return -1;

	ms_wait_unk1(); 

	//Kprintf("wait BREQ\n");
	do {
		ret = ms_get_reg_int();
		if (ret < 0)
			return -1;
	} while((ret & INT_REG_BREQ) == 0);

	if (ret & INT_REG_ERR)
	{
		#if SHOW_ERR_MSG
		Kprintf("err:ms wait int\n");
		#endif
		return -1;
	}

	//Kprintf("READ_PAGE_DATA\n");

	//send command to read data and get the data
	IO_MEM_STICK_CMD = 0xD000 | 512;
	ret = write_data(addr, 512);
	if (ret < 0)
		return -1;

	if (ms_wait_ready() < 0)
		return -1;
	ms_wait_unk1();
	ms_wait_ced();

	return 0;
}

/*static int send_data_and_syncw(int arg1)
{
	int ret;

	IO_MEM_STICK_DATA = arg1;
	ret = ms_wait_ready();

	return ret;
}*/

int pspMsEraseBlock()
{
	int ret;

	#if SHOW_SECTOR_ACCESS
	Kprintf("ms_read_sector(%08X,%08X)\n", sector, addr);
	#endif

	/*
	MS format
	SYS_PARAM_REG		(0x10)
	BLOCK_ADD_REG2		(0x11)
	BLOCK_ADD_REG1		(0x12)
	BLOCK_ADD_REG0		(0x13)
	CMD_PARAM_REG		(0x14)
	PAGE_ADD_REG		(0x15)
	OVER_WR_FLAG_REG	(0x16)

	MSPro format
	//size = 1;
	buf[0] = mode; // 0x10
	buf[1] = (size & 0xFF00) >> 8;
	buf[2] = (size & 0xFF);
	buf[3] = (address & 0xFF000000) >> 24;
	buf[4] = (address & 0x00FF0000) >> 16;
	buf[5] = (address & 0x0000FF00) >> 8;
	buf[6] = (address & 0x000000FF);
	buf[7] = 0x00;
	*/

	//send a command with 8 bytes of params, reverse endian. (0x200001XX 0xYYYYYY00) => READ_DATA
	IO_MEM_STICK_CMD = EX_SET_CMD | 0x7;
	ret = send_data_and_sync(0x26, 0);

	if (ret < 0)
		return -1;

	ms_wait_unk1();

	if (ms_wait_ready() < 0)
		return -1;

	return 0;
}

u32 FindProc(const char* szMod, const char* szLib, u32 nid)
{
	struct SceLibraryEntryTable *entry;
	SceModule *pMod;
	void *entTab;
	int entLen;

	//pMod = sceKernelFindModuleByName(szMod);
	pMod = pspXploitFindModuleByName(szMod);

	if (!pMod)
	{
		printf("Cannot find module %s\n", szMod);
		return 0;
	}

	int i = 0;

	entTab = pMod->ent_top;
	entLen = pMod->ent_size;
	//***printf("entTab %p - entLen %d\n", entTab, entLen);

	while (i < entLen)
	{
		int count;
		int total;
		unsigned int *vars;

		entry = (struct SceLibraryEntryTable *) (entTab + i);

		if (entry->libname && !strcmp(entry->libname, szLib))
		{
			total = entry->stubcount + entry->vstubcount;
			vars = entry->entrytable;

			if (entry->stubcount > 0)
			{
				for (count = 0; count < entry->stubcount; count++)
				{
					if (vars[count] == nid)
						return vars[count+total];
				}
			}
		}
		i += (entry->len * 4);
	}

	printf("Funtion not found.\n");
	return 0;
}

void *FindSysregFunction(u32 nid)
{
	return (void *)FindProc("sceSYSREG_Driver", "sceSysreg_driver", nid);
}

void pspMsBootStart()
{
	int (* sceSysregMsifClkDisable)(int);
	int (* sceSysregMsifBusClockDisable)(int);
	int (* sceSysregMsifResetEnable)(int);
	int (* sceSysregMsifResetDisble)(int);
	int (* sceSysregMsifClkEnable)(int);
	int (* sceSysregMsifBusClockEnable)(int);
	int (* sceSysregMsifIoEnable)(int);

	int (* sceSysconCtrlMsPower)(int);

	sceSysregMsifClkDisable = FindSysregFunction(0x8E2D835D);
	sceSysregMsifBusClockDisable = FindSysregFunction(0x826430C0);
	sceSysregMsifResetEnable = FindSysregFunction(0x00C2628E);
	sceSysregMsifResetDisble = FindSysregFunction(0xEC4BF81F);
	sceSysregMsifClkEnable = FindSysregFunction(0x31154490);
	sceSysregMsifBusClockEnable = FindSysregFunction(0x4716E71E);
	sceSysconCtrlMsPower = (void *)FindProc("sceSYSCON_Driver", "sceSyscon_driver", 0x99BBB24C);
	sceSysregMsifIoEnable = FindSysregFunction(0xD74F1D48);
	//======================================================
	sceSysregMsifClkDisable(0);
	sceSysregMsifBusClockDisable(0);
	sceSysregMsifResetEnable(0);
	sceSysregMsifResetDisble(0);
	sceSysregMsifClkEnable(0);
	sceSysregMsifBusClockEnable(0);
	sceSysregMsifIoEnable(0);

	//Kprintf("About to power.\n");

	sceSysconCtrlMsPower(1);
	//Kprintf("About to do stuff.\n");
	sceKernelDelayThread(10000);

	pspMsInit();
}
