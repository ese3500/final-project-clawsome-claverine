/*
 * ClawsomeClaverine.c
 *
 * Created: 4/7/2024 10:45:45 AM
 * Author : daria
 */ 



#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

// #define INPUT_DEV wifi_input
// #define INPUT_DEV wifi_input

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lib\uart.h"
#include "lib\ST7735.h"
#include "lib\LCD_GFX.h"

#include <stdio.h>

volatile char String[25]; //printing out message

void Initialize()
{
	// Initialization LCD Screen
	lcd_init();
	// Initialization USART Connection for debugging
	UART_init(BAUD_PRESCALER);
	// Initialization
	cli();
	sei();
}

int main(void)
{
	// Initialization
	Initialize();
	
	// Debug USART Printing test
	sprintf(String,"Wellcome to Clawesome Claverine \n");
	UART_putstring(String);
	// Code

	while (1);
}

