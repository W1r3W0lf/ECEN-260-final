#include "msp.h"
#include "stdlib.h"
//#include "time.h"

enum mode {MAIN, REMOTE};

#define board_mode MAIN;

#define LED BIT0
#define LED2RED BIT0
#define LED2GREEN BIT1
#define LED2BLUE BIT2


// The states that the game could be in
enum state {start_mode, game_mode, fail_mode};
// I am thinking of making this look like a FSM.

#define game_mode start;

void restart();
void game();

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

/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	// Setup the red LED.
	P1->DIR |= LED; // Set LED as output.
	P1->OUT &= ~LED; // Turn off LED

	P2->DIR |= (LED2RED|LED2GREEN|LED2BLUE);
	P2->OUT &= ~(LED2RED|LED2GREEN|LED2BLUE);

	// Testing out rand
	while(1)
	{
	    if (rand() %17 == 0)
	        P1->OUT ^= LED;
	    if (rand() %17 == 0)
	        P2->OUT ^= LED2RED;
	    if (rand() %17 == 0)
	        P2->OUT ^= LED2GREEN;
	    if (rand() %17 == 0)
	        P2->OUT ^= LED2BLUE;
	    sleep(1);
	}

}


void game()
{
    int points = 0;
    while(points < 10)
    {
        rand();
    }
}


void restart()
{

}
