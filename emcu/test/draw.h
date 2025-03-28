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
    /**/
    ELEMENT_RADIO_BUTTON_FRAME_NORMAL,
    ELEMENT_RADIO_BUTTON_FRAME_SELECTED,
    ELEMENT_RADIO_BUTTON_TEXT_NORMAL,
    ELEMENT_RADIO_BUTTON_TEXT_SELECTED,

    /*
     * normal -> the widget is not the one user selected
     * selected -> the widget is the one user is selected
     * active -> the state which is active (only one will be active)
     * inactive -> the rest of the states that are not active
     * focused -> the state which the user is focused
     * */
    ELEMENT_RADIO_BUTTON_ACTIVE_NORMAL,
    ELEMENT_RADIO_BUTTON_ACTIVE_SELECTED,
    ELEMENT_RADIO_BUTTON_INACTIVE_NORMAL,
    ELEMENT_RADIO_BUTTON_INACTIVE_SELECTED,

    ELEMENT_RADIO_BUTTON_ACTIVE_FOCUSED_NORMAL,
    ELEMENT_RADIO_BUTTON_ACTIVE_FOCUSED_SELECTED,
    ELEMENT_RADIO_BUTTON_INACTIVE_FOCUSED_NORMAL,
    ELEMENT_RADIO_BUTTON_INACTIVE_FOCUSED_SELECTED,

    /**/
    ELEMENT_SWITCH_FRAME_NORMAL,
    ELEMENT_SWITCH_FRAME_SELECTED,
    ELEMENT_SWITCH_TEXT_NORMAL,
    ELEMENT_SWITCH_TEXT_SELECTED,

    ELEMENT_SWITCH_ACTIVE_NORMAL,
    ELEMENT_SWITCH_ACTIVE_SELECTED,
    ELEMENT_SWITCH_INACTIVE_NORMAL,
    ELEMENT_SWITCH_INACTIVE_SELECTED,

    ELEMENT_SWITCH_ACTIVE_FOCUSED_NORMAL,
    ELEMENT_SWITCH_ACTIVE_FOCUSED_SELECTED,
    ELEMENT_SWITCH_INACTIVE_FOCUSED_NORMAL,
    ELEMENT_SWITCH_INACTIVE_FOCUSED_SELECTED,

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
    CHAR_FISHEYE,
    CHAR_CIRCLE,
	CHAR_END,
};

enum SensSubMenu {
    SENS_SUB_MENU_TEMPERATURE = 0,
    SENS_SUB_MENU_AIR_MOISTURE,
    SENS_SUB_MENU_LIGHT_SENSE,
    SENS_SUB_MENU_SOIL_MOISTURE,
    /*SENS_SUB_MENU_RESERVOIR_LEVEL,*/
    SENS_SUB_MENU_END,
};

enum GenSubMenu {
    GEN_SUB_MENU_TEMPERATURE     = SENS_SUB_MENU_TEMPERATURE,
    GEN_SUB_MENU_AIR_MOISTURE    = SENS_SUB_MENU_AIR_MOISTURE,
    GEN_SUB_MENU_LIGHT_SENSE     = SENS_SUB_MENU_LIGHT_SENSE,
    GEN_SUB_MENU_SOIL_MOISTURE   = SENS_SUB_MENU_SOIL_MOISTURE,
    /*GEN_SUB_MENU_RESERVOIR_LEVEL = SENS_SUB_MENU_RESERVOIR_LEVEL,*/

    GEN_SUB_MENU_EXHAUST         = SENS_SUB_MENU_END,
    GEN_SUB_MENU_HUMIDIFIER,
    GEN_SUB_MENU_LIGHTING,
    GEN_SUB_MENU_END,
};

enum ExhaustWidgetType {
    EXHAUST_WIDGET_TYPE_PELTIER = 0,
    EXHAUST_WIDGET_TYPE_FAN,
    EXHAUST_WIDGET_TYPE_PANEL,
    EXHAUST_WIDGET_TYPE_EXHAUST_MODE,
    EXHAUST_WIDGET_TYPE_END,
};

enum ExhaustMode {
    EXHAUST_MODE_CMODE = 0,
    EXHAUST_MODE_HMODE,
    EXHAUST_MODE_NONE,
    EXHAUST_MODE_END,
};

enum BistableState {
    BISTABLE_STATE_OFF = 0,
    BISTABLE_STATE_ON,
    BISTABLE_STATE_UNDEFINED,
    BISTABLE_STATE_END,
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

struct RadioButtonWidget {
    int width;
    int height;

    int xopad; /* outer padding along x direction*/
    int xipad; /* inner padding along x direction*/
    int yopad; /* outer padding along y direction*/
    int yipad; /* inner padding along y direction*/

    enum ExhaustMode mode;

    const char *text;
    int text_size;
    const char *first_state_text;
    int first_state_size;
    const char *second_state_text;
    int second_state_size;
    const char *third_state_text;
    int third_state_size;

    int frame_scheme;
    int text_scheme;
    int first_state_scheme;
    int second_state_scheme;
    int third_state_scheme;
};

struct SwitchWidget {
    int width;
    int height;

    int xopad; /* outer padding along x direction*/
    int xipad; /* inner padding along x direction*/
    int yopad; /* outer padding along y direction*/
    int yipad; /* inner padding along y direction*/

    enum BistableState state;

    const char *text;
    int text_size;

    const char *first_state_text;
    int first_state_size;
    const char *second_state_text;
    int second_state_size;

    int frame_scheme;
    int text_scheme;
    int first_state_scheme;
    int second_state_scheme;
};

/*
 * TODO: move this to somewhere else that makes sense.
 * */

#define DHT_COUNT 2
#define LDR_COUNT 3
#define SMS_COUNT 4
#define USO_COUNT 2

#define EXHAUST_PANEL_COUNT 7
#define EXHAUST_FAN_COUNT   2

#define LIGHT_ARRAY_COUNT 2

struct TempData {
    int dht[DHT_COUNT];
};

struct HumeData {
    int dht[DHT_COUNT];
};

struct LdrData {
    int ldr[LDR_COUNT];
};

struct SmsData {
    int sms[SMS_COUNT];
};

struct UsoData {
    int uso[USO_COUNT];
};

/*
 * TODO: create an ActuatorWidget by combining all these Widgets
 * */

struct ExhaustWidget {
    enum ExhaustWidgetType type;
    void *state;
    void *widget;
    union {
        enum ExhaustMode efocused;
        enum BistableState bfocused;
    };
    int idx; /* for fans and panels */
};

struct ExhaustData {
    enum ExhaustMode mode;

    enum BistableState panels[EXHAUST_PANEL_COUNT];
    enum BistableState peltier;
    enum BistableState fans[EXHAUST_FAN_COUNT];
};

struct ExhaustSubMenuState {
    struct ExhaustWidget *first_widget;
    /*
     * this feels lame
     * */
    int sel_widget_idx; /* index */
    int total_widgets;
    int cury;
};

struct HumidifierData {
    enum BistableState humidifier;
};

struct HumidifierWidget {
    void *state;
    void *widget;
    enum BistableState bfocused;
    int idx; /* not usefull right now */
};

struct HumidifierSubMenuState {
    struct HumidifierWidget *first_widget;
    /*
     * this feels lame
     * */
    int sel_widget_idx; /* index */
    int total_widgets;
    int cury;
};

struct LightingData {
    enum BistableState lights[LIGHT_ARRAY_COUNT];
};

struct LightingWidget {
    void *state;
    void *widget;
    enum BistableState bfocused;
    int idx;
};

struct LightingSubMenuState {
    struct LightingWidget *first_widget;
    /*
     * this feels lame
     * */
    int sel_widget_idx; /* index */
    int total_widgets;
    int cury;
};

void draw_top_win(const char *title);
void draw_bot_win(struct MainMenu *menu);
int draw_sub_menu(struct MainMenu *mainmenu);
void draw_main_menu(struct MainMenu *menu);
int draw_start_tui_app(void);

int draw_init_ncurses(void);
int draw_deinit_ncurses(void);
