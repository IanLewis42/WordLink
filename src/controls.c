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
void limit_scroll(void);

void mouse_move(ALLEGRO_EVENT event)
{
    Mouse.x = event.mouse.x * State.inv_scale;
    Mouse.y = event.mouse.y * State.inv_scale;

    if (Mouse.button)
    {
        if (State.ColourItem == ALPHA)
            Mouse.scrolla += event.mouse.dy*State.inv_scale;
        else
            Mouse.scrollb += event.mouse.dy*State.inv_scale;
        limit_scroll();
        Mouse.move   += fabsf(event.mouse.dy*State.inv_scale);
    }
    return;
}

void touch_move(ALLEGRO_EVENT event)
{
    Mouse.x = event.touch.x * State.inv_scale;
    Mouse.y = event.touch.y * State.inv_scale;

    if (State.ColourItem == ALPHA)
        Mouse.scrolla += event.touch.dy*State.inv_scale;
    else
        Mouse.scrollb += event.touch.dy*State.inv_scale;
    limit_scroll();
    Mouse.move   += fabsf(event.touch.dy*State.inv_scale);
    return;
}

void limit_scroll(void)
{
    int min_scroll;

    if (Mouse.scrolla > 0) Mouse.scrolla = 0;
    if (Mouse.scrollb > 0) Mouse.scrollb = 0;

    min_scroll = (int)(-1 * State.max_alphas *210 / 6);
    if (Mouse.scrolla < min_scroll)
        Mouse.scrolla = min_scroll;

    min_scroll = (int)(-1 * State.max_bgs * 300 / 4);
    if (Mouse.scrollb < min_scroll)
        Mouse.scrollb = min_scroll;

}

void touch_begin(ALLEGRO_EVENT event)
{
    float x,y;
    x = event.touch.x * State.inv_scale;
    y = event.touch.y * State.inv_scale;

    handle_new_touch(x,y);
}

void mouse_down(ALLEGRO_EVENT event)
{
    float x,y;
    x = event.mouse.x * State.inv_scale;
    y = event.mouse.y * State.inv_scale;

    handle_new_touch(x,y);
}

void handle_new_touch(float x, float y)
{
    float row,col;
    // check for picking up a letter
    Mouse.tile = NO_TILE;

    Mouse.button = true;
    Mouse.move = 0;
    int max_button;

    switch(State.screen)
    {
        case GAME:
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
        break;
        case COLOURS:
            if (x > CHAIN_X && y < CHAIN_Y)
            {
                al_play_sample_instance( misc_menu_inst);
                if (x < SCREENX/2)
                    Command = CMD_BG;
                else
                    Command = CMD_ALPHA;
            }
        break;
    }
    //no letter, so check for button press
    for (max_button=0 ; Buttons[max_button].command != NO_COMMAND; max_button++);

    if ((x > BUTTON_X) && (x < (BUTTON_X + TILE)))
    {
        y-=BUTTON_Y;
        y=y/(TILE*1.1);

        if (y>=0 && y<=max_button)
        {
            Command = Buttons[(int)y].command;
            al_play_sample_instance( misc_menu_inst);
        }
    }

    if(State.screen == HOME || State.timeout)
    {
        for (max_button=0 ; Buttons2[max_button].command != NO_COMMAND; max_button++);
        if ((y > BUTTON2_Y) && (y < (BUTTON2_Y + TILE)))
        {
            //x-=BUTTON2_X;
            x -= (al_get_display_width(display)/2)*State.inv_scale - TILE*1.0;
            x=x/(TILE*1.1);

            if (x>=0 && x<=max_button)
            {
                Command = Buttons2[(int)x].command;
                al_play_sample_instance( misc_menu_inst);
            }
        }
    }
}

void touch_end(ALLEGRO_EVENT event)
{
    float x,y;
    x = event.touch.x * State.inv_scale;
    y = event.touch.y * State.inv_scale;

    handle_touch_end(x,y);
}

void mouse_up(ALLEGRO_EVENT event)
{
    float x,y;
    x = event.mouse.x * State.inv_scale;
    y = event.mouse.y * State.inv_scale;

    handle_touch_end(x,y);
}

void handle_touch_end(float x, float y)
{
    int row,col,i,count;
    char temp[20];

    Mouse.button = false;

    switch(State.screen)
    {
        case GAME:      //Check for tile drop
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
                            al_play_sample_instance( click_inst);
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
                                    al_play_sample_instance( positive_inst);
                                    Command = CMD_SUCCESS;
                                } else {
                                    al_play_sample_instance( negative_inst);
                                    post_message(msg_chainfail);
                                    start_timer(TIMER_ERROR);                   //too many differences
                                    Chain.current--;                            //bump pointer back
                                    Chain.current->status = CURRENT;            //and remove 'VALID' status
                                }
                            } else                                              //NOT reached the end,
                            {
                                strncpy(Chain.current->user, temp,
                                        Chain.word_length + 1);                 //so copy word into next entry
                                Chain.current->status = CURRENT;                //and mark that as current
                            }
                        } else                                                    //not in dictionary
                        {
                            al_play_sample_instance( negative_inst);
                            post_message(msg_notindict);
                            start_timer(TIMER_ERROR);                                   //so flag error
                        }
                    }
                }
                Mouse.tile = NO_TILE;   //drop tile regardless
            }
        break;

        case COLOURS:   //check for selection
            if (Mouse.move < 10)
            {
                if (x > CHAIN_X && (x < CHAIN_X + 5*250 + 220) && y > TILE/2)
                {
                    al_play_sample_instance( misc_menu_inst);
                    switch (State.ColourItem)
                    {
                    case BG:
                        x = (int)((x-CHAIN_X)/250);
                        y = ((int)((y-CHAIN_Y-Mouse.scrollb)/250))*6;
                        i = x + y;
                        if (i >= 0 && i <= State.max_bgs)
                        {
                            if (State.bgs[(int)(i/32)]>>(i%32)&1)
                            {
                                background = backgrounds[i];
                                State.bg = i;
                                save_state();
                            }
                            else if (State.coins >= 100)
                            {
                                State.coins_down += 100;
                                State.coins -= 100;
                                State.bgs[(int)(i/32)] |= (1<<i%32);
                                State.bg = i;
                                save_state();
                                sprintf(temp,"bg/bg%03d.jpg",i);
                                backgrounds[i] = al_load_bitmap(temp);
                                background = backgrounds[i];
                            }
                            else
                            {
                                start_timer(TIMER_ERROR);
                                post_message(msg_nocoins);
                            }
                        }
                    break;
                    case ALPHA:
                        x = (int)((x-CHAIN_X)/210);
                        y = ((int)((y-CHAIN_Y-Mouse.scrolla)/210))*6;
                        i = x + y;

                        if (i >= 0 && i <= State.max_alphas)
                        {
                            if (State.alphas[(int)(i/32)]>>(i%32)&1)
                            {
                                alpha = alphas[i];
                                State.alpha = i;
                                save_state();
                            }
                            else if (State.coins >= 100)
                            {
                                State.coins_down += 100;
                                State.coins -= 100;
                                State.alphas[(int)(i/32)] |= (1<<i%32);
                                State.alpha = i;
                                save_state();
                                sprintf(temp,"tiles/tiles%02d.png",i);
                                alphas[i] = al_load_bitmap(temp);
                                alpha = alphas[i];
                            }
                            else
                            {
                                post_message(msg_nocoins);
                                start_timer(TIMER_ERROR);
                            }
                        }
                    break;
                    }
                }
            }
        break;
    }
   return;
}


