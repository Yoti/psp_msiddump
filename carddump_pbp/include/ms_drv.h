// табул€ци€ по версии no-no-notepad++
#define WSBW(d) ((((d)&0x000000ff) << 24) | (((d)&0x0000ff00) << 8) | (((d)&0x00ff0000) >> 8) | (((d)&0xff000000) >> 24))
#define WSBH(d) ((((d)&0x00ff) << 8) | (((d)&0xff00) >> 8))

#define MSPRO_DEVINFOID_SYSINFO			(0x10)
#define MSPRO_DEVINFOID_MODELNAME		(0x15)
#define MSPRO_DEVINFOID_MBR				(0x20)
#define MSPRO_DEVINFOID_PBR16			(0x21)
#define MSPRO_DEVINFOID_PBR32			(0x22)
#define MSPRO_DEVINFOID_SPECFILEVALUES1	(0x25)
#define MSPRO_DEVINFOID_SPECFILEVALUES2	(0x26)
#define MSPRO_DEVINFOID_IDENTIFYDEVINFO	(0x30)

typedef struct tag_wbmspro_attribute {
	u16 signature;
	u16 version;
	u8 device_information_entry_count;
	u8 reserved[11];
} __attribute__ ((packed)) wbmspro_attribute_t;

typedef struct tag_wbmspro_device_info_entry_item {
	u32 address;
	u32 size;
	u8 info_id;
	u8 reserved[3];
} __attribute__ ((packed)) wbmspro_device_info_entry_item_t;

typedef struct tag_wbmspro_sys_info {
	u8 class;				// must be ?
	u8 reserved;			// see below
	u16 block_size;			// n KB
	u16 block_count;		// number of physical block
	u16 user_block_count;	// number of logical block
	u16 page_size;			// must be 0x200
	u8 reserved1[2];		// MS original Extra data size and format reserved
	u8 assembly_date[8];
	u32 serial_number;
	u8 assembly_maker_code;
	u8 assembly_model_code[3];
	u16 memory_maker_code;
	u16 memory_model_code;
	u8 reserved2[4];		//reserved[6]
	u8 vcc;
	u8 vpp;
	u16 controller_number;
	u16 controller_function;
	u16 start_sector;
	u16 unit_size;
	u8 ms_sub_class;
	u8 reserved3[4];
	u8 interface_type;
	u16 controller_code;
	u8 format_type;
	u8 reserved4;			// ;;
	u8 device_type;
	u8 reserved5[7];
	u8 mspro_id[16];
	u8 reserved6[16];
} __attribute__ ((packed)) wbmspro_sys_info_t;

#define SHOW_READ_DATA		0
#define SHOW_ERR_MSG		0
#define SHOW_SECTOR_ACCESS	0
#define SHOW_STATUS_REG		0

#define IO_MEM_STICK_CMD	*((volatile int*)(0xBD200030))
#define IO_MEM_STICK_DATA	*((volatile int*)(0xBD200034))
#define IO_MEM_STICK_STATUS	*((volatile int*)(0xBD200038))
#define IO_MEM_STICK_SYS	*((volatile int*)(0xBD20003C))

#define MSRST 0x8000

#define MS_FIFO_RW		0x4000
#define MS_RDY			0x1000
#define MS_TIME_OUT		0x0100
#define MS_CRC_ERROR	0x0200

#define READ_PAGE_DATA	0x2000
#define READ_REG		0x4000
#define GET_INT			0x7000
#define SET_RW_REG_ADRS	0x8000
#define EX_SET_CMD		0x9000

#define WRITE_REG	(0xB000)
#define SET_CMD		(0xE000)

#define INT_REG_CED		0x80
#define INT_REG_ERR		0x40
#define INT_REG_BREQ	0x20
#define INT_REG_CMDNK	0x01
/*
NORM_COMP		= (ced && !err)
CMD_ERR_TER		= (ced && err)
NORM_DATA_TRANS	= (!err && breq)
DATA_REQ_ERR	= (err && breq)
CMD_EXE			= (!ced && !breq)
CMD_NOT_EXE		= cmdnk
*/

void pspMsBootStart(void); // -> pspMsInit
int pspMsInit(void);
int pspMsReadSector(int sector, void *addr);
int pspMsReadAttrB(int attr, void *addr);
int pspMsWriteSector(int sector, void *addr);
int pspMsEraseBlock(void);