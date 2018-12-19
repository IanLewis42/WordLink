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
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <conio.h>

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"
#ifdef ANDROID
#include <allegro5/allegro_android.h>
#endif

#include "wordlink.h"

#define NAME "WordLink"
#define VERSION "0.1"

//#define SCREENX 800 //540
//#define SCREENY 400 //960

ALLEGRO_FILE* dict;
long seek[26];
int change_map,tried_map;
int easy = TRUE,fail,success=FALSE,found=FALSE;
int error_timer,success;
char word[10],message[100],debug_tries[10],temp[10];

float scale, inv_scale;

int halted = 0;

ChainType Chain;
MouseType Mouse;
ScreenType Screen;
CommandType Command = NO_COMMAND;

void newword(void);
void initchain(void);
int findchain(void);
ChainEntryType* findword(ChainEntryType* current_word, int pos);
ChainEntryType* findword2(ChainEntryType* current_word, int pos);


ALLEGRO_DISPLAY *display;
ALLEGRO_FONT *font;         //debug
float font_scale;
ALLEGRO_BITMAP *background;
ALLEGRO_BITMAP *letters;
ALLEGRO_BITMAP *blank;
ALLEGRO_BITMAP *buttons;
ALLEGRO_BITMAP *ttglogo;
//Sounds
ALLEGRO_VOICE *voice;
ALLEGRO_MIXER *mixer;
ALLEGRO_SAMPLE *clunk;

ALLEGRO_FILE* logfile;

#ifndef ANDROID
int main(int argc, char **argv) {
    game (argc, argv);
    return 0;
}
#endif

int game(int argc, char **argv )
//int main(int argc, char* argv[])
{
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_EVENT event;
    int tries=0;
    int searching=FALSE,new_event;
    int count = 0;
    int error = 0;

    error = al_init();  /* Init Allegro 5 + addons. */

#ifndef ANDROID
	//set path
    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	al_change_directory(al_path_cstr(path, '/'));  // change the working directory

	//open logfile in executable directory
	logfile = al_fopen("logfile.txt","w");
    al_fprintf(logfile,"%s %s\n",NAME,VERSION);
    al_fprintf(logfile,"Init Allegro\n");
#endif // ANDROID

    //init other bits of allegro
    error = al_init_image_addon();
    error = al_init_primitives_addon();
    error = al_init_font_addon();
    error = al_init_ttf_addon();
    error = al_install_mouse();
    error = al_install_keyboard();
    error = al_install_audio();
     error = al_init_acodec_addon();
#ifdef ANDROID
    al_android_set_apk_fs_interface();
#endif


	//al_fprintf(logfile,"Init Allegro done\n");

    //al_fprintf(logfile,"Generate change-one-letter word puzzle\n");

    srand(time(0));

    //Screen = START;

#ifndef ANDROID
	//change directory to data, where all resources live (images, fonts, sounds and text files)
	al_append_path_component(path, "data");
	al_change_directory(al_path_cstr(path, '/'));  // change the working directory
#endif // ANDROID


    //al_fprintf (logfile,"\nSTART:%s (%d)\n",word,Chain.word_length);

#ifdef _WIN32
    al_set_new_display_flags(ALLEGRO_RESIZABLE);//ALLEGRO_WINDOWED);// | ALLEGRO_RESIZABLE);
    //#define SCREENX 800
    //#define SCREENY 400
#endif // _WIN32
#ifdef ANDROID
    al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS,ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE,ALLEGRO_REQUIRE);
#endif

    int screenx = SCREENX;
    int screeny = SCREENY;

    //display = al_create_display(SCREENX, SCREENY);
    display = al_create_display(1920/2, 1080/2);

    scale = 1.0*((float)al_get_display_width(display)/SCREENX);
    inv_scale = 1.0/scale;

    al_set_window_title(display, NAME);

    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);// + ALLEGRO_MIN_LINEAR);

    //if ((icon = al_load_bitmap("gs_icon.png")) == NULL)  al_fprintf(logfile,"gs_icon.png load fail\n");
    //if (icon) al_set_display_icon(display, icon);

    //APK FILE INTERFACE!!!!!!

    //LoadFonts();
	//al_fprintf(logfile,"Creating Events\n");
	timer = al_create_timer(1.0 / 30);
	queue = al_create_event_queue();
	//if (logfile) al_fflush(logfile);


    //if (al_install_touch_input())
    //    al_fprintf(logfile,"Init allegro touch input\n");


    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));

#ifdef ANDROID
    al_install_touch_input();
    al_register_event_source(queue, al_get_touch_input_event_source());
#else
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
#endif


    Mouse.tile = NO_TILE;
/*
    if ((background = al_load_bitmap("background.png")) == NULL)
        al_fprintf(logfile,"background.jpg load fail");
    if ((letters = al_load_bitmap("letters.png")) == NULL)
        al_fprintf(logfile,"letters.png load fail");
    if ((buttons = al_load_bitmap("buttons.png")) == NULL)
        al_fprintf(logfile,"buttons.png load fail");
    if ((ttglogo = al_load_bitmap("tootired.png")) == NULL)
        al_fprintf(logfile,"tootired.png load fail");
*/

#ifdef ANDROID
    al_android_set_apk_file_interface();
#endif

    font = al_load_font("Audiowide-Regular.ttf",50,0);
    al_draw_text(font, al_map_rgba(0,0,0,0), 0, 0, 0, "!.ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    int max_bmp = al_get_display_option(display, ALLEGRO_MAX_BITMAP_SIZE);

    background = al_load_bitmap("background.png");
    letters = al_load_bitmap("letters.png");
    blank = al_load_bitmap("blank.png");
    buttons = al_load_bitmap("buttons.png");
    ttglogo = al_load_bitmap("tootired.png");

    al_start_timer(timer);

    newword();
/*

    int i = 0;
    while(1)
    {
       int w,h,bgw,bgh;


        al_wait_for_event(queue,&event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
            al_acknowledge_resize(display);
        else {
            al_clear_to_color(al_map_rgb(i & 0xff, i & 0xff, i & 0xff));
            //w = al_get_display_width(display);
            //h = al_get_display_height(display);
            //bgw = al_get_bitmap_width(ttglogo);
            //bgh = al_get_bitmap_height(ttglogo);
            //al_draw_bitmap(ttglogo,(w-bgw)/2,(h-bgh)/2,0);

            i += 10;

            al_flip_display();
        }
    }
*/
    while (1)
    {
        //do a bit of looking for a chain, if we're looking
        if (searching)
        {
            tries++;
            sprintf(debug_tries,"%d",tries);
            messageptr = debug_tries;
            if (tries == 20)
            {
                newword();
                tries = 0;
            }
            else
            {
                if (findchain())
                {
                    searching = FALSE;
                    Chain.word[Chain.length].status = LAST;
                    Chain.current = &Chain.word[1];
                    Chain.current->status = CURRENT;
                    strncpy(Chain.word[0].user, Chain.word[0].cpu,Chain.word_length+1);
                    strncpy(Chain.word[1].user, Chain.word[0].cpu,Chain.word_length+1);
                    strncpy(Chain.word[Chain.length].user, Chain.word[Chain.length].cpu,Chain.word_length+1);
                }
            }
            if (!al_is_event_queue_empty(queue))
            {
                al_get_next_event(queue, &event);
                new_event = TRUE;
            }
        }

        else
        {
            al_wait_for_event(queue,&event);
            new_event = TRUE;
        }

        //handle events
        if (new_event)
        {
            new_event = FALSE;
            switch(event.type)
            {
                case ALLEGRO_EVENT_DISPLAY_RESIZE:
                    al_acknowledge_resize(display);
                    scale = 1.0*((float)al_get_display_width(display)/SCREENX);
                    inv_scale = 1/scale;
                break;
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    exit(0);
                break;
                case  ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:   //we've been sidelined by the user/os
                        al_acknowledge_drawing_halt(display);   //acknowledge
                        halted = true;                          //flag to drawing routines to do nothing
                        al_stop_timer(timer);                   //no more timer events, so we should do nothing, saving battery
                        //al_set_default_voice(NULL);             //destroy voice, so no more sound events, ditto.
                        //al_destroy_voice(voice);
                break;
                case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING: //we've been restored
                        al_acknowledge_drawing_resume(display); //ack
                        halted = false;                         //remove flag
                        al_start_timer(timer);                  //restart timer
                        //voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);  //restart audio
                        //al_attach_mixer_to_voice(mixer, voice);
                break;
                case ALLEGRO_EVENT_KEY_CHAR:
                    if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                        exit(0);
                    else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                    {
                        Command = FORWARD;
                        /*
                        newword();
                        success = FALSE;
                        //message[0]=0;
                        messageptr = msg_blank;
                        searching = TRUE;
                        tries = 0;
                         */
                    }
                break;

                //handle touches
                case ALLEGRO_EVENT_TOUCH_MOVE:
                    touch_move(event);
                    break;
                case ALLEGRO_EVENT_TOUCH_BEGIN:
                    touch_begin(event);
                    break;
                case ALLEGRO_EVENT_TOUCH_END:
                    touch_end(event);
                    break;

                //handle mouse
                case ALLEGRO_EVENT_MOUSE_AXES:
                    mouse_move(event);
                break;
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                    mouse_down(event);
                break;
                case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                    mouse_up(event);
                break;

                //redraw on timer - draw fn handles start, menu, game screens?
                case ALLEGRO_EVENT_TIMER:
                    if (count <= 30)
                    {
                        draw_screen(START, 1);
                        count++;
                        if (count == 30)
                            searching = TRUE;
                    }
                    else if ((al_event_queue_is_empty(queue)) || searching)
                        draw_screen(GAME, scale);

                    if (error_timer)
                    {
                        error_timer--;
                        if (error_timer == 0)
                        {
                            strncpy(Chain.current->user,((Chain.current)-1)->user,Chain.word_length+1);
                            messageptr = msg_blank;
                        }
                    }
                break;
                default:
                break;
                //when level finished, searching = true.
            }
        }
        //check for commands
        switch (Command)
        {
            case SUCCESS:
                success = TRUE;
                messageptr = msg_success;
            break;
            case FORWARD:
                if (success || Chain.show)
                {
                    newword();
                    success = FALSE;
                    //message[0]=0;
                    messageptr = msg_blank;
                    searching = TRUE;
                    tries = 0;
                    draw_screen(GAME, scale);
                }
                else
                {
                    Chain.show = TRUE;
                }

            break;
            case BACK:
                //copy word forward from valid->current.??
                if (Chain.current->status != LAST)
                    Chain.current->status = BLANK;       //wipe current word

                Chain.current--;                        //bump pointer back

                if (Chain.current->status == FIRST)     //if we've hit the start
                    Chain.current++;                    //bump it on again

                Chain.current->status = CURRENT;    //set this one to be current.

                Chain.current--;                        //bump pointer back
                strncpy(temp,Chain.current->user,Chain.word_length+1);
                Chain.current++;
                strncpy(Chain.current->user,temp,Chain.word_length+1);
            break;

            case NO_COMMAND:
            default:
            break;
        }
        Command = NO_COMMAND;
    }

    al_fclose(dict);
    return 0;
}

void newword()
{
    int chosen,count=0,i;
    char name[20], dict_word[50];

    Chain.word_length = 4+rand()%2;
    Chain.length = Chain.word_length;

    initchain();

    if (dict != NULL) al_fclose(dict);

    //printf("length = %d",Chain.word_length);
    sprintf(name,"dict%d.txt",Chain.word_length);
    dict = al_fopen(name,"r");
    if (dict == NULL)
    {
        printf("Failed to open dictionary file:%s\n",name);
        return;
    }

    //while (fgets(dict_word,50,dict) != NULL) count++;        //count words
    while (al_fgets(dict,dict_word,50) != NULL) count++;
    //rewind(dict);
    al_fseek(dict, 0, SEEK_SET);

    chosen = rand()%count;                              //pick random word
    //printf("count = %d, chosen = %d\n",count,chosen);
    //rewind(dict);
    al_fseek(dict, 0, SEEK_SET);

    for (i=0 ; i<chosen ; i++)
        //fgets(word,50,dict);
        al_fgets(dict,word,50);

    word[strcspn(word, "\n")] = 0;      //trim cr/lf
    //printf("word = %s\n",word);
    //rewind(dict);
    al_fseek(dict, 0, SEEK_SET);

    //work out seek positions for each letter
    for (i=0 ; i<26 ; i++)
    {
        while(1)
        {
            seek[i] = al_ftell(dict);          //mark position
            //fgets(dict_word,50,dict);       //read word
            al_fgets(dict,dict_word,50);
            if (dict_word[0] == i + 'a')    //if it starts with the right letter, break so we increment i
            {
                 //printf("%s",dict_word);
                 break;
            }
            if (dict_word[0] > i + 'a') //we've 'skipped ahead, meaning there are no words starting with this letter
            {
                break;                  //break anyway; we'll never have a word starting with this letter in a chain,
            }                           // if user puts one in, we won't find it regardless.
        }
    }

    Chain.current = &Chain.word[0];
    strncpy(Chain.current->cpu,word,Chain.word_length+1);

    Chain.word[0].status = FIRST;
    //Chain.word[Chain.length].status = LAST;
    return;
}

void initchain(void)
{
    int i;

    for(i=0 ; i<MAX_CHAIN_LENGTH ; i++)
    {
        Chain.word[i].cpu[0] =0; //terminate strings
        Chain.word[i].user[0]=0; //terminate strings
        Chain.word[i].status = BLANK;
        Chain.word[i].index = i;
    }
    Chain.current = & Chain.word[0];
    Chain.last = Chain.current;
    Chain.show = false;
    return;
}

int findchain(void)
{
    int changed,pos;

    Chain.current = &Chain.word[0];
    strncpy(Chain.current->cpu,word,Chain.word_length+1);

    //reset variables
    fail = 0;                                               //successive failures to find the next word
    tried_map = 0;                                          //bitmap of positions we've tried
    change_map = 0;                                         //bitmap of which letters have been changed

    if (Chain.length > Chain.word_length) easy = FALSE;       //otherwise we lock up....
                                                        //in this case, need to check new word against all old words....
    //change as many letters as the chain length
    for (changed=0 ; changed<Chain.length ; )//i++)
    {
        //printf(".");
        while(1)        //pick random position to change - don't duplicate
        {
            pos = rand() % Chain.word_length;
            if ( ((change_map & (1<<pos)) == 0) && ((tried_map & (1<<pos)) == 0 ) ) break;
        }

        tried_map |= (1<<pos);          //remember we tried this position

        Chain.last = Chain.current;
        Chain.current = findword2(Chain.current, pos);    //findword(2) takes and returns returns pointer into word list.
                                                        //If unchanged, no new word found
        if ( Chain.current != Chain.last)
        {
            fail=0;                                     //new word, so reset fail count
            tried_map = 0;                                  //and tried map
            changed++;
        }
        else
            fail++;

        if (changed==Chain.length)  return 1;      //changed every letter, so done!

        if (easy)
        {
            if (fail == (Chain.word_length - changed))            //consecutive failures == unchanged positions. So give up.
                break;
        }
        else
        {
            if (fail == (Chain.word_length))                      //consecutive failures == positions. So give up.
                break;
        }
    }
    return 0;
}

/*
Alternative approach:
Run through entire dictionary once. For each entry, check whether it differs from original word
by exactly one letter. If so, store in array (need big array..... could limit to (say)100 matches
though, I guess)
Pick randomly from array of matches.
Not sure if this is more efficient or not.......
*/
ChainEntryType* findword2(ChainEntryType* current_word, int pos)
{
    wordtype matches[100];    //matches
    ChainEntryType* wordptr;
    int i,j=0,fail,chosen;//,i;
    char dict_word[50];

    //rewind(dict);
    al_fseek(dict,0,SEEK_SET);

    //while (fgets(dict_word,50,dict) != NULL)        //get word, exit on eof
    while (al_fgets(dict,dict_word,50) != NULL)        //get word, exit on eof
    {
        //check for only difference in position 'pos'
        fail=0;
        for (i=0 ; i<Chain.word_length ; i++)
        {
            if (i==pos)
            {
                if (dict_word[i] == current_word->cpu[i])
                {
                    fail = 1;
                    break;
                }
            }
            else
            {
                if (dict_word[i] != current_word->cpu[i])
                {
                    fail = 1;
                    break;
                }
            }
        }
        if (fail == 0)
        {
            //check here for duplication
            for (wordptr=&Chain.word[0] ; wordptr<current_word ; wordptr++)
            {
                if (strncmp(dict_word,wordptr->cpu,Chain.word_length)==0)
                    fail = 1;
            }
            if (fail == 0)
            {
                strncpy(matches[j].word,dict_word,Chain.word_length);
                matches[j].word[Chain.word_length]=0;
                j++;
            }
        }
    }

    if (j==0) return current_word; //fail

    chosen = rand()%j;                              //pick one of our matches at randonm
    if (easy)
        change_map |= (1<<pos);                    //remember we changed this position
    else
    {

    }

    tried_map = 0;                                                  //reset which positions we tried
    current_word++;                                                 //inc pointer
    strncpy(current_word->cpu,matches[chosen].word,Chain.word_length+1); //copy this word to next word in sequence

    return(current_word);
}

#if 0
wordtype* findword(wordtype* current_word, int pos)
{
    int letter,letter_map;
    char temp[10];
/*
    //pick random position to change - don't duplicate
    while(1)
    {
        pos = rand() % word_length;
        if ( ((change_map & (1<<pos)) == 0) && ((tried_map & (1<<pos)) == 0 ) ) break;
    }
*/
    tried_map |= (1<<pos);          //remember we tried this position
    letter = word[pos];         //existing letter that we'll replace
    letter_map = 1<<(letter-'a');  //set bit to show us not to use it

    found = FALSE;
    //position decided - try every letter once, but in a random order, so we can generate different answers
    while(!found)
    {
        while ((letter_map & (1<<(letter-'a'))) != 0)      //don't use a letter we've already tried
        {
            letter = (rand() % 26) + 'a';                   //random letter
        }

        letter_map |= (1<<(letter-'a'));                   //remember we've tried this letter
        strncpy(temp,current_word->word,word_length+1);    //make copy with random change
        temp[pos] = letter;

        if (isindict(temp,word_length))                  //check if it's in dictionary
        {
            if (easy)
                change_map |= (1<<pos);                    //remember we changed this position
            else
            {
                //check against rest of chain
            }
            found = TRUE;                               //remember we've got a match
            //strncpy(word,temp,word_length+1);           //copy it back for next attempt

            //fail = 0;                                   //reset fail counter
            tried_map = 0;                                  //reset which positions we tried
            //i++;                                        //inc replaced letter count
            //strncpy(words[i].word, word,word_length+1); //remember next word in sequence
            //if (i==chain_length) success = TRUE;        //changed every letter, so done!
            current_word++;
            strncpy(current_word->word, temp,word_length+1); //remember next word in sequence
        }

        if (letter_map == 0x3ffffff)    //tried every letter
        {
            //fail++;
            break;

        }
    }//end of while(1) i.e. replaced one letter (or given up)

    letter_map = 0;        //so reset which letters were used

    //return i;
    return (current_word);
}
#endif

int isindict(char* word, int length)
{
    char dict_word[50];

    al_fseek(dict,seek[word[0]-'a'],SEEK_SET);         //send dictionary to start of this letter

    //while (fgets(dict_word,50,dict) != NULL)        //get word, exit on eof
    while (al_fgets(dict,dict_word,50) != NULL)        //get word, exit on eof
    {
        if (dict_word[0] != word[0])                //if the first letter doesn't match, we must be past, so skip
            return 0;

        if (strncmp(word, dict_word, length) == 0)  //if the words match we have a valid word
              return 1;
    }
    return 0;

}
