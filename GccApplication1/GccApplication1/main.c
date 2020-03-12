#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "nokia5110.h"
#include <stdio.h>
#include "dht.h"

enum States{start, TL, TM, TR, ML, MM, MR, BL, BM, BR} state;

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}
void TimerOff() {
	TCCR1B = 0x00;
}
void TimerISR() {
	TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

int ADC_Read(char channel)
{
	int ADC_value;
	
	ADMUX = (0x40) | (channel & 0x07); // set input channel to read
	ADCSRA |= (1<<ADSC);    // start conversion
	while((ADCSRA &(1<<ADIF))== 0);    // monitor end of conversion interrupt flag
	
	ADCSRA |= (1<<ADIF);    // clear interrupt flag 
	ADC_value = (int)ADCL;    // read lower byte 
	ADC_value = ADC_value + (int)ADCH*256;// read higher 2 bits, Multiply with weightage

	return ADC_value;        // return digital value
}

void Cursor() {
	int ADC_Value1;
	int ADC_Value2;
	
	ADC_Value1 = ADC_Read(0);/* Read the status on X-OUT pin using channel 0 */
	ADC_Value2 = ADC_Read(1);/* Read the status on Y-OUT pin using channel 0 */
	
	switch(state) { // Transitions
		case start:
			state = TL;
			break;
		case TL:
			
			if (ADC_Value1 > 700) {state = TM;}
			else if (ADC_Value2 < 250) {state = ML;}
			else if (ADC_Value2 > 700) {state = TL;}
			else if (ADC_Value1 < 300) {state = TL;}
			else {state = TL;}
		break;
		case TM:
			if (ADC_Value1 > 700) {state = TR;}
			else if (ADC_Value2 < 250) {state = MM;}
			else if (ADC_Value2 > 700) {state = TM;}
			else if (ADC_Value1 < 300) {state = TL;}
			else {state = TM;}
		break;
		case TR:
			if (ADC_Value1 > 700) {state = TR;}
			else if (ADC_Value2 < 250) {state = MR;}
			else if (ADC_Value2 > 700) {state = TR;}
			else if (ADC_Value1 < 300) {state = TM;}
			else {state = TR;}
		break;
		case ML:
			if (ADC_Value1 > 700) {state = MM;}
			else if (ADC_Value2 < 250) {state = BL;}
			else if (ADC_Value2 > 700) {state = TL;}
			else if (ADC_Value1 < 300) {state = ML;}
			else {state = ML;}
		break;
		case MM:
			if (ADC_Value1 > 700) {state = MR;}
			else if (ADC_Value2 < 250) {state = BM;}
			else if (ADC_Value2 > 700) {state = TM;}
			else if (ADC_Value1 < 300) {state = ML;}
			else {state = MM;}
		break;
		case MR:
			if (ADC_Value1 > 700) {state = MR;}
			else if (ADC_Value2 < 250) {state = BR;}
			else if (ADC_Value2 > 700) {state = TR;}
			else if (ADC_Value1 < 300) {state = MM;}
			else {state = MR;}
		break;
		case BL:
			if (ADC_Value1 > 700) {state = BM;}
			else if (ADC_Value2 < 250) {state = BL;}
			else if (ADC_Value2 > 700) {state = ML;}
			else if (ADC_Value1 < 300) {state = BL;}
			else {state = BL;}
		break;
		case BM:
			if (ADC_Value1 > 700) {state = BR;}
			else if (ADC_Value2 < 250) {state = BM;}
			else if (ADC_Value2 > 700) {state = MM;}
			else if (ADC_Value1 < 300) {state = BL;}
			else {state = BM;}
		break;
		case BR:
			if (ADC_Value1 > 700) {state = BR;}
			else if (ADC_Value2 < 250) {state = BR;}
			else if (ADC_Value2 > 700) {state = MR;}
			else if (ADC_Value1 < 300) {state = BM;}
			else {state = BR;}
		break;
		default:
			state = start;
		break;
	}	
	switch(state) { // State Actions
		case TL:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(0, 27);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(13, 7);
			nokia_lcd_write_string("-",1);
			nokia_lcd_render();
		break;
		case TM:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(0, 27);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(40, 7);
			nokia_lcd_write_string("-",1);
			nokia_lcd_render();
		break;
		case TR:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(0, 27);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(67, 7);
			nokia_lcd_write_string("-",1);
			nokia_lcd_render();
		break;
		case ML:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(0, 27);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(13, 24);
			nokia_lcd_write_string("-",1);
			nokia_lcd_render();
		break;
		case MM:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(0, 27);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(40, 24);
			nokia_lcd_write_string("-",1);
			nokia_lcd_render();
		break;
		case MR:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(0, 27);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(67, 24);
			nokia_lcd_write_string("-",1);
			nokia_lcd_render();
		break;
		case BL:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(0, 27);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(13, 41);
			nokia_lcd_write_string("-",1);
			nokia_lcd_render();
		break;
		case BM:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(0, 27);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(40, 41);
			nokia_lcd_write_string("-",1);
			nokia_lcd_render();
		break;
		case BR:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(0, 27);
			nokia_lcd_write_string("--------------",1);
			nokia_lcd_set_cursor(67, 41);
			nokia_lcd_write_string("-",1);
			nokia_lcd_render();
		break;
		default:
		break;
	}
}

int main(void)
{
	DDRA = 0x00; PORTA = 0x15;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	// Grid
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_write_string("--------------",1);
	nokia_lcd_set_cursor(0, 27);
	nokia_lcd_write_string("--------------",1);
	
	char buffer[40];
	
	nokia_lcd_init();
	nokia_lcd_clear();
	ADC_init();        // Initialize ADC
	LCD_init();        // Initialize LCD 
	LCD_ClearScreen();
	nokia_lcd_render();
	TimerSet(100);
	TimerOn();

	while(1){
		Cursor();
		
		/*sprintf(buffer, "X=%d   Y=%d ", ADC_Value1, ADC_Value2);
		LCD_DisplayString(1, buffer);
		LCD_Cursor(24);*/
		while (!TimerFlag) {};    // Wait 300ms
		TimerFlag = 0;
	}
	
	return 1;
}