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
	uint64_t	runcount;
	int		echo;
} StoredConfig;

#ifdef	main_c
		StoredConfig	config = {
			"MagickcigaM"
		};
#else
extern	StoredConfig	config;
#endif

#endif