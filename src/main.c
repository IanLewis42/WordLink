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
#define VERSION "1.2"

//#define SCREENX 800 //540
//#define SCREENY 400 //960

ALLEGRO_FILE* dictfile;
long seek[26];
int change_map,tried_map;
int easy = TRUE,fail,found=FALSE;
int searching=FALSE;
//int game_timer = 0;error_timer=0,success_timer = 0,success;
char word[10],message[100],debug_tries[10],temp[20];

DictType dict4, dict5;              //structures for referncing dictionaries in ram. 4 & 5 letter words
DictType* dict;                     //pointer to current dictionary
ChainType Chain;                    //word chain
MouseType Mouse;
StateType State;
CommandType Command = NO_COMMAND;
MessageType Message;
TimerType Timer[NUM_TIMERS] = { {45,0,0,CMD_FORWARD},     //Success timer
                                {30,0,0,CMD_REVERT},      //Error timer
                                {1000,0,0,CMD_TIMEOUT},  //Timed game timer
                                {110,0,0,CMD_RESTART}};  //home screen animation timer
//buttons for each screen type
ButtonType HomeButtons[] = {{ICON_INFO, CMD_INFO}, {ICON_COLOURS, CMD_COLOURS}, {NO_ICON, NO_COMMAND}};//{ICON_PLAY, CMD_PLAY}, {ICON_TIMED, CMD_TIMED},
ButtonType ColoursButtons[] = {{ICON_HOME, CMD_HOME}, {NO_ICON, NO_COMMAND}};
ButtonType InfoButtons[] = {{ICON_HOME, CMD_HOME}, {NO_ICON, NO_COMMAND}};
ButtonType GameButtons[] = {{ICON_HOME, CMD_HOME}, {ICON_SOLVE, CMD_SOLVE},  {ICON_UNDO, CMD_BACK}, {NO_ICON, NO_COMMAND}};
ButtonType TimedGameButtons[] = {{ICON_HOME, CMD_HOME}, {ICON_UNDO, CMD_BACK}, {NO_ICON, NO_COMMAND}};
ButtonType SolvedButtons[] = {{ICON_HOME, CMD_HOME}, {ICON_NEXT, CMD_FORWARD},  {ICON_UNDO, CMD_BACK}, {NO_ICON, NO_COMMAND}};
ButtonType* Buttons = HomeButtons;

//extra buttons
ButtonType HomeButtons2[]   = {{ICON_PLAY, CMD_PLAY},  {ICON_TIMED, CMD_TIMED}, {NO_ICON, NO_COMMAND}};
ButtonType TimeOutButtons[] = {{ICON_TIMED, CMD_TIMED},{ICON_HOME, CMD_HOME},   {NO_ICON, NO_COMMAND}};
ButtonType* Buttons2 = HomeButtons2;

//messages
char* messageptr;
char msg_blank[] = " ";
char msg_start[] = "Drag tiles to make new words.";
char msg_start_timed[] = "Complete the chain before time runs out.";
char msg_success[] = "Well done!";
char msg_notindict[] = "Sorry, I don't know that word.";
char msg_chainfail[] = "That doesn't complete the chain!";
char msg_nocoins[] = "Sorry, you don't have enough coins.";
char msg_costs[] = "Everything costs 100 coins.";
char msg_timeout[] = "Time's Up!";

//allegro vars
ALLEGRO_DISPLAY *display;
//ALLEGRO_FONT *font;
ALLEGRO_FONT *small_font;
ALLEGRO_BITMAP *background;
ALLEGRO_BITMAP *alpha;
ALLEGRO_BITMAP *icons;
ALLEGRO_BITMAP *ttglogo;
ALLEGRO_BITMAP *coin;
ALLEGRO_BITMAP *backgrounds[100];    //used in select screen
ALLEGRO_BITMAP *alphas[100];          //used in select screen
//Sounds
ALLEGRO_VOICE *voice;
ALLEGRO_MIXER *mixer;
ALLEGRO_SAMPLE *click;
ALLEGRO_SAMPLE *misc_menu;
ALLEGRO_SAMPLE *negative;
ALLEGRO_SAMPLE *positive;

ALLEGRO_SAMPLE_INSTANCE* click_inst;
ALLEGRO_SAMPLE_INSTANCE* misc_menu_inst;
ALLEGRO_SAMPLE_INSTANCE* negative_inst;
ALLEGRO_SAMPLE_INSTANCE* positive_inst;
ALLEGRO_PATH *path;

ALLEGRO_FILE* logfile;
ALLEGRO_FILE* statefile;

//local prototypes
void newword(void);
void initchain(void);
int findchain(void);
int makedict(DictType* dict, int wlen);
ChainEntryType* findword(ChainEntryType* current_word, int pos);
ChainEntryType* findword2(ChainEntryType* current_word, int pos);
void init_state(void);
void save_state(void);
void start_timer(TimerIdxType idx);
void stop_timer(TimerIdxType idx);
void set_timer(TimerIdxType idx, int value);
void update_timers(void);
void update_coins(void);


#ifndef ANDROID
int game(int argc, char **argv );
int main(int argc, char **argv) {
    game (argc, argv);
    return 0;
}
#endif

int game(int argc, char **argv )
{
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_EVENT event;

    int tries=0;
    int new_event;
    int count = 0;
    int error = 0;
    char temp[10];

    error = al_init();  /* Init Allegro 5 + addons. */

	//set path
    al_set_standard_file_interface();
    path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	al_change_directory(al_path_cstr(path, '/'));  // change the working directory

	//open logfile in executable directory
	logfile = al_fopen("logfile.txt","w");
    al_fprintf(logfile,"%s %s\n",NAME,VERSION);
    al_fprintf(logfile,"Init Allegro\n");

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

    srand(time(0));
	//change directory to data, where all resources live (images, fonts, sounds and text files)
	al_append_path_component(path, "data");
	al_change_directory(al_path_cstr(path, '/'));  // change the working directory


#ifdef _WIN32
    al_set_new_display_flags(ALLEGRO_RESIZABLE);//ALLEGRO_WINDOWED);// | ALLEGRO_RESIZABLE);
#endif // _WIN32
#ifdef ANDROID
    al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS,ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE,ALLEGRO_REQUIRE);
#endif

    int screenx = SCREENX;
    int screeny = SCREENY;

    //display = al_create_display(SCREENX, SCREENY);
    display = al_create_display(1920/2, 1080/2);

    calc_scale();

    al_set_window_title(display, NAME);

    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);// + ALLEGRO_MIN_LINEAR);

	timer = al_create_timer(1.0 / 30);
	queue = al_create_event_queue();

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

#ifdef ANDROID
    al_android_set_apk_file_interface();
#endif

    //font = al_load_font("Audiowide-Regular.ttf",100,0);
    small_font = al_load_font("Audiowide-Regular.ttf",50,0);
    al_draw_text(small_font, al_map_rgba(0,0,0,0), 0, 0, 0, "!.ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");    //pre-load characters to speed up writing

    //int max_bmp = al_get_display_option(display, ALLEGRO_MAX_BITMAP_SIZE);

    icons = al_load_bitmap("icons_130.png");
    ttglogo = al_load_bitmap("tootired.png");
    coin = al_load_bitmap("coin.png");

    State.screen = START;
    draw_screen(State.screen, 1); //draw TTG logo screen ASAP

    init_state();                   //open and read from file: current & unlocked backgrounds/tiles, coins etc.

    background = backgrounds[State.bg];
    alpha = alphas[State.alpha];

    al_start_timer(timer);          //allegro

    makedict(&dict4, 4);            //open text file dictionary, copy into RAM
    makedict(&dict5, 5);

    newword();                      //pick a start word

    searching = TRUE;
    State.screen = HOME;
    start_timer(TIMER_HOME);


    if (al_is_audio_installed())
    {
        voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
        mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
        al_set_default_mixer(mixer);
        al_attach_mixer_to_voice(mixer, voice);
        al_fprintf(logfile,"Setup audio voice and mixer\n");
    }
    else    //get here if no audio available - create mixer to stop rest of code whinging.
        mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);

    if ((click = al_load_sample    ("sounds/switch18.wav"))  == NULL) al_fprintf(logfile,"switch18.wav load fail");
    if ((misc_menu = al_load_sample("sounds/misc_menu.wav")) == NULL) al_fprintf(logfile,"misc_menu.wav load fail");
    if ((negative = al_load_sample ("sounds/negative_2.wav"))== NULL) al_fprintf(logfile,"negative_2.wav load fail");
    if ((positive = al_load_sample ("sounds/positive.wav"))  == NULL) al_fprintf(logfile,"positive.wav load fail");

    click_inst = al_create_sample_instance(click);
    al_attach_sample_instance_to_mixer( click_inst, mixer);

    misc_menu_inst = al_create_sample_instance(misc_menu);
    al_attach_sample_instance_to_mixer( misc_menu_inst, mixer);

    negative_inst = al_create_sample_instance(negative);
    al_attach_sample_instance_to_mixer( negative_inst, mixer);

    positive_inst = al_create_sample_instance(positive);
    al_attach_sample_instance_to_mixer( positive_inst, mixer);

    while (1)
    {
        //do a bit of looking for a chain, if we're looking
        if (searching)
        {
            tries++;
            //sprintf(debug_tries,"%d",tries);
            //messageptr = debug_tries;
            if (tries == 20)            //give up and try a new start word
            {
                newword();
                tries = 0;
            }
            else
            {
                if (findchain())    //returns true when found a complete chain
                {
                    searching = FALSE;                          //so stop looking
                    Chain.word[Chain.length].status = LAST;     //and set up stuff
                    Chain.current = &Chain.word[1];
                    Chain.current->status = CURRENT;
                    strncpy(Chain.word[0].user, Chain.word[0].cpu,Chain.word_length+1);
                    strncpy(Chain.word[1].user, Chain.word[0].cpu,Chain.word_length+1);
                    strncpy(Chain.word[Chain.length].user, Chain.word[Chain.length].cpu,Chain.word_length+1);
                }
            }
            if (!al_is_event_queue_empty(queue))    //check for events so we're responsive, but don't wait.
            {
                al_get_next_event(queue, &event);
                new_event = TRUE;
            }
        }

        else    //not looking, so idle until event
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
                    calc_scale();
                break;
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    exit(0);
                break;
                case  ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:   //we've been sidelined by the user/os
                        al_acknowledge_drawing_halt(display);   //acknowledge
                        State.halted = true;                          //flag to drawing routines to do nothing
                        al_stop_timer(timer);                   //no more timer events, so we should do nothing, saving battery
                        //al_set_default_voice(NULL);             //destroy voice, so no more sound events, ditto.
                        //al_destroy_voice(voice);
                break;
                case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING: //we've been restored
                        al_acknowledge_drawing_resume(display); //ack
                        State.halted = false;                         //remove flag
                        al_start_timer(timer);                  //restart timer
                        //voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);  //restart audio
                        //al_attach_mixer_to_voice(mixer, voice);
                break;
                case ALLEGRO_EVENT_KEY_CHAR:
                    if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                        exit(0);
                    else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                    {
                        Command = CMD_FORWARD;
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
                    if ((al_event_queue_is_empty(queue)) || searching)
                        draw_screen(State.screen, State.scale);

                    update_coins();
                    update_timers();
                break;
                default:
                break;
                //when level finished, searching = true.
            }
        }
        //check for commands
        switch (Command)
        {
            case CMD_HOME:                      //home button pressed
                State.screen = HOME;            //set screen to draw
                Buttons = HomeButtons;          //set buttons
                Buttons2 = HomeButtons2;
                State.timeout = FALSE;
                start_timer(TIMER_HOME);
            break;
            case CMD_INFO:
                State.screen = INFO;
                Buttons = InfoButtons;
                make_info_bitmap();
            break;
            case CMD_COLOURS:                   //colour button - go to colour unlocking / selection screen
                State.screen = COLOURS;
                Buttons = ColoursButtons;
                //Mouse.scroll = 0;
                post_message(msg_costs);
            break;
            case CMD_BG:                        //'background' option pressed in 'colour' screen
                State.ColourItem = BG;
            break;
            case CMD_ALPHA:                     //'letters' option....
                State.ColourItem = ALPHA;
            break;
            case CMD_PLAY:                      //'play normal' button
                State.screen = GAME;
                State.gametype = NORMAL;        //'normal' (i.e. not timed) play
                Buttons = GameButtons;
                newword();
                post_message(msg_start);        //show start message
            break;
            case CMD_TIMED:                     //play timed button
                State.screen = GAME;
                State.gametype = TIMED;         //set gametype
                State.timeout = FALSE;
                /*if (State.score > State.highscore)
                {
                    State.highscore = State.score;
                    save_state();
                }*/
                State.score = 0;
                Buttons = TimedGameButtons;
                newword();
                start_timer(TIMER_GAME);        //start timer
                post_message(msg_start_timed);        //NEED A DIFFERENT MESSAGE!!!
            break;
            case CMD_TIMEOUT:                   //triggered by running out of time in timed play
                al_play_sample_instance( negative_inst);
                Chain.show = TRUE;
                State.timeout = TRUE;
                Buttons2 = TimeOutButtons;
                if (State.score > State.highscore)
                {
                    State.highscore = State.score;
                    save_state();
                }
                //post_message(msg_timeout);
                //show overlay. no message??
            break;
            case CMD_SUCCESS:                           //completed chain
                State.success = TRUE;
                State.coins += 10;                      //so give coins
                State.coins_up += 10;                    //used for 'counting up' effect in display
                State.score += Timer[TIMER_GAME].value; //add score (only used in timed play)
                start_timer(TIMER_SUCCESS);             //start timer which will go to next chain on expiry
                stop_timer(TIMER_GAME);
                post_message(msg_success);
                save_state();                           //to remember coins
            break;
            case CMD_REVERT:                            //error, so put current word back to what it was.
                strncpy(Chain.current->user,((Chain.current)-1)->user,Chain.word_length+1);
            break;
            case CMD_FORWARD:                           //multiple uses? investigate / split?
                newword();
                State.success = FALSE;
                if (State.gametype == TIMED)
                    Buttons = TimedGameButtons;
                else
                    Buttons = GameButtons;
                stop_timer(TIMER_SUCCESS);
                //searching = TRUE;
                //tries = 0;
                draw_screen(GAME, State.scale);
                if (State.gametype == TIMED)
                {
                    int new_value = Timer[TIMER_GAME].saved_value+500;
                    if (new_value > 1000) new_value = 1000;
                    set_timer(TIMER_GAME, new_value);
                }
            break;
            case CMD_SOLVE:                             //'solve' button
                if (State.coins >= 10)                  //if enough money
                {
                    Chain.show = TRUE;                  //show solution
                    Buttons = SolvedButtons;            //change buttons
                    State.coins_down += 10;              //take coins
                    State.coins -= 10;
                    save_state();                       //and remember
                }
                else
                {
                    start_timer(TIMER_ERROR);           //not enough money
                    post_message(msg_nocoins);          //so show message
                }
            break;
            case CMD_BACK:                              //'undo' button
                if (Chain.current->status != LAST)
                    Chain.current->status = BLANK;       //wipe current word

                Chain.current--;                        //bump pointer back

                if (Chain.current->status == FIRST)     //if we've hit the start
                    Chain.current++;                    //bump it on again

                Chain.current->status = CURRENT;        //set this one to be current.

                Chain.current--;                        //bump pointer back
                strncpy(temp,Chain.current->user,Chain.word_length+1);
                Chain.current++;
                strncpy(Chain.current->user,temp,Chain.word_length+1);
                State.success = FALSE;
            break;
            case CMD_RESTART:
                Timer[TIMER_HOME].value = Timer[TIMER_HOME].start_value;
            break;
            case NO_COMMAND:
            default:
            break;
        }
        Command = NO_COMMAND;   //only process once.....
    }

    free(dict4.base);           //
    free(dict5.base);
    return 0;
}

void start_timer(TimerIdxType idx)                  //software timers, all count down to zero, so init to start value
{
    Timer[idx].value = Timer[idx].start_value;
}

void set_timer(TimerIdxType idx, int value)         //init to different value
{
    Timer[idx].value = value;
}

void stop_timer(TimerIdxType idx)                   //set to zero effectively stops.
{
    Timer[idx].saved_value = Timer[idx].value;
    Timer[idx].value = 0;
}

void update_timers(void)
{
    TimerIdxType i;

    for (i=0 ; i<NUM_TIMERS ; i++)                 //for each timer,  check for zero,
    {
        if (Timer[i].value)                         //if it's non-zero,
        {
            Timer[i].value--;                       //decrement,

            if (Timer[i].value == 0)                //if it's now expired,
            {
                Command = Timer[i].command;         //issue the associated command.
            }
        }
    }
}

void update_coins(void)
{
    if (State.coins_up)             //count up to give animated display
    {
        State.display_coins++;
        State.coins_up--;
    }
    if (State.coins_down)           //count down
    {
        if (State.coins_down > 10)  //fast if there's along way left to go
        {
            State.coins_down -=5;
            State.display_coins-=5;
        }
        else
        {
            State.display_coins--;
            State.coins_down--;
        }
    }
}

void calc_scale(void)       //work out scaling for graphics
{
    float xscale = 1.0*((float)al_get_display_width(display)/SCREENX);
    float yscale = 1.0*((float)al_get_display_height(display)/SCREENY);

    if (xscale < yscale)
        State.scale = xscale;
    else
        State.scale = yscale;

    State.inv_scale = 1.0/State.scale;
}

void init_state(void)   //read stuff from file
{
    int i,j;
    ALLEGRO_FILE* tempfile;
#ifdef ANDROID
    char pathstr[100];
    //char *pathptr;
    al_set_standard_file_interface();
    path = al_get_standard_path(ALLEGRO_USER_DATA_PATH);    //android
    //ALLEGRO_DEBUG(#std ": %s", al_path_cstr(path, '/'));
    sprintf(pathstr,"%s",al_path_cstr(path, '/'));
    al_change_directory(al_path_cstr(path, '/'));  // change the working directory
    //pathptr = al_get_current_directory();
#endif

    State.coins = 0;    //default values
    State.coins_up = 0;
    State.coins_down = 0;
    State.bg = 0;
    State.alpha = 0;
    State.bgs[0] = 1;
    State.bgs[1] = 0;
    State.bgs[2] = 0;
    State.bgs[3] = 0;
    State.alphas[0] = 1;
    State.alphas[1] = 0;
    State.alphas[2] = 0;
    State.alphas[3] = 0;
    State.gametype = NORMAL;
    State.ColourItem = BG;

    statefile = al_fopen("state.txt","r");     //try to open the file
    if (statefile)
    {
        al_fgets(statefile,temp,15);        //coins
        State.coins = strtol(temp,NULL,10);

        al_fgets(statefile,temp,15);        //current bg
        State.bg = strtol(temp,NULL,10);

        al_fgets(statefile,temp,15);        //current alphabet
        State.alpha = strtol(temp,NULL,10);

        for (i=0 ; i<4 ; i++)                   //unlocked backgrounds
        {
            if (al_fgets(statefile,temp,15))
                State.bgs[i] = strtol(temp,NULL,10);
        }
        for (i=0 ; i<4 ; i++)                   //unlocked tilesets/alphabets/letters/ whatever you call them.
        {
            if (al_fgets(statefile,temp,15))
                State.alphas[i] = strtol(temp,NULL,10);
        }
        al_fgets(statefile,temp,15);        //coins
        State.highscore = strtol(temp,NULL,10);
        al_fclose(statefile);
    }

#ifdef ANDROID
    al_android_set_apk_file_interface();
#endif

    State.display_coins = State.coins;                  //display the right number

    //run through the bitmasks from the state file to find the unlocked backgrounds / tiles
    for (i=0 ; i<4 ; i++)                               //4 words
    {
        for (j=0 ; j<32 ; j++)                          //32 bits in each
        {
                char bstr[20],tstr[20];

                sprintf(bstr,"bg/bg%03d.jpg",i*32+j);   //try to open a background image file
                tempfile = al_fopen(bstr,"r");
                if (tempfile)                           //if successful,
                {
                    State.max_bgs++;                    //inc count
                    al_fclose(tempfile);                //and close again
                }
                sprintf(tstr,"tiles/tiles%02d.png",i*32+j); //repeat for tile/alphabet files
                tempfile = al_fopen(tstr,"r");
                if (tempfile)
                {
                    State.max_alphas++;
                    al_fclose(tempfile);
                }

                if ((State.bgs[i] >> j) & 1)            //bit = 1 for unlocked images
                {
                    backgrounds[i*32+j] = al_load_bitmap(bstr); //so load. we have a big array of pointers.....
                }

                if ((State.alphas[i] >> j) & 1)
                {
                    alphas[i*32+j] = al_load_bitmap(tstr);
                }
        }
    }
    if (alphas[State.alpha] == NULL)    //if we've somehow screwed up loading / indexing
    {
        State.alpha = 0;                //default to first one
        State.alphas[0] |= 1;           //and make sure that's enabled
        alphas[0] = al_load_bitmap("tiles/tiles00.png");
    }
    if (backgrounds[State.bg] == NULL)    //if we've somehow screwed up loading / indexing
    {
        State.bg = 0;                //default to first one
        State.bgs[0] |= 1;           //and make sure that's enabled
        backgrounds[0] = al_load_bitmap("bg/bg000.jpg");
    }
}


void save_state(void)
{
    int i;
#ifdef ANDROID
    char pathstr[100];
    char *pathptr;
    al_set_standard_file_interface();
    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_USER_DATA_PATH);    //android
    //ALLEGRO_DEBUG(#std ": %s", al_path_cstr(path, '/'));
    sprintf(pathstr,"%s",al_path_cstr(path, '/'));
    al_change_directory(al_path_cstr(path, '/'));  // change the working directory
    pathptr = al_get_current_directory();
#endif

    statefile = al_fopen("state.txt","w");
    al_fprintf(statefile,"%d\n",State.coins);
    al_fprintf(statefile,"%d\n",State.bg);
    al_fprintf(statefile,"%d\n",State.alpha);
    for (i=0 ; i<4 ; i++)
        al_fprintf(statefile,"%d\n",State.bgs[i]);
    for (i=0 ; i<4 ; i++)
        al_fprintf(statefile,"%d\n",State.alphas[i]);
    al_fprintf(statefile,"%d\n",State.highscore);
    al_fclose(statefile);

#ifdef ANDROID
    al_android_set_apk_file_interface();
#endif
}

void post_message(char* ptr)
{
    Message.message = ptr;
    Message.count = 60;         //60 ticks = 2 seconds
    Message.x = -1*State.inv_scale*al_get_display_width(display);     //start offscreen to the left.
}

void newword()
{
    int chosen;//,count=0;//,i;
    //char name[20];//, dict_word[50];

    Chain.word_length = 4+rand()%2;                     //decide word length
    Chain.length = Chain.word_length;

    initchain();

    if (Chain.word_length == 4)                         //choose dictionary
        dict = &dict4;
    else
        dict = &dict5;

    chosen = rand()%dict->count;                                      //pick random word
    strncpy(word, dict->base+(chosen*(dict->len)), dict->len);   //copy random word from ram dict to 'word'

    //strncpy(word, "flesh",6); //debug force start word
    //Chain.word_length = 5;    //debug force word length
    //dict = &dict5;            //debug make sure we use the right dictionary.

    word[strcspn(word, "\n")] = 0;                              //trim cr/lf

    Chain.current = &Chain.word[0];
    strncpy(Chain.current->cpu,word,Chain.word_length+1);

    Chain.word[0].status = FIRST;

    searching = TRUE;

    return;
}

int makedict(DictType* dict, int wlen)
{
    char name[10];
    char dict_word[10];
    int count = 0;//,i;

    sprintf(name,"dict%d.txt",wlen);
    dictfile = al_fopen(name,"r");
    if (dictfile == NULL)
    {
        printf("Failed to open dictionary file:%s\n",name);
        return 0;
    }

    while (al_fgets(dictfile,dict_word,10) != NULL) count++;    //count the words (1 word per line)

    dict->count = count;                                        //init count
    dict->len = wlen+1;                                         // and entry length
    dict->base = malloc(count * dict->len);                     //allocate enough ram
    dict->current = dict->base;                                 //init pointer

    if (al_fclose(dictfile) == 0)
    {
        printf("Failed to open dictionary file:%s\n",name);
        return 0;
    }
    dictfile = al_fopen(name,"r");

    while (al_fgets(dictfile,dict_word,10) != NULL)
    {
        strncpy(dict->current,dict_word,dict->len);         //copy word from file to ram dictionary
        dict->current += dict->len;                         //bump pointer
    }

    al_fclose(dictfile);

    return 1;
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
Alternative approach: (original below, compiled out)
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
    int i,j=0,k,fail,chosen;//,i;
    char dict_word[50];

    dict->current = dict->base;

    for (k=0 ; k<dict->count ; k++)
    {
        strncpy(dict_word,dict->current,dict->len);
        dict->current += dict->len;

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

//check for word in dictionary. Only used to validate user input
int isindict(char* word, int length)
{
    int i;
    dict->current = dict->base; //rewind to start of dictionary

    for ( i=0 ; i<dict->count ; i++ )   //run through dictionary
    {
        if (strncmp(word, dict->current, Chain.word_length) == 0)  //if the words match we have a valid word
              return 1;
        dict->current += dict->len; //bump pointer
    }
    return 0;   //got to the end, no match, so fail.
}
