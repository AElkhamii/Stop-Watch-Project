/*
 * Mini_Project2.c
 *
 *  Created on: Sep 16, 2021
 *      Author: Abdelrahman
 */


#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>


struct Stop_watch
{
	unsigned char sec1;
	unsigned char sec2;
	unsigned char min1;
	unsigned char min2;
	unsigned char hour1;
	unsigned char hour2;
}time;

/*==================================Timer 1 CTC mode===================================*/
// Activate Timer 1 CTC mode
void Timer1_CTC(void)
{
	/* Configure timer control register TCCR1A
	 * 1. Disconnect OC1A and OC1B  COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2. FOC1A=1 FOC1B=0
	 * 3. CTC Mode WGM10=0 WGM11=0 (Mode Number 4)
	 */
	TCCR1A |= (1<<FOC1A);

	/* Configure timer control register TCCR1B
	 * 1. CTC Mode WGM12=1 WGM13=0 (Mode Number 4)
	 * 2. Prescaler = F_CPU/1024 CS10=1 CS11=0 CS12=0
	 */
	TCCR1B |= (1<<WGM12) | (1<<CS10) | (1<<CS12);

	TCNT1 = 0; 		/* Set timer1 initial count to zero */
	OCR1A = 976;	/* Set the compare value, it depend on the prescaler that used(1024)
	 	 	 	 	   This prescaler produce frequency equal to 976.5Hz. so, to make one
	 	 	 	 	   second, it is require to interrupt for each 976ms.*/

	 TIMSK |= (1<<OCIE1A); /*Activate interrupt*/
}

//ISR for Timer 1 CTC mode
ISR(TIMER1_COMPA_vect)
{
	time.sec1++;

	if(time.sec1 == 10)
	{
		time.sec1 = 0;
		time.sec2++;

		if(time.sec2 == 6)
		{
			time.sec2 = 0;
			time.min1++;

			if(time.min1==10)
			{
				time.min1=0;
				time.min2++;

				if(time.min2==6)
				{
					time.min2 = 0;
					time.hour1++;

					if(time.hour1 == 10)
					{
						time.hour1 = 0;
						time.hour2++;

						if(time.hour2 == 10) // If the timer reach max value, the timer will reset
						{
							time.sec1 = 0;
							time.sec2 = 0;
							time.min1 = 0;
							time.min2 = 0;
							time.hour1 = 0;
							time.hour2 = 0;
						}
					}
				}
			}
		}
	}
	OCR1A = 976; //Make the next compare with the same old compare
}
/*=====================================================================================*/


/*===============================Interrupt 0 clear timer===============================*/
//Activate interrupt 0
void Interrupt0_INT(void)
{
	 MCUCR |= (1<<ISC01); //The falling edge of INT0 generates an interrupt request.
	 GICR |= (1<<INT0);  //Enable eternal interrupt 0
}

//ISR for interrupt 0
ISR (INT0_vect)
{
	//when interrupt 0 occur the timer will reset
	time.sec1 = 0;
	time.sec2 = 0;
	time.min1 = 0;
	time.min2 = 0;
	time.hour1 = 0;
	time.hour2 = 0;
}
/*=====================================================================================*/


/*===============================Interrupt 1 clear timer===============================*/
//Activate interrupt 1
void Interrupt1_INT(void)
{
	MCUCR |= (1<<ISC10) | (1<<ISC11);   //The rising edge of INT1 generates an interrupt request.
	GICR |= (1<<INT1);					//Enable external interrupt 1
}

//ISR for interrupt 1
ISR (INT1_vect)
{
	TCCR1B &= ~(1<<CS10) & ~(1<<CS12); //clear prescaler CS10=0, CS11=0, CS12=0 (No clock source)
}
/*=====================================================================================*/


/*===============================Interrupt 2 clear timer===============================*/
//Activate interrupt 2
void Interrupt2_INT(void)
{
	MCUCSR &= ~(1<<ISC2); 	// ISC2 is written to zero, a falling edge on INT2 activates the interrupt.
	GICR |= (1<<INT2); 	  	//Enable external interrupt 1
}

//ISR for interrupt 2
ISR (INT2_vect)
{
	TCCR1B |= (1<<CS10) | (1<<CS12); //Activate the clock source again to prescaler = 1024
}
/*=====================================================================================*/


int main(void)
{
	SREG |= (1<<7); /* Enable global interrupts in MC */

	/*initial condition for timer*/
	time.sec1 = 0;
	time.sec2 = 0;
	time.min1 = 0;
	time.min2 = 0;
	time.hour1 = 0;
	time.hour2 = 0;


	/*activate input/output pins*/
	DDRA &= 0xC0; 		//Activate switches for control the enable/disable for each 7-segment

	DDRD &= ~(1<<PD2);	//Activate switch for Stop Watch time should be reset(INT0)
	PORTD |= (1<<PD2); 	//Activate internal Pull up resistor

	DDRB &= ~(1<<PD3);  //Activate switch for Stop Watch time should be Paused(INT2)

	DDRB &= ~(1<<PB2);	//Activate switch for Stop Watch time should be resumed(INT2)
	PORTB |= (1<<PB2);  //Activate internal Pull up resistor

	DDRC |= 0x0F;       //Activate 4 output pins for IC 7447

	PORTC &= 0xF0;      //Initial value for 7-segment

	/*Calling Timer1 and interrupts*/
	Timer1_CTC();
	Interrupt0_INT();
	Interrupt1_INT();
	Interrupt2_INT();

	while(1)
	{
		/*==================Control 7-segments gates and present time==================*/
		PORTA &= 0xC0;		//Close all gates
		PORTA |= (1<<PA0);  //Open required gate
		PORTC = (PORTC & 0xF0) | (time.sec1 & 0x0F); //set the seconds value in decimal
		_delay_us(5);

		PORTA &= 0xC0;		//Close all gates
		PORTA |= (1<<PA1);  //Open required gate
		PORTC = (PORTC & 0xF0) | (time.sec2 & 0x0F); //set the seconds value in tenth
		_delay_us(5);

		PORTA &= 0xC0;		//Close all gates
		PORTA |= (1<<PA2);  //Open required gate
		PORTC = (PORTC & 0xF0) | (time.min1 & 0x0F); //set the minute value in decimal
		_delay_us(5);

		PORTA &= 0xC0;		//Close all gates
		PORTA |= (1<<PA3);  //Open required gate
		PORTC = (PORTC & 0xF0) | (time.min2 & 0x0F); //set the minute value in tenth
		_delay_us(5);

		PORTA &= 0xC0;		//Close all gates
		PORTA |= (1<<PA4);  //Open required gate
		PORTC = (PORTC & 0xF0) | (time.hour1 & 0x0F); //set the hour value in decimal
		_delay_us(5);

		PORTA &= 0xC0;		//Close all gates
		PORTA |= (1<<PA5);  //Open required gate
		PORTC = (PORTC & 0xF0) | (time.hour2 & 0x0F); //set the hour value in tenth
		_delay_us(5);
		/*=============================================================================*/
	}
}
