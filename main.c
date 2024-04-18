/*
 * ClawsomeClaverine.c
 *
 * Created: 4/7/2024 10:45:45 AM
 * Author : daria
 */ 

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

#define ADC_LEFTRIGHT_CHANNEL 0x00
#define ADC_BACKFORTH_CHANNEL 0x01

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

volatile uint8_t right_boundary_hit = 0;
volatile uint8_t left_boundary_hit = 0;

// Enumeration for game modes
enum GameMode {
    SLEEP,
	PLAY,
    WIN,
    LOSS
};

// Global variable to store the current game mode
enum GameMode game_mode = SLEEP;

#include <stdio.h>

volatile char String[25]; //printing out message

void selectADCchannel(uint8_t channel){
    //0xE0 is 11100000
    //0x1F is 00011111
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);
}

ISR(ADC_vect) {
    //get current channel
    uint8_t currentChannel = ADMUX & 0x0F;

    //set output compare registers for current channel, and switch to next channel
    switch(currentChannel){
        case ADC_LEFTRIGHT_CHANNEL: 
			adc_value_leftright = ADC;
// 			sprintf(String,"ADC left/right channel read %u \n", adc_value_leftright);
// 			UART_putstring(String);
			selectADCchannel(ADC_BACKFORTH_CHANNEL);
            break;
        case ADC_BACKFORTH_CHANNEL: 
			adc_value_backforth = ADC;
// 			sprintf(String,"ADC forward/backward channel read %u \n", adc_value_backforth);
// 			UART_putstring(String);
			selectADCchannel(ADC_LEFTRIGHT_CHANNEL);
            break;
    }

    //restart conversion
    ADCSRA |= 1 << ADSC;
}

void setUpADC() {
	 // Setup for ADC (10bit = 0-1023)
	 // Clear power reduction bit for ADC
	 PRR0 &= ~(1 << PRADC);

	 // Select Vref = AVcc
	 ADMUX |= (1 << REFS0);
	 ADMUX &= ~(1 << REFS1);

	// //enable result shift to ADCH to only read ADCH (8 bits)
    // ADMUX |= 1<<ADLAR;

	 // Set the ADC clock div by 128
	 // 16M/128=125kHz
	 ADCSRA |= (1 << ADPS0);
	 ADCSRA |= (1 << ADPS1);
	 ADCSRA |= (1 << ADPS2);

	 // Select Channel ADC0 (pin C0)
	 ADMUX &= ~(1 << MUX0);
	 ADMUX &= ~(1 << MUX1);
	 ADMUX &= ~(1 << MUX2);
	 ADMUX &= ~(1 << MUX3);

	 ADCSRA |= (1 << ADATE);   // Autotriggering of ADC

	// Free running mode ADTS[2:0] = 000
	//  ADCSRB &= ~(1 << ADTS0);
	//  ADCSRB &= ~(1 << ADTS1);
	//  ADCSRB &= ~(1 << ADTS2);

	//enable interrupt
    ADCSRA |= 1<<ADIE;

	// Disable digital input buffer on ADC pin
	DIDR0 |= (1 << ADC0D);

	// Enable ADC
	ADCSRA |= (1 << ADEN);

	// Start conversion
	ADCSRA |= (1 << ADSC);	
}

// void set_up_catch_sensors() {
// 	// PD7 as input, with pull-up resistors enabled
// //     DDRD &= ~(1 << PIND7);
// //     PORTD |= (1 << PIND7);
// // 
// //     EIMSK |= (1 << INT2); // enable external interrupts
// // 
// // 	EICRA |= (1 << ISC21); // trigger on falling edge
// }

// ISR for INT2 (PD7)
// ISR(INT2_vect) {
// 	sprintf(String,"ISR PD7\n");
// 	UART_putstring(String);
//     // check if PD7 is low (falling edge)
//     if (!(PIND & (1 << PIND7))) {
// 		sprintf(String,"PD7 triggered, enter winning mode \n");
// 		UART_putstring(String);
//         game_mode = WIN;
//     }
// }

// void set_up_rightleft_sensors() {
// // 	// PD0 and PD1 as input, with pull-up resistors enabled
// //  	DDRD &= ~(1 << PIND0) & ~(1 << PIND1);
// //  	PORTD |= (1 << PIND0) | (1 << PIND1);
// 
// // 	PCICR |= (1 << PCIE2); // Enable PCINT group 2
// // 	PCMSK2 |= (1 << PCINT16) | (1 << PCINT17); // Enable PD0 (PCINT16) and PD1 (PCINT17)
// }

// ISR for INT0 (PD0)
// ISR(PCINT2_vect) {
// // 	if(PIND & (1<<PIND0))
// // 	{
// // 		sprintf(String,"ISR PD0\n");
// // 		UART_putstring(String);
// // 		_delay_ms(50);
// // 	}
// // 	sprintf(String,"ISR PD0\n");
// // 	UART_putstring(String);
// //     if (!(PIND & (1 << PIND0))) {
// // 		sprintf(String,"PD0 triggered, stop left right sensors\n");
// // 		UART_putstring(String);
// //         motor_stop_rightleft();
// //     }
// 	
// 	// 	sprintf(String,"ISR PD1\n");
// 	// 	UART_putstring(String);
// // 	if ((PIND & (1 << PIND0))) {
// // 		right_boundary_hit = 1;
// // 		// 		sprintf(String,"PD1 triggered, stop left right sensors\n");
// // 		// 		UART_putstring(String);
// // 		//         motor_stop_rightleft();
// // 	}else
// // 	{
// // 		right_boundary_hit = 0;
// // 	}
// }
// 
// // ISR for INT1 (PD1)
// ISR(INT1_vect) {
// 	sprintf(String,"ISR PD1\n");
// 	UART_putstring(String);
// 	_delay_ms(50);
// //     if ((PIND & (1 << PIND1))) {
// // 		right_boundary_hit = 1;
// // // 		sprintf(String,"PD1 triggered, stop left right sensors\n");
// // // 		UART_putstring(String);
// // //         motor_stop_rightleft();
// //     }else
// // 	{
// // 		right_boundary_hit = 0;
// // 	}
// }

void motor_stop_backforth()
{
		DDRD |= (1<<DDD3);
	 	DDRD |= (1<<DDD5);
// 	PORTC &= ~(1<<PORTC2); //right
// 	PORTC &= ~(1<<PORTC3); //left
	// 	PORTC &= ~(1<<PORTC4); //back
	// 	PORTC &= ~(1<<PORTC5); //forth
	 	PORTD &= ~(1<<PORTD3);
	 	PORTD &= ~(1<<PORTD4); //back
	 	PORTD &= ~(1<<PORTD5); //forth
}

void motor_stop_rightleft()
{
// 	DDRD |= (1<<DDD3);
// 	DDRD |= (1<<DDD5);
	PORTC &= ~(1<<PORTC2); //right
	PORTC &= ~(1<<PORTC3); //left
// 	PORTC &= ~(1<<PORTC4); //back
// 	PORTC &= ~(1<<PORTC5); //forth
// 	PORTD &= ~(1<<PORTD3);
// 	PORTD &= ~(1<<PORTD4);
// 	PORTD &= ~(1<<PORTD5);
}

void motor_stop_updown()
{
	// 	DDRD |= (1<<DDD3);
	// 	DDRD |= (1<<DDD5);
// 	PORTC &= ~(1<<PORTC2); //right
// 	PORTC &= ~(1<<PORTC3); //left
	PORTC &= ~(1<<PORTC4); //up
	PORTC &= ~(1<<PORTC5); //down
// 	PORTD &= ~(1<<PORTD3);
// 	PORTD &= ~(1<<PORTD4);
// 	PORTD &= ~(1<<PORTD5);
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
// 	sprintf(String,"gone through\n");
// 	UART_putstring(String);
// 	PORTD |= (1<<PORTD3);
// 	PORTD |= (1<<PORTD5);
// 	DDRD &= ~(1<<DDD5);
// 	DDRD |= (1<<DDD3);
// 	PIND &= ~(1<<PIND5);
// 	PORTD |= (1<<PORTD5);
	PORTD |= (1<<PORTD4);
	PORTD &= ~(1<<PORTD5);
}

void drive_motor_forth()
{
// 	sprintf(String,"gone through\n");
// 	UART_putstring(String);
// 	PORTD &= ~(1<<PORTD3);
// 	PORTD &= ~(1<<PORTD5);
//  	DDRD &= ~(1<<DDD3);
// 	DDRD |= (1<<DDD5);
	PORTD |= (1<<PORTD5);
	PORTD &= ~(1<<PORTD4);
}

void drive_motors() {
    if((adc_value_backforth<50) && !right_boundary_hit)
    {
        drive_motor_right();
    }else if((adc_value_backforth>950) && !left_boundary_hit)
    {
		drive_motor_left();
    }else
    {
        motor_stop_rightleft();
    }
    
// 		_delay_ms(500);
    
    if(adc_value_leftright<50)
    {
// 			drive_motor_back();
        drive_motor_back();
    }else if(adc_value_leftright>950)
    {
// 			drive_motor_forth();
        drive_motor_forth();
        }else
    {
        motor_stop_backforth();
    }
    
    if(!(PIND & (1<<PIND1)))
    {
        drive_motor_down();
    }else if (!(PIND & (1<<PIND7)))
    {
		drive_motor_up();
    }else
    {
        motor_stop_updown();
    }
}

void set_up_interrupt() {
	sprintf(String,"setting up interrupt sensor\n");
	UART_putstring(String);
	
	DDRE &= ~(1<<DDRE0);
	DDRE &= ~(1<<DDRE1);
	DDRE &= ~(1<<DDRE2);
	DDRE &= ~(1<<DDRE3);

	// Enable Pin Change Interrupts for Port E
	PCICR |= (1 << PCIE3);
	
	// Enable Pin Change Interrupts for specific pins in Port E
	PCMSK3 |= (1 << PCINT24);  // PE0
	PCMSK3 |= (1 << PCINT25);  // PE1
	PCMSK3 |= (1 << PCINT26);  // PE2
	PCMSK3 |= (1 << PCINT27);  // PE3

	sei();  // Enable global interrupts

	sprintf(String,"finish set up interrupt sensor\n");
	UART_putstring(String);
}

ISR(PCINT3_vect) {
	
// 
	sprintf(String,"enter ISR\n");
	UART_putstring(String);

//     uint8_t current_pin_state = PINE;
    
    // Check if PE0 (PCINT24) has changed
	if (PINE & (1 << PINE0)) {
		// PE0 went from low to high (rising edge)
		left_boundary_hit = 1;
// 		sprintf(String,"%u \n", left_boundary_hit);
// 		UART_putstring(String);
	} else {
		// PE0 went from high to low (falling edge)
		left_boundary_hit = 0;
// 		sprintf(String,"falling  %u \n", left_boundary_hit);
// 		UART_putstring(String);
	}

// 	sprintf(String,"leave ISR\n");
// 	UART_putstring(String);
}


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
	
	
	// buttons up down
	
// 		PORTD &= ~(1<<PORTD0);
// 		PORTD &= ~(1<<PORTD0);
	
		DDRD |= (1<<DDD7);
		DDRD |= (1<<DDD1);
		PORTD |= (1<<PORTD7);
		PORTD |= (1<<PORTD1);
		DDRD &= ~(1<<DDD7);
		DDRD &= ~(1<<DDD1);
	 	PIND |= (1<<PIND7);
	 	PIND |= (1<<PIND1);
		 

	set_up_interrupt();
//  	set_up_rightleft_sensors();
	
	setUpADC();
	
	sei();
	
	// 	UART_init(BAUD_PRESCALER);
}

int main(void)
{
	// Initialization
	Initialize();
//  	UART_init(BAUD_PRESCALER);
	
	// Debug USART Printing test
	char String[25];
	sprintf(String,"Wellcome to Clawesome Claverine \n");
	UART_putstring(String);
	_delay_ms(500);
// 	while(1){}
	// Code
	LCD_setScreen(GREEN);
	LCD_drawBlock(1,19,158,127,BLUE);
	sprintf(String,"gone through\n");
	UART_putstring(String);

    /* Set game mode, this could be an ISR from a button press, but 
	 *for now this is hardcoded to be PLAY mode */
	game_mode = PLAY;
	sprintf(String,"setting game mode to PLAY \n");
	UART_putstring(String);
	
	while (1)
	{
		drive_motors();
	}

	while(1)
	{	
		// Check the current game mode and perform actions accordingly
		switch (game_mode) {
			case SLEEP:
				sprintf(String,"SLEEP mode\n");
				UART_putstring(String);
				break;
			case PLAY:
				sprintf(String,"PLAY mode, driving motors\n");
				UART_putstring(String);
				drive_motors();
				break;
			case WIN:
				sprintf(String,"WIN mode starts, turn on LED lights\n");
				UART_putstring(String);
				/* Turn on LED lights */
				_delay_ms(500);
				sprintf(String,"WIN mode ends, turn off LED lights\n");
				UART_putstring(String);
				sprintf(String,"Setting game mode to SLEEP\n");
				UART_putstring(String);
				game_mode = SLEEP;
				break;
			case LOSS:
				sprintf(String,"LOSS mode\n");
				UART_putstring(String);
				break;
			default:	
				break;
		}
	}
	
	while (1);
}