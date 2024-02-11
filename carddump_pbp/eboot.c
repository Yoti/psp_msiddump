#include <pspsdk.h>
#include <pspkernel.h>
#include <pspmodulemgr.h>
#include <psprtc.h>
#include <stdio.h> // sprintf()
#include <string.h> // strlen()

PSP_MODULE_INFO("carddump", 0, 3, 2);
PSP_MAIN_THREAD_ATTR(0);
PSP_HEAP_SIZE_KB(20480);

#include "libpspexploit.h"
#define printf pspDebugScreenPrintf

#include <pspctrl.h>
SceCtrlData pad;

static KernelFunctions _ktbl; KernelFunctions* k_tbl = &_ktbl;
unsigned char msid_header[1536];

int CheckDev(unsigned char byte_for_check, char*vendor_name)
{
	switch (byte_for_check)
	{
		case 0x52:
			sprintf(vendor_name, "Lexar");
			return 3; //lexar
		break;

		case 0x4B:
			sprintf(vendor_name, "SanDisk");
			return 2; //sandisk
		break;

		case 0x59:
			sprintf(vendor_name, "SONY");
			return 1; //sony
		break;

		default:
			sprintf(vendor_name, "0x%02X", byte_for_check);
			return 0; //unknown
		break;
	}
}

void Title()
{
	pspDebugScreenInit();
	pspDebugScreenClear(); // особо не нужно
	printf("Welcome to CardDump (v3.2)!\n");
	printf("\n X - Dump\n O - Exit\n");
}


void Setup() 
{
	int k1 = pspSdkSetK1(0);
	int userLevel =  pspXploitSetUserLevel(8);
	pspXploitRepairKernel();
	pspXploitScanKernelFunctions(k_tbl);

	
	pspMsReadAttrB(0, msid_header);
	pspMsReadAttrB(1, msid_header + 512);
	pspMsReadAttrB(2, msid_header + 1024);


	

	pspSdkSetK1(k1);
	pspXploitSetUserLevel(userLevel);
}

void Dump()
{
	int i;
	char sn[4] = "";
	char id[16] = "";
	char ven[8] = "";
	unsigned char tmp[1] = "";
	char path[16] = "";
	SceUID f;


	CheckDev(msid_header[0x1E6], ven);
	printf("\nYour card type is %s \n", ven);

	printf("SN: ");
	for(i = 4; i > 0; i--)
		sprintf(sn, "%s%02x", sn, msid_header[i+0x1B4]);
	printf("%s\n", sn);

	printf("ID: ");
	for(i = 0; i < 16; i++)
		sprintf(id, "%s%02x", id, msid_header[i+0x1E0]);
	printf("%s\n", id);

	sprintf(path, "%s", "ms0:/msid.bin");
	f = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT, 0777);
	for(i = 0; i < 16; i++) {
		tmp[0] = msid_header[i+0x1E0];
		sceIoWrite(f, tmp, sizeof(tmp));
	}
	sceIoClose(f);

	printf("\nSaving to %s... ", path);
	f = sceIoOpen(path, PSP_O_RDONLY, 0777);
	if (f < 0)
		printf("ng");
	else
		printf("ok");
	sceIoClose(f);
	printf("!\n");
	sceKernelDelayThread(1.00*1000*1000);
	printf("\n X - Dump\n O - Exit\n");
}

int main(int argc, char*argv[])
{


	Title();

	for(;;)
	{
		sceCtrlReadBufferPositive(&pad, 1);
		if ((pad.Buttons & PSP_CTRL_CROSS) == PSP_CTRL_CROSS) {
			int res = pspXploitInitKernelExploit();
			if(res == 0) {
				res = pspXploitDoKernelExploit();
			}
			if(res == 0) {
				pspXploitExecuteKernel(Setup);
				Dump();
			}
		}
		else if ((pad.Buttons & PSP_CTRL_CIRCLE) == PSP_CTRL_CIRCLE) {
			sceKernelExitGame();
		}
	}

	return 0;
}
