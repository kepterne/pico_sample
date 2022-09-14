#define	common_c
#include	"pico/bootrom.h"
#include	"hardware/watchdog.h"
#include	<hardware/flash.h>
#include	"hardware/adc.h"

#include	"project.h"
#include	"common.h"


bool __no_inline_not_in_flash_func(get_bootsel_button)() {
    const uint CS_PIN_INDEX = 1;

    // Must disable interrupts, as interrupt handlers may be in flash, and we
    // are about to temporarily disable flash access!
    uint32_t flags = save_and_disable_interrupts();

    // Set chip select to Hi-Z
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // Note we can't call into any sleep functions in flash right now
    for (volatile int i = 0; i < 1000; ++i);

    // The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
    // Note the button pulls the pin *low* when pressed.
    bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));

    // Need to restore the state of chip select, else we are going to have a
    // bad time when we return to code in flash!
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);

    return button_state;
}

void	GetBoardID(char *p) {
	int		l = 0;

	pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);

    for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i) {
        l += sprintf(p + l, "%02x", board_id.id[i]);
    }
}

extern char __flash_binary_end;

void	SaveConfig(SystemConfig *s) {
	uint32_t 	ints;
	int		sz = sizeof(config);
	ints = save_and_disable_interrupts();
	if (sz % FLASH_SECTOR_SIZE)
		sz = sz + FLASH_SECTOR_SIZE - (sz % FLASH_SECTOR_SIZE);
	flash_range_erase(flash_addr, sz);
	sz = sizeof(config);
	if (sz % FLASH_PAGE_SIZE)
		sz = sz + FLASH_PAGE_SIZE - (sz % FLASH_PAGE_SIZE);
	flash_range_program(flash_addr, (char *) &config, sz);
	restore_interrupts (ints);
	if (s->cb)
		(*s->cb)(CMD_CONFIG_STORED, (char *) &config, NULL, NULL, NULL);
	
}

void	UpdateConfig(SystemConfig *s) {
	StoredConfig	*sc;
	sc = (StoredConfig *) flash_start;
	if (memcmp(sc, &config, sizeof(config))) {
		SaveConfig(s);
	}
} 

void	initSys(SystemConfig *s, void (*f)(uint32_t, char *, char *, char *, char *)) {
	StoredConfig	*sc;
	char	*pp;
#ifndef	DEBUG
	stdio_uart_init_full(uart1, 115200, 4, 5);
#else
	stdio_init_all();
#endif
	sleep_ms(100);
	s->usb_connected = 0;
	s->usb_ack = 0;
	s->ustart = time_us_64();
	s->cb = f;
	s->bootsel = 0;
	s->bootsel_start = 0;
	s->usb_connected = 0;
	GetBoardID(s->id);
	gpio_init(PICO_DEFAULT_LED_PIN);
   	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	strcpy(s->version, completeVersion);
	
	pp = &__flash_binary_end;
	
	s->size = pp - flash_start;
	uintptr_t tt = (uintptr_t) pp;
	if (tt % FLASH_SECTOR_SIZE)
		tt = tt + FLASH_SECTOR_SIZE - (tt % FLASH_SECTOR_SIZE);
	flash_start = (char *) tt;
	flash_addr = tt - (uintptr_t) XIP_BASE;
	sc = (StoredConfig *) flash_start;
	if (strcmp(sc->magic, config.magic))
		SaveConfig(s);
	else
		memcpy(&config, flash_start, sizeof(config));
	config.runcount++;
	UpdateConfig(s);
	if (s->cb)
		(*s->cb)(CMD_PROGRAM_INIT, (char *) s, (char *) &config, NULL, NULL);
		
}

void	resetPico(void) {
	initSys(&sys, sys.cb);
	//sys.usb_ack = 0;
	watchdog_reboot(0, 0, 0);
}

void	LoopButton(SystemConfig *s) {
	int	r = get_bootsel_button();	
	uint64_t	d;
	if (s->bootsel) {
		int	k = 0;
		d = s->unow - s->bootsel_start;
		if (d >= BOOTSEL_COUNTER) {
			k = ((d - BOOTSEL_COUNTER)/BOOTSEL_BLINKER) & 1;
		} else if (d >= RESET_COUNTER) {
			k = ((d - RESET_COUNTER) / RESET_BLINKER) & 1;
		} else
			k = 0;
		gpio_put(PICO_DEFAULT_LED_PIN, (k ^ 1) ^ PICO_DEFAULT_LED_PIN_INVERTED);	
	}
	if (r != s->bootsel) {	
		s->bootsel = r;
		if (r) {
			s->bootsel_start = s->unow;
		} else {
			gpio_put(PICO_DEFAULT_LED_PIN, 0 ^ PICO_DEFAULT_LED_PIN_INVERTED);
			if (d > BOOTSEL_COUNTER)
				reset_usb_boot(0, 0);
			else if (d > RESET_COUNTER) 
				resetPico();
			else if (s->cb)
				(*s->cb)(CMD_BUTTON_PRESS, (char *) ((int) (d & 0xffff)), NULL, NULL, NULL);
		}
	}
}


#ifdef DEBUG
char		line[512];
int		linec = 0;
int		readon = 0;


void  processLine(char *p, int l) {
	int		idx;
//	printf("L %d:%s\r\n", l, p);
	if (sscanf(p, "ECHO %d", &config.echo) == 1) {
		printf("\r\nECHO: %s\r\n", config.echo ? "ON" : "OFF");
		UpdateConfig(&sys);
	} else if (strcasecmp(p, "ID") == 0) {
		printf("\r\nID: %s v:%s f:%p s:%d c:%llu\r\n", sys.id, sys.version, flash_start, sys.size, config.runcount);
	}
}

void	input_loop(void) {
	int		c;
	if ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {		
		if (!sys.usb_ack) {
			printf("\r\n\033[2J%s > ", sys.id);
			if (sys.cb)
				(*sys.cb)(CMD_USB_CONNECTED, NULL, NULL, NULL, NULL);
			//Esc[2J printf("\r\n\x1B[2J");
			
		}
		sys.usb_connected = sys.unow;
		if (config.echo)
			putchar(c);
		if (c == 10)
			return;
		if (c == 13) {
			line[linec] = 0;
			if (linec) {
				//processLine(line, linec);
				if (sys.cb)
					(sys.cb)(CMD_UART_DATA, line, (char *) linec, NULL, NULL);
				
			} 
			printf("\r\n%s > ", sys.id);
			linec = 0;
		} else {
			line[linec] = c & 0xFF;
			linec += linec < 511 ? 1 : 0;
		}
	} else if (sys.usb_connected) {
		if ((sys.unow - sys.usb_connected) > 10000000) {
			sys.usb_connected = 0;
			sys.usb_ack = 0;
			if (sys.cb)
				(*sys.cb)(CMD_USB_DISCONNECTED, NULL, NULL, NULL, NULL);
		}
	}
}

#endif

void	loopSys(SystemConfig *s) {
	s->unow = time_us_64();
	s->seconds = (s->unow - s->ustart) / 1000000;
	/*
	adc_select_input(4);
	uint16_t raw = adc_read();
	const float conversion_factor = 3.3f / (1 << 12);
	float result = raw * conversion_factor;
	float temp = 27 - (result - 0.706)/0.001721;
	if (ABSDIFF(s->internal_temp, temp) > 0.2) {
		s->internal_temp = temp;
		//printf("\r\nTemp = %f C\r\n", temp);  
	}
*/
	LoopButton(s);
#ifdef	DEBUG
	input_loop();
#endif
}