/*
	WordLink
    Copyright (C) 2017 Ian Lewis

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <math.h>   //for rounding

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"

#include "wordlink.h"


void handle_new_touch(float x, float y);
void handle_touch_end(float x, float y);

void mouse_move(ALLEGRO_EVENT event)
{
    Mouse.x = event.mouse.x * inv_scale;
    Mouse.y = event.mouse.y * inv_scale;
    return;
}

void touch_move(ALLEGRO_EVENT event)
{
    Mouse.x = event.touch.x * inv_scale;
    Mouse.y = event.touch.y * inv_scale;
    return;
}

void touch_begin(ALLEGRO_EVENT event)
{
    float x,y;
    x = event.touch.x * inv_scale;
    y = event.touch.y * inv_scale;

    handle_new_touch(x,y);

}

void mouse_down(ALLEGRO_EVENT event)
{
    float x,y;
    x = event.mouse.x * inv_scale;
    y = event.mouse.y * inv_scale;

    handle_new_touch(x,y);
}

void handle_new_touch(float x, float y)
{
    float row,col;
    // check for picking up a letter
    Mouse.tile = NO_TILE;

    row = ((y - ALPHA_Y)/TILE); //work out letter from x and y, if not in range 0-25, do nothing.

    if (row >= 0.0 && row < (float)ALPHA_ROWS) //return;
    {
        col = ((x - ALPHA_X) / TILE);
        if (col >= 0.0 && col < (float) ALPHA_COLS)// return;
        {
            Mouse.tile = (int) row * ALPHA_COLS + (int) col;

            if (Mouse.tile > 25) Mouse.tile = NO_TILE;
        }
        if (Mouse.tile != NO_TILE) {
            Mouse.x = x;
            Mouse.y = y;
            return;
        }
    }
    //no letter, so check for button press
    if ((x > BUTTON_X) && (x < (BUTTON_X + TILE)))
    {
        if ((y > BUTTON_Y) && (y < (BUTTON_Y + TILE)))
            Command = FORWARD;
        else if ((y > BUTTON_Y + TILE) && (y < (BUTTON_Y + 2*TILE)))
            Command = BACK;

    }

}

void touch_end(ALLEGRO_EVENT event)
{
    float x,y;
    x = event.touch.x * inv_scale;
    y = event.touch.y * inv_scale;

    handle_touch_end(x,y);

}

void mouse_up(ALLEGRO_EVENT event)
{
    float x,y;
    x = event.mouse.x * inv_scale;
    y = event.mouse.y * inv_scale;

    handle_touch_end(x,y);
}

void handle_touch_end(float x, float y)
{
    int row,col,i,count;
    char temp[10];

    if (Mouse.tile != NO_TILE)
    {
        row = (int) ((y - CHAIN_Y) / TILE);

        if (row == Chain.current->index)                                //are we level with current word?
        {
            col = (int) ((x - CHAIN_X) / TILE);

            if (col >= 0 && col < Chain.word_length)                    //yes, are we in the word?
            {
                Chain.current->user[col] = Mouse.tile + 'a';                //change the letter

                if (isindict(Chain.current->user,
                             Chain.word_length))   //is the word in the dictionary?
                {
                    Chain.current->status = VALID;                        //yes, mark as good
                    strncpy(temp, Chain.current->user, Chain.word_length + 1);//take a copy
                    Chain.current++;                                      //and bump pointer

                    if (Chain.current->status == LAST)                    //if we've reached the end
                    {
                        count = 0;                                        //check current usr / cpu one letter different
                        for (i = 0; i < Chain.word_length; i++) {
                            if (temp[i] != Chain.current->cpu[i])
                                count++;
                        }

                        if (count == 1) {
                            Command = SUCCESS;
                        } else {
                            messageptr = msg_chainfail;
                            error_timer = 30;                           //too many differences
                            Chain.current--;                            //bump pointer back
                            Chain.current->status = CURRENT;            //and remove 'VALID' status
                        }
                    } else                                                        //NOT reached the end,
                    {
                        strncpy(Chain.current->user, temp,
                                Chain.word_length + 1);  //so copy word into next entry
                        Chain.current->status = CURRENT;                        //and mark that as current
                    }
                } else                                                    //not in dictionary
                {
                    messageptr = msg_notindict;
                    error_timer = 30;                                   //so flag error
                }
            }
        }

        Mouse.tile = NO_TILE;   //drop tile regardless
    }

   return;
}


