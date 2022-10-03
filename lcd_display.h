#ifndef	lcd_display_h
#define	lcd_display_h

#ifdef	lcd_display_c
#else
#endif

void lcd_set_cursor(int line, int position);
void lcd_string(const char *s);
void	lcd_init(void);

#endif