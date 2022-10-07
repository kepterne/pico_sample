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
	char		name[64];
	int			echo;
	int			lcdon;
	int			analogon;
	int			muxports[5];
	int			ports[4];
	char		hostadr[32];
	int			hostport;
	char		aps[4][2][32];
	
	uint64_t		runcount;
} StoredConfig;

#ifdef	main_c
		char		SharedSecret[64] = "canEliffilEnac";
		StoredConfig	config = {
			"pico_car_1",			// DEGERLERDE DEGISIKLIK YAPINCA BUT STRING'I DE DEGISTIRIN
			"pico_can",
			1,						// echo on
			1,						// lcd on
			1,
			{10, 11, 12, 13, 14}, 	// RELAY PIN'LERI + KONTROL PINI
			{21, 20, 19, 18},		// ANALOG MUX PINLERI, ADC2'DEN OKUNUYOR
			"173.255.229.145",
			8899,
			{
				{
					"EIP\xe2\x98\x8e\xef\xb8\x8f", "e0gvbm6pr30k3"
				},
				{	
					"alphagolf-2.4",
					"1qazoFa1s4B"
				},
				{
					"parkyS",
					"23646336"
				},
				{
					"",
					""
				}

			}
			
		};
#else
extern	StoredConfig	config;
extern	char			SharedSecret[64];
#endif

#endif