#define	main_c

#include	"pico/bootrom.h"
#include	"project.h"
#include	"common.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"


char	map[32];
char	*getMap(int i, int j) {
	int k = 1, l = 0;
	k <<= j;
	map[0] = 0;
	for ( ; k; k >>= 1) {
		l += sprintf(map + l, "%c", k & i ? '1' : '.');
	}
	return map;
}
void	listThresholds(void) {
	int	i, j;
	printf("\r\n#\t\tMAP\t\tANALOG THRESHOLD\r\n");
	for (i = 15; i >= 0; i--) {
		printf("%3d\t\t%8s\t\t%10d\r\n", 15 - i, getMap(i, 3), config.vmap[15 - i]);
	}
	printf("--------------------------------------------\r\n");
}
void	selectChannel(int idx, int verbose) {
	int	i;
	if (verbose)
		printf("\r\nSelecting channel %d\r\n", idx);
	for (i = 0; i < 3; i++) {
		gpio_init(config.ports[i]);
      	gpio_set_dir(config.ports[i], GPIO_OUT);
      	
		if (idx & (1 << i)) {
			gpio_put(config.ports[i], 1);
			if (verbose)
				printf("GPIO%02d ", config.ports[i]);
		} else
			gpio_put(config.ports[i], 0);
	}
	if (verbose)
		printf("\r\n----------------------------\r\n");
//	adc_init();
	adc_select_input(2);
}
int	readn = 0;
int	channel = 0;
void	System(uint32_t cmd, char *p1, char *p2, char *p3, char *p4) {
	switch (cmd) {
	case CMD_UART_DATA: {
		int		idx, val;
		if (sscanf(p1, "ECHO %d", &config.echo) == 1) {
			printf("\r\nECHO: %s\r\n", config.echo ? "ON" : "OFF");
			UpdateConfig(&sys);
		} else if (strcasecmp(p1, "CLR") == 0) {
			printf("\r\n\x1B[2J");
		} else if (strcasecmp(p1, "ID") == 0) {
			printf("\r\nJID: %s v:%s f:%p s:%d c:%llu\r\n", sys.id, sys.version, flash_start, sys.size, config.runcount);
		} else if (strcasecmp(p1, "RESET") == 0) {
			resetPico();
		} else if (strcasecmp(p1, "TRH") == 0) {
				listThresholds();
		} else if (sscanf(p1, "TRH %d %d", &idx, &val) == 2) {
			if (idx < 16 && idx >= 0) {
				config.vmap[idx] = val;
				UpdateConfig(&sys);
				listThresholds();
			}
		} else if (sscanf(p1, "READ %d %d", &idx, &val) == 2) {
		/*	adc_init();
			adc_gpio_init(28);
			adc_select_input(2);
			selectChannel(idx, 1);
		*/	readn = val;
			channel = idx;
		}
	}
	break;
	case CMD_BUTTON_PRESS: {
			uint32_t	d;
			d = (uint32_t) p1;
			printf("\r\nBUTTON PRESS %u\r\n", d);
		}
	break;
	case CMD_CONFIG_STORED: 
			printf("CONFIG STORED\r\n");
	break;
	case CMD_PROGRAM_INIT: {
			printf("PROGRAM INIT\r\n");
			sys.usb_ack = 0;
	
		/*	adc_init();
			adc_gpio_init(28);
			adc_select_input(2);
		*/
		}
	break;
	case CMD_USB_CONNECTED: {
#ifndef	DEBUG
			printf(
				"WELCOME\r\n"
					"\tID.........: %s\r\n"
					"\tVERSION....: %s\r\n"
					"\tUPTIME.....: %llu secs.\r\n"
					"\tRUN COUNT..: %llu\r\n"
					"\tECHO %s\r\n",
				sys.id,
				sys.version,
				sys.seconds,
				config.runcount,
				config.echo ? "ON" : "OFF"
			);
			sys.usb_ack = 1;
#else

#endif
		}
	break;
	case CMD_USB_DISCONNECTED: 
			printf("\r\nBYE\r\n");
	break;
	}
}
#define	MAX_READING	32

int	peaks[16]; 
int	reading[16];

int	MeasureADC(int idx) {
	selectChannel(idx, 0);
	
	sleep_ms(1);
	int	avg = 0, value = 0;
	for (int k = 0; k < MAX_READING; k++) {
		value = adc_read();
		avg += value;
	}
	avg /= MAX_READING;
	if (ABSDIFF(peaks[idx], avg) > 6) {
		int		K = 15;
		peaks[idx] = avg;
		for (int i = 0; i < 15; i++, K--) {
			if (avg >= config.vmap[i]) {
				break;
			}
		}
		if (reading[idx] != K) {
			reading[idx] = K;
		#ifndef	DEBUG	
			return K;
		#endif
		}
	}
#ifdef	DEBUG
	return reading[idx];
#else
	return -1;
#endif
}
int	main(void) {
	int	i;
	uint32_t	owner;
	uint64_t	seconds = 0;
	initSys(&sys, System);
	adc_init();
	adc_gpio_init(28);

//	adc_set_temp_sensor_enabled(true); // Enable on board temp sensor
    
//	sleep_ms(100);
	for ( ; ; ) {
		loopSys(&sys);
#ifdef	DEBUG
		sleep_ms(10);
		if (readn) {
			printf("\r");
			// \033[<N>A
			//for (int i = 0; i < 8; i++) {
			i = channel;
			{
				int	v = MeasureADC(i);
				if (i == 4)
					printf("\r\n");
				printf("%2d: %4d %2d, ", i, peaks[i], 15 - v);
				//printf("%2d: %6d [%2d], ", i, v, 15 - reading[i]);
			}
			printf("\r");
			printf("\033[1A");
		}	
#else
		if (sys.seconds == seconds) {
			int	v;
			float temp = 0;
			/*adc_select_input(4);
			const float conversion_factor = 3.3f / (1 << 12);
			float result = adc_read() * conversion_factor;
			float temp = 27 - (result - 0.706)/0.001721;
			*/
			seconds += 4;
			sys.internal_temp = temp;
			printf("~version(%s) id(%s) seconds(%llu) temp(%f)~\r\n",
			 	sys.version,
				sys.id,
				sys.seconds,
				sys.internal_temp
			);
			/*
			for (int i = 0; i < 8; i++) {
				reading[i] = -100;
				peaks[i] = -100;
			}
			*/
			for (int i = 0; i < 8; i++) 
				printf("~analog(%d %s %d)~\r\n", i, getMap(reading[i], 3), peaks[i]);
		} else
		for (int i = 0; i < 8; i++) {
			int	v = MeasureADC(i);
			if (v >= 0)
				printf("~analog(%d %s %d)~\r\n", i, getMap(v, 3), peaks[i]);
				//printf("%2d: %6d [%2d], ", i, v, 15 - reading[i]);
		}
#endif
	}
}
