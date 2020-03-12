#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "nokia5110.h"
#include <stdio.h>
#include "dht.h"

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

enum Cursor_States{start, TL, TM, TR, ML, MM, MR, BL, BM, BR} cstate;
unsigned char cnt, tl, tm, tr, ml, mm, mr, bl, bm, br, win, p1_wins, p2_wins = 0;

void Cursor() {
	int ADC_Value1;
	int ADC_Value2;
	unsigned press = ~PINA & 0x04;
	ADC_Value1 = ADC_Read(0);/* Read the status on X-OUT pin using channel 0 */
	ADC_Value2 = ADC_Read(1);/* Read the status on Y-OUT pin using channel 0 */
	
	switch(cstate) { // Transition Actions
		case start:
			cstate = TL;
			break;
		case TL:
			if (press) {
				nokia_lcd_set_cursor(13, 0);
				if ((cnt % 2) == 0) {
					nokia_lcd_write_string("x",1);
					tl = 1;
				}
				else {
					nokia_lcd_write_string("o",1);
					tl = 2;
				}
				cnt++;
			}
			if (ADC_Value1 > 700) {cstate = TM;}
			else if (ADC_Value2 < 250) {cstate = ML;}
			else if (ADC_Value2 > 700) {cstate = TL;}
			else if (ADC_Value1 < 300) {cstate = TL;}
			else {cstate = TL;}
			break;
		case TM:
			if (press) {
				nokia_lcd_set_cursor(40, 0);
				if ((cnt % 2) == 0) {
					nokia_lcd_write_string("x",1);
					tm = 1;
				}
				else {
					nokia_lcd_write_string("o",1);
					tm = 2;
				}
				cnt++;
			}
			if (ADC_Value1 > 700) {cstate = TR;}
			else if (ADC_Value2 < 250) {cstate = MM;}
			else if (ADC_Value2 > 700) {cstate = TM;}
			else if (ADC_Value1 < 300) {cstate = TL;}
			else {cstate = TM;}
			break;
		case TR:
			if (press) {
				nokia_lcd_set_cursor(67, 0);
				if ((cnt % 2) == 0) {
					nokia_lcd_write_string("x",1);
					tr = 1;
				}
				else {
					nokia_lcd_write_string("o",1);
					tr = 2;
				}
				cnt++;
			}
			if (ADC_Value1 > 700) {cstate = TR;}
			else if (ADC_Value2 < 250) {cstate = MR;}
			else if (ADC_Value2 > 700) {cstate = TR;}
			else if (ADC_Value1 < 300) {cstate = TM;}
			else {cstate = TR;}
			break;
		case ML:
			if (press) {
				nokia_lcd_set_cursor(13, 17);
				if ((cnt % 2) == 0) {
					nokia_lcd_write_string("x",1);
					ml = 1;
				}
				else {
					nokia_lcd_write_string("o",1);
					ml = 2;
				}
				cnt++;
			}
			if (ADC_Value1 > 700) {cstate = MM;}
			else if (ADC_Value2 < 250) {cstate = BL;}
			else if (ADC_Value2 > 700) {cstate = TL;}
			else if (ADC_Value1 < 300) {cstate = ML;}
			else {cstate = ML;}
			break;
		case MM:
			if (press) {
				nokia_lcd_set_cursor(40, 17);
				if ((cnt % 2) == 0) {
					nokia_lcd_write_string("x",1);
					mm = 1;
				}
				else {
					nokia_lcd_write_string("o",1);
					mm = 2;
				}
				cnt++;
			}
			if (ADC_Value1 > 700) {cstate = MR;}
			else if (ADC_Value2 < 250) {cstate = BM;}
			else if (ADC_Value2 > 700) {cstate = TM;}
			else if (ADC_Value1 < 300) {cstate = ML;}
			else {cstate = MM;}
			break;
		case MR:
			if (press) {
				nokia_lcd_set_cursor(67, 17);
				if ((cnt % 2) == 0) {
					nokia_lcd_write_string("x",1);
					mr = 1;
				}
				else {
					nokia_lcd_write_string("o",1);
					mr = 2;
				}
				cnt++;
			}
			if (ADC_Value1 > 700) {cstate = MR;}
			else if (ADC_Value2 < 250) {cstate = BR;}
			else if (ADC_Value2 > 700) {cstate = TR;}
			else if (ADC_Value1 < 300) {cstate = MM;}
			else {cstate = MR;}
			break;
		case BL:
			if (press) {
				nokia_lcd_set_cursor(13, 34);
				if ((cnt % 2) == 0) {
					nokia_lcd_write_string("x",1);
					bl = 1;
				}
				else {
					nokia_lcd_write_string("o",1);
					bl = 2;
				}
				cnt++;
			}
			if (ADC_Value1 > 700) {cstate = BM;}
			else if (ADC_Value2 < 250) {cstate = BL;}
			else if (ADC_Value2 > 700) {cstate = ML;}
			else if (ADC_Value1 < 300) {cstate = BL;}
			else {cstate = BL;}
			break;
		case BM:
			if (press) {
				nokia_lcd_set_cursor(40, 34);
				if ((cnt % 2) == 0) {
					nokia_lcd_write_string("x",1);
					bm = 1;
				}
				else {
					nokia_lcd_write_string("o",1);
					bm = 2;
				}
				cnt++;
			}
			if (ADC_Value1 > 700) {cstate = BR;}
			else if (ADC_Value2 < 250) {cstate = BM;}
			else if (ADC_Value2 > 700) {cstate = MM;}
			else if (ADC_Value1 < 300) {cstate = BL;}
			else {cstate = BM;}
			break;
		case BR:
			if (press) {
				nokia_lcd_set_cursor(67, 34);
				if ((cnt % 2) == 0) {
					nokia_lcd_write_string("x",1);
					br = 1;
				}
				else {
					nokia_lcd_write_string("o",1);
					br = 2;
				}
				cnt++;
			}
			if (ADC_Value1 > 700) {cstate = BR;}
			else if (ADC_Value2 < 250) {cstate = BR;}
			else if (ADC_Value2 > 700) {cstate = MR;}
			else if (ADC_Value1 < 300) {cstate = BM;}
			else {cstate = BR;}
		break;
		default:
			cstate = start;
			break;
	}	
	switch(cstate) { // State Actions
		case TL:
			if ( ((tl == 1) && (tm == 1) && (tr == 1)) || ((ml == 1) && (mm == 1) && (mr == 1)) || ((bl == 1) && (bm == 1) && (br == 1)) || ((tl == 1) && (ml == 1) && (bl == 1)) ||
			     ((tm == 1) && (mm == 1) && (bm == 1)) || ((tr == 1) && (mr == 1) && (br == 1)) || ((tl == 1) && (mm == 1) && (br == 1)) || ((tr == 1) && (mm == 1) && (bl == 1)) ) {
				nokia_lcd_1win();
				p1_wins = p1_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else if ( (tl == 2 && tm == 2 && tr == 2) || (ml == 2 && mm == 2 && mr == 2) || (bl == 2 && bm == 2 && br == 2) || (tl == 2 && ml == 2 && bl == 2) ||
			        (tm == 2 && mm == 2 && bm == 2) || (tr == 2 && mr == 2 && br == 2) || (tl == 2 && mm == 2 && br == 2) || (tr == 2 && mm == 2 && bl == 2) ) {
				nokia_lcd_2win();
				p2_wins = p2_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else {
				nokia_lcd_clear_cursors();
				nokia_lcd_set_cursor(13, 7);
				nokia_lcd_write_string("-",1);
				nokia_lcd_render();
			}
			break;
		case TM:
			if ( ((tl == 1) && (tm == 1) && (tr == 1)) || ((ml == 1) && (mm == 1) && (mr == 1)) || ((bl == 1) && (bm == 1) && (br == 1)) || ((tl == 1) && (ml == 1) && (bl == 1)) ||
			((tm == 1) && (mm == 1) && (bm == 1)) || ((tr == 1) && (mr == 1) && (br == 1)) || ((tl == 1) && (mm == 1) && (br == 1)) || ((tr == 1) && (mm == 1) && (bl == 1)) ) {
    			nokia_lcd_1win();
				p1_wins = p1_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else if ( (tl == 2 && tm == 2 && tr == 2) || (ml == 2 && mm == 2 && mr == 2) || (bl == 2 && bm == 2 && br == 2) || (tl == 2 && ml == 2 && bl == 2) ||
			(tm == 2 && mm == 2 && bm == 2) || (tr == 2 && mr == 2 && br == 2) || (tl == 2 && mm == 2 && br == 2) || (tr == 2 && mm == 2 && bl == 2) ) {
    			nokia_lcd_2win();
				p2_wins = p2_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else {
				nokia_lcd_clear_cursors();
				nokia_lcd_set_cursor(40, 7);
				nokia_lcd_write_string("-",1);
				nokia_lcd_render();
			}
			break;
		case TR:
			if ( ((tl == 1) && (tm == 1) && (tr == 1)) || ((ml == 1) && (mm == 1) && (mr == 1)) || ((bl == 1) && (bm == 1) && (br == 1)) || ((tl == 1) && (ml == 1) && (bl == 1)) ||
			((tm == 1) && (mm == 1) && (bm == 1)) || ((tr == 1) && (mr == 1) && (br == 1)) || ((tl == 1) && (mm == 1) && (br == 1)) || ((tr == 1) && (mm == 1) && (bl == 1)) ) {
    			nokia_lcd_1win();
				p1_wins = p1_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else if ( (tl == 2 && tm == 2 && tr == 2) || (ml == 2 && mm == 2 && mr == 2) || (bl == 2 && bm == 2 && br == 2) || (tl == 2 && ml == 2 && bl == 2) ||
			(tm == 2 && mm == 2 && bm == 2) || (tr == 2 && mr == 2 && br == 2) || (tl == 2 && mm == 2 && br == 2) || (tr == 2 && mm == 2 && bl == 2) ) {
    			nokia_lcd_2win();
				p2_wins = p2_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else {
				nokia_lcd_clear_cursors();
				nokia_lcd_set_cursor(67, 7);
				nokia_lcd_write_string("-",1);
				nokia_lcd_render();
			}
			break;
		case ML:
			if ( ((tl == 1) && (tm == 1) && (tr == 1)) || ((ml == 1) && (mm == 1) && (mr == 1)) || ((bl == 1) && (bm == 1) && (br == 1)) || ((tl == 1) && (ml == 1) && (bl == 1)) ||
			((tm == 1) && (mm == 1) && (bm == 1)) || ((tr == 1) && (mr == 1) && (br == 1)) || ((tl == 1) && (mm == 1) && (br == 1)) || ((tr == 1) && (mm == 1) && (bl == 1)) ) {
    			nokia_lcd_1win();
				p1_wins = p1_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else if ( (tl == 2 && tm == 2 && tr == 2) || (ml == 2 && mm == 2 && mr == 2) || (bl == 2 && bm == 2 && br == 2) || (tl == 2 && ml == 2 && bl == 2) ||
			(tm == 2 && mm == 2 && bm == 2) || (tr == 2 && mr == 2 && br == 2) || (tl == 2 && mm == 2 && br == 2) || (tr == 2 && mm == 2 && bl == 2) ) {
    			nokia_lcd_2win();
				p2_wins = p2_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else {
				nokia_lcd_clear_cursors();
				nokia_lcd_set_cursor(13, 24);
				nokia_lcd_write_string("-",1);
				nokia_lcd_render();
			}
			break;
		case MM:
			if ( ((tl == 1) && (tm == 1) && (tr == 1)) || ((ml == 1) && (mm == 1) && (mr == 1)) || ((bl == 1) && (bm == 1) && (br == 1)) || ((tl == 1) && (ml == 1) && (bl == 1)) ||
			((tm == 1) && (mm == 1) && (bm == 1)) || ((tr == 1) && (mr == 1) && (br == 1)) || ((tl == 1) && (mm == 1) && (br == 1)) || ((tr == 1) && (mm == 1) && (bl == 1)) ) {
    			nokia_lcd_1win();
				p1_wins = p1_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else if ( (tl == 2 && tm == 2 && tr == 2) || (ml == 2 && mm == 2 && mr == 2) || (bl == 2 && bm == 2 && br == 2) || (tl == 2 && ml == 2 && bl == 2) ||
			(tm == 2 && mm == 2 && bm == 2) || (tr == 2 && mr == 2 && br == 2) || (tl == 2 && mm == 2 && br == 2) || (tr == 2 && mm == 2 && bl == 2) ) {
    			nokia_lcd_2win();
				p2_wins = p2_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else {
				nokia_lcd_clear_cursors();
				nokia_lcd_set_cursor(40, 24);
				nokia_lcd_write_string("-",1);
				nokia_lcd_render();
			}
			break;
		case MR:
			if ( ((tl == 1) && (tm == 1) && (tr == 1)) || ((ml == 1) && (mm == 1) && (mr == 1)) || ((bl == 1) && (bm == 1) && (br == 1)) || ((tl == 1) && (ml == 1) && (bl == 1)) ||
			((tm == 1) && (mm == 1) && (bm == 1)) || ((tr == 1) && (mr == 1) && (br == 1)) || ((tl == 1) && (mm == 1) && (br == 1)) || ((tr == 1) && (mm == 1) && (bl == 1)) ) {
    			nokia_lcd_1win();
				p1_wins = p1_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else if ( (tl == 2 && tm == 2 && tr == 2) || (ml == 2 && mm == 2 && mr == 2) || (bl == 2 && bm == 2 && br == 2) || (tl == 2 && ml == 2 && bl == 2) ||
			(tm == 2 && mm == 2 && bm == 2) || (tr == 2 && mr == 2 && br == 2) || (tl == 2 && mm == 2 && br == 2) || (tr == 2 && mm == 2 && bl == 2) ) {
    			nokia_lcd_2win();
				p2_wins = p2_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else {
				nokia_lcd_clear_cursors();
				nokia_lcd_set_cursor(67, 24);
				nokia_lcd_write_string("-",1);
				nokia_lcd_render();
			}
			break;
		case BL:
			if ( ((tl == 1) && (tm == 1) && (tr == 1)) || ((ml == 1) && (mm == 1) && (mr == 1)) || ((bl == 1) && (bm == 1) && (br == 1)) || ((tl == 1) && (ml == 1) && (bl == 1)) ||
			((tm == 1) && (mm == 1) && (bm == 1)) || ((tr == 1) && (mr == 1) && (br == 1)) || ((tl == 1) && (mm == 1) && (br == 1)) || ((tr == 1) && (mm == 1) && (bl == 1)) ) {
    			nokia_lcd_1win();
				p1_wins = p1_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else if ( (tl == 2 && tm == 2 && tr == 2) || (ml == 2 && mm == 2 && mr == 2) || (bl == 2 && bm == 2 && br == 2) || (tl == 2 && ml == 2 && bl == 2) ||
			(tm == 2 && mm == 2 && bm == 2) || (tr == 2 && mr == 2 && br == 2) || (tl == 2 && mm == 2 && br == 2) || (tr == 2 && mm == 2 && bl == 2) ) {
    			nokia_lcd_2win();
				p2_wins = p2_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else {
				nokia_lcd_clear_cursors();
				nokia_lcd_set_cursor(13, 41);
				nokia_lcd_write_string("-",1);
				nokia_lcd_render();
			}
			break;
		case BM:
			if ( ((tl == 1) && (tm == 1) && (tr == 1)) || ((ml == 1) && (mm == 1) && (mr == 1)) || ((bl == 1) && (bm == 1) && (br == 1)) || ((tl == 1) && (ml == 1) && (bl == 1)) ||
			((tm == 1) && (mm == 1) && (bm == 1)) || ((tr == 1) && (mr == 1) && (br == 1)) || ((tl == 1) && (mm == 1) && (br == 1)) || ((tr == 1) && (mm == 1) && (bl == 1)) ) {
    			nokia_lcd_1win();
    			p1_wins = p1_wins + 1;
    			win = 1;
    			
    			cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else if ( (tl == 2 && tm == 2 && tr == 2) || (ml == 2 && mm == 2 && mr == 2) || (bl == 2 && bm == 2 && br == 2) || (tl == 2 && ml == 2 && bl == 2) ||
			(tm == 2 && mm == 2 && bm == 2) || (tr == 2 && mr == 2 && br == 2) || (tl == 2 && mm == 2 && br == 2) || (tr == 2 && mm == 2 && bl == 2) ) {
    			nokia_lcd_2win();
    			p2_wins = p2_wins + 1;
    			win = 1;
    			
    			cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else {
				nokia_lcd_clear_cursors();
				nokia_lcd_set_cursor(40, 41);
				nokia_lcd_write_string("-",1);
				nokia_lcd_render();
			}
			break;
		case BR:
			if ( ((tl == 1) && (tm == 1) && (tr == 1)) || ((ml == 1) && (mm == 1) && (mr == 1)) || ((bl == 1) && (bm == 1) && (br == 1)) || ((tl == 1) && (ml == 1) && (bl == 1)) ||
			((tm == 1) && (mm == 1) && (bm == 1)) || ((tr == 1) && (mr == 1) && (br == 1)) || ((tl == 1) && (mm == 1) && (br == 1)) || ((tr == 1) && (mm == 1) && (bl == 1)) ) {
    			nokia_lcd_1win();
				p1_wins = p1_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else if ( (tl == 2 && tm == 2 && tr == 2) || (ml == 2 && mm == 2 && mr == 2) || (bl == 2 && bm == 2 && br == 2) || (tl == 2 && ml == 2 && bl == 2) ||
			(tm == 2 && mm == 2 && bm == 2) || (tr == 2 && mr == 2 && br == 2) || (tl == 2 && mm == 2 && br == 2) || (tr == 2 && mm == 2 && bl == 2) ) {
    			nokia_lcd_2win();
				p2_wins = p2_wins + 1;
				win = 1;
				
				cnt, tl, tm, tr, ml, mm, mr, bl, bm, br = 0;
			}
			else {
				nokia_lcd_clear_cursors();
				nokia_lcd_set_cursor(67, 41);
				nokia_lcd_write_string("-",1);
				nokia_lcd_render();
			}
			break;
		default:
			break;
	}
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	char buffer[40];
	unsigned press = ~PINA & 0x04;
	
	nokia_lcd_init();  // Initialize Nokia 5110
	nokia_lcd_clear();

	nokia_lcd_grid();  // Initialize Tic-Tac-Toe Grid
	nokia_lcd_render();
	
	ADC_init();        // Initialize ADC
	LCD_init();        // Initialize LCD 
	LCD_ClearScreen();

	TimerSet(100);
	TimerOn();

	while(1){
		Cursor();
		sprintf(buffer, "Player 1: %d     Player 2: %d     ", p1_wins,p2_wins);
		LCD_DisplayString(1, buffer);

		while (!TimerFlag) {};    // Wait 300ms
		TimerFlag = 0;
	}
	return 1;
}