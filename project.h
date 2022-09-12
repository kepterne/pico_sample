#ifndef	project_h
#define	project_h

#include	<stdio.h>
#include	<string.h>
#include	"pico/stdlib.h"
#include	"pico/unique_id.h"
#include	"hardware/gpio.h"
#include	"hardware/sync.h"
#include	"hardware/structs/ioqspi.h"
#include	"hardware/structs/sio.h"

#define	VERSION_MAJOR	1
#define	VERSION_MINOR	0

#include	"version.h"

#define	RESET_COUNTER	1000000
#define	RESET_BLINKER	100000
#define	BOOTSEL_COUNTER	3000000
#define	BOOTSEL_BLINKER	300000

typedef	struct {
	char		magic[128];
	int		ports[3];
	int		vmap[16];
	uint64_t	runcount;
	int		echo;
} StoredConfig;

#ifdef	main_c
		StoredConfig	config = {
			"MagickcigaM",
			{16, 18, 17},
			{
				1040,				// 1 1 1 1
				990,				// 1 1 1 0
				930,				// 1 1 0 1                                             
				870,				// 1 1 0 0
				800,				// 1 0 1 1
				730,				// 1 0 1 0
				650,				// 1 0 0 1
				560,				// 1 0 0 0
				500,				// 0 1 1 1
				430,				// 0 1 1 0
				360,				// 0 1 0 1
				310,				// 0 1 0 0
				220,				// 0 0 1 1
				140,				// 0 0 1 0
				110,				// 0 0 0 1
				0				// 0 0 0 0
			}
		};
#else
extern	StoredConfig	config;
#endif

#endif