#include <pspsdk.h>
#include <pspkernel.h>
#include <stdio.h> // sprintf()
#include <string.h> // strlen()

PSP_MODULE_INFO("carddump", 0, 3, 1);
PSP_MAIN_THREAD_ATTR(0);
PSP_HEAP_SIZE_KB(20480);

#include "../mdumper_prx/mdumper.h"
#define printf pspDebugScreenPrintf

#include <pspctrl.h>
SceCtrlData pad;

void ExitError(char*text, int delay, int error)
{
	printf(text, error);
	sceKernelDelayThread(delay*1000*1000);
	sceKernelExitGame();
}

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

int pspSdkLoadStartModule_Smart(const char*file)
{
	SceUID module_file;
	u8 module_type = 0;

	module_file = sceIoOpen(file, PSP_O_RDONLY, 0777);
	if (module_file >= 0)
	{
		sceIoLseek(module_file, 0x7C, PSP_SEEK_SET);
		sceIoRead(module_file, &module_type, 1);
		sceIoClose(module_file);

		if (module_type == 0x02)
			return pspSdkLoadStartModule(file, PSP_MEMORY_PARTITION_KERNEL);
		else if (module_type == 0x04)
			return pspSdkLoadStartModule(file, PSP_MEMORY_PARTITION_USER);
		else
			return -2; // неизвестный тип
	}
	else
		sceIoClose(module_file);

	return -1; // нет файла
}

void Title()
{
	pspDebugScreenInit();
	pspDebugScreenClear(); // особо не нужно
	printf("Welcome to CardDump (v3.1)!\n");
	printf("\n X - Dump\n O - Exit\n");
}

void Dump()
{
	Title();

	unsigned char msid_header[1536];
	pspMsReadAttrB(0, msid_header);
	pspMsReadAttrB(1, msid_header + 512);
	pspMsReadAttrB(2, msid_header + 1024);

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
	SceUID mod = pspSdkLoadStartModule_Smart("mdumper.prx");
	if (mod < 0)
		ExitError("Error: LoadStart() returned 0x%08x\n", 3, mod);

	Title();

	for(;;)
	{
		sceKernelDelayThread(0.50*1000*1000);
		sceCtrlReadBufferPositive(&pad, 1);
		if (pad.Buttons & PSP_CTRL_CROSS)
			Dump();
		else if (pad.Buttons & PSP_CTRL_CIRCLE)
			sceKernelExitGame();
	}

	return 0;
}