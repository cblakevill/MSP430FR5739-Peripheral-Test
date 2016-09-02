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

#define DEBOUNCE_TIME_CONST 1600 //200ms debounce

void initialise(void);
void disableButtons(void);
void enableButtons(void);
void debounce(void);
void updateTemp(int);
void updateAcc(void);
void updateLEDs(void);

int bool_button = 1;
unsigned int x_bit, y_bit, z_bit;
unsigned int TempInitial = 0;
unsigned int TempMeasure = 0;
unsigned int TempResult = 0;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	initialise();
	updateTemp(1);

	while(1)
	{
		updateAcc();
		updateTemp(0);
		if(bool_button)
			updateLEDs();
		else
		{
			//clear display
			PJOUT = 0x00;
			P3OUT = 0x00;
		}
	}
}

void initialise(void)
{
 	//Initialise LEDS
	PJDIR |= (LED1 + LED2 + LED3 + LED4);
	P3DIR |= (LED5 + LED6 + LED7 + LED8);
	PJOUT &= ~(LED1 + LED2 + LED3 + LED4);
	P3OUT &= ~(LED5+LED6+LED7+LED8);

	P4DIR &= 0x00; //PUSH BUTTONS as input
	P4IE |= (PUSH1 + PUSH2); //Enable interrupts
	P4REN = (PUSH1 + PUSH2); //Enable resitor
	P4OUT = (PUSH1 + PUSH2); //Pull-up resistor
	P4IFG &= 0x00; //Initialise interrupt flag

	//enable accelerometer and NTC
	P2DIR |= BIT7;
	P2OUT |= BIT7;

	//intialise accelerometer
	P3DIR &= ~(AXIS_X + AXIS_Y + AXIS_Z); //P3.0, P3.1, P3.2 input

	//intialise thermistor
	P1DIR &= ~(BIT4); //P1.4 input
	P1OUT &= ~(BIT4);

	//ADC input channel 4 (A4)
	P1SEL0 = BIT4;
	P1SEL1 = BIT4;

	//Initialise ADC
	ADC10CTL0 &= ~ADC10ENC; //disable conversion
	ADC10CTL0 = ADC10ON + ADC10SHT_5; //enable adc + sample hold select 5
	ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
	ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_4; //reference 0 + input A4
	
	__enable_interrupt();
}

//ISR for butttons
#pragma vector = PORT4_VECTOR
__interrupt void P4_ISR(void)
{
	disableButtons();

	P4IFG &= ~(BIT0 + BIT1);
	bool_button ^= 1; //toggle output mode (peripheral demo or blank)

	debounce(); //200ms debounce
}

//ISR for timer0 debouncing
#pragma vector = TIMER1_A0_VECTOR
__interrupt void timer0_ISR(void)
{
	TA1CCTL0 = 0x00; //TACCR0 interrupt disabled
	TA1CTL = 0x00; // clock off
	enableButtons(); //debounce finished
}

void disableButtons(void)
{
	P4IFG = 0x00; //clear flag
	P4IE = 0x00; //disable interrupt
}

void enableButtons(void)
{
	P4IFG = 0x00; //clear flag
	P4IE |= (PUSH1 + PUSH2); //enable interrupt
}

void debounce(void)
{
	TA1CCTL0 = CCIE; //TACCR0 interrupt enabled
	TA1CCR0 = DEBOUNCE_TIME_CONST; //rise up time of 200ms
	TA1CTL = TASSEL_1 + MC_1; //ACLK + up mode
}


void updateTemp(int firstTime)
{
	__disable_interrupt();
	//enable and start ADC conversion
	ADC10CTL0 |= ADC10ENC + ADC10SC;
	while (ADC10CTL1 & BUSY);

	if(firstTime)
		TempInitial = ADC10MEM0; //reference point for future measurements
	else
		TempMeasure = ADC10MEM0;

	//end ADC conversion
	ADC10CTL0 &= ~ADC10ENC;
	__enable_interrupt();
}

void updateLEDs(void)
{
	//update accelerometer LEDs
	//x_bit: P3IN.0  -> P3OUT.7   (<< 7)
	//y_bit: P3IN.1  -> P3OUT.6   (<< 5)
	//z_bit: P3IN.2  -> P3OUT.5   (<< 3)
	//update accelerometer LEDs
	P3OUT = ((x_bit << 7) + (y_bit << 5) + (z_bit << 3));

	//update thermistor LEDs
	TempResult = TempMeasure - TempInitial;
	if(TempResult < 160)
		PJOUT = 0x00;
	else
		PJOUT = BIT0;
}

void updateAcc(void)
{
	//update accelerometer input
	x_bit = (P3IN & AXIS_X);
	y_bit = (P3IN & AXIS_Y);
	z_bit = (P3IN & AXIS_Z);
}
