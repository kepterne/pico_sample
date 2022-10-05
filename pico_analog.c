#define	main_c

#include	"pico/bootrom.h"
#include	"project.h"
#include	"common.h"
#include	<time.h>

#include	"hardware/gpio.h"
#include	"pico/multicore.h"
#include	"analog_reader.h"
#include	"lcd_display.h"
#include	"wifi.h"
#include	"md5.h"


#include "pico/cyw43_arch.h"

int	anok = 1, lcdok = 1;

uint64_t	get64(char *p) {
	uint64_t	r = 0;
	for ( ; *p; p++) {
		if (*p >= '0' && *p <= '9')
			r = (r * 10) + *p - '0';
	}
	return r;
}

void	ProcessFields(TCP_CLIENT_T *tc, char *p) {
	if (*p != '~')
		return;
	char	*name;
	char	*value;
	for (p++; *p != '~' && *p; ) {
		while (*p == 9 || *p == 32)
			p++;
		if (!isalpha(*p))
			return;
		for (name = p++; isalnum(*p); p++);
		if (*p != '(')
			return;
		*(p++) = 0;
		value = p;
		while (*p != ')' && *p)
			p++;
		if (!*p)
			return;
		*(p++) = 0;
		if (sys.cb)
			(*sys.cb)(CMD_PARAM, tc, name, value, NULL);
	}
}

void	System(uint32_t cmd, char *p1, char *p2, char *p3, char *p4) {
	switch (cmd) {
	case CMD_PARAM: {
		printf("[%s] = [%s]\r\n", p2, p3);

		if (strcasecmp(p2, "now") == 0) {
			time_t	ts = 0;
			sscanf(p3, "%lu", &ts);
			ts += 3 * 3600;
			sys.tbase = ts;
			sys.tstart = sys.seconds;
			struct tm	*t;
			t = localtime(&ts);
			printf("@ TIME SYNCED: %04d %02d %02d - %02d %02d %02d\r\n",
				t->tm_year + 1900,
				t->tm_mon + 1,
				t->tm_mday,
				t->tm_hour,
				t->tm_min,
				t->tm_sec
			);
		} else if (strcasecmp(p2, "unow") == 0) {
			uint32_t	ts;
			sscanf(p3, "%lu", &ts);
			sys.toff = ts / 1000;
			struct tm	*t;
			ts = getTime();
			t = localtime(&ts);
			printf("@ T: %04d %02d %02d - %02d %02d %02d\r\n",
				t->tm_year + 1900,
				t->tm_mon + 1,
				t->tm_mday,
				t->tm_hour,
				t->tm_min,
				t->tm_sec
			);
		}
	}
	break;
	case CMD_TCP_DATA: {
		TCP_CLIENT_T	*tc = (TCP_CLIENT_T *) p1;
		int		l = (int) p3;
		char	*p = p2;
		printf("TCP DATA < %3d:[%s]\r\n", l, p);
		if (strncasecmp(p, "+SERVER: ", 9))
			return;
		p += 9;
		l -= 9;
		if (*p == '~') {
			ProcessFields(tc, p);
		} else if (strncmp(p, "PING ", 5) == 0) {
			uint64_t		i;
			
			if ((i = get64(p + 5))) {
				((uint32_t *) &i)[0] ^= 0x17891789l;
				((uint32_t *) &i)[1] ^= 0x29091999l;
				sprintf(tc->senddata, "\nPING: %" PRIu64 "\n", i);
				
			}
		} else if (strncmp(p, "KIMO ", 5) == 0) {
			char  challenge[64];
			uint8_t md5[32];
			p += 5;
			sscanf(p, "%s", challenge);
			strcat(challenge, SharedSecret);
			md5_buffer((uint8_t *) challenge, strlen(challenge), md5);
			md5_digest_string(md5, challenge);
		//	up(" !------------- CHALLENGE -------------! ");
		//	upr(challenge);
			sprintf(tc->senddata, "HELO: %s %u.%u ~type(%s) uptime(%llu) id(%s) ver(%s) wifi(%s)~\n", challenge, 1, 1,
				"WEMOS_MINI",
				sys.seconds, 
				sys.id,
				sys.version,
				config.aps[current_ap][0]
			);
		}
	}
	break;
	case CMD_WIFI_CONNECTING: {
		printf("Connecting to : \"%s\"\r\n", p1);
		lcd_set_cursor(2, 0);
		lcd_string(p1);
	}
	break;

	case CMD_WIFI_CONNECTED: {
		printf("Connected to : \"%s\" ip: %s\r\n", p1, p2);
		lcd_set_cursor(3, 0);
		lcd_string(p2);
	}
	break;

	case CMD_WIFI_DISCONNECTED: {
		printf("Disconnected from : \"%s\"\r\n", p1);
	}
	break;
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
				anok = 1;
				while (anok != 2)
					sleep_ms(1);
				anok = 1;
				multicore_reset_core1();
				if (!lcdok) {
					lcd_clear();
				}
				
			}
			//core1_analog();
		}
		if (strcmp(p[0], "LCD") == 0) {
			if (lcdok) {
				lcd_init();
				lcdok = 0;
			//	core1_analog();
			} else {
				lcd_clear();
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
/*	multicore_launch_core1(core1_analog);
	anok = 0;
*/

	for ( ; ; ) {
		loopSys(&sys);
		{
			
			if (!anok) {
				int	a = reading16[ADC_2];
				float v = reading16[ADC_1];
				float	vcc = 3 * reading16[ADC_VCC] * 3.3 / 4095.0;

				v = 3.3 * v / (4096.0);
				v = ABSDIFF(v, 2.5);
				v /= 2.5;
				v *= 30.0;
				//float aT = NTCTemp(a, 10, 25, 100, 3950);
				float aT = NTCTemp(a, 10, 25, 10, 3950);
				static float t[5] = {0, 0, 0, 0, 0};
				const float conversionFactor = 3.3f / (1 << 12);

				float adc = (float)reading16[ADC_TEMP] * conversionFactor;
				float tempC = 27.0f - (adc - 0.706f) / 0.001721f;


				//if (a || reading16[3])
				if (sys.unow - last > 250000)
				if (samples > 100000)
				//if (sys.seconds != seconds)
				if (ABSDIFF(t[0], aT) >= 0.1 
				|| ABSDIFF(t[1], tempC) >= 0.1
				|| ABSDIFF(t[2], v) >= 0.1
				|| ABSDIFF(t[3], vcc) >= 0.1
				) {
					t[0] = aT;
					t[1] = tempC;
					t[2] = v;
					t[3] = vcc;
					last = sys.unow;
					seconds = sys.seconds;
					//printf("\r\nT: %5.1f\r\n", aT);
					if (!lcdok) {
						char	st[32];
						sprintf(st, "%4.1f%5.1f %3.1fA %4.1fV", aT, tempC, v, vcc);
						lcd_set_cursor(0, 0);
						lcd_string(st);
					}
				}
			}
		}
	}
}
