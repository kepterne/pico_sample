#define	analog_reader_c

#include	<math.h>

#include	"hardware/adc.h"
#include	"hardware/dma.h"

#include	"analog_reader.h"

float	NTCTemp(int Adc, int RSeries, int RefT, int RefR, int B) {
	float Vi = Adc * (3.3 / 4095.0);
	float R = (Vi * RSeries) / (3.3 - Vi);
	float To = 273.15 + RefT; 
	float T =  1 / ((1.0 / To) + ((log(R / RefR)) / B));
	return T - 273.15; 
}
extern	int	anok;

void	core1_analog(void) {
	dma_channel_config 	cfg0, cfg1;
	int				cur_buf = 0;
	uint16_t			capture_buf[2][NSAMP];
	int				channel = 0, selected = 0;
	
	
	adc_init();
	gpio_init(25);
	gpio_set_dir(25, 1);
	gpio_put(25, 1);
	adc_gpio_init(26 + 0);
	adc_gpio_init(26 + 1);
	adc_gpio_init(26 + 2);
	adc_gpio_init(26 + 3);
    adc_set_temp_sensor_enabled(true);
	adc_select_input(0);
	adc_set_round_robin(1 | 2 | 4 | 8 | 16);
	adc_fifo_setup(
		true,  // Write each completed conversion to the sample FIFO
		true,  // Enable DMA data request (DREQ)
		1,     // DREQ (and IRQ) asserted when at least 1 sample present
		false, // We won't see the ERR bit because of 8 bit reads; disable.
		false   // Shift each sample to 8 bits when pushing to FIFO
	);
	adc_set_clkdiv(CLOCK_DIV);
	
	uint 	dma_chan0 = dma_claim_unused_channel(true);
	uint	dma_chan1 = dma_claim_unused_channel(true);
	cfg0 = dma_channel_get_default_config(dma_chan0);
	cfg1 = dma_channel_get_default_config(dma_chan1);

	// Reading from constant address, writing to incrementing byte addresses
	channel_config_set_transfer_data_size(&cfg0, DMA_SIZE_16);
	channel_config_set_transfer_data_size(&cfg1, DMA_SIZE_16);
	
	channel_config_set_read_increment(&cfg0, false);
	channel_config_set_read_increment(&cfg1, false);

	channel_config_set_write_increment(&cfg0, true);
	channel_config_set_write_increment(&cfg1, true);
	// Pace transfers based on availability of ADC samples
	channel_config_set_dreq(&cfg0, DREQ_ADC);
	channel_config_set_dreq(&cfg1, DREQ_ADC);

	channel_config_set_chain_to(&cfg0, dma_chan1);
	channel_config_set_chain_to(&cfg1, dma_chan0);
	adc_fifo_drain();
	adc_run(false);

	dma_channel_configure(dma_chan1, &cfg1,
		capture_buf[1],   // dst
		&adc_hw->fifo, // src
		NSAMP,         // transfer count
		false           // start immediately
	);
	
	dma_channel_configure(dma_chan0, &cfg0,
		capture_buf[0],   // dst
		&adc_hw->fifo, 	// src
		NSAMP,         	// transfer count
		false           	// start immediately
	);
	dma_channel_start(dma_chan0);
	adc_run(true);
	
	for (samples = 0, cur_buf = 0; !anok; cur_buf ^= 1) {
		dma_channel_wait_for_finish_blocking(cur_buf ? dma_chan1 : dma_chan0);
		dma_hw->ch[cur_buf ? dma_chan1 : dma_chan0].al3_write_addr = (io_rw_32) capture_buf[cur_buf];

		uint32_t	total[5] = {0, 0, 0, 0, 0};
		for (int i = 0; i < NSAMP; i++, samples++) {
			//switch (i % 4) {
			//case 0:

			total[i % 5] += capture_buf[cur_buf][i];
			//	break;
			//}
		}
		for (int i = 0; i < 5; i++)
			reading16[i] = total[i] / (NSAMP / 5);
		
		
		//printf("\r\nDMA %d finished [%d]\r\n", cur_buf, total);
	}
	adc_fifo_drain();
	adc_run(false);
	dma_channel_abort(dma_chan0);
	dma_channel_abort(dma_chan1);
	dma_channel_unclaim(dma_chan0);
	dma_channel_unclaim(dma_chan1);
	
	anok = 2;
}