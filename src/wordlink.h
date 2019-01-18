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

#define TRUE  1
#define FALSE 0

//screen positions
#define TILE 130
#define BUTTON_X (TILE/2) //(ALPHA_X + 1 * TILE + TILE/2)
#define BUTTON_Y (TILE) //(ALPHA_Y + ALPHA_ROWS*TILE)
#define CHAIN_X  (BUTTON_X + TILE + TILE/2)
#define CHAIN_Y  TILE
#define ALPHA_X  (CHAIN_X + 6*TILE)// + TILE/2)
#define ALPHA_Y  TILE
#define ALPHA_ROWS 6
#define ALPHA_COLS 5

#define BUTTON2_X CHAIN_X+4*TILE
#define BUTTON2_Y CHAIN_Y+4*TILE

#define NUM_BUTTONS 7

#define SCREENX (ALPHA_X + ALPHA_COLS * TILE + TILE/2) //1280//1920 //1024
#define SCREENY (ALPHA_Y + ALPHA_ROWS * TILE + TILE) //720//1080 //768

//typedefs
typedef enum
{
  FIRST = 0,
  VALID,
  CURRENT,
  BLANK,
  LAST,
}
WordStatusType;

typedef struct
{
  int index;
  char cpu[7];
  char user[7];
  WordStatusType status;
}
ChainEntryType;

#define MAX_CHAIN_LENGTH 7

typedef struct {
    ChainEntryType word[MAX_CHAIN_LENGTH];
    ChainEntryType *current;
    ChainEntryType *last; //i.e. previous
    int length; //?
    int word_length;
    int show;
} ChainType;

typedef struct{
    char* base;         //pointer to base of malloc'ed memory
    char* current;      //pointer to current word
    int count;          //number of words
    int len;            //*ENTRY* length (i.e. word length + 1)
}DictType;

typedef enum
{
    START = 0,
    HOME,
    SETTINGS,
    COLOURS,
    GAME
} ScreenType;

typedef enum
{
    BG = 0,
    ALPHA
}ColourItemType;

typedef enum
{
    NORMAL = 0,
    TIMED
}GameTypeType;

typedef struct
{
    char word[10];
}wordtype;

typedef struct
{
    int x;
    int y;
    int tile;
    int button;
    int scrolla;    //alphas
    int scrollb;    //backgrounds
    int move;
}MouseType;

typedef struct
{
    char* message;
    int count;
    int x;
}MessageType;

typedef struct
{
    int coins;//coins for normal play
    int bg;
    int alpha;
    long int bgs[4];
    long int alphas[4];

    ScreenType screen;
    ColourItemType ColourItem;
    GameTypeType gametype;
    int timeout;

    int max_bgs;
    int max_alphas;

    int display_coins;
    int coins_up;
    int coins_down;
    int score;          //score for timed round
    int highscore;

    int halted;
    int success;
    float scale;
    float inv_scale;
}StateType;

typedef enum
{
    NO_COMMAND = 0,
    CMD_HOME,          //Screens
    CMD_INFO,
    CMD_COLOURS,
    CMD_PLAY,
    CMD_TIMED,
    CMD_TIMEOUT,
    CMD_BG,             //Sub screens for colour selection
    CMD_ALPHA,
    CMD_SOLVE,          //buttons on game sreen
    CMD_FORWARD,
    CMD_BACK,
    CMD_SUCCESS,        //when we solve it
    CMD_REVERT,
}CommandType;

typedef enum    //order matches icon bitmap
{
    ICON_HOME = 0,
    ICON_COLOURS,
    ICON_PLAY,
    ICON_TIMED,
    ICON_INFO,
    ICON_SOLVE,
    ICON_NEXT,
    ICON_UNDO,
    ICON_LOCK,
    NO_ICON,
}IconType;

typedef struct
{
    IconType    icon;
    CommandType command;
}ButtonType;

typedef struct
{
    int start_value;
    int value;
    CommandType command;
}TimerType;

typedef enum
{
    TIMER_SUCCESS = 0,
    TIMER_ERROR,
    TIMER_GAME,
    NUM_TIMERS,
}TimerIdxType;

#define NO_TILE 0xFFFF

//global vars
extern ALLEGRO_DISPLAY *display;
extern ALLEGRO_BITMAP *background;
extern ALLEGRO_BITMAP *alpha;
extern ALLEGRO_BITMAP *icons;
extern ALLEGRO_BITMAP *ttglogo;
extern ALLEGRO_BITMAP *coin;
extern ALLEGRO_BITMAP *backgrounds[100];    //used in select screen
extern ALLEGRO_BITMAP *alphas[100];          //used in select screen

//extern ALLEGRO_FONT *font;
extern ALLEGRO_FONT *small_font;
extern ALLEGRO_FILE* logfile;

extern ChainType Chain;
extern MouseType Mouse;
extern CommandType Command;
extern MessageType Message;
extern StateType State;
extern ButtonType* Buttons;
extern ButtonType* Buttons2;
extern TimerType Timer[];

extern char msg_blank[];
extern char msg_start[];
extern char msg_success[];
extern char msg_notindict[];
extern char msg_chainfail[];
extern char msg_nocoins[];
extern char msg_costs[];
extern char msg_timeout[];

//function prototypes
void calc_scale(void);
void draw_screen(ScreenType screen, float scale);
void mouse_move(ALLEGRO_EVENT event);
void mouse_down(ALLEGRO_EVENT event);
void mouse_up(ALLEGRO_EVENT event);
void touch_begin(ALLEGRO_EVENT event);
void touch_end(ALLEGRO_EVENT event);
void touch_move(ALLEGRO_EVENT event);
int isindict(char* word, int length);
void post_message(char* ptr);
void save_state(void);
void start_timer(TimerIdxType idx);