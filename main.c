#include <msp430fr5739.h>

#define LED1  BIT0
#define LED2  BIT1
#define LED3  BIT2
#define LED4  BIT3
#define LED5  BIT4
#define LED6  BIT5
#define LED7  BIT6
#define LED8  BIT7

#define PUSH1 BIT0
#define PUSH2 BIT1

#define AXIS_X BIT0
#define AXIS_Y BIT1
#define AXIS_Z BIT2

int bool_button = 0; //false

int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
    //Initialise LEDS
	PJDIR |= (LED1 + LED2 + LED3 + LED4);
	P3DIR |= (LED5+LED6+LED7+LED8);
	PJOUT &= P1IN;
	P3OUT &= ~(LED5+LED6+LED7+LED8);

	P4DIR &= 0x00; //PUSH BUTTONS as input
	P4IE |= (PUSH1 + PUSH2); //Enable interrupts
	P4REN = (PUSH1 + PUSH2); //Enable resitor
	P4OUT = (PUSH1 + PUSH2); //Pull-up resistor
	P4IFG &= 0x00; //Initialise interrupt flag
	__enable_interrupt();

	//enable accelerometer and NTC
	P2DIR |= BIT7;
	P2OUT |= BIT7;

	//intialise accelerometer
	P3DIR &= ~(AXIS_X + AXIS_Y + AXIS_Z); //P3.0, P3.1, P3.2 input
	P3IN &= 0x00; //initialise input

	//intialise thermistor
	P1DIR &= ~(BIT4); //P1.4 input
	P1IN &= 0x00; //initialise input

	int x_bit, y_bit, z_bit;
	int ntc;

	while(1)
	{
		//update NTC
		ntc = P1IN;

		//update accelerometer input
		x_bit = (P3IN & AXIS_X);
		y_bit = (P3IN & AXIS_Y);
		z_bit = (P3IN & AXIS_Z);

		//x_bit: P3IN.0  -> P3OUT.7   (<< 7)
		//y_bit: P3IN.1  -> P3OUT.6   (<< 5)
		//z_bit: P3IN.2  -> P3OUT.5   (<< 3)
		//update accelerometer
		P3OUT = ((x_bit << 7) + (y_bit << 5) + (z_bit << 3));

		//update button controlled LEDS OR NTC
		if(bool_button)
			//display NTC
			PJOUT = ntc;
		else
			//blank display
			PJOUT &= 0x00;
	}
}

#pragma vector = PORT4_VECTOR
__interrupt void P4_ISR(void)
{
		P4IFG &= 0x00; //reset flag
		bool_button = ~bool_button; //toggle PJOUT output (NTC or blank)
}
