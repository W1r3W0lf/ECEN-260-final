#include "msp.h"
#include "stdlib.h"
//#include "time.h"

enum mode {MAIN, REMOTE};


// massages from remote to main
enum remote_messages {BUTTON2, BUTTON3};

// color is a char where the first 3 bits are red green and blue.
typedef int color;

#define board_mode MAIN

#define LED BIT0

enum colors {
    RED = BIT0,
    GREEN = BIT1,
    BLUE = BIT2,
    YELLOW = (BIT0 | BIT1),
    CYAN = (BIT1 | BIT2),
    MEGANTA = ( BIT0 |  BIT2),
    WHITE = ( BIT0 |  BIT1 |  BIT2),
    BLACK = 0
};




// The states that the game could be in
enum state {start_mode, game_mode, fail_mode};
// I am thinking of making this look like a FSM.

#define game_mode start

void restart();
void game();
void helper();


// It's about a second per sec
void sleep(unsigned int sec){
    while(sec-- > 0)
    {
        int tick = 7;
        while(tick-- > 0)
        {
            unsigned int delay = 65535;
            while(delay-- > 0);
        }
    }
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
        P1->DIR &= ~BIT1;     // set up pin P1.1 as input
        P1->REN |= BIT1;      // connect pull resistor to pin P1.1
        P1->OUT |= BIT1;      // configure pull resistor as pull up
        P1->IFG &= ~BIT1;     // clear the interrupt flag for pin P1.1
        P1->IE |= BIT1;       // enable the interrupt for pin P1.1

        // Setup button 1
        P1->DIR &= ~BIT4;     // set up pin P1.1 as input
        P1->REN |= BIT4;      // connect pull resistor to pin P1.1
        P1->OUT |= BIT4;      // configure pull resistor as pull up
        P1->IFG &= ~BIT4;     // clear the interrupt flag for pin P1.1
        P1->IE |= BIT4;       // enable the interrupt for pin P1.1

}


/**
 * main.c
 */
void main(void)
{
	setup();

	if (board_mode == MAIN)
	{
	    game();
	}
	else
	{
	    helper();
	}


}


color random_color()
{
    color new_color;
    new_color = 0; // Zero out the new color

    // make the random colors.
    new_color = (RED | GREEN | BLUE) & rand();

    return new_color;
}


void show_color(color display_color)
{
    P2->OUT &= ~WHITE;   // Clear last COLOR
    P2->OUT |= display_color; // Update the RGB LED
}


void game()
{
    int points = 0;
    color current_color;
    int inputs = 0;
    while(points < 10)
    {
        // Make new color to be displayed
        current_color = random_color();
        show_color(current_color);
        P1->OUT ^= LED;
        sleep(1);

        // If the Inputs match the color add points
        if (! (current_color ^ inputs) )
        {

        }
        else // If the colors and the Inputs don't match do something
        {

        }

    }
}


void restart()
{

}
