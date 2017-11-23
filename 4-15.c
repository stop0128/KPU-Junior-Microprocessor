#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include	<util/delay.h>
#include "lcd.h"

#define EX_LCD_DATA			(*(volatile unsigned char *)0x8000)
#define EX_LCD_CONTROL 		(*(volatile unsigned char *)0x8001)
#define EX_SS_DATA 			(*(volatile unsigned char *)0x8002)
#define EX_SS_SEL 			(*(volatile unsigned char *)0x8003)
#define EX_DM_SEL 			(*(volatile unsigned int *)0x8004)
#define EX_DM_DATA 			(*(volatile unsigned int *)0x8006)
#define EX_LED 				(*(volatile unsigned char *)0x8008)
#define EX_STEPPING			(*(volatile unsigned char *)0x8009)
#define EX_DCMOTOR			(*(volatile unsigned char *)0x800A)
#define EX_SERVO			(*(volatile unsigned char *)0x800B)

#define CW					0x11
#define CCW					0x22
#define CLI() cli()
#define SEI() sei()

void port_init(void)
{
	PORTA = 0x00;
	DDRA = 0x00;
	PORTB = 0x00;
	DDRB = 0x00;
	PORTC = 0x00;	//m103 output only·
	DDRC = 0x00;
	PORTD = 0x00;
	DDRD = 0x00;
	PORTE = 0x00;
	DDRE = 0x00;
	PORTF = 0x00;
	DDRF = 0x00;
	PORTG = 0x00;
	DDRG = 0x00;
}

void DCMotor(unsigned int fast){
	int i=0;
	for(i=0;i<1000;i++){
	EX_DCMOTOR = CW;
	delay(fast);

	}
	//EX_DCMOTOR = CCW;
	//delay(2000);
}


void init_devices(void)
	{
	CLI();
	XDIV = 0x00;
	XMCRA = 0x00;
	port_init();
	MCUCR = 0x80;
	EICRA = 0x00;
	EICRB = 0x00;
	EIMSK = 0x00;
	TIMSK = 0x05;
	ETIMSK = 0x00;
	SEI();
	//all peripherals are now initialized
}

void delay_us(unsigned int dt){
	unsigned int dc;
	for(dc=0;dc<dt;dc++)
	{
		asm volatile("nop"::);
		asm volatile("nop"::);
		asm volatile("nop"::);
	}
}

void delay(unsigned int dt){
	unsigned int dc;
	for(dc=0;dc<dt;dc++){
		delay_us(700);
	}
}

int main(void)
{
	unsigned int fast = 1000, keydata;
	
	lcdInit();
	init_devices();
	
	while(1)
	{
		keydata = (PINB & 0xff);
		switch(keydata){
			case 0x01:
				fast = fast - 100;
				_delay_ms(100); 
				break;
			case 0x02:
				fast = fast + 100;
 
				break;
			default:
				break;
		}
		DCMotor(fast);
		keydata = (PINB & 0x00);
		lcd_gotoxy(1,2);
		lcd_putn4(fast);
	}
}

