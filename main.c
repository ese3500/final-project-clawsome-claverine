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
// #include "lib\LCD_GFX.c"

volatile uint16_t adc_value_backforth = 500;
volatile uint16_t adc_value_leftright = 500;

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
	
	// making ports output
// 	DDRC |= (1<<DDC0);
// 	DDRC |= (1<<DDC1);
	DDRC |= (1<<DDC2);
	DDRC |= (1<<DDC3);
	DDRC |= (1<<DDC4);
	DDRC |= (1<<DDC5);
	DDRD |= (1<<DDD3);
	DDRD |= (1<<DDD4);
	DDRD |= (1<<DDD5);
	
	// pulling all motor ports down initially
// 	PORTC &= ~(1<<PORTC0); //right
// 	PORTC &= ~(1<<PORTC1); //left
	PORTC &= ~(1<<PORTC2); //right
	PORTC &= ~(1<<PORTC3); //left
	PORTC &= ~(1<<PORTC4); //up
	PORTC &= ~(1<<PORTC5); //down
	PORTD &= ~(1<<PORTD3);
	PORTD &= ~(1<<PORTD4);
	PORTD &= ~(1<<PORTD5);
	//////////////////////////////// start ADC setup
// 	 // Setup for ADC (10bit = 0-1023)
// 	 // Clear power reduction bit for ADC
// 	 PRR0 &= ~(1 << PRADC);
// 
// 	 // Select Vref = AVcc
// 	 ADMUX |= (1 << REFS0);
// 	 ADMUX &= ~(1 << REFS1);
// 
// 	 // Set the ADC clock div by 128
// 	 // 16M/128=125kHz
// 	 ADCSRA |= (1 << ADPS0);
// 	 ADCSRA |= (1 << ADPS1);
// 	 ADCSRA |= (1 << ADPS2);
// 
// 	 // Select Channel ADC0 (pin C0)
// 	 ADMUX &= ~(1 << MUX0);
// 	 ADMUX &= ~(1 << MUX1);
// 	 ADMUX &= ~(1 << MUX2);
// 	 ADMUX &= ~(1 << MUX3);
// 
// 	 ADCSRA |= (1 << ADATE);   // Autotriggering of ADC
// 
// 	 // Free running mode ADTS[2:0] = 000
// 	 ADCSRB &= ~(1 << ADTS0);
// 	 ADCSRB &= ~(1 << ADTS1);
// 	 ADCSRB &= ~(1 << ADTS2);
// 
// 	 // Disable digital input buffer on ADC pin
// 	 DIDR0 |= (1 << ADC0D);
// 
// 	 // Enable ADC
// 	 ADCSRA |= (1 << ADEN);
// 
// 	 // Start conversion
// 	 ADCSRA |= (1 << ADSC	 
/////////////////////////////////////////////////////// end ADC
	
	sei();
}

// uint16_t read_adc (uint8_t channel)
// {
// 	ADMUX &= ~((1 << MUX0) | (1 << MUX1) | (1 << MUX2) | (1 << MUX3));
// 	
// 	if(channel){
// 		ADMUX |= (1 << MUX0);
// // 		sprintf(String,"ADC 1 read %u \n", ADC);
// // 		UART_putstring(String		
// // 		return ADC;
// 	}else
// 	{
// // 		sprintf(String,"ADC 0 read %u \n", ADC);
// // 		UART_putstring(String		
// // 		return ADC;
// 	}
// 	 ADCSRA |= (1 << ADSC);
// 	 
// 	 // Wait for conversion to complete
// 	 while (ADCSRA & (1 << ADSC))
// 	 {
// 		 // You can add some delay or other tasks here
// 	 }
// 	 
// 	 // Read ADC result (16-bit value, ADCL must be read first)
// 	 uint16_t adcValue = ADCL | (ADCH << 8);
// 	 
// 		 sprintf(String,"ADC 1 read %u \n", ADC);
//  		UART_putstring(String);
// 	 
// 	 return adcValue;
// }

void motor_stop()
{
// 	DDRD |= (1<<DDD3);
// 	DDRD |= (1<<DDD5);
	PORTC &= ~(1<<PORTC2); //right
	PORTC &= ~(1<<PORTC3); //left
	PORTC &= ~(1<<PORTC4); //back
	PORTC &= ~(1<<PORTC5); //forth
	PORTD &= ~(1<<PORTD3);
	PORTD &= ~(1<<PORTD4);
	PORTD &= ~(1<<PORTD5);
}

void drive_motor_right()
{
	PORTC |= (1<<PORTC2);
	PORTC &= ~(1<<PORTC3);
// 	_delay_ms(500);
// 	PORTC &= ~(1<<PORTC2); //right
// 	PORTC &= ~(1<<PORTC3); //left
}

void drive_motor_left()
{
	PORTC |= (1<<PORTC3);
	PORTC &= ~(1<<PORTC2);
// 	_delay_ms(500);
// 	PORTC &= ~(1<<PORTC2); //back
// 	PORTC &= ~(1<<PORTC3); //forth
}

void drive_motor_down()
{
	PORTC |= (1<<PORTC4);
	PORTC &= ~(1<<PORTC5);
}

void drive_motor_up()
{
	PORTC |= (1<<PORTC5);
	PORTC &= ~(1<<PORTC4);
}

void drive_motor_back()
{
	sprintf(String,"gone through\n");
	UART_putstring(String);
	DDRD &= (1<<DDD3);
	PORTD |= (1<<PORTD4);
	PORTD &= ~(1<<PORTD5);
}

void drive_motor_forth()
{
	sprintf(String,"gone through\n");
	UART_putstring(String);
	DDRD &= (1<<DDD5);
	PORTD |= (1<<PORTD3);
	PORTD &= ~(1<<PORTD4);
}

int main(void)
{
	// Initialization
	Initialize();
	
	// Debug USART Printing test
	sprintf(String,"Wellcome to Clawesome Claverine %u \n", ADC);
	UART_putstring(String);
	// Code
	LCD_setScreen(MAGENTA);
	LCD_drawBlock(1,19,158,127,BLUE);
	sprintf(String,"gone through\n");
	UART_putstring(String);
// 	while(1)
// 	{
// 		sprintf(String,"gone through %u \n",ADC);
// 		UART_putstring(String);
// 		drive_motor_right();
// 		motor_stop();
// 		_delay_ms(500);
// 		motor_stop();
// 		_delay_ms(500);
//  		drive_motor_left();
//  		_delay_ms(500);
//  		motor_stop();
// 		_delay_ms(500);


	while(1)
	{
		if(adc_value_leftright>950)
		{
			drive_motor_right();
		}else if(adc_value_leftright<50)
		{
			drive_motor_left();
		}else
		{
			motor_stop();
		}
		
		if(adc_value_backforth>950)
		{
// 			drive_motor_back();
			drive_motor_down();
		}else if(adc_value_backforth<50)
		{
// 			drive_motor_forth();
			drive_motor_up();
			}else
		{
			motor_stop();
		}
	}
	
	while (1);
}