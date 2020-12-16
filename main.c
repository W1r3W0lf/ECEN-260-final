#include "msp.h"
#include "stdlib.h"
#include "colors.h"

enum mode {MAIN, REMOTE};

// massages from remote to main
enum remote_messages {BUTTON2, BUTTON3};

#define board_mode MAIN

#define LED BIT0

#define BUTTON1 BIT1
#define BUTTON2 BIT4

// Using a nibble makes it easy to read when debugging
#define PRESSED1 0xA0
#define RELEASED1 0xF0

// Use the other nibble so they don't overlap
#define PRESSED2 0x0A
#define RELEASED2 0x0F


int points = 0;

#define game_mode start

void game();
void helper();

/* sleep
 *
 * Hold the CPU still for a limited amount of time.
 */
void sleep(unsigned int sec){
	while(sec-- > 0)
	{
		int tick = 40000; // The amount of times to spin around
		while(tick-- > 0);
	}
}

/* UART_OutChar
 * IN
 * char data	the character to be sent to the other board.
 *
 * Sends a character to the other board.
 */
void UART_OutChar (char data) {
	while ( (EUSCI_A2->IFG & BIT1) == 0);  // Busy.  Wait for previous output.
	EUSCI_A2->TXBUF = data;        // Start transmission when IFG = 1.
}

/* UART_InChar
 * OUT
 * char 		The character from the other board.
 *
 * Gets a character from the other bord
 */
char UART_InChar (void) {
	while ( (EUSCI_A2->IFG & BIT0) == 0);  // Busy.  Wait for received data.
	return ( (char) (EUSCI_A2->RXBUF) );   // Get new input when IFG = 1.
}


/* setup
 *
 * Sets up all of the hardware for I/O.
 */
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


/* main
 *
 * Chooses witch function the board goes into.
 */
void main(void)
{

	setup(); 

	if (board_mode == MAIN)
		game();
	else
		helper();

}

/* fail_blink
 *
 * Blinks to show the loss of a point.
 */
void fail_blink()
{
    int i;
    for (i=0; i< 10; i++)
    {
        P1->OUT ^= LED; // Flip the LED off or on
        sleep(1); 
    }
}

/* pass_blink
 * IN
 * color passed		The last color that was displayed
 *
 * Blinks the passed color to show that a point was gained.
 */
void pass_blink(color passed)
{
    int i;
    for (i=0 ; i < 10 ; i++)
    {
        P2->OUT ^= passed; // Flip on or off the last color on the RGB LED.
        sleep(1);
    }
}

/* get_color
 * OUT
 * color	The color sent from the other board.
 *
 * Receives input from the MAIN and REMOTE board and returns the intended color.
 */
color get_color()
{
	color new_color = 0; // Set a new color and clear it out

	char message = UART_InChar();
	char partA = 0xF0 & message;
	char partB = 0x0F & message;

	if (partA == PRESSED1)
	    new_color &= ~RED; // turn on the RED LED
	else
	    new_color |= RED; // turn off the RED LED

	if (partB == PRESSED2)
	    new_color &= ~GREEN; // turn on the GREEN LED
	else
	    new_color |= GREEN; // turn off the GREEN LED

	if (P1->IN & BUTTON1)
		new_color &= ~BLUE; // turn on the BLUE LED
	else
		new_color |= BLUE; // turn off the BLUE LED

	return new_color;
}

/* game
 *
 * Runs the main game logic
 */
void game()
{

	color current_color = BLACK;
	color input_color = BLACK;
	int ticks;

	// Show that the game is starting
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
		while (points == 15) // If 15 points are gained you win
		{
			// Show random colors on both boards as a prize
			show_color(random_color());
			UART_OutChar((char)random_color());
			sleep(1);
		}

		// Make a new color
		current_color = random_color();

		// Spin around taking inputs from the user and displaying them
		for (ticks = 0 ; ticks < 15 ; ticks++)
		{
		    input_color = get_color(); // Get color from REMOTE

		    UART_OutChar((char)current_color); // Sends the current color to REMOTE

		    show_color(input_color); // Show the color they user chose

			// If they guessed the color early skip the wait
		    if (input_color == current_color)
		        break;

		    sleep(1);
		}

		// increase points and display that fact
		if (input_color == current_color)
		{
		    points++;
		    pass_blink(input_color);
		} 
		else // decrease the pints and display that fact
		{
		    points--;
		    fail_blink();
		}

	}
}

/* helper
 *
 * Takes gets colors from the MAIN board to display
 * Also sends the status of it's buttons back to the MAIN board.
 */
void helper()
{
	while(1)
	{
	    char message = 0;

	    if (P1->IN & BUTTON1) // Sets the status of button1 to the message
	        message |= PRESSED1;
	    else
	        message |= RELEASED1;

	    if (P1->IN & BUTTON2) // Sets the status of button2 to the message
	        message |= PRESSED2;
	    else
	        message |= RELEASED2;

	    UART_OutChar(message); // Sends the message to the MAIN board

		// Show the color that the MAIN board sent
		show_color((color)UART_InChar());
	}
}


 /*
 * IRQ handler for port 1
 *
 * Reset the game.
 */
void PORT1_IRQHandler(void){

  uint32_t status;

  status = P1->IFG; // get the interrupt status for Port 1
  P1->IFG &= ~BIT4; // Clear the interrupt for bit 4  

  if(status & BIT4) // Check the bit 4
  {
      points = 0;  // Reset points
      sleep(1);
      fail_blink(); // Show that the game has been reset
  }
}
