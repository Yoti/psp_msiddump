#include <pspsdk.h>

PSP_MODULE_INFO("mdumper_prx", 0x1006, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

int module_start(SceSize args, void *argp)
{
	return 0;
}

int module_stop(void)
{
	return 0;
}
