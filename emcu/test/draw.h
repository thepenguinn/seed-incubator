#include<ncurses.h>

/*
 * we are using use_default_colors() from ncurses library
 * */

#define COLOR_DEFAULT_BG  -1
#define COLOR_DEFAULT_FG  -1

/*
 * Since we are using use_default_colors() from ncurses library, the zeroth
 * pair doesn't seems to be overriden by assigning a new pair to it.
 *
 * So instead of changing the zeroth pair we will start from 1
 * */

#define ColorPair(x) COLOR_PAIR((x)+1)


/*
 * This will override the predefined colors from 1-7 in the ncurses library
 * */

enum Colors {
	COLOR_PRI_ACCENT,
	COLOR_SEC_ACCENT,
    COLOR_PRI_WHITE,    /* really bright */
    COLOR_SEC_WHITE,  /* normal        */
    COLOR_TER_WHITE,   /* dim           */
	COLOR_END,
};

/*
 * When using */

enum ColorPairs {
	/*   fg            bg    */
	PAIR_PRI_ACCENT_DEFAULT_BG,
	PAIR_SEC_ACCENT_DEFAULT_BG,
	PAIR_PRI_WHITE_DEFAULT_BG,
	PAIR_SEC_WHITE_DEFAULT_BG,
	PAIR_TER_WHITE_DEFAULT_BG,
	PAIR_END,
};

enum ColorSchemes {
	SCHEME_DEFAULT,
	// SCHEME_MONOCHROME,
	SCHEME_END,
};

enum ColorElements {
    ELEMENT_MENU_TITLE,
    ELEMENT_SUBMENU_TITLE_NORMAL,
    ELEMENT_SUBMENU_TITLE_SELECTED,
    ELEMENT_SUBMENU_INFO_NORMAL,
    ELEMENT_SUBMENU_INFO_SELECTED,
    ELEMENT_INFO_FRAME,
    ELEMENT_INFO_TEXT,
    ELEMENT_INFO_VALUE,
    ELEMENT_DOTS_NORMAL,
    ELEMENT_DOTS_SELECTED,
    ELEMENT_EMCU_CONNECTED,
    ELEMENT_EMCU_NOT_CONNECTED,
    /**/
    ELEMENT_DATA_WIDGET_FRAME_NORMAL,
    ELEMENT_DATA_WIDGET_FRAME_SELECTED,
    ELEMENT_DATA_WIDGET_KEY_NORMAL,
    ELEMENT_DATA_WIDGET_KEY_SELECTED,
    ELEMENT_DATA_WIDGET_VALUE_NORMAL,
    ELEMENT_DATA_WIDGET_VALUE_SELECTED,
    /*element_toggle_button_frame,*/
    /*element_toggle_button_text,*/
    /*element_toggle_button_inactive_state,*/
    /*element_toggle_button_active_state,*/
    ELEMENT_END,
};

enum SeperatorChars {
	CHAR_CORNER_TOPLEFT,
	CHAR_CORNER_TOPRIGHT,
	CHAR_CORNER_BOTRIGHT,
	CHAR_CORNER_BOTLEFT,
	CHAR_CORNER_ROUNDED_TOPLEFT,
	CHAR_CORNER_ROUNDED_TOPRIGHT,
	CHAR_CORNER_ROUNDED_BOTRIGHT,
	CHAR_CORNER_ROUNDED_BOTLEFT,
	CHAR_EDGE_TOP,
	CHAR_EDGE_RIGHT,
	CHAR_EDGE_BOT,
	CHAR_EDGE_LEFT,
	CHAR_MINUS,
	CHAR_PIPE,
	CHAR_CROSS,
	CHAR_DOT,
	CHAR_END,
};

struct MainMenu;
struct SubMenu;

struct MainMenu {
    int sel_sub_menu_idx;
    int total_sub_menus;
    int cury;
    int event;
    const struct SubMenu *first_sub_menu;
    int emcu_connected;
};

struct SubMenu {
    /*
     * title and info should be short as possible,
     * because it shouldn't bleed out at the right edge.
     * */
    char title[64];
    char info[128];
    void *(*handler)(void *param);
    void *data;
};

struct SubMenuParam {
    WINDOW *win;
    /*
     * something else
     * */
};

struct DataWidget {
    int width;
    int height;
    int xopad; /* outer padding along x direction*/
    int xipad; /* inner padding along x direction*/
    int yopad; /* outer padding along y direction*/
    int yipad; /* inner padding along y direction*/
    const char *data_key;
    int data_key_size;
    const char *data_value;
    int data_value_size;
    int frame_scheme;
    int data_key_scheme;
    int data_value_scheme;
};

/*
* TODO: move this to somewhere else that makes sense.
 * */

#define DHT_COUNT 2
#define LDR_COUNT 3
#define SMS_COUNT 4
#define USO_COUNT 2

struct TempData {
    int dht[2];
};

void draw_top_win(const char *title);
void draw_bot_win(struct MainMenu *menu);
int draw_sub_menu(struct MainMenu *mainmenu);
void draw_main_menu(struct MainMenu *menu);
void start_tui_app(void);

int draw_init_ncurses(void);
