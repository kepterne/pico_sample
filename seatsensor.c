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
void	selectChannel(int idx) {
	int	i;
	printf("\r\nSelecting channel %d\r\n", idx);
	for (i = 0; i < 3; i++) {
		gpio_init(config.ports[i]);
      	gpio_set_dir(config.ports[i], GPIO_OUT);
      	
		if (idx & (1 << i)) {
			gpio_put(config.ports[i], 1);
			printf("GPIO%02d ", config.ports[i]);
		} else
			gpio_put(config.ports[i], 0);
	}
	printf("\r\n----------------------------\r\n");
}
int	readn = 0;
void	System(uint32_t cmd, char *p1, char *p2, char *p3, char *p4) {
	switch (cmd) {
	case CMD_UART_DATA: {
		int		idx, val;
		if (sscanf(p1, "ECHO %d", &config.echo) == 1) {
			printf("\r\nECHO: %s\r\n", config.echo ? "ON" : "OFF");
			UpdateConfig(&sys);
		} else if (strcasecmp(p1, "ID") == 0) {
			printf("\r\nID: %s v:%s f:%p s:%d c:%llu\r\n", sys.id, sys.version, flash_start, sys.size, config.runcount);
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
			selectChannel(idx);
			readn = val;
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
			adc_init();
			adc_gpio_init(28);
			adc_select_input(2);
		}
	break;
	case CMD_USB_CONNECTED: {
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
		}
	break;
	case CMD_USB_DISCONNECTED: 
			printf("\r\nBYE\r\n");
	break;
	}
}

int	main(void) {
	initSys(&sys, System);
	for ( ; ; ) {
		loopSys(&sys);
		if (readn) {
			int	adc = adc_read();
			printf("\r%12d            \r", adc);
			readn--;
			if (readn <= 0)
				printf("\r\nREAD DONE\r\n");
		}
    }
}
