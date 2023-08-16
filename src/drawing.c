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

ALLEGRO_BITMAP* info_bmp;

void draw_background(ALLEGRO_TRANSFORM *transform);
void draw_buttons(void);
void draw_coins(void);
void draw_message(void);
void draw_timeout(void);
void draw_more_buttons(void);

void draw_screen(ScreenType screen, float scale)
{
    ALLEGRO_TRANSFORM transform;
    ALLEGRO_COLOR tint;
    ALLEGRO_COLOR White = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR Red   = al_map_rgba(192, 0, 0, 128);
    ALLEGRO_COLOR Black = al_map_rgba(0, 0, 0, 128);
    int w,h,i,j;
    int bgx,bgy,bgw,bgh;    //background
    int x1,x,y;                //temporary coords.
    int letter_x,letter_y;
    char temp[5];

    if (State.halted) return;

	w = al_get_display_width(display);
    h = al_get_display_height(display);

	al_set_clipping_rectangle(0, 0, w, h);

    al_identity_transform(&transform);  		            /* Initialize transformation. */
    al_scale_transform(&transform, scale,scale);
    al_use_transform(&transform);

    //logo
    //tint = al_map_rgba(128,128,128,128);
    //al_draw_tinted_scaled_bitmap(ttglogo,tint,0,0,640,640,w-400,-80,400,400,0);


    switch (State.screen)
    {
        case START: //TTG logo
            al_clear_to_color(al_map_rgb(255, 255, 255));
            w = al_get_display_width(display);
            h = al_get_display_height(display);
            bgw = al_get_bitmap_width(ttglogo);
            bgh = al_get_bitmap_height(ttglogo);
            al_draw_bitmap(ttglogo,(w-bgw)/2,(h-bgh)/2,0);
        break;

        case HOME:
            draw_background(&transform);

            al_scale_transform(&transform,1.5,1.5);
            al_use_transform(&transform);

            y = CHAIN_Y+TILE/2;
            x1 = ((al_get_display_width(display)/2)*State.inv_scale )- 3*TILE;
            x=x1/1.5;
            y/=1.5;

            strncpy(temp,"word",5);
            for (i=0 ; i<4 ; i++)
            {
                if (Timer[TIMER_HOME].value < Timer[TIMER_HOME].start_value-((i+1) *10))
                {
                    letter_x = ((temp[i] - 'a') % 5) * TILE;
                    letter_y = ((temp[i] - 'a') / 5) * TILE;
                }
                else
                {
                    letter_x = 0;
                    letter_y = 6 * TILE;
                }
                al_draw_bitmap_region(alpha, letter_x, letter_y, TILE, TILE, x, y, 0);
                x+=TILE;
            }

            //x = ((al_get_display_width(display)/2)*State.inv_scale )- 3*TILE;
            x=x1/1.5;
            y+=TILE;
            strncpy(temp,"link",5);
            for (i=0 ; i<4 ; i++)
            {
                if (Timer[TIMER_HOME].value < Timer[TIMER_HOME].start_value-((i+5) *10))
                {
                    letter_x = ((temp[i] - 'a') % 5) * TILE;
                    letter_y = ((temp[i] - 'a') / 5) * TILE;
                }
                else
                {
                    letter_x = 0;
                    letter_y = 6 * TILE;
                }
                al_draw_bitmap_region(alpha, letter_x, letter_y, TILE, TILE, x, y, 0);
                x+=TILE;
            }
            al_scale_transform(&transform,0.6666666,0.66666666);
            al_use_transform(&transform);

            draw_coins();
            draw_buttons();
            draw_more_buttons();
        break;
        case INFO:
            draw_background(&transform);

            al_draw_bitmap(info_bmp,0,0,0);

            draw_coins();
            draw_buttons();
        break;
        case COLOURS:
            draw_background(&transform);
            switch(State.ColourItem)
            {
            case BG:
                for (i=0 ; i<State.max_bgs ; i++)
                {
                    x = CHAIN_X+250*(i%6);
                    y = CHAIN_Y+250*(i/6);

                    y+=Mouse.scrollb;

                    if (backgrounds[i] != NULL)
                    {
                        int ybox = al_get_bitmap_height(backgrounds[i])+15;
                        if (ybox > 220)
                            ybox = 220;
                        al_draw_filled_rounded_rectangle(x-15,y-15,x+220,y+ybox,10,10,(State.bg==i)?Red:Black);
                        al_draw_bitmap_region(backgrounds[i],0,0,205,205,x,y,0);
                    }
                    else
                    {
                        al_draw_filled_rectangle(x-15,y-15,x+250,y+250,al_map_rgba(128,128,128,128));
                        al_draw_bitmap_region(icons,ICON_LOCK*TILE,0,TILE,TILE,x+50,y+50,0);
                    }
                }
            break;
            case ALPHA:
                for (i=0 ; i<State.max_alphas ; i++)
                {
                    x = CHAIN_X+210*(i%6);
                    y = CHAIN_Y+210*(i/6);

                    y+=Mouse.scrolla;

                    if (alphas[i] != NULL)
                    {
                        al_draw_filled_rounded_rectangle(x-5,y-5,x+138,y+138,10,10,(State.alpha==i)?Red:Black);
                        al_draw_bitmap_region(alphas[i],0,0,130,130,x,y,0);
                    }
                    else
                    {
                        al_draw_filled_rectangle(x-5,y-5,x+135,y+135,al_map_rgba(128,128,128,128));
                        al_draw_bitmap_region(icons,ICON_LOCK*TILE,0,TILE,TILE,x,y,0);
                    }
                }
            break;
            }
            draw_buttons();
            draw_coins();
            draw_message();

            al_draw_textf(small_font, (State.ColourItem==BG)?Red:White, CHAIN_X + 1*300,  0, ALLEGRO_ALIGN_CENTRE, "Backgrounds");
            al_draw_textf(small_font, (State.ColourItem!=BG)?Red:White, CHAIN_X + 3*300,  0, ALLEGRO_ALIGN_CENTRE, "Letters");
        break;

        case GAME:
            draw_background(&transform);

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


                        if ((i > 0) && (Chain.word[i].cpu[j] != Chain.word[i-1].cpu[j]))
                            al_draw_tinted_bitmap_region(alpha, al_map_rgba(0, 128, 192, 225), letter_x, letter_y, TILE, TILE, x, y,0);
                        else
                            al_draw_bitmap_region(alpha, letter_x, letter_y, TILE, TILE, x, y, 0);


                        //al_draw_bitmap_region(alpha, letter_x, letter_y, TILE, TILE, x, y, 0);
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

                                if ((i > 0) && State.success && (Chain.word[i].cpu[j] != Chain.word[i-1].user[j]))
                                    al_draw_tinted_bitmap_region(alpha, al_map_rgba(0, 128, 192, 225), letter_x, letter_y, TILE, TILE, x, y,0);
                                else
                                    al_draw_bitmap_region(alpha, letter_x, letter_y, TILE, TILE, x, y, 0);



                                //al_draw_bitmap_region(alpha, letter_x, letter_y, TILE, TILE, x, y, 0);
                                x += TILE;
                            }
//                            y += TILE;
                        break;
                        case VALID:
                            for (j = 0; j < Chain.word_length; j++) {
                                letter_x = ((Chain.word[i].user[j] - 'a') % 5) * TILE;
                                letter_y = ((Chain.word[i].user[j] - 'a') / 5) * TILE;
                                if (Chain.word[i].user[j] != Chain.word[i-1].user[j])
                                    al_draw_tinted_bitmap_region(alpha, al_map_rgba(0, 128, 192, 225), letter_x, letter_y, TILE, TILE, x, y,0);
                                else
                                    al_draw_bitmap_region(alpha, letter_x, letter_y, TILE, TILE, x, y, 0);
                                x += TILE;
                            }
                          //  y += TILE;
                        break;
                        case BLANK:
                            for (j = 0; j < Chain.word_length; j++) {
                                //al_draw_tinted_bitmap_region(blank, al_map_rgba(128, 128, 128, 128), 0, 0, TILE, TILE, x, y,0);
                                al_draw_tinted_bitmap_region(alpha, al_map_rgba(128, 128, 128, 128), 0, 6*TILE, TILE, TILE, x, y,0);
                                x += TILE;
                            }
                          //  y += TILE;
                        break;
                        case CURRENT:
                            for (j = 0; j < Chain.word_length; j++) {
                                if (Timer[TIMER_ERROR].value)
                                    tint = al_map_rgba(128, 0, 0, 192);
                                else
                                    tint = al_map_rgba(128, 128, 128, 128);

                                letter_x = ((Chain.word[i].user[j] - 'a') % 5) * TILE;
                                letter_y = ((Chain.word[i].user[j] - 'a') / 5) * TILE;
                                al_draw_tinted_bitmap_region(alpha, tint, letter_x, letter_y, TILE, TILE, x, y, 0);
                                x += TILE;
                            }
                           // y += TILE;
                        break;
                    }
                }
                y += TILE;
            }

            //draw alphabet
            al_draw_bitmap_region(alpha,0,0,TILE*5,TILE*6,ALPHA_X, ALPHA_Y,0);

            //draw dragged tile
            if (Mouse.tile != NO_TILE)
            {
                letter_x = (Mouse.tile % 5) * TILE;
                letter_y = (Mouse.tile / 5) * TILE;
                al_draw_bitmap_region(alpha,letter_x,letter_y,TILE,TILE,Mouse.x-TILE/2,Mouse.y-TILE/2,0);
            }
            al_hold_bitmap_drawing(FALSE);

            draw_buttons();
            draw_message();
            draw_coins();

            if (State.gametype == TIMED)//score and timer bar, for timed play
            {
                al_draw_filled_rectangle(CHAIN_X + TILE, 0, CHAIN_X+TILE+(8*TILE*Timer[TIMER_GAME].value/1000),TILE/2,Red);
                al_draw_textf(small_font, al_map_rgb(255, 255, 255),CHAIN_X+8*TILE, 0,  ALLEGRO_ALIGN_LEFT, "%d",State.score);

                if (State.timeout)
                {
                    draw_timeout();
                    draw_more_buttons();
                }
            }

        break;
    }

    al_identity_transform(&transform);  		            /* Initialize transformation. */
    al_use_transform(&transform);

    al_flip_display();

    return;
}

void draw_background(ALLEGRO_TRANSFORM *transform_ptr)
{
    int bgx,bgy, bgw, bgh, w , h;

    al_clear_to_color(al_map_rgb(0, 0, 0));

    al_scale_transform(transform_ptr,1.5,1.5);
    al_use_transform(transform_ptr);

	w = al_get_display_width(display);
    h = al_get_display_height(display);
    w*=State.inv_scale;
    h*=State.inv_scale;

	bgw = al_get_bitmap_width(background);
	bgh = al_get_bitmap_height(background);

    for(bgy=0 ; bgy<h ; bgy+=bgh)
    {
        for(bgx=0 ; bgx<w ; bgx+=bgw)
        {
            al_draw_bitmap(background,bgx,bgy,0);
        }
    }

    al_scale_transform(transform_ptr,0.6666666666,0.6666666666);
    al_use_transform(transform_ptr);
}


void draw_buttons(void)
{
    int i;
    float x = BUTTON_X;
    float y = BUTTON_Y;
    for (i=0 ; Buttons[i].icon != NO_ICON ; i++) {
        al_draw_filled_circle(x+TILE/2,y+TILE/2,TILE/2,al_map_rgba(128,128,128,128));
        al_draw_bitmap_region(icons, Buttons[i].icon * TILE, 0, TILE, TILE, x, y, 0);
        y+= TILE*1.1;
    }
}

void draw_more_buttons(void)
{
    int i;
    //float x = BUTTON2_X;
    float x = (al_get_display_width(display)/2)*State.inv_scale - TILE*1.0;
    float y = BUTTON2_Y;
    for (i=0 ; Buttons2[i].icon != NO_ICON ; i++) {
        al_draw_filled_circle(x+TILE/2,y+TILE/2,TILE/2,al_map_rgba(128,128,128,128));
        al_draw_bitmap_region(icons, Buttons2[i].icon * TILE, 0, TILE, TILE, x, y, 0);
        x+= TILE*1.1;
    }
}

void draw_coins(void)
{
    static int coinframe = 0;

    coinframe++;
    if (coinframe == 60)
        coinframe = 0;

    int w = al_get_display_width(display);
    w*=State.inv_scale;

    al_draw_filled_rectangle(0,0,w,TILE/2,al_map_rgba(0,0,0,128));
    al_draw_bitmap_region(coin,(coinframe)*50,0,50,50,65,10,0);
    al_draw_textf(small_font, al_map_rgb(255, 255, 255),TILE, 0,  ALLEGRO_ALIGN_LEFT, "%d",State.display_coins);
    //al_draw_textf(small_font, al_map_rgb(255, 255, 255),TILE, TILE/2,  ALLEGRO_ALIGN_LEFT, "%d",Mouse.scroll);
}

void draw_message(void)
{
    static int alpha=0;

    if (Message.count)
    {
        int w = al_get_display_width(display);

        int sx = SCREENX;

        w*=State.inv_scale;
        if (Message.count == 60)
            alpha = 0;

        Message.count--;

        if(Message.count < 15)
        {
            Message.x += w/15;
            alpha -= 10;
        }

        if (Message.count >= 45)
        {
            Message.x += w/15;
            alpha +=10;
        }
        //Message.x = 0;
        al_draw_filled_rectangle(0,TILE*7.5,w,TILE*8,al_map_rgba(0,0,0,alpha));
        al_draw_textf(small_font, al_map_rgb(255, 255, 255),w/2+Message.x, TILE*7.5,  ALLEGRO_ALIGN_CENTRE, "%s",Message.message);
    }
}

void draw_timeout(void)
{
    int w = al_get_display_width(display)  * State.inv_scale;
    int h = al_get_display_height(display) * State.inv_scale;

    al_draw_filled_rounded_rectangle(w/4,CHAIN_Y,3*w/4,CHAIN_Y+6*TILE,50,50,al_map_rgba(0,0,0,160));

    al_draw_textf(small_font, al_map_rgb(255, 255, 255), w/2, CHAIN_Y+1*TILE, ALLEGRO_ALIGN_CENTRE, "TIME'S UP!");
    al_draw_textf(small_font, al_map_rgb(255, 255, 255), w/2, CHAIN_Y+2*TILE, ALLEGRO_ALIGN_CENTRE, "You scored %d",State.score);
    if (State.score >= State.highscore)
    {
        al_draw_textf(small_font, al_map_rgb(255, 255, 255), w/2, CHAIN_Y+3*TILE, ALLEGRO_ALIGN_CENTRE, "New Highscore!");
    }
    else
        al_draw_textf(small_font, al_map_rgb(255, 255, 255), w/2, CHAIN_Y+3*TILE, ALLEGRO_ALIGN_CENTRE, "Highscore: %d",State.highscore);

}

void make_info_bitmap(void)
{
    char temp[100];
    int w,h;
    int y = CHAIN_Y;

    w = al_get_display_width(display)*State.inv_scale;
    h = al_get_display_height(display)*State.inv_scale;

    info_bmp = al_create_bitmap(w, h*3);			//create a bitmap
    al_set_target_bitmap(info_bmp);					//set it as the default target for all al_draw_ operations

    al_clear_to_color(al_map_rgba(0,0,0,128));

    ALLEGRO_FILE* infofile = al_fopen("info.txt","r");

    while(al_fgets(infofile,temp,100))
    {
        al_draw_textf(small_font, al_map_rgb(255, 255, 255), w/2, y, ALLEGRO_ALIGN_CENTRE, "%s",temp);
        y += 60;
    }
    al_set_target_backbuffer(display);			//Put default target back
}
