/*
 * colors.h
 *
 *  Created on: Dec 10, 2020
 *      Author: Michael Jury
 */

#ifndef COLORS_H_
#define COLORS_H_

#include "msp.h"

// color is a char where the first 3 bits are red green and blue.
typedef int color;

// list of colors
enum colors {
    RED = BIT0,
    GREEN = BIT1,
    BLUE = BIT2,
    YELLOW = (BIT0 | BIT1),
    CYAN = (BIT1 | BIT2),
    MEGANTA = (BIT0 | BIT2),
    WHITE = (BIT0 | BIT1 | BIT2),
    BLACK = 0
};

// Return a random color
color random_color()
{
    return WHITE & rand();
}

// Display a color on the RGB LED
void show_color(color display_color)
{
    P2->OUT &= BLACK; // Clear last COLOR
    P2->OUT |= display_color; // Update the RGB LED
}


#endif /* COLORS_H_ */
