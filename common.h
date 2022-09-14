#ifndef	common_h
#define	common_h

#define	ABSDIFF(x, y)	((x) > (y) ? (x) - (y) : (y) - (x))

#define	CMD_PROGRAM_INIT		0x0001
#define	CMD_CONFIG_STORED		0x0002
#define	CMD_BUTTON_PRESS		0x0003
#define	CMD_UART_DATA		0x0004
#define	CMD_USB_DISCONNECTED	0x0005
#define	CMD_USB_CONNECTED		0x0006

typedef	struct {
	char		id[64];
	char		version[64];
	uint64_t	unow, ustart;
	uint64_t	seconds;
	uint32_t	size;
	int		bootsel;
	uint64_t	bootsel_start;
	uint64_t	usb_connected;
	uint8_t	usb_ack;
	float		internal_temp;
	void		(*cb)(uint32_t, char *, char *, char *, char *);
} SystemConfig;

bool __no_inline_not_in_flash_func(get_bootsel_button)();
void	initSys(SystemConfig *s, void (*f)(uint32_t, char *, char *, char *, char *));
void	loopSys(SystemConfig *s);
void	GetBoardID(char *p);
void	resetPico(void);
void	SaveConfig(SystemConfig *s);
void	UpdateConfig(SystemConfig *s);

#ifdef	common_c
		char			*flash_start = (char *) XIP_BASE;
		uintptr_t		flash_addr;
		SystemConfig	sys;
#else
extern	char			*flash_start;
extern	uintptr_t		flash_addr;
extern	SystemConfig	sys;
#endif

#endif