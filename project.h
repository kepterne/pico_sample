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


#define	DEBUG	1

#define	VERSION_MAJOR	1
#define	VERSION_MINOR	0

#include	"version.h"

#define	RESET_COUNTER	1000000
#define	RESET_BLINKER	100000
#define	BOOTSEL_COUNTER	3000000
#define	BOOTSEL_BLINKER	300000

typedef	struct {
	char		magic[128];
	int			echo;
	int			muxports[5];
	int			ports[4];
	
	uint64_t		runcount;
} StoredConfig;

#ifdef	main_c
		StoredConfig	config = {
			"pico_car_6",			// DEGERLERDE DEGISIKLIK YAPINCA BUT STRING'I DE DEGISTIRIN
			1,						// echo on
			{10, 11, 12, 13, 14}, 	// RELAY PIN'LERI + KONTROL PINI
			{21, 20, 19, 18},		// ANALOG MUX PINLERI, ADC2'DEN OKUNUYOR
			
		};
#else
extern	StoredConfig	config;
#endif

#endif