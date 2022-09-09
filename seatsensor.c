#define	main_c

#include	"pico/bootrom.h"
#include	"project.h"
#include	"common.h"


void	System(uint32_t cmd, char *p1, char *p2, char *p3, char *p4) {
	switch (cmd) {
	case CMD_UART_DATA:
		if (sscanf(p1, "ECHO %d", &config.echo) == 1) {
			printf("\r\nECHO: %s\r\n", config.echo ? "ON" : "OFF");
			UpdateConfig(&sys);
		} else if (strcasecmp(p1, "ID") == 0) {
			printf("\r\nID: %s v:%s f:%p s:%d c:%llu\r\n", sys.id, sys.version, flash_start, sys.size, config.runcount);
		} else if (strcasecmp(p1, "RESET") == 0) {
			resetPico();
		}
	break;
	case CMD_BUTTON_PRESS: {
			uint32_t	d;
			d = (uint32_t) p1;
			printf("\r\nBUTTON PRESS %u\r\n", d);
		}
	break;
	case CMD_CONFIG_STORED: {
			printf("CONFIG STORED\r\n");
		}
	break;
	case CMD_PROGRAM_INIT: {
			printf("PROGRAM INIT\r\n");
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
	case CMD_USB_DISCONNECTED: {
			printf("\r\nBYE\r\n");
		}
	break;
	}
}

int	main(void) {
	initSys(&sys, System);
	for ( ; ; ) {
		loopSys(&sys);
    }
}
