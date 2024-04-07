/*
 * ClawsomeClaverine.c
 *
 * Created: 4/7/2024 10:45:45 AM
 * Author : daria
 */ 



#define F_CPU                16000000UL   // 16MHz clock
#define USART_BAUD_RATE      9600
#define USART_BAUD_PRESCALER (((F_CPU / (USART_BAUD_RATE * 16UL))) - 1)

// #define INPUT_DEV wifi_input
// #define INPUT_DEV wifi_input

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lib\uart.h"
#include "lib\ST7735.h"
#include "lib\LCD_GFX.h"

#include <stdio.h>

void Initialize()
{
	UART_init(USART_BAUD_PRESCALER);
	lcd_init();
	cli();
	sei();
}

int main(void)
{
    /* Replace with your application code */
	
	char String[25];
	sprintf(String,"Hello World\n");
	UART_putstring(String);
    while (1);
}

