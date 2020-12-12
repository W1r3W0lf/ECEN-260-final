#include "msp.h"
#include "stdlib.h"
//#include "time.h"
#include "colors.h"

enum mode {MAIN, REMOTE};


// massages from remote to main
enum remote_messages {BUTTON2, BUTTON3};

#define board_mode MAIN

#define LED BIT0

#define BUTTON1 BIT1
#define BUTTON2 BIT4

#define PRESSED1 0xA0
#define RELEASED1 0xF0

#define PRESSED2 0x0A
#define RELEASED2 0x0F


int points = 0;

#define game_mode start

void game();
void helper();


void sleep(unsigned int sec){
	while(sec-- > 0)
	{
		int tick = 2;
		while(tick-- > 0)
		{
			unsigned int delay = 20000;
			while(delay-- > 0);
		}
	}
}


void UART_OutChar (char data) {
	while ( (EUSCI_A2->IFG & BIT1) == 0);  // Busy.  Wait for previous output.
	EUSCI_A2->TXBUF = data;        // Start transmission when IFG = 1.
}


char UART_InChar (void) {
	while ( (EUSCI_A2->IFG & BIT0) == 0);  // Busy.  Wait for received data.
	return ( (char) (EUSCI_A2->RXBUF) );   // Get new input when IFG = 1.
}


void setup()
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

	// Setup the red LED.
	P1->DIR |= LED; // Set LED as output.
	P1->OUT &= ~LED; // Turn off LED

	P2->DIR |= (RED | GREEN | BLUE);
	P2->OUT &= ~(RED | GREEN | BLUE);

	// Setup button 0
	// BLUE for MAIN and RED for REMOTE
	P1->DIR &= ~BIT1;     // set up pin P1.1 as input
	P1->REN |= BIT1;      // connect pull resistor to pin P1.1
	P1->OUT |= BIT1;      // configure pull resistor as pull up

	// Setup button 1
	// RESET for MAIN and GREEN for REMTOE
	P1->DIR &= ~BIT4;     // set up pin P1.4 as input
	P1->REN |= BIT4;      // connect pull resistor to pin P1.4
	P1->OUT |= BIT4;      // configure pull resistor as pull up
	P1->IFG &= ~BIT4;     // clear the interrupt flag for pin P1.4
	P1->IE |= BIT4;       // enable the interrupt for pin P1.4

	// Set up the reset interupt.
	if (board_mode == MAIN)
	{
	    NVIC->ISER[1] |= 0x08;
	    _enable_interrupts();
	}

	// Setup bord communiation settings
	// Setup Input/Output pins.
	P3->SEL0 |= BIT2 | BIT3;    // Set bit 2 and bit 3 of P3SEL0 to 1
	P3->SEL1 &= ~(BIT2 | BIT3);     // Reset bit 2 and bit 3 of P3SEL1 to 0

	EUSCI_A2->CTLW0 |= BIT0;        // Set WRST to put UART0 in reset
	// Leave all other bits = 0
	EUSCI_A2->CTLW0 |= BIT6 | BIT7; // Use SMCLK

	// Store the BRW in UCA2BRW to set the baud rate at 9600.
	// 3M/9600 = 312.5 -> 313 -> 0x139
	EUSCI_A2->BRW= 0x139;        // Baud rate = 9600

	EUSCI_A2->MCTLW &= ~BIT0;       // UCOS16 bit = 0

	EUSCI_A2->CTLW0 &= ~ BIT0;  // Clear WRST to resume UART operation.

}


/**
 * main.c
 */
void main(void)
{

	setup();

	if (board_mode == MAIN)
		game();
	else
		helper();

}


void fail_blink()
{
    int i;
    for (i=0; i< 10; i++)
    {
        P1->OUT ^= LED;
        sleep(1);
    }
}


void pass_blink(color passed)
{
    int i;
    for (i=0 ; i < 10 ; i++)
    {
        P2->OUT ^= passed;
        sleep(1);
    }
}


void send_color(color remote_color)
{
	UART_OutChar((char)remote_color);
}


color get_color()
{
	color new_color = 0;

	char message = UART_InChar();
	char partA = 0xF0 & message;
	char partB = 0x0F & message;

	if (partA == PRESSED1)
	    new_color &= ~RED;
	else
	    new_color |= RED;

	if (partB == PRESSED2)
	    new_color &= ~GREEN;
	else
	    new_color |= GREEN;

	if (P1->IN & BUTTON1)
		new_color &= ~BLUE;
	else
		new_color |= BLUE;

	return new_color;
}


void game()
{

	color current_color = BLACK;
	color input_color = BLACK;
	int ticks;

	show_color(RED);
	sleep(3);
	show_color(GREEN);
	sleep(3);
	show_color(BLUE);
	sleep(3);
	show_color(BLACK);



	while(1)
	{
	    // Win condition
		while (points >= 15)
		{
			show_color(random_color());
			UART_OutChar((char)random_color());
			sleep(1);
		}

		// Make a new color
		current_color = random_color();



		for (ticks = 0 ; ticks < 15 ; ticks++)
		{
		    input_color = get_color();

		    UART_OutChar((char)current_color); // This could break communications. If so put in loop.


		    show_color(input_color);
		    if (input_color == current_color)
		        break;
		    sleep(1);
		}

		if (input_color == current_color)
		{
		    points++;
		    pass_blink(input_color);
		}
		else
		{
		    points--;
		    fail_blink();
		}

	}
}


void helper()
{
	while(1)
	{
	    char message = 0;

	    if (P1->IN & BUTTON1)
	        message |= PRESSED1;
	    else
	        message |= RELEASED1;

	    if (P1->IN & BUTTON2)
	        message |= PRESSED2;
	    else
	        message |= RELEASED2;

	    UART_OutChar(message);

		show_color((color)UART_InChar());

	}
}


/***
* IRQ handler for port 1
***/
void PORT1_IRQHandler(void){

  uint32_t status;

  status = P1->IFG;    /* get the interrupt status for port 1 */
  P1->IFG &= ~BIT4;    /* clear the interrupt for port 1, pin 1 */

  if(status & BIT4){   /* constant for the pin 1 mask */
      points = 0;
      sleep(1);
      fail_blink();

  }
}
