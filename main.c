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
volatile uint8_t red_button_pressed = 0;
volatile uint8_t object_detected = 0;

char *string;

// Enumeration for game modes
// enum GameMode {
//     SLEEP,
// 	PLAY,
//     WIN,
//     LOSS
// };

// Global variable to store the current game mode
/*enum GameMode game_mode = SLEEP;*/

#include <stdio.h>

char *countdown[] = {
	"60.0", "59.5", "59.0", "58.5", "58.0", "57.5", "57.0", "56.5", "56.0", "55.5",
	"55.0", "54.5", "54.0", "53.5", "53.0", "52.5", "52.0", "51.5", "51.0", "50.5",
	"50.0", "49.5", "49.0", "48.5", "48.0", "47.5", "47.0", "46.5", "46.0", "45.5",
	"45.0", "44.5", "44.0", "43.5", "43.0", "42.5", "42.0", "41.5", "41.0", "40.5",
	"40.0", "39.5", "39.0", "38.5", "38.0", "37.5", "37.0", "36.5", "36.0", "35.5",
	"35.0", "34.5", "34.0", "33.5", "33.0", "32.5", "32.0", "31.5", "31.0", "30.5",
	"30.0", "29.5", "29.0", "28.5", "28.0", "27.5", "27.0", "26.5", "26.0", "25.5",
	"25.0", "24.5", "24.0", "23.5", "23.0", "22.5", "22.0", "21.5", "21.0", "20.5",
	"20.0", "19.5", "19.0", "18.5", "18.0", "17.5", "17.0", "16.5", "16.0", "15.5",
	"15.0", "14.5", "14.0", "13.5", "13.0", "12.5", "12.0", "11.5", "11.0", "10.5",
	"10.0", "09.5", "09.0", "08.5", "08.0", "07.5", "07.0", "06.5", "06.0", "05.5",
	"05.0", "04.5", "04.0", "03.5", "03.0", "02.5", "02.0", "01.5", "01.0", "00.5",
	"00.0"
};

char mozart[] = {
	'G', 'G', 'G', '#', 'E', 'E', 'F', 'D', '#', 'B', 'B', 'C', 'A', '#',
	'A', 'A', 'B', 'G', '#', 'D', 'D', 'E', 'C', '#', 'B', 'B', 'C', 'A', '#',
	'G', 'G', 'F', 'D', '#', 'E', 'E', 'D', 'C', '#', 'A', 'A', 'B', 'G', '#',
	'D', 'D', 'E', 'C', '#', 'B', 'B', 'A', '#', 'G', 'G', 'F', 'D', '#',
	'E', 'E', 'D', 'C', '#', 'A', 'A', 'G', '#', 'G', 'G', 'F', 'D', '#',
	'E', 'E', 'D', 'C', '#', 'A', 'A', 'G', '#', 'G', 'G', 'F', 'D', '#',
	'E', 'E', 'D', 'C', '#', 'A', 'A', 'G', '#', 'G', 'G', 'F', 'D', '#',
	'E', 'E', 'D', 'C', '#', 'A', 'A', 'B', 'G', '#', 'D', 'D', 'E', 'C', '#',
	'B', 'B', 'A', '#', 'G', 'G', 'F', 'D', '#', 'E', 'E', 'D', 'C', '#',
	'A', 'A', 'B', 'G', '#', 'D', 'D', 'E', 'C', '#', 'B', 'B', 'A', '#',
	'0'  // Null-terminating character
}; //Mozart

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
		drive_motor_forth();
        
    }else if(adc_value_leftright>950)
    {
// 			drive_motor_forth();
		drive_motor_back();
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

void setup_sound()
{
	DDRD |= (1 << PIND2);
	PORTD |= (1<<PORTD2);

	// Set Timer3 in Fast PWM mode, non-inverted
	TCCR3A |= (1 << COM3B1);
	TCCR3A |= (1 << COM3A1);
	TCCR3A &= ~(1<<WGM31);
	TCCR3A |= (1<<WGM30);
	TCCR3B &= ~(1 << WGM32);
	TCCR3B |= (1 << WGM33);
	TCCR3B |= (1 << CS30);  // No prescaling
	TCCR3B |= (1 << CS31);
	TCCR3B &= ~(1 << CS32);

	// Set the duty cycle (OCR3B value). For a 50% duty cycle square wave,
	// set this to half the maximum counter value.
	OCR3A = 127;
	OCR3B = 0;
}

void stop_sound()
{
	OCR3A = 127;
	OCR3B = 0;
}

void play_sound(char tone)
{	
	switch(tone)
	{
		case 'C':
			OCR3A = 239;
			OCR3B = OCR3A/2;
			break;
		case 'D':
			OCR3A = 213;
			OCR3B = OCR3A/2;
			break;
		case 'E':
			OCR3A = 190;
			OCR3B = OCR3A/2;
			break;
		case 'F':
			OCR3A = 179;
			OCR3B = OCR3A/2;
			break;
		case 'G':
			OCR3A = 159;
			OCR3B = OCR3A/2;
			break;
		case 'A':
			OCR3A = 142;
			OCR3B = OCR3A/2;
			break;
		case 'B':
			OCR3A = 127;
			OCR3B = OCR3A/2;
			break;
		default:
			OCR3A = 127;
			OCR3B = 0;
			break;
	}
	
// 	_delay_ms(125);
// 	OCR3B = 	
//	247 = 512
//	C 523 = 239
//	D 587 = 213
//  E 659 = 190
//  F 698 = 179
//  G 784 = 159
//  A 880 = 142
//  B 988 = 127
}

ISR(PCINT3_vect) {
	
// 
// 	sprintf(String,"enter ISR\n");
// 	UART_putstring(String);

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
	
	if (PINE & (1 << PINE1)) {
		// PE0 went from low to high (rising edge)
		right_boundary_hit = 1;
		// 		sprintf(String,"%u \n", left_boundary_hit);
		// 		UART_putstring(String);
		} else {
		// PE0 went from high to low (falling edge)
		right_boundary_hit = 0;
		// 		sprintf(String,"falling  %u \n", left_boundary_hit);
		// 		UART_putstring(String);
	}
	
	if (PINE & (1 << PINE2)) {
		// PE0 went from low to high (rising edge)
		red_button_pressed = 1;
// 				sprintf(String,"%u \n", left_boundary_hit);
// 		 		UART_putstring(String);
		} else {
		// PE0 went from high to low (falling edge)
		red_button_pressed = 0;
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
	
	setup_sound();
	
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
		 
		 // red button
// 		DDRB &= ~(1<<DDB4);
// 		PINB |= (1<<PINB4);
		 

	set_up_interrupt();
//  	set_up_rightleft_sensors();
	
	setUpADC();
	
	sei();
	
	// 	UART_init(BAUD_PRESCALER);
}

long_delay(uint8_t duration)
{
	for(uint8_t i=0;i<duration;++i)
	{
		_delay_ms(500);
	}
}

draw_frame()
{
	LCD_setScreen(BLACK);
	string = "CLAWSOME CLAVERINE";
	LCD_drawString(34, 7, string, GREEN, BLACK);
}

void loading_screen()
{
	LCD_setScreen(YELLOW);
	char *string;
	string = "Loading...";
	LCD_drawString(10, 5, string, BLUE, YELLOW);
	for(uint8_t i = 0; i<14;++i)
	{
		LCD_drawBlock(10+i*10,20,15+i*10,114,BLUE);
		_delay_ms(200);
	}
}

void start_countdown(uint16_t front, uint16_t back)
{
	LCD_drawBlock(1,19,158,127,back);
	string = "Game Starts in";
	LCD_drawString(55, 55, string, front, back);
	string = "3.0";
	_delay_ms(500);
	LCD_drawString(55, 65, string, front, back);
	string = "2.5";
	_delay_ms(500);
	LCD_drawString(55, 65, string, front, back);
	string = "2.0";
	_delay_ms(500);
	LCD_drawString(55, 65, string, front, back);
	string = "1.5";
	_delay_ms(500);
	LCD_drawString(55, 65, string, front, back);
	string = "1.0";
	_delay_ms(500);
	LCD_drawString(55, 65, string, front, back);
	_delay_ms(500);
	string = "0.5";
	LCD_drawString(55, 65, string, front, back);
	_delay_ms(500);
	string = "Goo";
	LCD_drawString(55, 65, string, front, back);
	_delay_ms(500);
}

void loosing_game()
{
	draw_frame();
	LCD_drawBlock(1,19,158,127,RED);
	string = "You loose!";
	LCD_drawString(60, 55, string, BLACK, RED);
	play_sound('E');
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
	play_sound('F');
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
	play_sound('G');
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
	play_sound('B');
	_delay_ms(500);
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
}

winning_game()
{
	draw_frame();
	LCD_drawBlock(1,19,158,127,RED);
	string = "You win!";
	LCD_drawString(60, 55, string, BLACK, RED);
	play_sound('G');
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
	play_sound('G');
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
	play_sound('G');
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
	play_sound('D');
	_delay_ms(500);
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
}

void play_easy()
{
	LCD_drawBlock(1,19,158,127,MAGENTA);
	string = "EASY MODE";
	LCD_drawString(45, 45, string, BLACK, MAGENTA);
	string = "down";
	LCD_drawString(40, 55, string, YELLOW, MAGENTA);
	string = "up";
	LCD_drawString(80, 55, string, BLUE, MAGENTA);

	
	uint8_t l = 0;
	
	for(uint8_t j =0;j<121 && !object_detected;++j)
	{
		string = countdown[j];
		LCD_drawString(60, 100, string, BLUE, MAGENTA);
		
		for(uint8_t i=0;i<2 && !object_detected;++i)
		{
			play_sound(mozart[l]);
			if(mozart[l]=='0')
			{
				l=0;
			}else
			{
				++l;
			}
			for(uint16_t k=0;k<45535 && !object_detected;++k)
			{
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
				if(adc_value_leftright<50)
				{
					drive_motor_forth();
					
				}else if(adc_value_leftright>950)
				{
					drive_motor_back();
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
				if(!red_button_pressed)
				{
					object_detected = 1;
				}
			}
		}
	}
	stop_sound();
	if(object_detected)
	{
		winning_game();
	}else
	{
		loosing_game();
	}
	object_detected = 0;
}

void play_hard()
{
	LCD_drawBlock(1,19,158,127,MAGENTA);
	string = "CRAZY MODE";
	LCD_drawString(45, 45, string, BLACK, MAGENTA);

	
	uint8_t l = 0;
	
	for(uint8_t j =0;j<121 && !object_detected;++j)
	{
		string = countdown[j];
		LCD_drawString(60, 100, string, BLUE, MAGENTA);
		
		for(uint8_t i=0; i<2 && !object_detected ;++i)
		{
			play_sound(mozart[l]);
			if(mozart[l]=='0')
			{
				l=0;
			}else
			{
				++l;
			}
			for(uint16_t k=0; k<5000 && !object_detected;++k)
			{
				if((adc_value_backforth<50) && !right_boundary_hit)
				{
					drive_motor_up();
				}else if((adc_value_backforth>950) && !left_boundary_hit)
				{
					drive_motor_down();
					
				}else
				{
					motor_stop_updown();
				}
				if(adc_value_leftright<50)
				{
					drive_motor_right();
					
				}else if(adc_value_leftright>950)
				{
					drive_motor_left();
					
				}else
				{
					motor_stop_rightleft();
				}
				if(!(PIND & (1<<PIND1)))
				{
					drive_motor_forth();
				}else if (!(PIND & (1<<PIND7)))
				{
					drive_motor_back();
				}else
				{
					motor_stop_backforth();
				}
				if(!red_button_pressed)
				{
					object_detected = 1;
				}
			}
		}
	}
	stop_sound();
	if(object_detected)
	{
		winning_game();
	}else
	{
		loosing_game();
	}
	object_detected = 0;
}

play_machine()
{
	draw_frame();
	
	if(1)
	{
		LCD_drawBlock(1,19,158,127,BLACK);
		string = "Welcome to";
		LCD_drawString(50, 45, string, WHITE, BLACK);
		string = "Clawsome Claverine";
		LCD_drawString(34, 55, string, WHITE, BLACK);
		string = "$ Insert coin to play $";
		LCD_drawString(20, 65, string, WHITE, BLACK);
	}
	
	while(!red_button_pressed);
	while(1)
	{
		if(!red_button_pressed)
		{
			_delay_ms(10);
			if(!red_button_pressed)
			{
				break;
			}
		}
	}
	_delay_ms(500);
	
	if(1)
	{
		LCD_drawBlock(1,19,158,127,BLACK);
		string = "Choose Mode";
		LCD_drawString(50, 45, string, WHITE, BLACK);
		string = "EASY";
		LCD_drawString(55, 55, string, YELLOW, BLACK);
		string = "CRAZY";
		LCD_drawString(55, 65, string, BLUE, BLACK);
		string = "SURRENDER";
		LCD_drawString(52, 75, string, RED, BLACK);
		string = "(And donate money to charity)";
		LCD_drawString(5, 83, string, RED, BLACK);
		string = "(Press button with given color)";
		LCD_drawString(5, 100, string, BLACK, BLACK);
		while(1)
		{
			if(!(PIND & (1<<PIND7)) || !(PIND & (1<<PIND1)) || !red_button_pressed)
			{
				_delay_ms(10);
				if(!(PIND & (1<<PIND7))) // yellow
			{
				loading_screen();
				draw_frame();
				start_countdown(BLACK, MAGENTA);
				play_easy();
				break;
			}else if(!(PIND & (1<<PIND1))) // blue
			{
				loading_screen();
				draw_frame();
				start_countdown(BLACK, MAGENTA);
				play_hard();
				break;
			}else if(!red_button_pressed) // red
			{
				LCD_drawBlock(1,19,158,127,CYAN);
				string = "THANK YOU";
				LCD_drawString(60, 55, string, BLACK, CYAN);
				string = "Enjoy your day!!";
				LCD_drawString(50, 65, string, BLACK, CYAN);
				long_delay(4);
				break;
			}
			}
		}
	}
	
	
	
	
	
	
// 	LCD_drawBlock(1,19,158,127,BLACK);
}

int main(void)
{
	// Initialization
	Initialize();
	LCD_setScreen(BLUE);
//  	UART_init(BAUD_PRESCALER);
	while(1)
	{
		play_machine();
	}

	while (0)
	{
// 		char notes[] = {
// 			'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D',
// 			'A', 'C', 'D', 'D', 'D', 'E', 'F', 'F', 'F', 'G',
// 			'E', 'E', 'D', 'C', 'C', 'D',
// 			'A', 'C', 'D', 'D', 'D', 'E', 'F', 'F', 'F', 'G',
// 			'E', 'E', 'D', 'C', 'D',
// 			'A', 'C', 'D', 'D', 'D', 'F', 'G', 'G', 'G', 'A',
// 			'A', '#', 'A', '#', 'A', 'G', 'A',
// 			'D', 'D', 'E', 'F', 'F', 'F', 'G',
// 			'A', 'D', 'D', 'F', 'E', 'E', 'F', 'D',
// 			'E', 'A', 'C', 'D', 'D', 'D', 'E', 'F', 'F', 'F', 'G',
// 			'E', 'E', 'D', 'C', 'C', 'D',
// 			'A', 'C', 'D', 'D', 'D', 'E', 'F', 'F', 'F', 'G',
// 			'E', 'E', 'D', 'C', 'D',
// 			'A', 'C', 'D', 'D', 'D', 'F', 'G', 'G', 'G', 'A',
// 			'A', '#', 'A', '#', 'A', 'G',
// 			'A', 'D', 'D', 'E', 'F', 'F', 'G',
// 			'A', 'D', 'D', 'F', 'E', 'E', 'F', 'D',
// 			'D', 'D', 'E', 'F', 'F', 'F', 'G',
// 			'A', 'F', 'F', 'D', 'A',
// 			'D', 'N', 'D', 'D', 'D', 'D', 'D', 'P',
// 			'P', 'G', 'G', 'M', 'G', 'G', 'R',
// 			'D', 'N', 'D', 'D', 'R', 'D', 'P',
// 			'P', 'G', 'G', 'M', 'G', 'G', 'R'			//Pirrates of the carrebian
			
		char notes[] = {
			'G', 'G', 'G', '#', 'E', 'E', 'F', 'D', '#', 'B', 'B', 'C', 'A', '#',
			'A', 'A', 'B', 'G', '#', 'D', 'D', 'E', 'C', '#', 'B', 'B', 'C', 'A', '#',
			'G', 'G', 'F', 'D', '#', 'E', 'E', 'D', 'C', '#', 'A', 'A', 'B', 'G', '#',
			'D', 'D', 'E', 'C', '#', 'B', 'B', 'A', '#', 'G', 'G', 'F', 'D', '#',
			'E', 'E', 'D', 'C', '#', 'A', 'A', 'G', '#', 'G', 'G', 'F', 'D', '#',
			'E', 'E', 'D', 'C', '#', 'A', 'A', 'G', '#', 'G', 'G', 'F', 'D', '#',
			'E', 'E', 'D', 'C', '#', 'A', 'A', 'G', '#', 'G', 'G', 'F', 'D', '#',
			'E', 'E', 'D', 'C', '#', 'A', 'A', 'B', 'G', '#', 'D', 'D', 'E', 'C', '#',
			'B', 'B', 'A', '#', 'G', 'G', 'F', 'D', '#', 'E', 'E', 'D', 'C', '#',
			'A', 'A', 'B', 'G', '#', 'D', 'D', 'E', 'C', '#', 'B', 'B', 'A', '#',
			'0'  // Null-terminating character
			}; //Mozart

// 		play_sound('A');
// 		_delay_ms(250);
// 		play_sound('B');
// 		_delay_ms(250);
		uint8_t i = 0;
		while(1)
		{
			play_sound(notes[i]);
			_delay_ms(125);
			if(notes[i]=='0')
			{
				i = 0;
			}
			++i;
		}
	}
	
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
// 	game_mode = PLAY;
// 	sprintf(String,"setting game mode to PLAY \n");
// 	UART_putstring(String);
	
	while (1)
	{
		drive_motors();
	}

// 	while(1)
// 	{	
// 		// Check the current game mode and perform actions accordingly
// 		switch (game_mode) {
// 			case SLEEP:
// 				sprintf(String,"SLEEP mode\n");
// 				UART_putstring(String);
// 				break;
// 			case PLAY:
// 				sprintf(String,"PLAY mode, driving motors\n");
// 				UART_putstring(String);
// 				drive_motors();
// 				break;
// 			case WIN:
// 				sprintf(String,"WIN mode starts, turn on LED lights\n");
// 				UART_putstring(String);
// 				/* Turn on LED lights */
// 				_delay_ms(500);
// 				sprintf(String,"WIN mode ends, turn off LED lights\n");
// 				UART_putstring(String);
// 				sprintf(String,"Setting game mode to SLEEP\n");
// 				UART_putstring(String);
// 				game_mode = SLEEP;
// 				break;
// 			case LOSS:
// 				sprintf(String,"LOSS mode\n");
// 				UART_putstring(String);
// 				break;
// 			default:	
// 				break;
// 		}
	/*}*/
	
	while (1);
}