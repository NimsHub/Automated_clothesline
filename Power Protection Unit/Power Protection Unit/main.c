/*
 * Power Protection Unit.c
 *
 * Created: 08/13/2022 4:30:58 PM
 * Author : Nirmala
 */ 

# include <avr/io.h>
#include<avr/interrupt.h>
# define F_CPU 9600000
# include <util/delay.h>

# define idle 1
# define cutoff 0
# define sense_idle 1
# define sense_cutoff 2

void init_Ports(void);
void init_ADC(void);
int read_ADC(int);
void init_timer(void);
int state = idle;
int buzz_flag = 0;
int buzz_state = 0;
int buzz_count = 0;

int main(void)
{
	init_Ports();
	init_ADC();
	init_timer();
	
	
	
	while (1)
	{
		if(state == idle){
			if(read_ADC(sense_idle) > 500){
				state = cutoff;
				//PORTB |=  (1<<0);
				PORTB |=  (1<<1);
				PORTB |=  (1<<3);
				
				_delay_ms(500);
			}
			}else{
			if(read_ADC(sense_cutoff) < 900){
				state = idle;
				PORTB &= ~(1<<0);
				PORTB &= ~(1<<1);
				PORTB &= ~(1<<3);
				_delay_ms(500);
			}
		}
	}
}

void init_Ports(void){
	DDRB |= 0B00101011;
	DDRB &= 0B11101011;
}

void init_ADC(void){
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

int read_ADC(int pin_no){
	ADMUX = (0<<REFS0) | pin_no;
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	return ADC;
}

ISR(TIM0_OVF_vect)
{
	if(state==cutoff){
		buzz_count++;
		if(buzz_count>10){
			PORTB ^= (1<<0);
			buzz_count = 0;
		}
	}
}

void init_timer(void){
	TCCR0B |=(1<<CS02)|(1<<CS00);
	TIMSK0 |= (1<<TOIE0);
	sei();
}
