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
#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"

#include "wordlink.h"

#define BLANK_TILE 26

//messages
char* messageptr;
char msg_blank[] = " ";
char msg_start[] = "Drag tiles to make new words.";
char msg_success[] = "Well done!";
char msg_notindict[] = "Sorry, I don't know that word.";
char msg_chainfail[] = "That doesn't complete the chain!";

void draw_screen(ScreenType screen, float scale)
{
    ALLEGRO_TRANSFORM transform;
    ALLEGRO_COLOR tint;
    int w,h,i,j;
    int bgx,bgy,bgw,bgh;    //background
    int x,y;                //temporary coords.
    int letter_x,letter_y;

    if (halted) return;

	w = al_get_display_width(display);
    h = al_get_display_height(display);

	al_set_clipping_rectangle(0, 0, w, h);

	bgw = al_get_bitmap_width(background);
	bgh = al_get_bitmap_height(background);

    al_clear_to_color(al_map_rgb(0, 0, 0));

    w*=inv_scale;
    h*=inv_scale;


	 for(bgy=0 ; bgy<h ; bgy+=bgh)
    {
        for(bgx=0 ; bgx<w ; bgx+=bgw)
        {
            al_draw_bitmap(background,bgx,bgy,0);
        }
    }

    al_identity_transform(&transform);  		            /* Initialize transformation. */
    al_scale_transform(&transform, scale,scale);
    al_use_transform(&transform);

    //logo
    //tint = al_map_rgba(128,128,128,128);
    //al_draw_tinted_scaled_bitmap(ttglogo,tint,0,0,640,640,w-400,-80,400,400,0);


    switch (screen)
    {
        case START:
            al_clear_to_color(al_map_rgb(255, 255, 255));
            w = al_get_display_width(display);
            h = al_get_display_height(display);
            bgw = al_get_bitmap_width(ttglogo);
            bgh = al_get_bitmap_height(ttglogo);
            al_draw_bitmap(ttglogo,(w-bgw)/2,(h-bgh)/2,0);
        break;

        case MENU:
        break;

        case GAME:
            //al_hold_bitmap_drawing(TRUE);
            //draw chain
            y = CHAIN_Y;
            for (i=0 ; i<Chain.length+1 ; i++)
            {
                x = CHAIN_X;
                if (Chain.show)
                {
                    for (j = 0; j < Chain.word_length; j++) {
                        letter_x = ((Chain.word[i].cpu[j] - 'a') % 5) * TILE;
                        letter_y = ((Chain.word[i].cpu[j] - 'a') / 5) * TILE;
                        al_draw_bitmap_region(letters, letter_x, letter_y, TILE, TILE, x, y, 0);
                        x += TILE;
                    }
                }
                else
                {
                    switch (Chain.word[i].status)
                    {
                        case FIRST:
                        case LAST:
                            for (j = 0; j < Chain.word_length; j++) {
                                letter_x = ((Chain.word[i].cpu[j] - 'a') % 5) * TILE;
                                letter_y = ((Chain.word[i].cpu[j] - 'a') / 5) * TILE;
                                al_draw_bitmap_region(letters, letter_x, letter_y, TILE, TILE, x, y, 0);
                                x += TILE;
                            }
//                            y += TILE;
                        break;
                        case VALID:
                            for (j = 0; j < Chain.word_length; j++) {
                                letter_x = ((Chain.word[i].user[j] - 'a') % 5) * TILE;
                                letter_y = ((Chain.word[i].user[j] - 'a') / 5) * TILE;
                                if (Chain.word[i].user[j] != Chain.word[i-1].user[j])
                                    al_draw_tinted_bitmap_region(letters, al_map_rgba(0, 128, 192, 225), letter_x, letter_y, TILE, TILE, x, y,0);
                                else
                                    al_draw_bitmap_region(letters, letter_x, letter_y, TILE, TILE, x, y, 0);
                                x += TILE;
                            }
                          //  y += TILE;
                        break;
                        case BLANK:
                            for (j = 0; j < Chain.word_length; j++) {
                                al_draw_tinted_bitmap_region(blank, al_map_rgba(128, 128, 128, 128), 0, 0, TILE, TILE, x, y,0);
                                x += TILE;
                            }
                          //  y += TILE;
                        break;
                        case CURRENT:
                            for (j = 0; j < Chain.word_length; j++) {
                                if (error_timer)
                                    tint = al_map_rgba(128, 0, 0, 192);
                                else
                                    tint = al_map_rgba(128, 128, 128, 128);

                                letter_x = ((Chain.word[i].user[j] - 'a') % 5) * TILE;
                                letter_y = ((Chain.word[i].user[j] - 'a') / 5) * TILE;
                                al_draw_tinted_bitmap_region(letters, tint, letter_x, letter_y, TILE, TILE, x, y, 0);
                                x += TILE;
                            }
                           // y += TILE;
                        break;
                    }
                }
                y += TILE;
            }
            //draw alphabet
            al_draw_bitmap(letters,ALPHA_X, ALPHA_Y,0);
            /*
            y = ALPHA_Y;
            for (i=0 ; i<ALPHA_ROWS ; i++)
            {
                x=ALPHA_X;
                for (j=0 ; j<ALPHA_COLS ; j++)
                {
                    al_draw_bitmap_region(letters,(i*ALPHA_COLS+j)*TILE,0,TILE,TILE,x,y,0);
                    x+=TILE;
                    if (i*ALPHA_COLS+j == 25) break;
                }
                y+=TILE;
            }*/
            //draw buttons
            x = BUTTON_X;
            y = BUTTON_Y;
            for (i=0 ; i< NUM_BUTTONS ; i++) {
                al_draw_bitmap_region(buttons, i * TILE, 0, TILE, TILE, x, y, 0);
                y+= TILE;
            }

            //draw dragged tile
            if (Mouse.tile != NO_TILE)
            {
                letter_x = (Mouse.tile % 5) * TILE;
                letter_y = (Mouse.tile / 5) * TILE;
                al_draw_bitmap_region(letters,letter_x,letter_y,TILE,TILE,Mouse.x-TILE/2,Mouse.y-TILE/2,0);
            }
            //buttons:
            //reset, back one, hint??, solve? settings

            //score??

            if (success)
            {
                //draw 'next' button
            }

            al_identity_transform(&transform);  		            /* Initialize transformation. */
            //al_scale_transform(&transform, scale,scale);
            al_use_transform(&transform);

            w = al_get_display_width(display);
            h = al_get_display_height(display);
            //message
            al_draw_textf(font, al_map_rgb(255, 255, 255),w/2, 0.85*h,  ALLEGRO_ALIGN_CENTRE, "%s",messageptr);
            al_hold_bitmap_drawing(FALSE);
        break;

    }

    al_flip_display();

    return;
}
