	.set noreorder

#include "pspstub.s"

	STUB_START "mdumper_prx",0x40090000,0x00020005
	STUB_FUNC  0x8B002469,pspMsReadAttrB
	STUB_FUNC  0xACC98241,pspMsReadSector
	STUB_END
