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
#define TILE 200
#define BUTTON_X (TILE/2) //(ALPHA_X + 1 * TILE + TILE/2)
#define BUTTON_Y (TILE/2) //(ALPHA_Y + ALPHA_ROWS*TILE)
#define CHAIN_X  (BUTTON_X + TILE + TILE/2)
#define CHAIN_Y  TILE/2
#define ALPHA_X  (CHAIN_X + 6*TILE)// + TILE/2)
#define ALPHA_Y  TILE/2
#define ALPHA_ROWS 6
#define ALPHA_COLS 5

#define NUM_BUTTONS 2

#define SCREENX (ALPHA_X + ALPHA_COLS * TILE + TILE/2) //1280//1920 //1024
#define SCREENY (ALPHA_Y + ALPHA_ROWS * TILE + TILE/2) //720//1080 //768

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

typedef enum
{
    START = 0,
    MENU,
    GAME
} ScreenType;

typedef struct
{
    char word[10];
}wordtype;

typedef struct
{
    int x;
    int y;
    int tile;
}MouseType;

typedef enum
{
    NO_COMMAND = 0,
    SUCCESS,
    FORWARD,
    BACK,
}CommandType;

#define NO_TILE 0xFFFF

//global vars
extern ALLEGRO_DISPLAY *display;
extern ALLEGRO_BITMAP *background;
extern ALLEGRO_BITMAP *letters;
extern ALLEGRO_BITMAP *buttons;
extern ALLEGRO_BITMAP *blank;
extern ALLEGRO_BITMAP *ttglogo;
extern ALLEGRO_FONT *font;
extern ALLEGRO_FILE* logfile;
extern ChainType Chain;
extern MouseType Mouse;
extern ScreenType Screen;
extern CommandType Command;
extern float scale, inv_scale;
extern int halted;
extern int error_timer,success;
extern char* messageptr;

extern char msg_blank[];
extern char msg_start[];
extern char msg_success[];
extern char msg_notindict[];
extern char msg_chainfail[];

//function prototypes
void draw_screen(ScreenType screen, float scale);
void mouse_move(ALLEGRO_EVENT event);
void mouse_down(ALLEGRO_EVENT event);
void mouse_up(ALLEGRO_EVENT event);
void touch_begin(ALLEGRO_EVENT event);
void touch_end(ALLEGRO_EVENT event);
void touch_move(ALLEGRO_EVENT event);
int isindict(char* word, int length);
