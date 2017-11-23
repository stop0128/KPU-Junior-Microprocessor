#include	<avr/io.h>
#include	<avr/interrupt.h>
#include	<stdio.h>
#include	<util/delay.h>
#include	"lcd.h"

#define SEI() sei()
#define CLI() cli()

#define EX_LCD_DATA			(*(volatile unsigned char *)0x8000)
#define EX_LCD_CONTROL 		(*(volatile unsigned char *)0x8001)
#define EX_SS_DATA 			(*(volatile unsigned char *)0x8002)
#define EX_SS_SEL 			(*(volatile unsigned char *)0x8003)
#define EX_DM_SEL 			(*(volatile unsigned int *)0x8004)
#define EX_DM_DATA 			(*(volatile unsigned int *)0x8006)
#define EX_LED 				(*(volatile unsigned char *)0x8008)
#define ON 1
#define OFF 2
#define DELAYTIME 1
#define DELAYTIME1 0

#define DLY_1 DLY_4*4
#define DLY_2 DLY_4*2
#define DLY_4 400
#define DLY_8 DLY_4/2
#define DLY_16 DLY_8/2

unsigned char count_int, sec = 0;
volatile long T1HIGHCNT=0xFD, T1LOWCNT =0x66;
volatile int SoundState=ON;
volatile int Soundonoff = ON;
unsigned int noteon;

void port_init(void)
{
	PORTA = 0x00;
	DDRA = 0x00;
	PORTB = 0x00;
	DDRB = 0x00;
	PORTC = 0x00;
	DDRC = 0x00;
	PORTD = 0x00;
	DDRD = 0x00;
	PORTE = 0x00;
	DDRE = 0x02;
	PORTF = 0x00;
	DDRF = 0x00;
	PORTG = 0x00;
	DDRG = 0x03;
	
}

//ADC initialize 
// Conversion time: 104uS
void adc_init(void)
{
	ADCSRA = 0x00;    //disable adc
	ADMUX = 0x42;
	ACSR = 0x80;
	ADCSRA = 0x87;
}
//__________온도함수
void tempstartConvertion()
{
	ADCSRA = 0xc7;
	ADMUX = 0x42;				//43조도 42 온도 40 가변저항
	ADCSRA = ADCSRA | 0xc7;
}

unsigned int tempreadConvertData(void)
{
	volatile unsigned int temp=0;
	while((ADCSRA & 0x10)==0);
	ADCSRA = ADCSRA | 0x10;
	temp += (int)ADCL+(int)ADCH*256;
	ADCSRA = ADCSRA | 0x10;
	return temp;
}
//_________조도함수
void lightstartConvertion()
{
	ADCSRA = 0xc7;
	ADMUX = 0x43;				//43조도 42 온도 40 가변저항
	ADCSRA = ADCSRA | 0xc7;
}

unsigned int lightreadConvertData(void)
{
	volatile unsigned int temp=0;
	while((ADCSRA & 0x10)==0);
	ADCSRA = ADCSRA | 0x10;
	temp += (int)ADCL+(int)ADCH*256;
	ADCSRA = ADCSRA | 0x10;
	return temp;
}
//_________가변저항함수
void voltstartConvertion()
{
	ADCSRA = 0xc7;
	ADMUX = 0x40;				//43조도 42 온도 40 가변저항
	ADCSRA = ADCSRA | 0xc7;
}

unsigned int voltreadConvertData(void)
{
	volatile unsigned int temp=0;
	while((ADCSRA & 0x10)==0);
	ADCSRA = ADCSRA | 0x10;
	temp += (int)ADCL+(int)ADCH*256;
	ADCSRA = ADCSRA | 0x10;
	return temp;
}

void timer1_init(void)
{
	TCCR1B = 0x00;
	TCNT1H = 0xFD;
	TCNT1L = 0x66;
	OCR1AH = 0x02;
	OCR1AL = 0x9A;
	OCR1BH = 0x02;
	OCR1BL = 0x9A;
	OCR1CH = 0x02;
	OCR1CL = 0x9A;
	ICR1H = 0x02;
	ICR1L = 0x9A;
	TCCR1A = 0x00;
	TCCR1B = 0x02;
}

SIGNAL(SIG_OVERFLOW1)
{
	TCNT1H = T1HIGHCNT;
	TCNT1L = T1LOWCNT;
	if(Soundonoff==ON){
		PORTG = PORTG ^ 0x10;
	}
}


void sound(int freq)
{
	Soundonoff=ON;
	T1HIGHCNT = (0xFFFF-floor(1000000/freq)) / 0x100;
	T1LOWCNT = 0XFFFF-floor(1000000/freq) - 0XFF00;
}

void nosound(void)
{
	Soundonoff=OFF;
	_delay_ms(10);
}

void bellsong(void)
{
	sound(320);
	_delay_ms(25);
	nosound();
	
	sound(480);
	_delay_ms(25);
	nosound();
	
	sound(320);
	_delay_ms(25);
	nosound();
	
	sound(480);
	_delay_ms(25);
	nosound();
}


void init_devices(void)
	{
	CLI();
	XDIV = 0x00;
	XMCRA = 0x00;
	port_init();
	adc_init();
	timer1_init();
	MCUCR = 0x80;
	EICRA = 0x00;
	EICRB = 0x00;
	EIMSK = 0x00;
	TCCR0 = 0x07;
	TIMSK = 0x05;
	TCNT0 = 0x00;
	ASSR = 0x00;
	ETIMSK = 0x00;
	SEI();
	//all peripherals are now initialized
}

int plusnumber(int a, int max);
void sevenmonitor(int h, int n, int j, int i);
void timesetting(int *h, int *n, int *j, int *i);
void alarm(int *ah, int *an, int *aj, int *ai);
void timer();
void tempcheck();

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 0x00;
	count_int++;
	if(count_int == 61)
	{
		sec++;
		count_int = 0;
	}
}
void main(void)
{
	int keydata , i;
	unsigned int abc_data;
	float tempo;
	init_devices();
	nosound();
	lcdInit();
	while(1)
	{
		lcd_puts(1, "1.temp/2.light");
		lcd_puts(2, "3.volt/4.timer");
		
		keydata = (PINB & 0xff);
		switch(keydata)
		{
			case 0x01:
				lcdInit();
				tempcheck();
				break;
			case 0x02:
				lcdInit();
				lightcheck();
				break;
			case 0x04:
				lcdInit();
				voltcheck();
				break;
			case 0x08:
				lcdInit();
				timer();
				break;
			default:
				break;
		}
	}
}

int plusnumber(int a, int max)
{
	if(a == max)
	{
		a = 0;
	}
	else{
		a = a + 1;
	}
	return a;
}

void monitor(int h, int n, int j, int i)
{
	lcd_gotoxy(1,2);
	lcd_putn1(h);
	lcd_putn1(n);
	lcd_putn1(j);
	lcd_putn1(i);
}

void timesetting(int *h, int *n, int *j, int *i)
{
// __________________________________________________TIME SETTING__________________________________________________
	int keydata;					
	while(1)
	{
		keydata = (PINB & 0xff);
		CLI();
		switch(keydata)
		{
			case 0x80:
				*h = plusnumber(*h, 9);
				_delay_ms(500); 
				break;
			case 0x40:
				*n = plusnumber(*n, 9);
				_delay_ms(500);
				break;
			case 0x20:
				*j = plusnumber(*j, 5);
				_delay_ms(500);
				break;
			case 0x10:
				*i = plusnumber(*i, 9);
				_delay_ms(500);
				break;
			case 0x08:
				EX_LED = 0x00;
				SEI();
				return;
				break;
			default:
				break;
		}
		monitor(*h, *n, *j, *i);
	}	
}

void alarm(int *ah, int *an, int *aj, int *ai)
{
	int keydata;
	while(1)
	{
		keydata = (PINB & 0xff);
		CLI();										
		timesetting(ah, an, aj, ai);
		if(keydata == 0x08)
		{
			EX_LED = 0x00;
			SEI();
			break;
		}
	}
	return;
}

void timer()
{
	int h = 0,n = 0,j = 0, a = 0;
	int ah = 0, an = 0, ai = -1, aj = 0;
	int keydata;
	
	while(1)
	{
		if(sec == 10) {
			j++;
			sec = 0;
			if(j == 6) {
				n++;
				j = 0;
				if(n == 10) {
					h++;
					n = 0;
					if(h == 10) {
						h = 0;
					}
				}
			}
		}
		lcd_puts(1, "4.TIMER");
		monitor(h,n,j,sec);
		keydata = (PINB & 0xff);
// __________________________________________________TIME SETTING__________________________________________________
		if(keydata == 0x01)
		{				
			EX_LED = 0x01;
			lcd_puts(1, "TIMER SETTING");
			timesetting(&h, &n, &j, &sec);
			lcdInit();
		}
		fflush(stdin);
		if(keydata == 0x02)
		{
			EX_LED = 0x02;
			lcd_puts(1, "ALARM SETTING");
			alarm(&ah, &an, &aj, &ai);
			lcdInit();
		}
// __________________________________________________ALARM CHECK__________________________________________________
		if(h == ah && n == an && j == aj && sec == ai)
		{
			CLI();
			for(a = 0; a < 5; a++)
			{
				EX_LED = 0xff;			//LED ON
				_delay_ms(5);
				EX_LED = 0x00;			//LED OFF
				_delay_ms(5);
			}
			SEI();
		}
		if(keydata == 0x80)
		{
			lcdInit();
			return;
		}
	}
}

void tempcheck()
{
	int i, keydata;
	unsigned int abc_data;
	float tempo;
	while(1)
	{
		lcd_puts(1, "1.temp");
		tempstartConvertion();
		abc_data = tempreadConvertData();
		tempo = (float)((5000./1023.)*(abc_data)/10.);
		lcd_gotoxy(1,2);
		lcd_putn4(tempo);
		if(tempo > 27) {
			lcdInit();
			lcd_puts(1, "       1. WARNNING");
			for(i=0;i<10;i++) {	
				bellsong();
			}
			_delay_ms(1000);
			lcdInit();
		}
		_delay_ms(1000);
		PINB = (PINB & 0x00);
		keydata = (PINB & 0xff);
		if(keydata == 0x80)
		{
			lcdInit();
			keydata = 0x00;
			break;
		}
	}
}
void lightcheck()
{
	int i, keydata;
	unsigned int abc_data;
	float tempo;
	while(1)
	{
		lcd_puts(1, "2.light");
		lightstartConvertion();
		abc_data = lightreadConvertData();
		tempo = (float)((5000./1023.)*(abc_data)/10.);
		lcd_gotoxy(1,2);
		lcd_putn4(tempo);
		_delay_ms(1000);
		if(tempo > 300) {
			lcdInit();
			lcd_puts(1, "       2. WARNNING");
			for(i=0;i<10;i++) {	
				bellsong();
			}
			_delay_ms(1000);
			lcdInit();
		}
		_delay_ms(1000);
		PINB = (PINB & 0x00);
		keydata = (PINB & 0xff);
		if(keydata == 0x80)
		{
			lcdInit();
			keydata = 0x00;
			break;
		}
		
	}
}
void voltcheck()
{
	int i, keydata;
	unsigned int abc_data;
	float tempo;
	while(1)
	{
		lcd_puts(1, "3.volt");
		voltstartConvertion();
		abc_data = voltreadConvertData();
		tempo = (float)(5*abc_data)/1024.;
		lcd_gotoxy(1,2);
		lcd_putn4(tempo);
		_delay_ms(500);
		if(tempo > 3) {
			lcdInit();
			lcd_puts(1, "       3. WARNNING");
			for(i=0;i<10;i++) {	
				bellsong();
			}
			_delay_ms(1000);
			lcdInit();
		}
		_delay_ms(1000);
		PINB = (PINB & 0x00);
		keydata = (PINB & 0xff);
		if(keydata == 0x80)
		{
			lcdInit();
			keydata = 0x00;
			break;
		}
	}
	return;

}