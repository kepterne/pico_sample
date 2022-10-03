#define	main_c

#include	"pico/bootrom.h"
#include	"project.h"
#include	"common.h"

#include	"hardware/gpio.h"
#include	"pico/multicore.h"
#include	"analog_reader.h"
#include	"lcd_display.h"


int	anok = 1, lcdok = 1;

void	System(uint32_t cmd, char *p1, char *p2, char *p3, char *p4) {
	switch (cmd) {
	case CMD_UART_DATA: {
		int		idx, val;
		int		segs = (int) p4;
		char		**p = (char **) p3;
		if (strcmp(p[0], "DMA") == 0) {
			if (anok) {
				multicore_launch_core1(core1_analog);
				anok = 0;
			//	core1_analog();
			} else {
				multicore_reset_core1();
				anok = 1;
			}
			//core1_analog();
		}
		if (strcmp(p[0], "LCD") == 0) {
			if (lcdok) {
				lcd_init();
				lcdok = 0;
			//	core1_analog();
			} else {
				lcdok = 1;
			}
			//core1_analog();
		}
		if (strcmp(p[0], "ECHO") == 0) {
			if (segs > 1) {
				int	j = -1;
				if (strcmp(p[1], "ON") == 0) 
					j = 1;
				else if (strcmp(p[1], "OFF") == 0)
					j = 0;
				if (j >= 0) {
					if (config.echo != j) {
						config.echo = j;
						UpdateConfig(&sys);
					};
				}
			}
			printf("\r\nECHO: %s\r\n", config.echo ? "ON" : "OFF");
		} else if (strcmp(p[0], "CLR") == 0) {
			printf("\r\n\x1B[2J");
		} else if (strcmp(p[0], "ID") == 0) {
			printf("\r\nID: %s v:%s f:%p s:%d c:%llu FS:%u\r\n", sys.id, sys.version, flash_start, sys.size/1024, config.runcount, sys.flashsize/1024);
		} else if (strcmp(p[0], "RESET") == 0) {
			resetPico();
		} else if (strcmp(p[0], "USB") == 0) {
			reset_usb_boot(0, 0);
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
		}
	break;
	case CMD_USB_CONNECTED: {
			printf("\r\nCONNECTED\r\n");
		}
	break;
	case CMD_USB_DISCONNECTED: 
			printf("\r\nDISCONNECTED\r\n");
	break;
	}
}

int	bootsel_button = 0;

int	main(void) {
	uint64_t	last = 0;
	int		seconds = 0;
	initSys(&sys, System);
	lcd_init();	
	lcdok = 0;
	multicore_launch_core1(core1_analog);
	anok = 0;
	for ( ; ; ) {
		loopSys(&sys);
		{
			
			if (!anok) {
				int	a = reading16[2];
				float v = reading16[1];

				v = 3.3 * v / (4096.0);
				v = ABSDIFF(v, 2.5);
				v /= 2.5;
				v *= 30.0;
				//float aT = NTCTemp(a, 10, 25, 100, 3950);
				float aT = NTCTemp(a, 10, 25, 10, 3950);
				static float t[4] = {0, 0, 0, 0};
				const float conversionFactor = 3.3f / (1 << 12);

				float adc = (float)reading16[3] * conversionFactor;
				float tempC = 27.0f - (adc - 0.706f) / 0.001721f;


				//if (a || reading16[3])
				if (sys.unow - last > 250000)
				if (samples > 100000)
				//if (sys.seconds != seconds)
				if (ABSDIFF(t[0], aT) >= 0.1 
				|| ABSDIFF(t[1], tempC) >= 0.1
				|| ABSDIFF(t[2], v) >= 0.1
				) {
					t[0] = aT;
					t[1] = tempC;
					t[2] = v;
					last = sys.unow;
					seconds = sys.seconds;
					//printf("\r\nT: %5.1f\r\n", aT);
					if (!lcdok) {
						char	st[32];
						sprintf(st, "%5.1f%5.1f %3.1fA", aT, tempC, v);
						lcd_set_cursor(0, 0);
						lcd_string(st);
					}
				}
				

			}
		}
	}
}
