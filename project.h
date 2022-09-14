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


//#define	DEBUG	1

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
			"MagicKcigaM",
			{18, 17, 16},
			{
				1040, // 1111
				1000, // 1110
				950,
				890,
				830,
				760,
				690,
				620,
				550,
				490,
				420,
				340,
				260,
				185,
				95,
				0
			}
		};
#else
extern	StoredConfig	config;
#endif

#endif