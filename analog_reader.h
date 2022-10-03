#ifndef	analog_reader_h
#define	analog_reader_h

#define	CLOCK_DIV 		1
#define	NSAMP			(1024)
#define	CAPTURE_CHANNEL	2

#ifdef	analog_reader_c
		int			reading16[4] = {0, 0, 0, 0};
		uint64_t		samples = 0;
#else
extern	int			reading16[4];
extern	uint64_t		samples;
#endif

void	core1_analog(void);
float	NTCTemp(int Adc, int RSeries, int RefT, int RefR, int B);

#endif