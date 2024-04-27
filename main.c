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

volatile uint8_t light_mode = 0;

char *string;

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

void selectADCchannel(uint8_t channel){ //selects the ADC channel given as a variable (0 or 1 in our exa ple)
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);
}

ISR(ADC_vect) { //reads the ADC value and changes the channels
    //get current channel
    uint8_t currentChannel = ADMUX & 0x0F;

    //set output compare registers for current channel, and switch to next channel
    switch(currentChannel){
        case ADC_LEFTRIGHT_CHANNEL: 
			adc_value_leftright = ADC;
			selectADCchannel(ADC_BACKFORTH_CHANNEL);
            break;
        case ADC_BACKFORTH_CHANNEL: 
			adc_value_backforth = ADC;
			selectADCchannel(ADC_LEFTRIGHT_CHANNEL);
            break;
    }

    //restart conversion
    ADCSRA |= 1 << ADSC;
}

void setUpADC() { // setup of the ADC
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

	//enable interrupt
    ADCSRA |= 1<<ADIE;

	// Disable digital input buffer on ADC pin
	DIDR0 |= (1 << ADC0D);

	// Enable ADC
	ADCSRA |= (1 << ADEN);

	// Start conversion
	ADCSRA |= (1 << ADSC);	
}

void motor_stop_backforth() // stops the back forth motor
{
		DDRD |= (1<<DDD3);
	 	DDRD |= (1<<DDD5);
	 	PORTD &= ~(1<<PORTD3);
	 	PORTD &= ~(1<<PORTD4); //back
	 	PORTD &= ~(1<<PORTD5); //forth
}

void motor_stop_rightleft() // stops the right left motor
{
	PORTC &= ~(1<<PORTC2); //right
	PORTC &= ~(1<<PORTC3); //left
}

void motor_stop_updown() // stops the up down motor
{
	PORTC &= ~(1<<PORTC4); //up
	PORTC &= ~(1<<PORTC5); //down
}

void drive_motor_right() // drives motor right
{
	PORTC |= (1<<PORTC2);
	PORTC &= ~(1<<PORTC3);
}

void drive_motor_left() //drives motor left
{
	PORTC |= (1<<PORTC3);
	PORTC &= ~(1<<PORTC2);
}

void drive_motor_down() // drives motor down
{
	PORTC |= (1<<PORTC4);
	PORTC &= ~(1<<PORTC5);
}

void drive_motor_up() // drives motor up
{
	PORTC |= (1<<PORTC5);
	PORTC &= ~(1<<PORTC4);
}

void drive_motor_back() // drives motor back
{
	PORTD |= (1<<PORTD4);
	PORTD &= ~(1<<PORTD5);
}

void drive_motor_forth() // drives motor forth
{
	PORTD |= (1<<PORTD5);
	PORTD &= ~(1<<PORTD4);
}

void drive_motors() { // drives and stops motors according to button and ADC inputs
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
    }else if (!(PINE & (1<<PINE3)))
    {
		drive_motor_up();
    }else
    {
        motor_stop_updown();
    }
}

void set_up_interrupt() { // sets up boudary sensor interrupts and coin as well as button interupts at the e pins
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

void setup_sound() // sets up the sound system with the rigth timer but not playing yet
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

void stop_sound() // stops sound means it sets the duty cycle to zero
{
	OCR3A = 127;
	OCR3B = 0;
}

void play_sound(char tone) // plays sound means the duty cycle gets nonzero and the frequency gets changed according to the demanded letter which belongs to a tone and therefore a frequency, the letter is passed into the function as a parameter and the function only plays a single tone
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
}

ISR(PCINT3_vect) { // pin chang interrupts at the e pins
    
    // Check if PE0 (PCINT24) has changed
	if (PINE & (1 << PINE0)) {
		// PE0 went from low to high (rising edge)
		left_boundary_hit = 1;
	} else {
		// PE0 went from high to low (falling edge)
		left_boundary_hit = 0;
	}
	
	if (PINE & (1 << PINE1)) {
		// PE0 went from low to high (rising edge)
		right_boundary_hit = 1;
		} else {
		// PE0 went from high to low (falling edge)
	}
	
	if (PINE & (1 << PINE2)) {
		// PE0 went from low to high (rising edge)
		red_button_pressed = 1;
		} else {
		// PE0 went from high to low (falling edge)
		red_button_pressed = 0;
	}
}


void Initialize() // initializing all the general pins and calling all the initialize funtions to initialies certain bundeled functionalities
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
	PORTC &= ~(1<<PORTC2); //right
	PORTC &= ~(1<<PORTC3); //left
	PORTC &= ~(1<<PORTC4); //up
	PORTC &= ~(1<<PORTC5); //down
	PORTD &= ~(1<<PORTD3);
	PORTD &= ~(1<<PORTD4);
	PORTD &= ~(1<<PORTD5);
	
	
	// buttons up down

		DDRE |= (1<<DDE3);
		DDRD |= (1<<DDD1);
		// PORTD |= (1<<PORTD7);
		PORTD |= (1<<PORTD1);
		// DDRD &= ~(1<<DDD7);
		DDRD &= ~(1<<DDD1);
	 	PINE |= (1<<PINE3);
	 	PIND |= (1<<PIND1);

		// lighting LED
		DDRD |= (1<<DDD0);
		DDRD |= (1<<DDD7);
		PORTD &= ~(1<<PORTD0);
		PORTD &= ~(1<<PORTD7);	 

	set_up_interrupt();
	
	setUpADC();
	
	sei();

}

long_delay(uint8_t duration) // delay function
{
	for(uint8_t i=0;i<duration;++i)
	{
		_delay_ms(500);
	}
}

draw_frame() // drawing game frame on LCD screen
{
	LCD_setScreen(BLACK);
	string = "CLAWSOME CLAVERINE";
	LCD_drawString(34, 7, string, GREEN, BLACK);
}

void loading_screen() // displaying the loading screen
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

void start_countdown(uint16_t front, uint16_t back) // doing the initial 3 seconds countdown
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

void loosing_game() // displaying the loosing screen and playing the loosing sound
{
	draw_frame();
	LCD_drawBlock(1,19,158,127,RED);
	string = "You loose!";
	LCD_drawString(60, 55, string, BLACK, RED);
	play_sound('B');
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
	play_sound('G');
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
	play_sound('F');
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
	play_sound('E');
	_delay_ms(500);
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
}

winning_game() // displaying the winning screen and playing the winning sound
{
	draw_frame();
	LCD_drawBlock(1,19,158,127,RED);
	string = "You win!";
	LCD_drawString(60, 55, string, BLACK, RED);
	play_sound('D');
	_delay_ms(150);
	stop_sound();
	_delay_ms(150);
	play_sound('D');
	_delay_ms(150);
	stop_sound();
	_delay_ms(150);
	play_sound('D');
	_delay_ms(150);
	stop_sound();
	_delay_ms(150);
	play_sound('G');
	_delay_ms(500);
	_delay_ms(250);
	stop_sound();
	_delay_ms(250);
}

display_light(uint8_t chosen_lightmode) // light the demanded LED
{
	chosen_lightmode %= 3;
	if(chosen_lightmode==0)
	{
		PORTD &= ~(1<<PORTD0);
		PORTD &= ~(1<<PORTD7);
	}else if(chosen_lightmode==1)
	{
		PORTD |= (1<<PORTD0);
		PORTD &= ~(1<<PORTD7);
	}else if(chosen_lightmode==2)
	{
		PORTD &= ~(1<<PORTD0);
		PORTD |= (1<<PORTD7);
	}
	light_mode = chosen_lightmode;
}

change_light() // change the LED in a circular fashion
{
	++light_mode;
	light_mode %=3;
	display_light(light_mode);
}

void play_easy() // gameplay for the easy mode
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
			change_light();
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
				}else if (!(PINE & (1<<PINE3)))
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
	display_light(0);
	stop_sound();
	motor_stop_updown();
	motor_stop_rightleft();
	motor_stop_backforth();
	if(object_detected)
	{
		winning_game();
	}else
	{
		loosing_game();
	}
	object_detected = 0;
}

void play_hard() // gameplay for the hard mode
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
			change_light();
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
				}else if (!(PINE & (1<<PINE3)))
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
	motor_stop_updown();
	motor_stop_rightleft();
	motor_stop_backforth();
	display_light(0);
	if(object_detected)
	{
		winning_game();
	}else
	{
		loosing_game();
	}
	object_detected = 0;
}

play_machine() // playing the machine, this is the primary function that runs and does everyting and call all the other functions and functionallities of the machine, so this funciton is the one which has to be called to make the machine run it is the basic frame around all the functions and also calls all the other functions like the funcitons playing the game
{
	draw_frame();
	display_light(0);
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
			_delay_ms(50);
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
			if(!(PINE & (1<<PINE3)) || !(PIND & (1<<PIND1)) || !red_button_pressed)
			/*if(0)*/
			{
				_delay_ms(200);
				if(!(PINE & (1<<PINE3))) // yellow
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
}

int main(void) // main function which initializes the screen and call the `play_machine()` function in a while loop to keep the machine running
{
	// Initialization of screen
	Initialize();
	LCD_setScreen(BLUE);
	display_light(0);

	// playing machine
	while(1)
	{
		play_machine();
	}

}