#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>
#include<stdint.h>
#include<assert.h>

#include "draw.h"
#include "emcu.h"
#include "tcp.h"
#include "utils.h"

static WINDOW *TopWin, *MainWin, *BotWin;

/*
 * padding config
 * */
static unsigned int edge_pad_vertical = 3;
static unsigned int edge_pad_horizontal = 1;
static unsigned int topwin_nlines = 3;
static unsigned int botwin_nlines = 2;
static unsigned int mainwin_pad_horizontal = 1;

/*
 * for ncurses
 * */
#define KEY_RETURN  10
#define KEY_ESCAPE  27

#define EMCU_CONNECTED_TEXT          "Connected"
#define EMCU_NOT_CONNECTED_TEXT  "Not Connected"

#define DATA_WIDGET_DEFAULT_KEY    "SENSOR"
#define DATA_WIDGET_DEFAULT_VALUE  "VALUE"

#define RADIO_BUTTON_WIDGET_DEFAULT_TEXT         "Radio Button Name"
#define RADIO_BUTTON_WIDGET_DEFAULT_FIRST_STATE            "STATE 1"
#define RADIO_BUTTON_WIDGET_DEFAULT_SECOND_STATE           "STATE 2"
#define RADIO_BUTTON_WIDGET_DEFAULT_THIRD_STATE            "STATE 3"

static void *sub_menu_sensor_handler(void *param);

static void *sub_menu_exhaust_handler(void *param);
static void *sub_menu_humidifier_handler(void *param);
static void *sub_menu_lighting_handler(void *param);

static void draw_sub_menu_temp(WINDOW *win, const struct SubMenu *submenu);
static void draw_sub_menu_hume(WINDOW *win, const struct SubMenu *submenu);
static void draw_sub_menu_ldr(WINDOW *win, const struct SubMenu *submenu);
static void draw_sub_menu_sms(WINDOW *win, const struct SubMenu *submenu);
static void draw_sub_menu_uso(WINDOW *win, const struct SubMenu *submenu);

static void draw_sub_menu_exhaust(WINDOW *win, const struct SubMenu *submenu);
static void draw_sub_menu_humidifier(WINDOW *win, const struct SubMenu *submenu);
static void draw_sub_menu_lighting(WINDOW *win, const struct SubMenu *submenu);

static void fill_data_widget_config(struct DataWidget *widget);
static void draw_data_widget(WINDOW *win, const struct DataWidget *widget);

/*static void fill_radio_button_widget_config(struct DataWidget *widget);*/
static void draw_radio_button_widget(WINDOW *win, void *widget_data);

static void init_colorschemes(void);
static void create_windows(void);
static void resize_windows(void);

static struct TempData TempSensorData  = { .dht = { 327, 182 }             };
static struct HumeData HumeSensorData  = { .dht = { 239, 789 }             };
static struct LdrData LdrSensorData    = { .ldr = { 3199, 199, 2400 }      };
static struct SmsData SmsSensorData    = { .sms = { 2400, 122, 3199, 1600 }};
static struct UsoData UsoSensorData    = { .uso = { 140, 188 }             };

static struct ExhaustData IncExhaustData = {
    .mode = EXHAUST_MODE_NONE,
    .panels = {
        BISTABLE_STATE_OFF,
        BISTABLE_STATE_OFF,
        BISTABLE_STATE_OFF,
        BISTABLE_STATE_OFF,
        BISTABLE_STATE_OFF,
        BISTABLE_STATE_OFF,
        BISTABLE_STATE_OFF,
    },
    .peltier = BISTABLE_STATE_OFF,
    .fans = {
        BISTABLE_STATE_OFF,
        BISTABLE_STATE_OFF,
    },
};

static struct HumidifierData IncHumidifierData = {
    .humidifier = BISTABLE_STATE_OFF,
};

static struct LightingData IncLightingData = {
    .lights = {
        BISTABLE_STATE_OFF,
        BISTABLE_STATE_OFF,
    },
};

#define EXHAUST_RADIO_MODE_HEIGHT       7
#define EXHAUST_RADIO_MODE_TEXT         "Exhaust Mode"
#define EXHAUST_RADIO_FIRST_MODE_TEXT  "CMODE"
#define EXHAUST_RADIO_SECOND_MODE_TEXT "HMODE"
#define EXHAUST_RADIO_THIRD_MODE_TEXT  "NONE"

static struct RadioButtonWidget exhaust_mode_widget = {

    /*
     * ALERT: need to fill the rest of the fields before using.
     * */

    .width = 5,
    .height = EXHAUST_RADIO_MODE_HEIGHT,
    .xopad = 1,
    .xipad = 3,
    .yopad = 0,
    .yipad = 1,

    .mode = EXHAUST_MODE_NONE,

    .text = EXHAUST_RADIO_MODE_TEXT,
    .text_size = sizeof(EXHAUST_RADIO_MODE_TEXT) - 1,

    .first_state_text = EXHAUST_RADIO_FIRST_MODE_TEXT,
    .first_state_size = sizeof(EXHAUST_RADIO_FIRST_MODE_TEXT) - 1,

    .second_state_text = EXHAUST_RADIO_SECOND_MODE_TEXT,
    .second_state_size = sizeof(EXHAUST_RADIO_SECOND_MODE_TEXT) - 1,

    .third_state_text = EXHAUST_RADIO_THIRD_MODE_TEXT,
    .third_state_size = sizeof(EXHAUST_RADIO_THIRD_MODE_TEXT) - 1,

};

#define SWITCH_WIDGET_HEIGHT  5

#define PANEL_SWITCH_JUST_TEXT                                     "Panel "
#define PANEL_SWITCH_NUM_IDX             sizeof(PANEL_SWITCH_JUST_TEXT) - 1
#define PANEL_SWITCH_FIRST_STATE_TEXT                                "LEFT"
#define PANEL_SWITCH_SECOND_STATE_TEXT                              "RIGHT"

static char panel_switch_text[] = PANEL_SWITCH_JUST_TEXT "0";

static struct SwitchWidget panel_switch_widget = {

    /*
     * ALERT: need to fill the rest of the fields before using.
     * */

    .width = 5,
    .height = SWITCH_WIDGET_HEIGHT,
    .xopad = 1,
    .xipad = 3,
    .yopad = 0,
    .yipad = 1,

    .state = BISTABLE_STATE_OFF,

    .text = panel_switch_text,
    .text_size = sizeof(panel_switch_text) - 1,

    .first_state_text = PANEL_SWITCH_FIRST_STATE_TEXT,
    .first_state_size = sizeof(PANEL_SWITCH_FIRST_STATE_TEXT) - 1,

    .second_state_text = PANEL_SWITCH_SECOND_STATE_TEXT,
    .second_state_size = sizeof(PANEL_SWITCH_SECOND_STATE_TEXT) - 1,
};

#define FAN_SWITCH_JUST_TEXT                                       "Fan "
#define FAN_SWITCH_NUM_IDX               sizeof(FAN_SWITCH_JUST_TEXT) - 1
#define FAN_SWITCH_FIRST_STATE_TEXT                                 "OFF"
#define FAN_SWITCH_SECOND_STATE_TEXT                                 "ON"

static char fan_switch_text[] = FAN_SWITCH_JUST_TEXT "0";

static struct SwitchWidget fan_switch_widget = {

    /*
     * ALERT: need to fill the rest of the fields before using.
     * */

    .width = 5,
    .height = SWITCH_WIDGET_HEIGHT,
    .xopad = 1,
    .xipad = 3,
    .yopad = 0,
    .yipad = 1,

    .state = BISTABLE_STATE_OFF,

    .text = fan_switch_text,
    .text_size = sizeof(fan_switch_text) - 1,

    .first_state_text = FAN_SWITCH_FIRST_STATE_TEXT,
    .first_state_size = sizeof(FAN_SWITCH_FIRST_STATE_TEXT) - 1,

    .second_state_text = FAN_SWITCH_SECOND_STATE_TEXT,
    .second_state_size = sizeof(FAN_SWITCH_SECOND_STATE_TEXT) - 1,
};

#define PELTIER_SWITCH_TEXT                                  "Peltier"
#define PELTIER_SWITCH_FIRST_STATE_TEXT                          "OFF"
#define PELTIER_SWITCH_SECOND_STATE_TEXT                          "ON"

static struct SwitchWidget peltier_switch_widget = {

    /*
     * ALERT: need to fill the rest of the fields before using.
     * */

    .width = 5,
    .height = SWITCH_WIDGET_HEIGHT,
    .xopad = 1,
    .xipad = 3,
    .yopad = 0,
    .yipad = 1,

    .state = BISTABLE_STATE_OFF,

    .text = PELTIER_SWITCH_TEXT,
    .text_size = sizeof(PELTIER_SWITCH_TEXT) - 1,

    .first_state_text = PELTIER_SWITCH_FIRST_STATE_TEXT,
    .first_state_size = sizeof(PELTIER_SWITCH_FIRST_STATE_TEXT) - 1,

    .second_state_text = PELTIER_SWITCH_SECOND_STATE_TEXT,
    .second_state_size = sizeof(PELTIER_SWITCH_SECOND_STATE_TEXT) - 1,
};

static struct ExhaustWidget exhaust_widgets[] = {
    /* mode    */
    {
        .type = EXHAUST_WIDGET_TYPE_EXHAUST_MODE,
        .state = (void *) &(IncExhaustData.mode),
        .widget = (void *) &exhaust_mode_widget,
        .idx = 0, /* we are not using this */
        .efocused = EXHAUST_MODE_CMODE,
    },

    /* peltier */
    {
        .type = EXHAUST_WIDGET_TYPE_PELTIER,
        .state = (void *) &(IncExhaustData.panels),
        .widget = (void *) &peltier_switch_widget,
        .idx = 0, /* we are not using this */
        .bfocused = BISTABLE_STATE_OFF,
    },

    /* fans    */
    {
        .type = EXHAUST_WIDGET_TYPE_FAN,
        .state = (void *) &(IncExhaustData.fans[0]),
        .widget = (void *) &fan_switch_widget,
        .idx = 0,
        .bfocused = BISTABLE_STATE_OFF,
    },
    {
        .type = EXHAUST_WIDGET_TYPE_FAN,
        .state = (void *) &(IncExhaustData.fans[1]),
        .widget = (void *) &fan_switch_widget,
        .idx = 1,
        .bfocused = BISTABLE_STATE_OFF,
    },

    /* panels  */
    {
        .type = EXHAUST_WIDGET_TYPE_PANEL,
        .state = (void *) &(IncExhaustData.panels[0]),
        .widget = (void *) &panel_switch_widget,
        .idx = 0,
        .bfocused = BISTABLE_STATE_OFF,
    },
    {
        .type = EXHAUST_WIDGET_TYPE_PANEL,
        .state = (void *) &(IncExhaustData.panels[1]),
        .widget = (void *) &panel_switch_widget,
        .idx = 1,
        .bfocused = BISTABLE_STATE_OFF,
    },
    {
        .type = EXHAUST_WIDGET_TYPE_PANEL,
        .state = (void *) &(IncExhaustData.panels[2]),
        .widget = (void *) &panel_switch_widget,
        .idx = 2,
        .bfocused = BISTABLE_STATE_OFF,
    },
    {
        .type = EXHAUST_WIDGET_TYPE_PANEL,
        .state = (void *) &(IncExhaustData.panels[3]),
        .widget = (void *) &panel_switch_widget,
        .idx = 3,
        .bfocused = BISTABLE_STATE_OFF,
    },
    {
        .type = EXHAUST_WIDGET_TYPE_PANEL,
        .state = (void *) &(IncExhaustData.panels[4]),
        .widget = (void *) &panel_switch_widget,
        .idx = 4,
        .bfocused = BISTABLE_STATE_OFF,
    },
    {
        .type = EXHAUST_WIDGET_TYPE_PANEL,
        .state = (void *) &(IncExhaustData.panels[5]),
        .widget = (void *) &panel_switch_widget,
        .idx = 5,
        .bfocused = BISTABLE_STATE_OFF,
    },
    {
        .type = EXHAUST_WIDGET_TYPE_PANEL,
        .state = (void *) &(IncExhaustData.panels[6]),
        .widget = (void *) &panel_switch_widget,
        .idx = 6,
        .bfocused = BISTABLE_STATE_OFF,
    },
};

static struct ExhaustSubMenuState exhaust_sub_menu_state = {
    .first_widget = exhaust_widgets,
    .sel_widget_idx = 0,
    .total_widgets = sizeof(exhaust_widgets) / sizeof(struct ExhaustWidget),
    .cury = 0,
};

#define HUMIDIFIER_SWITCH_JUST_TEXT                                       "Humidifier "
#define HUMIDIFIER_SWITCH_NUM_IDX               sizeof(HUMIDIFIER_SWITCH_JUST_TEXT) - 1
#define HUMIDIFIER_SWITCH_FIRST_STATE_TEXT                                 "OFF"
#define HUMIDIFIER_SWITCH_SECOND_STATE_TEXT                                 "ON"

static char humidifier_switch_text[] = HUMIDIFIER_SWITCH_JUST_TEXT "0";

static struct SwitchWidget humidifier_switch_widget = {

    /*
     * ALERT: need to fill the rest of the fields before using.
     * */

    .width = 5,
    .height = SWITCH_WIDGET_HEIGHT,
    .xopad = 1,
    .xipad = 3,
    .yopad = 0,
    .yipad = 1,

    .state = BISTABLE_STATE_OFF,

    .text = humidifier_switch_text,
    .text_size = sizeof(humidifier_switch_text) - 1,

    .first_state_text = HUMIDIFIER_SWITCH_FIRST_STATE_TEXT,
    .first_state_size = sizeof(HUMIDIFIER_SWITCH_FIRST_STATE_TEXT) - 1,

    .second_state_text = HUMIDIFIER_SWITCH_SECOND_STATE_TEXT,
    .second_state_size = sizeof(HUMIDIFIER_SWITCH_SECOND_STATE_TEXT) - 1,
};

static struct HumidifierWidget humidifier_widgets[] = {
    {
        .state = (void *) &(IncHumidifierData.humidifier),
        .widget = (void *) &humidifier_switch_widget,
        .bfocused = BISTABLE_STATE_OFF,
        .idx = 0,
    },
};

static struct HumidifierSubMenuState humidifier_sub_menu_state = {
    .first_widget = humidifier_widgets,
    .sel_widget_idx = 0,
    .total_widgets = sizeof(humidifier_widgets) / sizeof(struct HumidifierWidget),
    .cury = 0,
};

#define LIGHTING_SWITCH_JUST_TEXT                                       "Light "
#define LIGHTING_SWITCH_NUM_IDX               sizeof(LIGHTING_SWITCH_JUST_TEXT) - 1
#define LIGHTING_SWITCH_FIRST_STATE_TEXT                                 "OFF"
#define LIGHTING_SWITCH_SECOND_STATE_TEXT                                 "ON"

static char lighting_switch_text[] = LIGHTING_SWITCH_JUST_TEXT "0";

static struct SwitchWidget lighting_switch_widget = {

    /*
     * ALERT: need to fill the rest of the fields before using.
     * */

    .width = 5,
    .height = SWITCH_WIDGET_HEIGHT,
    .xopad = 1,
    .xipad = 3,
    .yopad = 0,
    .yipad = 1,

    .state = BISTABLE_STATE_OFF,

    .text = lighting_switch_text,
    .text_size = sizeof(lighting_switch_text) - 1,

    .first_state_text = LIGHTING_SWITCH_FIRST_STATE_TEXT,
    .first_state_size = sizeof(LIGHTING_SWITCH_FIRST_STATE_TEXT) - 1,

    .second_state_text = LIGHTING_SWITCH_SECOND_STATE_TEXT,
    .second_state_size = sizeof(LIGHTING_SWITCH_SECOND_STATE_TEXT) - 1,
};

static struct LightingWidget lighting_widgets[] = {
    {
        .state = (void *) &(IncLightingData.lights[0]),
        .widget = (void *) &lighting_switch_widget,
        .bfocused = BISTABLE_STATE_OFF,
        .idx = 0,
    },
    {
        .state = (void *) &(IncLightingData.lights[1]),
        .widget = (void *) &lighting_switch_widget,
        .bfocused = BISTABLE_STATE_OFF,
        .idx = 1,
    },
};

static struct LightingSubMenuState lighting_sub_menu_state = {
    .first_widget = lighting_widgets,
    .sel_widget_idx = 0,
    .total_widgets = sizeof(lighting_widgets) / sizeof(struct LightingWidget),
    .cury = 0,
};

static const struct SubMenu sub_menus[GEN_SUB_MENU_END] = {
    [GEN_SUB_MENU_TEMPERATURE] = {
        .title = "Temperature",
        .info = "Indoor and outdoor temperature",
        .handler = sub_menu_sensor_handler,
        .data = (void *) &TempSensorData,
    },
    [GEN_SUB_MENU_AIR_MOISTURE] = {
        .title = "Air Moisture",
        .info = "Indoor and outdoor air moisture",
        .handler = sub_menu_sensor_handler,
        .data = (void *) &HumeSensorData,
    },
    [GEN_SUB_MENU_LIGHT_SENSE] = {
        .title = "Lighting",
        .info = "LDR data within the incubator",
        .handler = sub_menu_sensor_handler,
        .data = (void *) &LdrSensorData,
    },
    [GEN_SUB_MENU_SOIL_MOISTURE] = {
        .title = "Soil Moisture",
        .info = "Soil moisture at the base of each plant",
        .handler = sub_menu_sensor_handler,
        .data = (void *) &SmsSensorData,
    },
    /*[GEN_SUB_MENU_RESERVOIR_LEVEL] = {*/
    /*    .title = "Reservoir Level",*/
    /*    .info = "Water and fertilizer level",*/
    /*    .handler = sub_menu_sensor_handler,*/
    /*    .data = (void *) &UsoSensorData,*/
    /*},*/
    [GEN_SUB_MENU_EXHAUST] = {
        .title = "Exhaust",
        .info = "Exhaust control",
        .handler = sub_menu_exhaust_handler,
        .data = (void *) &TempSensorData, /* TODO: remove this */
    },
    [GEN_SUB_MENU_HUMIDIFIER] = {
        .title = "Humidifier",
        .info = "Humidifier control",
        .handler = sub_menu_humidifier_handler,
        .data = (void *) &TempSensorData,
    },
    [GEN_SUB_MENU_LIGHTING] = {
        .title = "Lighting",
        .info = "Lighting control",
        .handler = sub_menu_lighting_handler,
        .data = (void *) &TempSensorData,
    },
};

static void (*sensor_sub_menu_draw_lut[SENS_SUB_MENU_END])
    (WINDOW *, const struct SubMenu *) = {
        [SENS_SUB_MENU_TEMPERATURE]     = draw_sub_menu_temp,
        [SENS_SUB_MENU_AIR_MOISTURE]    = draw_sub_menu_hume,
        [SENS_SUB_MENU_LIGHT_SENSE]     = draw_sub_menu_ldr,
        [SENS_SUB_MENU_SOIL_MOISTURE]   = draw_sub_menu_sms,
        /*[SENS_SUB_MENU_RESERVOIR_LEVEL] = draw_sub_menu_uso,*/
};

static int sensor_sub_cmd_lut[SENS_SUB_MENU_END][2] = {
        [SENS_SUB_MENU_TEMPERATURE]     = { SUB_CMD_MONITOR_TEMP,  DHT_COUNT},
        [SENS_SUB_MENU_AIR_MOISTURE]    = { SUB_CMD_MONITOR_HUME,  DHT_COUNT},
        [SENS_SUB_MENU_LIGHT_SENSE]     = { SUB_CMD_MONITOR_LDR,   LDR_COUNT},
        [SENS_SUB_MENU_SOIL_MOISTURE]   = { SUB_CMD_MONITOR_SMS,   SMS_COUNT},
        /*[SENS_SUB_MENU_RESERVOIR_LEVEL] = { SUB_CMD_MONITOR_USO,   USO_COUNT},*/
};

static const char app_name[] = "Seed Incubator";

static const char *char_symbols[CHAR_END] = {
    [CHAR_CORNER_TOPLEFT]           = "┌",
    [CHAR_CORNER_TOPRIGHT]          = "┐",
    [CHAR_CORNER_BOTRIGHT]          = "┘",
    [CHAR_CORNER_BOTLEFT]           = "└",
    [CHAR_CORNER_ROUNDED_TOPLEFT]   = "╭",
    [CHAR_CORNER_ROUNDED_TOPRIGHT]  = "╮",
    [CHAR_CORNER_ROUNDED_BOTRIGHT]  = "╯",
    [CHAR_CORNER_ROUNDED_BOTLEFT]   = "╰",
    [CHAR_EDGE_TOP]                 = "┬",
    [CHAR_EDGE_RIGHT]               = "┤",
    [CHAR_EDGE_BOT]                 = "┴",
    [CHAR_EDGE_LEFT]                = "├",
    [CHAR_MINUS]                    = "─",
    [CHAR_PIPE]                     = "│",
    [CHAR_CROSS]                    = "┼",
    [CHAR_DOT]                      = "•",
    [CHAR_FISHEYE]                  = "◉",
    [CHAR_CIRCLE]                   = "○",
};

/*❰ 5 ◉ hai*/
/*  4  ◎ lol*/
/*  3   ○ llkk*/

static const char *colors[COLOR_END] = {
	[COLOR_PRI_ACCENT] = "#CCE099",
	[COLOR_SEC_ACCENT] = "#8AB872",
    [COLOR_PRI_WHITE]  = "#DDDDDD",
    [COLOR_SEC_WHITE]  = "#979797",
    [COLOR_TER_WHITE]  = "#3C3C3A",
};

static int16_t color_pairs[PAIR_END][2] = {
	[PAIR_PRI_ACCENT_DEFAULT_BG] = {COLOR_PRI_ACCENT, COLOR_DEFAULT_BG},
	[PAIR_SEC_ACCENT_DEFAULT_BG] = {COLOR_SEC_ACCENT, COLOR_DEFAULT_BG},
	[PAIR_PRI_WHITE_DEFAULT_BG]  = {COLOR_PRI_WHITE, COLOR_DEFAULT_BG},
	[PAIR_SEC_WHITE_DEFAULT_BG]  = {COLOR_SEC_WHITE, COLOR_DEFAULT_BG},
	[PAIR_TER_WHITE_DEFAULT_BG]  = {COLOR_TER_WHITE, COLOR_DEFAULT_BG},
};

static int color_schemes[SCHEME_END][ELEMENT_END] = {
	[SCHEME_DEFAULT] = {
        /*
         * TODO: there's a bug here, adding A_BOLD screws up the colors
         * instead of bolding it, here in termux terminal A_BLINK seems to
         * do bold the text.
         * */
        [ELEMENT_MENU_TITLE]                 = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG)  | A_BLINK,
        [ELEMENT_SUBMENU_TITLE_NORMAL]       = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG),
        [ELEMENT_SUBMENU_TITLE_SELECTED]     = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG),
        [ELEMENT_SUBMENU_INFO_NORMAL]        = ColorPair(PAIR_SEC_WHITE_DEFAULT_BG),
        [ELEMENT_SUBMENU_INFO_SELECTED]      = ColorPair(PAIR_SEC_ACCENT_DEFAULT_BG),
        [ELEMENT_INFO_FRAME]                 = ColorPair(PAIR_SEC_WHITE_DEFAULT_BG),
        [ELEMENT_INFO_TEXT]                  = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG),
        [ELEMENT_INFO_VALUE]                 = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG)  | A_BOLD,
        [ELEMENT_DOTS_NORMAL]                = ColorPair(PAIR_TER_WHITE_DEFAULT_BG),
        [ELEMENT_DOTS_SELECTED]              = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG),
        [ELEMENT_EMCU_CONNECTED]             = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG),
        [ELEMENT_EMCU_NOT_CONNECTED]         = ColorPair(PAIR_SEC_WHITE_DEFAULT_BG),

        [ELEMENT_DATA_WIDGET_FRAME_NORMAL]   = ColorPair(PAIR_SEC_WHITE_DEFAULT_BG)  | A_NORMAL,
        [ELEMENT_DATA_WIDGET_FRAME_SELECTED] = ColorPair(PAIR_SEC_ACCENT_DEFAULT_BG) | A_NORMAL,

        [ELEMENT_DATA_WIDGET_KEY_NORMAL]     = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG)  | A_BLINK,
        [ELEMENT_DATA_WIDGET_KEY_SELECTED]   = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG) | A_BLINK,
        [ELEMENT_DATA_WIDGET_VALUE_NORMAL]   = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG)  | A_BLINK,
        [ELEMENT_DATA_WIDGET_VALUE_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG) | A_BLINK,
        /* Radio button */
        [ELEMENT_RADIO_BUTTON_FRAME_NORMAL]   = ColorPair(PAIR_SEC_WHITE_DEFAULT_BG),
        [ELEMENT_RADIO_BUTTON_FRAME_SELECTED] = ColorPair(PAIR_SEC_ACCENT_DEFAULT_BG),
        [ELEMENT_RADIO_BUTTON_TEXT_NORMAL]   = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG) | A_BLINK,
        [ELEMENT_RADIO_BUTTON_TEXT_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG) | A_BLINK,

        [ELEMENT_RADIO_BUTTON_ACTIVE_NORMAL] = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG),
        [ELEMENT_RADIO_BUTTON_ACTIVE_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG),
        [ELEMENT_RADIO_BUTTON_INACTIVE_NORMAL] = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG)    | A_DIM,
        [ELEMENT_RADIO_BUTTON_INACTIVE_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG) | A_DIM,

        [ELEMENT_RADIO_BUTTON_ACTIVE_FOCUSED_NORMAL] = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG) | A_BLINK,
        [ELEMENT_RADIO_BUTTON_ACTIVE_FOCUSED_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG)  | A_BLINK,
        [ELEMENT_RADIO_BUTTON_INACTIVE_FOCUSED_NORMAL] = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG)    | A_DIM | A_BLINK,
        [ELEMENT_RADIO_BUTTON_INACTIVE_FOCUSED_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG) | A_DIM | A_BLINK,

        /* Switche */

        [ELEMENT_SWITCH_FRAME_NORMAL]   = ColorPair(PAIR_SEC_WHITE_DEFAULT_BG),
        [ELEMENT_SWITCH_FRAME_SELECTED] = ColorPair(PAIR_SEC_ACCENT_DEFAULT_BG),
        [ELEMENT_SWITCH_TEXT_NORMAL]   = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG) | A_BLINK,
        [ELEMENT_SWITCH_TEXT_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG) | A_BLINK,

        [ELEMENT_SWITCH_ACTIVE_NORMAL] = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG),
        [ELEMENT_SWITCH_ACTIVE_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG),
        [ELEMENT_SWITCH_INACTIVE_NORMAL] = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG)    | A_DIM,
        [ELEMENT_SWITCH_INACTIVE_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG) | A_DIM,

        [ELEMENT_SWITCH_ACTIVE_FOCUSED_NORMAL] = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG) | A_BLINK,
        [ELEMENT_SWITCH_ACTIVE_FOCUSED_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG)  | A_BLINK,
        [ELEMENT_SWITCH_INACTIVE_FOCUSED_NORMAL] = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG)    | A_DIM | A_BLINK,
        [ELEMENT_SWITCH_INACTIVE_FOCUSED_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG) | A_DIM | A_BLINK,

	},
	/* ill set this later */
	//[SCHEME_MONOCHROME] = NULL,
};

static void init_colorschemes(void) {

	const char *hexstring;
	int r, g, b;
	int i;

	start_color();
	use_default_colors();

	/* initializing colors */

	for (i = 0;i < COLOR_END; i++) {
		hexstring = colors[i];
		if (*hexstring == '#')
			hexstring++;

		sscanf(hexstring, "%02x%02x%02x", &r, &g, &b);
		init_color(i, r*1000/255, g*1000/255, b*1000/255);
	}

	/* initializing color pairs */

	/*
	 * Since we are using use_default_colors() from ncurses library, the zeroth
	 * pair doesn't seems to be overriden by assigning a new pair to it.
	 *
	 * So instead of changing the zeroth pair we will start from 1
	 * */

	for (i = 0;i < PAIR_END;i++)
		init_pair(i + 1, color_pairs[i][0], color_pairs[i][1]);

}

static void create_windows(void) {

	int xmax, ymax;

	getmaxyx(stdscr, ymax, xmax);

	TopWin = newwin(topwin_nlines,
			xmax - (2 * edge_pad_vertical),
			edge_pad_horizontal,
			edge_pad_vertical);

	MainWin = newwin(ymax - (topwin_nlines + botwin_nlines)
			- (2 * edge_pad_horizontal) - (2 * mainwin_pad_horizontal),
			xmax - (2 * edge_pad_vertical),
			edge_pad_horizontal + topwin_nlines + mainwin_pad_horizontal,
			edge_pad_vertical);

	BotWin = newwin(botwin_nlines,
			xmax - (2 * edge_pad_vertical),
			ymax - botwin_nlines - edge_pad_horizontal,
			edge_pad_vertical);

}

static void resize_windows(void) {

	int xmax, ymax;

	getmaxyx(stdscr, ymax, xmax);

	werase(stdscr);
	wrefresh(stdscr);

	wresize(TopWin, topwin_nlines, xmax - (2 * edge_pad_vertical));

	wresize(MainWin, ymax - (topwin_nlines + botwin_nlines)
			- (2 * edge_pad_horizontal) - (2 * mainwin_pad_horizontal),
			xmax - (2 * edge_pad_vertical));

	wresize(BotWin, botwin_nlines, xmax - (2 * edge_pad_vertical));

	mvwin(BotWin, ymax - botwin_nlines - edge_pad_horizontal, edge_pad_vertical);

}

void draw_top_win(const char *title) {

    WINDOW *win = TopWin;

	wmove(win, 1, 0);
	wclrtoeol(win);
	wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_MENU_TITLE]);
	wprintw(win, "%s", title);

}

void draw_bot_win(struct MainMenu *menu) {

    WINDOW *win = BotWin;

    int ymax, xmax;

	int maxitems;
	int left_smenus, right_smenus;

	getmaxyx(win, ymax, xmax);

	maxitems = menu->total_sub_menus;

	left_smenus = menu->sel_sub_menu_idx;
	right_smenus = menu->total_sub_menus - menu->sel_sub_menu_idx - 1;

	wmove(win, 0, 0);
	wclrtoeol(win);

	wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_DOTS_NORMAL]);
	while (left_smenus) {
		wprintw(win, "%s", char_symbols[CHAR_DOT]);
		left_smenus--;
	}

	wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_DOTS_SELECTED]);
	wprintw(win, "%s", char_symbols[CHAR_DOT]);

	wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_DOTS_NORMAL]);
	while (right_smenus) {
		wprintw(win, "%s", char_symbols[CHAR_DOT]);
		right_smenus--;
	}

    if (menu->emcu_connected == 1) {
        wmove(win, 0, xmax - sizeof(EMCU_CONNECTED_TEXT));
        wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_EMCU_CONNECTED]);
        wprintw(win, "%s", EMCU_CONNECTED_TEXT);
    } else {
        wmove(win, 0, xmax - sizeof(EMCU_NOT_CONNECTED_TEXT));
        wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_EMCU_NOT_CONNECTED]);
        wprintw(win, "%s", EMCU_NOT_CONNECTED_TEXT);
    }

}

int draw_sub_menu(struct MainMenu *mainmenu) {

    int event;

    const struct SubMenu *submenu = mainmenu->first_sub_menu + mainmenu->sel_sub_menu_idx;

    draw_top_win(submenu->title);

    /*
     * just filling a dummy value that the handler functions should
     * ignore.
     * */
    mainmenu->event = KEY_ESCAPE;
    submenu->handler((void *) mainmenu);

    /*draw_main_menu(&mainmenu);*/
    draw_bot_win(mainmenu);

	wrefresh(TopWin);
	wrefresh(MainWin);
    wrefresh(BotWin);

    if (mainmenu->sel_sub_menu_idx < SENS_SUB_MENU_END) {
        wtimeout(MainWin, 100);
    }

	while ((event = wgetch(MainWin))) {

        mainmenu->event = event;
        switch (event) {
            case 'q':
            case KEY_ESCAPE:
                if (mainmenu->sel_sub_menu_idx < SENS_SUB_MENU_END) {
                    wtimeout(MainWin, -1);
                }
                return 0;
            case KEY_UP:
                break;
            case 'p':
                if (mainmenu->sel_sub_menu_idx > 0) {
                    mainmenu->cury = mainmenu->cury - 3;
                    /*assert((mainmenu.cury >= 0));*/
                    mainmenu->sel_sub_menu_idx--;
                    submenu--;
                    draw_top_win(submenu->title);
                    wrefresh(TopWin);
                }
                if (mainmenu->sel_sub_menu_idx < SENS_SUB_MENU_END) {
                    wtimeout(MainWin, 100);
                }
                break;
            case KEY_DOWN:
                break;
            case 'n':
                if (mainmenu->sel_sub_menu_idx < mainmenu->total_sub_menus - 1) {
                    mainmenu->cury = mainmenu->cury + 3;
                    mainmenu->sel_sub_menu_idx++;
                    submenu++;
                    draw_top_win(submenu->title);
                    wrefresh(TopWin);
                }
                if (mainmenu->sel_sub_menu_idx >= SENS_SUB_MENU_END) {
                    wtimeout(MainWin, -1);
                }
                break;
            case KEY_RESIZE:
                resize_windows();
                draw_top_win(submenu->title);
                wrefresh(TopWin);
                break;
        }

        submenu->handler((void *) mainmenu);
        wrefresh(MainWin);

        draw_bot_win(mainmenu);
        wrefresh(BotWin);

	}

    return 0;
}

void draw_main_menu(struct MainMenu *menu) {

    /*
     * TODO: there's a bug lurking somewhere here,
     * fix it.
     * */

    int i;
    int ymax, xmax;
    int cury;

    WINDOW *win = MainWin;

    getmaxyx(win, ymax, xmax);
    werase(win);

    /*
     * if the screen has been resized after the last
     * draw we need to adjust the position of currently
     * selected submenu
     * */
    if (menu->cury + 3 > ymax) {
        menu->cury = ymax - 3;
    }
    if (menu->cury < 0) {
        menu->cury = 0;
    }

    int smenus_above;
    smenus_above = menu->sel_sub_menu_idx;

    if (menu->cury > smenus_above * 3) {
        menu->cury = smenus_above * 3;
    }

    int smenu_draw_start;
    smenu_draw_start = menu->sel_sub_menu_idx - (menu->cury / 3);
    cury = 0;

    /* draw the items above the selected */
    for (i = smenu_draw_start; i < menu->sel_sub_menu_idx; i++) {

        wmove(win, cury, 2);
        wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_SUBMENU_TITLE_NORMAL]);
        wprintw(win, "%s", sub_menus[i].title);

        cury++;

        wmove(win, cury, 2);
        wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_SUBMENU_INFO_NORMAL]);
        wprintw(win, "%s", sub_menus[i].info);

        cury = cury + 2;

    }

    /*
     * drawing the selected sub menu
     * */
    int pcury = cury;

    wmove(win, cury, 0);
    wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_SUBMENU_INFO_SELECTED]);
    wprintw(win, "%s", char_symbols[CHAR_PIPE]);

    wmove(win, cury, 2);
    wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_SUBMENU_TITLE_SELECTED]);
    wprintw(win, "%s", sub_menus[i].title);

    cury++;

    wmove(win, cury, 0);
    wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_SUBMENU_INFO_SELECTED]);
    wprintw(win, "%s", char_symbols[CHAR_PIPE]);

    wmove(win, cury, 2);
    wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_SUBMENU_INFO_SELECTED]);
    wprintw(win, "%s", sub_menus[i].info);

    cury = cury + 2;

    int smenu_below = (ymax - (cury - 2) - 2) / 3;
    int smenu_draw_end = smenu_below + menu->sel_sub_menu_idx + 1;

    if (smenu_draw_end > menu->total_sub_menus) {
        smenu_draw_end = menu->total_sub_menus;
    }

    /* draw the sub menus below the selected */
    for (i = menu->sel_sub_menu_idx + 1; i < smenu_draw_end; i++) {

        wmove(win, cury, 2);
        wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_SUBMENU_TITLE_NORMAL]);
        wprintw(win, "%s", sub_menus[i].title);

        cury++;

        wmove(win, cury, 2);
        wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_SUBMENU_INFO_NORMAL]);
        wprintw(win, "%s", sub_menus[i].info);

        cury = cury + 2;

    }
}

static void draw_data_widget(WINDOW *win, const struct DataWidget *widget) {

    int i;
    int cury, curx;
    int oriy, orix;
    getyx(win, cury, curx);
    oriy = cury;
    orix = curx;

    cury = cury + widget->yopad;
    curx = curx + widget->xopad;

    int frame_width = widget->width - (2 * (widget->xopad));
    int frame_height = widget->height - (2 * (widget->yopad));

	wattron(win, widget->frame_scheme);

    /*
     * top line
     * */
    wmove(win, cury, curx);
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_TOPLEFT]);
    curx++;
    for (i = 0; i < frame_width - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_MINUS]);
        curx++;
    }
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_TOPRIGHT]);

    /*
     * bot line
     * */

    cury = oriy + widget->height - widget->yopad - 1;
    curx = orix + widget->xopad;
    wmove(win, cury, curx);
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_BOTLEFT]);
    curx++;
    for (i = 0; i < frame_width - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_MINUS]);
        curx++;
    }
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_BOTRIGHT]);

    /*
     * left side
     * */
    cury = oriy + widget->yopad + 1;
    curx = orix + widget->xopad;
    for (i = 0; i < frame_height - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_PIPE]);
        cury++;
    }

    /*
     * right side
     * */
    cury = oriy + widget->yopad + 1;
    curx = orix + widget->xopad + frame_width - 1;
    for (i = 0; i < frame_height - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_PIPE]);
        cury++;
    }

	wattroff(win, widget->frame_scheme);

    wattron(win, widget->data_key_scheme);

    cury = oriy + widget->yopad + 1 + widget->yipad;
    curx = orix + widget->xopad + 1 + widget->xipad;
    wmove(win, cury, curx);
    wprintw(win, "%s", widget->data_key);

    wattroff(win, widget->data_key_scheme);

    wattron(win, widget->data_value_scheme);

    cury = oriy + frame_height - 2 - widget->yipad;
    curx = orix + frame_width - widget->xipad - widget->data_value_size;
    wmove(win, cury, curx);
    wprintw(win, "%s", widget->data_value);

    wattroff(win, widget->data_value_scheme);

}

static void fill_data_widget_config(struct DataWidget *widget) {

    if (!widget) {
        return;
    }

    widget->width = 5;
    widget->height = 5;
    widget->xopad = 1;
    widget->xipad = 3;
    widget->yopad = 0;
    widget->yipad = 1;
    widget->data_key = DATA_WIDGET_DEFAULT_KEY;
    widget->data_key_size = sizeof(DATA_WIDGET_DEFAULT_KEY) - 1;
    widget->data_value = DATA_WIDGET_DEFAULT_VALUE;
    widget->data_value_size = sizeof(DATA_WIDGET_DEFAULT_VALUE) - 1;
    widget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_DATA_WIDGET_FRAME_NORMAL];
    widget->data_key_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_DATA_WIDGET_KEY_NORMAL];
    widget->data_value_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_DATA_WIDGET_VALUE_NORMAL];

}

static void draw_sub_menu_temp(WINDOW *win, const struct SubMenu *submenu) {

    werase(win);

    int ymax, xmax;
    struct DataWidget widget;
    struct TempData *data = (struct TempData *)submenu->data;
    char key[] = "DHT 0";
    char val_buf[128];
    int wrtn;
    int i;
    int cury = 0;

    getmaxyx(win, ymax, xmax);

    fill_data_widget_config(&widget);
    widget.width = xmax;

    widget.data_key = key;
    widget.data_key_size = sizeof(key) - 1;
    widget.data_value = val_buf;

    for (i = 0; i < DHT_COUNT; i++) {
        snprintf(key + 4, 2, "%d", i);
        wrtn = snprintf(val_buf, sizeof(val_buf), "%0.02f deg. Cel.", ((float) data->dht[i]) / 10);
        widget.data_value_size = wrtn - 1;
        if (cury + 5 < ymax) {
            wmove(win, cury, 0);
            draw_data_widget(win, &widget);
            cury = cury + 5;
        }
    }

}

static void draw_sub_menu_hume(WINDOW *win, const struct SubMenu *submenu) {

    werase(win);

    int ymax, xmax;
    struct DataWidget widget;
    struct HumeData *data = (struct HumeData *)submenu->data;
    char key[] = "DHT 0";
    char val_buf[128];
    int wrtn;
    int i;
    int cury = 0;

    getmaxyx(win, ymax, xmax);

    fill_data_widget_config(&widget);
    widget.width = xmax;

    widget.data_key = key;
    widget.data_key_size = sizeof(key) - 1;
    widget.data_value = val_buf;

    for (i = 0; i < DHT_COUNT; i++) {
        snprintf(key + 4, 2, "%d", i);
        wrtn = snprintf(val_buf, sizeof(val_buf), "%0.02f %%", ((float) data->dht[i]) / 10);
        widget.data_value_size = wrtn - 1;
        if (cury + 5 < ymax) {
            wmove(win, cury, 0);
            draw_data_widget(win, &widget);
            cury = cury + 5;
        }
    }

}

static void draw_sub_menu_ldr(WINDOW *win, const struct SubMenu *submenu) {

    werase(win);

    int ymax, xmax;
    struct DataWidget widget;
    struct LdrData *data = (struct LdrData *)submenu->data;
    char key[] = "LDR 0";
    char val_buf[128];
    int wrtn;
    int i;
    int cury = 0;

    getmaxyx(win, ymax, xmax);

    fill_data_widget_config(&widget);
    widget.width = xmax;

    widget.data_key = key;
    widget.data_key_size = sizeof(key) - 1;
    widget.data_value = val_buf;

    for (i = 0; i < LDR_COUNT; i++) {
        snprintf(key + 4, 2, "%d", i);
        wrtn = snprintf(val_buf, sizeof(val_buf), "%d mV", data->ldr[i] );
        widget.data_value_size = wrtn - 1;
        if (cury + 5 < ymax) {
            wmove(win, cury, 0);
            draw_data_widget(win, &widget);
            cury = cury + 5;
        }
    }

}

static void draw_sub_menu_sms(WINDOW *win, const struct SubMenu *submenu) {

    werase(win);

    int ymax, xmax;
    struct DataWidget widget;
    struct SmsData *data = (struct SmsData *)submenu->data;
    char key[] = "SMS 0";
    char val_buf[128];
    int wrtn;
    int i;
    int cury = 0;

    getmaxyx(win, ymax, xmax);

    fill_data_widget_config(&widget);
    widget.width = xmax;

    widget.data_key = key;
    widget.data_key_size = sizeof(key) - 1;
    widget.data_value = val_buf;

    for (i = 0; i < SMS_COUNT; i++) {
        snprintf(key + 4, 2, "%d", i);
        wrtn = snprintf(val_buf, sizeof(val_buf), "%d mV", data->sms[i] );
        widget.data_value_size = wrtn - 1;
        if (cury + 5 < ymax) {
            wmove(win, cury, 0);
            draw_data_widget(win, &widget);
            cury = cury + 5;
        }
    }

}

static void draw_sub_menu_uso(WINDOW *win, const struct SubMenu *submenu) {

    werase(win);

    int ymax, xmax;
    struct DataWidget widget;
    struct UsoData *data = (struct UsoData *)submenu->data;
    char key[] = "USO 0";
    char val_buf[128];
    int wrtn;
    int i;
    int cury = 0;

    getmaxyx(win, ymax, xmax);

    fill_data_widget_config(&widget);
    widget.width = xmax;

    widget.data_key = key;
    widget.data_key_size = sizeof(key) - 1;
    widget.data_value = val_buf;

    for (i = 0; i < USO_COUNT; i++) {
        snprintf(key + 4, 2, "%d", i);
        wrtn = snprintf(val_buf, sizeof(val_buf), "%0.02f cm", ((float) data->uso[i]) / 10);
        widget.data_value_size = wrtn - 1;
        if (cury + 5 < ymax) {
            wmove(win, cury, 0);
            draw_data_widget(win, &widget);
            cury = cury + 5;
        }
    }

}

static void *sub_menu_sensor_handler(void *param) {

    /*
     * handlers will be called with a pointer to mainmenu,
     * and the mainmenu->event will be populated and the
     * handler can respond to the event and draw the sub menu,
     * but never refresh the window.
     * */

    assert(param);
    struct MainMenu *mainmenu = (struct MainMenu *) param;
    const struct SubMenu *submenu = mainmenu->first_sub_menu + mainmenu->sel_sub_menu_idx;
    int i;

    int sens_cmd = sensor_sub_cmd_lut[mainmenu->sel_sub_menu_idx][0];
    int sens_count = sensor_sub_cmd_lut[mainmenu->sel_sub_menu_idx][1];
    int *data;

    switch (mainmenu->sel_sub_menu_idx) {
        case SENS_SUB_MENU_TEMPERATURE:
            data = ((struct TempData *)submenu->data)->dht;
        case SENS_SUB_MENU_AIR_MOISTURE:
            data = ((struct HumeData *)submenu->data)->dht;
        case SENS_SUB_MENU_LIGHT_SENSE:
            data = ((struct LdrData *)submenu->data)->ldr;
        case SENS_SUB_MENU_SOIL_MOISTURE:
            data = ((struct SmsData *)submenu->data)->sms;
        /*case SENS_SUB_MENU_RESERVOIR_LEVEL:*/
        /*    data = ((struct UsoData *)submenu->data)->uso;*/
        /*    break;*/
    }

    if (mainmenu->emcu_connected) {
        tcp_send_packet(sens_cmd, 0);
        for (i = 0; i < sens_count; i++) {
            *(data + i) = (int)tcp_recieve_uint32_t();
        }
    }

    sensor_sub_menu_draw_lut[mainmenu->sel_sub_menu_idx](MainWin, submenu);
    switch(mainmenu->event) {
        case KEY_ESCAPE:
            break;
    }

    return NULL;
}

static void fill_radio_button_widget_config(struct RadioButtonWidget *widget) {

    if (!widget) {
        return;
    }

    widget->width = 5;
    widget->height = 7;
    widget->xopad = 1;
    widget->xipad = 3;
    widget->yopad = 0;
    widget->yipad = 1;

    widget->text = RADIO_BUTTON_WIDGET_DEFAULT_TEXT;
    widget->text_size = sizeof(RADIO_BUTTON_WIDGET_DEFAULT_TEXT) - 1;
    widget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_TEXT_NORMAL];

    widget->first_state_text = RADIO_BUTTON_WIDGET_DEFAULT_FIRST_STATE;
    widget->first_state_size = sizeof(RADIO_BUTTON_WIDGET_DEFAULT_FIRST_STATE) - 1;
    widget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_ACTIVE_NORMAL];

    widget->second_state_text = RADIO_BUTTON_WIDGET_DEFAULT_SECOND_STATE;
    widget->second_state_size = sizeof(RADIO_BUTTON_WIDGET_DEFAULT_SECOND_STATE) - 1;
    widget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_NORMAL];

    widget->third_state_text = RADIO_BUTTON_WIDGET_DEFAULT_THIRD_STATE;
    widget->third_state_size = sizeof(RADIO_BUTTON_WIDGET_DEFAULT_THIRD_STATE) - 1;
    widget->third_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_NORMAL];

    /*widget->data_key = DATA_WIDGET_DEFAULT_KEY;*/
    /*widget->data_key_size = sizeof(DATA_WIDGET_DEFAULT_KEY) - 1;*/
    /*widget->data_value = DATA_WIDGET_DEFAULT_VALUE;*/
    /*widget->data_value_size = sizeof(DATA_WIDGET_DEFAULT_VALUE) - 1;*/

    widget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_FRAME_NORMAL];

    /*widget->data_key_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_DATA_WIDGET_KEY_NORMAL];*/
    /*widget->data_value_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_DATA_WIDGET_VALUE_NORMAL];*/


}

static void draw_radio_button_widget(WINDOW *win, void *widget_data) {

    struct RadioButtonWidget *widget = (struct RadioButtonWidget *) widget_data;
    int i;
    int cury, curx;
    int oriy, orix;
    getyx(win, cury, curx);
    oriy = cury;
    orix = curx;

    cury = cury + widget->yopad;
    curx = curx + widget->xopad;

    int frame_width = widget->width - (2 * (widget->xopad));
    int frame_height = widget->height - (2 * (widget->yopad));

    const char *dot_char;

	wattron(win, widget->frame_scheme);

    /*
     * top line
     * */
    wmove(win, cury, curx);
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_TOPLEFT]);
    curx++;
    for (i = 0; i < frame_width - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_MINUS]);
        curx++;
    }
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_TOPRIGHT]);

    /*
     * bot line
     * */
    cury = oriy + widget->height - widget->yopad - 1;
    curx = orix + widget->xopad;
    wmove(win, cury, curx);
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_BOTLEFT]);
    curx++;
    for (i = 0; i < frame_width - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_MINUS]);
        curx++;
    }
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_BOTRIGHT]);

    /*
     * left side
     * */
    cury = oriy + widget->yopad + 1;
    curx = orix + widget->xopad;
    for (i = 0; i < frame_height - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_PIPE]);
        cury++;
    }

    /*
     * right side
     * */
    cury = oriy + widget->yopad + 1;
    curx = orix + widget->xopad + frame_width - 1;
    for (i = 0; i < frame_height - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_PIPE]);
        cury++;
    }

	wattroff(win, widget->frame_scheme);

    wattron(win, widget->text_scheme);

    cury = oriy + widget->yopad + 1 + widget->yipad;
    curx = orix + widget->xopad + 1 + widget->xipad;
    wmove(win, cury, curx);
    wprintw(win, "%s", widget->text);

    wattroff(win, widget->text_scheme);

    /*
     * first state
     * */

    /*cury = oriy + frame_height - 2 - widget->yipad;*/
    /*curx = orix + frame_width - widget->xipad - widget->text_size;*/

    wattron(win, widget->first_state_scheme);

    if (widget->mode == EXHAUST_MODE_CMODE) {
        dot_char = char_symbols[CHAR_FISHEYE];
    } else {
        dot_char = char_symbols[CHAR_CIRCLE];
    }

    cury = oriy + frame_height - 2 - widget->yipad;
    curx = orix + widget->xopad + 1 + widget->xipad;
    wmove(win, cury, curx);
    wprintw(win, "%s %s", dot_char, widget->first_state_text);

    wattroff(win, widget->first_state_scheme);

    /*
     * second state
     * */

    wattron(win, widget->second_state_scheme);

    if (widget->mode == EXHAUST_MODE_HMODE) {
        dot_char = char_symbols[CHAR_FISHEYE];
    } else {
        dot_char = char_symbols[CHAR_CIRCLE];
    }

    cury = oriy + frame_height - 2 - widget->yipad;
    curx = orix + (frame_width - widget->second_state_size) / 2 + 1;
    /*curx = orix + frame_width - widget->xipad - widget->third_state_size - 2;*/
    wmove(win, cury, curx);
    wprintw(win, "%s %s", dot_char, widget->second_state_text);

    wattroff(win, widget->second_state_scheme);

    /*
     * third state
     * */

    wattron(win, widget->third_state_scheme);

    if (widget->mode == EXHAUST_MODE_NONE) {
        dot_char = char_symbols[CHAR_FISHEYE];
    } else {
        dot_char = char_symbols[CHAR_CIRCLE];
    }

    cury = oriy + frame_height - 2 - widget->yipad;
    curx = orix + frame_width - widget->xipad - widget->third_state_size - 2;
    wmove(win, cury, curx);
    wprintw(win, "%s %s", dot_char, widget->third_state_text);

    wattroff(win, widget->third_state_scheme);

}

static void draw_switch_widget(WINDOW *win, void *widget_data) {

    struct SwitchWidget *widget = (struct SwitchWidget *) widget_data;
    const char *dot_char;
    int i;
    int cury, curx;
    int oriy, orix;
    getyx(win, cury, curx);
    oriy = cury;
    orix = curx;

    cury = cury + widget->yopad;
    curx = curx + widget->xopad;

    int frame_width = widget->width - (2 * (widget->xopad));
    int frame_height = widget->height - (2 * (widget->yopad));

	wattron(win, widget->frame_scheme);

    /*
     * top line
     * */
    wmove(win, cury, curx);
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_TOPLEFT]);
    curx++;
    for (i = 0; i < frame_width - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_MINUS]);
        curx++;
    }
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_TOPRIGHT]);

    /*
     * bot line
     * */
    cury = oriy + widget->height - widget->yopad - 1;
    curx = orix + widget->xopad;
    wmove(win, cury, curx);
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_BOTLEFT]);
    curx++;
    for (i = 0; i < frame_width - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_MINUS]);
        curx++;
    }
    wprintw(win, "%s", char_symbols[CHAR_CORNER_ROUNDED_BOTRIGHT]);

    /*
     * left side
     * */
    cury = oriy + widget->yopad + 1;
    curx = orix + widget->xopad;
    for (i = 0; i < frame_height - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_PIPE]);
        cury++;
    }

    /*
     * right side
     * */
    cury = oriy + widget->yopad + 1;
    curx = orix + widget->xopad + frame_width - 1;
    for (i = 0; i < frame_height - 2; i++) {
        wmove(win, cury, curx);
        wprintw(win, "%s", char_symbols[CHAR_PIPE]);
        cury++;
    }

	wattroff(win, widget->frame_scheme);

    wattron(win, widget->text_scheme);

    cury = oriy + widget->yopad + 1 + widget->yipad;
    curx = orix + widget->xopad + 1 + widget->xipad;
    wmove(win, cury, curx);
    wprintw(win, "%s", widget->text);

    wattroff(win, widget->text_scheme);

    /*
     * first state
     * */

    wattron(win, widget->first_state_scheme);

    if (widget->state == BISTABLE_STATE_OFF) {
        dot_char = char_symbols[CHAR_FISHEYE];
    } else {
        dot_char = char_symbols[CHAR_CIRCLE];
    }

    cury = oriy + frame_height - 2 - widget->yipad;
    curx = orix + (frame_width - widget->first_state_size) / 2 + 1;
    wmove(win, cury, curx);
    wprintw(win, "%s %s", dot_char, widget->first_state_text);

    wattroff(win, widget->first_state_scheme);

    /*
     * second state
     * */

    wattron(win, widget->second_state_scheme);

    if (widget->state == BISTABLE_STATE_ON) {
        dot_char = char_symbols[CHAR_FISHEYE];
    } else {
        dot_char = char_symbols[CHAR_CIRCLE];
    }

    cury = oriy + frame_height - 2 - widget->yipad;
    curx = orix + frame_width - widget->xipad - widget->second_state_size - 2;
    wmove(win, cury, curx);
    wprintw(win, "%s %s", dot_char, widget->second_state_text);

    wattroff(win, widget->second_state_scheme);

}

static void draw_sub_menu_exhaust(WINDOW *win, const struct SubMenu *submenu) {

    struct ExhaustSubMenuState *exmenu = &exhaust_sub_menu_state;
    struct ExhaustWidget *widget;

    struct RadioButtonWidget *rwidget;
    struct SwitchWidget *swidget;

    int ymax, xmax;
    int cury, curx;
    int i;

    int sel_widget_height = 5;
    void *widget_data;

    widget = exmenu->first_widget + exmenu->sel_widget_idx;

    getmaxyx(win, ymax, xmax);
    cury = curx = 0;

    widget_data = widget->widget;

    switch (widget->type) {
        case EXHAUST_WIDGET_TYPE_PANEL:
        case EXHAUST_WIDGET_TYPE_FAN:
        case EXHAUST_WIDGET_TYPE_PELTIER:
            ((struct SwitchWidget *) widget_data)->width = xmax;
            sel_widget_height = ((struct SwitchWidget *)widget_data)->height;
            break;
        case EXHAUST_WIDGET_TYPE_EXHAUST_MODE:
            sel_widget_height = ((struct RadioButtonWidget *)widget_data)->height;
            ((struct RadioButtonWidget *) widget_data)->width = xmax;
            break;
    }

    werase(win);

    /*
     * if the screen has been resized after the last
     * draw we need to adjust the position of currently
     * selected widget
     * */

    if (exmenu->cury + sel_widget_height > ymax) {
        exmenu->cury = ymax - sel_widget_height;
    }
    if (exmenu->cury < 0) {
        exmenu->cury = 0;
    }

    int widgets_above = exmenu->sel_widget_idx;
    int widget_draw_start = widgets_above;

    int lines_left_above = exmenu->cury;
    int widget_height;
    for (i = 0; i < widgets_above; i++) {
        /*
         * assuming the first widget is always the radio button
         * then the switches...
         * TODO: Stop Assuming!!
         * */
        if (i == widgets_above - 1) {
            /* this is radio button at the top */
            widget_height = EXHAUST_RADIO_MODE_HEIGHT;
        } else {
            widget_height = SWITCH_WIDGET_HEIGHT;
        }

        if ((lines_left_above - widget_height) >= 0) {
            lines_left_above = lines_left_above - widget_height;
            widget_draw_start--;
        } else {
            break;
        }
    }

    if (lines_left_above > 0) {
        /* if there is lines left above then push everything above */
        exmenu->cury = exmenu->cury - lines_left_above;
    }

    int widgets_below = exmenu->total_widgets - exmenu->sel_widget_idx - 1;
    int widget_draw_end = exmenu->sel_widget_idx + 1;

    int lines_left_below = ymax - exmenu->cury - sel_widget_height;

    for (i = 0; i < widgets_below; i++) {
        /*
         * Assuming everything below will be switch widget
         * TODO: Again, Stop Assuming!!
         * */
        widget_height = SWITCH_WIDGET_HEIGHT;
        if (lines_left_below - widget_height >= 0) {
            lines_left_below = lines_left_below - widget_height;
            widget_draw_end++;
        } else {
            break;
        }
    }

    int cury_offset;
    for (i = widget_draw_start; i < exmenu->sel_widget_idx; i++) {
        wmove(win, cury, curx);
        widget = exmenu->first_widget + i;
        widget_data = widget->widget;

        if (widget->type == EXHAUST_WIDGET_TYPE_EXHAUST_MODE) {
            rwidget = (struct RadioButtonWidget *) widget_data;

            rwidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_FRAME_NORMAL];
            rwidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_TEXT_NORMAL];

            /* filling inactive selected for all */
            rwidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_NORMAL];
            rwidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_NORMAL];
            rwidget->third_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_NORMAL];

            rwidget->width = xmax;
            rwidget->mode = *((enum ExhaustMode *) widget->state);

            /* filling active one */
            switch (rwidget->mode) {
                case EXHAUST_MODE_CMODE:
                    rwidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_ACTIVE_NORMAL];
                    break;
                case EXHAUST_MODE_HMODE:
                    rwidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_ACTIVE_NORMAL];
                    break;
                default:
                    rwidget->third_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_ACTIVE_NORMAL];
                    break;
            }

            draw_radio_button_widget(win, widget_data);
            cury_offset = EXHAUST_RADIO_MODE_HEIGHT;
        } else {
            swidget = (struct SwitchWidget *) widget_data;

            if (widget->type == EXHAUST_WIDGET_TYPE_FAN) {
                snprintf(fan_switch_text + FAN_SWITCH_NUM_IDX, 2, "%d", widget->idx);
            } else if (widget->type == EXHAUST_WIDGET_TYPE_PANEL) {
                snprintf(panel_switch_text + PANEL_SWITCH_NUM_IDX, 2, "%d", widget->idx);
            }

            swidget->width = xmax;
            swidget->state = *((enum BistableState *) widget->state);

            swidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_FRAME_NORMAL];
            swidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_TEXT_NORMAL];

            /* fill with inactive */
            swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];
            swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];

            switch (swidget->state) {
                case BISTABLE_STATE_OFF:
                    swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                    break;
                case BISTABLE_STATE_ON:
                    swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                    break;
                default:
                    /* other wise leave as it is */
                    break;
            }

            draw_switch_widget(win, widget_data);
            cury_offset = SWITCH_WIDGET_HEIGHT;
        }

        cury = cury + cury_offset;
    }

    wmove(win, cury, curx);
    widget = exmenu->first_widget + exmenu->sel_widget_idx;
    widget_data = widget->widget;

    if (widget->type == EXHAUST_WIDGET_TYPE_EXHAUST_MODE) {
        rwidget = (struct RadioButtonWidget *) widget_data;

        rwidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_FRAME_SELECTED];
        rwidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_TEXT_SELECTED];

        /* filling inactive selected for all */
        rwidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_SELECTED];
        rwidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_SELECTED];
        rwidget->third_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_SELECTED];

        rwidget->width = xmax;
        rwidget->mode = *((enum ExhaustMode *) widget->state);

        /* filling active one */
        switch (rwidget->mode) {
            case EXHAUST_MODE_CMODE:
                rwidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_RADIO_BUTTON_ACTIVE_SELECTED];
                break;
            case EXHAUST_MODE_HMODE:
                rwidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_RADIO_BUTTON_ACTIVE_SELECTED];
                break;
            default:
                rwidget->third_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_RADIO_BUTTON_ACTIVE_SELECTED];
                break;
        }

        /* filling focused one */
        switch (widget->efocused) {
            case EXHAUST_MODE_CMODE:
                if (rwidget->mode == EXHAUST_MODE_CMODE) {
                    rwidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_ACTIVE_FOCUSED_SELECTED];
                } else {
                    rwidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_INACTIVE_FOCUSED_SELECTED];
                }
                break;
            case EXHAUST_MODE_HMODE:
                if (rwidget->mode == EXHAUST_MODE_HMODE) {
                    rwidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_ACTIVE_FOCUSED_SELECTED];
                } else {
                    rwidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_INACTIVE_FOCUSED_SELECTED];
                }
                break;
            default:
                if (rwidget->mode != EXHAUST_MODE_CMODE || rwidget->mode != EXHAUST_MODE_HMODE) {
                    rwidget->third_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_ACTIVE_FOCUSED_SELECTED];
                } else {
                    rwidget->third_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_INACTIVE_FOCUSED_SELECTED];
                }
                break;
        }

        draw_radio_button_widget(win, widget_data);
        cury_offset = EXHAUST_RADIO_MODE_HEIGHT;
    } else {
        swidget = (struct SwitchWidget *) widget_data;

        if (widget->type == EXHAUST_WIDGET_TYPE_FAN) {
            snprintf(fan_switch_text + FAN_SWITCH_NUM_IDX, 2, "%d", widget->idx);
        } else if (widget->type == EXHAUST_WIDGET_TYPE_PANEL) {
            snprintf(panel_switch_text + PANEL_SWITCH_NUM_IDX, 2, "%d", widget->idx);
        }

        swidget->width = xmax;
        swidget->state = *((enum BistableState *) widget->state);

        swidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_FRAME_SELECTED];
        swidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_TEXT_SELECTED];

        /* fill with inactive */
        swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_SELECTED];
        swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_SELECTED];

        /* filling active one */
        switch (swidget->state) {
            case BISTABLE_STATE_OFF:
                swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_SELECTED];
                break;
            case BISTABLE_STATE_ON:
                swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_SELECTED];
                break;
            default:
                /* other wise leave as it is */
                break;
        }

        /* filling focused one */
        switch (widget->bfocused) {
            case BISTABLE_STATE_OFF:
                if (swidget->state == BISTABLE_STATE_OFF) {
                    swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_SWITCH_ACTIVE_FOCUSED_SELECTED];
                } else {
                    swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_SWITCH_INACTIVE_FOCUSED_SELECTED];
                }
                break;
            case BISTABLE_STATE_ON:
                if (swidget->state == BISTABLE_STATE_ON) {
                    swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_SWITCH_ACTIVE_FOCUSED_SELECTED];
                } else {
                    swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_SWITCH_INACTIVE_FOCUSED_SELECTED];
                }
                break;
            default:
                /* other wise leave as it is */
                break;
        }

        draw_switch_widget(win, widget_data);
        cury_offset = SWITCH_WIDGET_HEIGHT;
    }

    cury = cury + cury_offset;

    for (i = exmenu->sel_widget_idx + 1; i < widget_draw_end; i++) {
        wmove(win, cury, curx);
        widget = exmenu->first_widget + i;
        widget_data = widget->widget;
        if (widget->type == EXHAUST_WIDGET_TYPE_EXHAUST_MODE) {
            /* This would never happen */
            rwidget = (struct RadioButtonWidget *) widget_data;

            rwidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_FRAME_NORMAL];
            rwidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_TEXT_NORMAL];

            /* filling inactive selected for all */
            rwidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_NORMAL];
            rwidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_NORMAL];
            rwidget->third_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_RADIO_BUTTON_INACTIVE_NORMAL];

            rwidget->width = xmax;
            rwidget->mode = *((enum ExhaustMode *) widget->state);

            /* filling active one */
            switch (rwidget->mode) {
                case EXHAUST_MODE_CMODE:
                    rwidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_ACTIVE_NORMAL];
                    break;
                case EXHAUST_MODE_HMODE:
                    rwidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_ACTIVE_NORMAL];
                    break;
                default:
                    rwidget->third_state_scheme = color_schemes[SCHEME_DEFAULT]
                        [ELEMENT_RADIO_BUTTON_ACTIVE_NORMAL];
                    break;
            }

            draw_radio_button_widget(win, widget_data);
            cury_offset = EXHAUST_RADIO_MODE_HEIGHT;
        } else {
            swidget = (struct SwitchWidget *) widget_data;

            if (widget->type == EXHAUST_WIDGET_TYPE_FAN) {
                snprintf(fan_switch_text + FAN_SWITCH_NUM_IDX, 2, "%d", widget->idx);
            } else if (widget->type == EXHAUST_WIDGET_TYPE_PANEL) {
                snprintf(panel_switch_text + PANEL_SWITCH_NUM_IDX, 2, "%d", widget->idx);
            }

            swidget->width = xmax;
            swidget->state = *((enum BistableState *) widget->state);

            swidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_FRAME_NORMAL];
            swidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_TEXT_NORMAL];

            /* fill with inactive */
            swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];
            swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];

            switch (swidget->state) {
                case BISTABLE_STATE_OFF:
                    swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                    break;
                case BISTABLE_STATE_ON:
                    swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                    break;
                default:
                    /* other wise leave as it is */
                    break;
            }

            draw_switch_widget(win, widget_data);
            cury_offset = SWITCH_WIDGET_HEIGHT;
        }
        cury = cury + cury_offset;
    }

}

static void *sub_menu_exhaust_handler(void *param) {

    assert(param);
    struct MainMenu *mainmenu = (struct MainMenu *) param;
    const struct SubMenu *submenu = mainmenu->first_sub_menu + mainmenu->sel_sub_menu_idx;
    struct ExhaustSubMenuState *exmenu = &exhaust_sub_menu_state;

    struct ExhaustWidget *widget;

    int widget_height;

    widget = exmenu->first_widget + exmenu->sel_widget_idx;

    switch(mainmenu->event) {
        case KEY_UP:
        case 'k':
            if (widget->type == EXHAUST_WIDGET_TYPE_EXHAUST_MODE) {
                widget_height = EXHAUST_RADIO_MODE_HEIGHT;
            } else {
                widget_height = SWITCH_WIDGET_HEIGHT;
            }
            if (exmenu->sel_widget_idx > 0) {
                exmenu->cury = exmenu->cury - widget_height;
                /*assert((mainmenu.cury >= 0));*/
                exmenu->sel_widget_idx--;
            }
            break;

        case KEY_DOWN:
        case 'j':
            if (widget->type == EXHAUST_WIDGET_TYPE_EXHAUST_MODE) {
                widget_height = EXHAUST_RADIO_MODE_HEIGHT;
            } else {
                widget_height = SWITCH_WIDGET_HEIGHT;
            }
            if (exmenu->sel_widget_idx < exmenu->total_widgets - 1) {
                exmenu->cury = exmenu->cury + widget_height;
                /*assert((mainmenu.cury >= 0));*/
                exmenu->sel_widget_idx++;
            }
            break;

        case KEY_RIGHT:
        case 'l':
            if (widget->type == EXHAUST_WIDGET_TYPE_EXHAUST_MODE) {
                if (widget->efocused < EXHAUST_MODE_END - 1) {
                    widget->efocused++;
                }
            } else {
                if (widget->bfocused == BISTABLE_STATE_OFF) {
                    widget->bfocused = BISTABLE_STATE_ON;
                }
            }
            break;

        case KEY_LEFT:
        case 'h':
            if (widget->type == EXHAUST_WIDGET_TYPE_EXHAUST_MODE) {
                if (widget->efocused > 0) {
                    widget->efocused--;
                }
            } else {
                if (widget->bfocused == BISTABLE_STATE_ON) {
                    widget->bfocused = BISTABLE_STATE_OFF;
                }
            }
            break;

        case KEY_ESCAPE:
            break;
    }

    draw_sub_menu_exhaust(MainWin, submenu);

    return NULL;
}

static void draw_sub_menu_humidifier(WINDOW *win, const struct SubMenu *submenu) {

    struct HumidifierSubMenuState *humenu = &humidifier_sub_menu_state;
    struct HumidifierWidget *widget;

    struct SwitchWidget *swidget;

    int ymax, xmax;
    int cury, curx;
    int i;

    int sel_widget_height = 5;
    void *widget_data;

    widget = humenu->first_widget + humenu->sel_widget_idx;

    getmaxyx(win, ymax, xmax);
    cury = curx = 0;

    widget_data = widget->widget;

    sel_widget_height = ((struct SwitchWidget *)widget_data)->height;
    ((struct SwitchWidget *) widget_data)->width = xmax;

    werase(win);

    /*
     * if the screen has been resized after the last
     * draw we need to adjust the position of currently
     * selected widget
     * */

    if (humenu->cury + sel_widget_height > ymax) {
        humenu->cury = ymax - sel_widget_height;
    }
    if (humenu->cury < 0) {
        humenu->cury = 0;
    }

    int widgets_above = humenu->sel_widget_idx;
    int widget_draw_start = widgets_above;

    int lines_left_above = humenu->cury;
    int widget_height;
    for (i = 0; i < widgets_above; i++) {
        widget_height = SWITCH_WIDGET_HEIGHT;

        if ((lines_left_above - widget_height) >= 0) {
            lines_left_above = lines_left_above - widget_height;
            widget_draw_start--;
        } else {
            break;
        }
    }

    if (lines_left_above > 0) {
        /* if there is lines left above then push everything above */
        humenu->cury = humenu->cury - lines_left_above;
    }

    int widgets_below = humenu->total_widgets - humenu->sel_widget_idx - 1;
    int widget_draw_end = humenu->sel_widget_idx + 1;

    int lines_left_below = ymax - humenu->cury - sel_widget_height;

    for (i = 0; i < widgets_below; i++) {
        widget_height = SWITCH_WIDGET_HEIGHT;
        if (lines_left_below - widget_height >= 0) {
            lines_left_below = lines_left_below - widget_height;
            widget_draw_end++;
        } else {
            break;
        }
    }

    int cury_offset;
    for (i = widget_draw_start; i < humenu->sel_widget_idx; i++) {
        wmove(win, cury, curx);
        widget = humenu->first_widget + i;
        widget_data = widget->widget;

        swidget = (struct SwitchWidget *) widget_data;

        snprintf(humidifier_switch_text + HUMIDIFIER_SWITCH_NUM_IDX, 2, "%d", widget->idx);

        swidget->width = xmax;
        swidget->state = *((enum BistableState *) widget->state);

        swidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_FRAME_NORMAL];
        swidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_TEXT_NORMAL];

        /* fill with inactive */
        swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];
        swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];

        switch (swidget->state) {
            case BISTABLE_STATE_OFF:
                swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                break;
            case BISTABLE_STATE_ON:
                swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                break;
            default:
                /* other wise leave as it is */
                break;
        }

        draw_switch_widget(win, widget_data);
        cury_offset = SWITCH_WIDGET_HEIGHT;
        cury = cury + cury_offset;
    }

    wmove(win, cury, curx);
    widget = humenu->first_widget + humenu->sel_widget_idx;
    widget_data = widget->widget;

    swidget = (struct SwitchWidget *) widget_data;

    snprintf(humidifier_switch_text + HUMIDIFIER_SWITCH_NUM_IDX, 2, "%d", widget->idx);

    swidget->width = xmax;
    swidget->state = *((enum BistableState *) widget->state);

    swidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_FRAME_SELECTED];
    swidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_TEXT_SELECTED];

    /* fill with inactive */
    swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_SELECTED];
    swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_SELECTED];

    /* filling active one */
    switch (swidget->state) {
        case BISTABLE_STATE_OFF:
            swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_SELECTED];
            break;
        case BISTABLE_STATE_ON:
            swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_SELECTED];
            break;
        default:
            /* other wise leave as it is */
            break;
    }

    /* filling focused one */
    switch (widget->bfocused) {
        case BISTABLE_STATE_OFF:
            if (swidget->state == BISTABLE_STATE_OFF) {
                swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_SWITCH_ACTIVE_FOCUSED_SELECTED];
            } else {
                swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_SWITCH_INACTIVE_FOCUSED_SELECTED];
            }
            break;
        case BISTABLE_STATE_ON:
            if (swidget->state == BISTABLE_STATE_ON) {
                swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_SWITCH_ACTIVE_FOCUSED_SELECTED];
            } else {
                swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_SWITCH_INACTIVE_FOCUSED_SELECTED];
            }
            break;
        default:
            /* other wise leave as it is */
            break;
    }

    draw_switch_widget(win, widget_data);
    cury_offset = SWITCH_WIDGET_HEIGHT;

    cury = cury + cury_offset;

    for (i = humenu->sel_widget_idx + 1; i < widget_draw_end; i++) {
        wmove(win, cury, curx);
        widget = humenu->first_widget + i;
        widget_data = widget->widget;

        swidget = (struct SwitchWidget *) widget_data;

        snprintf(humidifier_switch_text + HUMIDIFIER_SWITCH_NUM_IDX, 2, "%d", widget->idx);

        swidget->width = xmax;
        swidget->state = *((enum BistableState *) widget->state);

        swidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_FRAME_NORMAL];
        swidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_TEXT_NORMAL];

        /* fill with inactive */
        swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];
        swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];

        switch (swidget->state) {
            case BISTABLE_STATE_OFF:
                swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                break;
            case BISTABLE_STATE_ON:
                swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                break;
            default:
                /* other wise leave as it is */
                break;
        }

        draw_switch_widget(win, widget_data);

        cury_offset = SWITCH_WIDGET_HEIGHT;
        cury = cury + cury_offset;
    }

}

static void *sub_menu_humidifier_handler(void *param) {

    assert(param);
    struct MainMenu *mainmenu = (struct MainMenu *) param;
    const struct SubMenu *submenu = mainmenu->first_sub_menu + mainmenu->sel_sub_menu_idx;
    struct HumidifierSubMenuState *humenu = &humidifier_sub_menu_state;

    struct HumidifierWidget *widget;

    int widget_height;

    widget = humenu->first_widget + humenu->sel_widget_idx;

    switch(mainmenu->event) {
        case KEY_RIGHT:
        case 'l':
            if (widget->bfocused == BISTABLE_STATE_OFF) {
                widget->bfocused = BISTABLE_STATE_ON;
            }
            break;

        case KEY_LEFT:
        case 'h':
            if (widget->bfocused == BISTABLE_STATE_ON) {
                widget->bfocused = BISTABLE_STATE_OFF;
            }
            break;

        case KEY_ESCAPE:
            break;
    }

    draw_sub_menu_humidifier(MainWin, submenu);

    return NULL;
}

static void draw_sub_menu_lighting(WINDOW *win, const struct SubMenu *submenu) {

    struct LightingSubMenuState *limenu = &lighting_sub_menu_state;
    struct LightingWidget *widget;

    struct SwitchWidget *swidget;

    int ymax, xmax;
    int cury, curx;
    int i;

    int sel_widget_height = 5;
    void *widget_data;

    widget = limenu->first_widget + limenu->sel_widget_idx;

    getmaxyx(win, ymax, xmax);
    cury = curx = 0;

    widget_data = widget->widget;

    sel_widget_height = ((struct SwitchWidget *)widget_data)->height;
    ((struct SwitchWidget *) widget_data)->width = xmax;

    werase(win);

    /*
     * if the screen has been resized after the last
     * draw we need to adjust the position of currently
     * selected widget
     * */

    if (limenu->cury + sel_widget_height > ymax) {
        limenu->cury = ymax - sel_widget_height;
    }
    if (limenu->cury < 0) {
        limenu->cury = 0;
    }

    int widgets_above = limenu->sel_widget_idx;
    int widget_draw_start = widgets_above;

    int lines_left_above = limenu->cury;
    int widget_height;
    for (i = 0; i < widgets_above; i++) {
        widget_height = SWITCH_WIDGET_HEIGHT;

        if ((lines_left_above - widget_height) >= 0) {
            lines_left_above = lines_left_above - widget_height;
            widget_draw_start--;
        } else {
            break;
        }
    }

    if (lines_left_above > 0) {
        /* if there is lines left above then push everything above */
        limenu->cury = limenu->cury - lines_left_above;
    }

    int widgets_below = limenu->total_widgets - limenu->sel_widget_idx - 1;
    int widget_draw_end = limenu->sel_widget_idx + 1;

    int lines_left_below = ymax - limenu->cury - sel_widget_height;

    for (i = 0; i < widgets_below; i++) {
        widget_height = SWITCH_WIDGET_HEIGHT;
        if (lines_left_below - widget_height >= 0) {
            lines_left_below = lines_left_below - widget_height;
            widget_draw_end++;
        } else {
            break;
        }
    }

    int cury_offset;
    for (i = widget_draw_start; i < limenu->sel_widget_idx; i++) {
        wmove(win, cury, curx);
        widget = limenu->first_widget + i;
        widget_data = widget->widget;

        swidget = (struct SwitchWidget *) widget_data;

        snprintf(lighting_switch_text + LIGHTING_SWITCH_NUM_IDX, 2, "%d", widget->idx);

        swidget->width = xmax;
        swidget->state = *((enum BistableState *) widget->state);

        swidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_FRAME_NORMAL];
        swidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_TEXT_NORMAL];

        /* fill with inactive */
        swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];
        swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];

        switch (swidget->state) {
            case BISTABLE_STATE_OFF:
                swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                break;
            case BISTABLE_STATE_ON:
                swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                break;
            default:
                /* other wise leave as it is */
                break;
        }

        draw_switch_widget(win, widget_data);
        cury_offset = SWITCH_WIDGET_HEIGHT;
        cury = cury + cury_offset;
    }

    wmove(win, cury, curx);
    widget = limenu->first_widget + limenu->sel_widget_idx;
    widget_data = widget->widget;

    swidget = (struct SwitchWidget *) widget_data;

    snprintf(lighting_switch_text + LIGHTING_SWITCH_NUM_IDX, 2, "%d", widget->idx);

    swidget->width = xmax;
    swidget->state = *((enum BistableState *) widget->state);

    swidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_FRAME_SELECTED];
    swidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_TEXT_SELECTED];

    /* fill with inactive */
    swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_SELECTED];
    swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_SELECTED];

    /* filling active one */
    switch (swidget->state) {
        case BISTABLE_STATE_OFF:
            swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_SELECTED];
            break;
        case BISTABLE_STATE_ON:
            swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_SELECTED];
            break;
        default:
            /* other wise leave as it is */
            break;
    }

    /* filling focused one */
    switch (widget->bfocused) {
        case BISTABLE_STATE_OFF:
            if (swidget->state == BISTABLE_STATE_OFF) {
                swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_SWITCH_ACTIVE_FOCUSED_SELECTED];
            } else {
                swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_SWITCH_INACTIVE_FOCUSED_SELECTED];
            }
            break;
        case BISTABLE_STATE_ON:
            if (swidget->state == BISTABLE_STATE_ON) {
                swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_SWITCH_ACTIVE_FOCUSED_SELECTED];
            } else {
                swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT]
                    [ELEMENT_SWITCH_INACTIVE_FOCUSED_SELECTED];
            }
            break;
        default:
            /* other wise leave as it is */
            break;
    }

    draw_switch_widget(win, widget_data);
    cury_offset = SWITCH_WIDGET_HEIGHT;

    cury = cury + cury_offset;

    for (i = limenu->sel_widget_idx + 1; i < widget_draw_end; i++) {
        wmove(win, cury, curx);
        widget = limenu->first_widget + i;
        widget_data = widget->widget;

        swidget = (struct SwitchWidget *) widget_data;

        snprintf(lighting_switch_text + LIGHTING_SWITCH_NUM_IDX, 2, "%d", widget->idx);

        swidget->width = xmax;
        swidget->state = *((enum BistableState *) widget->state);

        swidget->frame_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_FRAME_NORMAL];
        swidget->text_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_TEXT_NORMAL];

        /* fill with inactive */
        swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];
        swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_INACTIVE_NORMAL];

        switch (swidget->state) {
            case BISTABLE_STATE_OFF:
                swidget->first_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                break;
            case BISTABLE_STATE_ON:
                swidget->second_state_scheme = color_schemes[SCHEME_DEFAULT][ELEMENT_SWITCH_ACTIVE_NORMAL];
                break;
            default:
                /* other wise leave as it is */
                break;
        }

        draw_switch_widget(win, widget_data);

        cury_offset = SWITCH_WIDGET_HEIGHT;
        cury = cury + cury_offset;
    }

}

static void *sub_menu_lighting_handler(void *param) {

    assert(param);
    struct MainMenu *mainmenu = (struct MainMenu *) param;
    const struct SubMenu *submenu = mainmenu->first_sub_menu + mainmenu->sel_sub_menu_idx;
    struct LightingSubMenuState *limenu = &lighting_sub_menu_state;

    struct LightingWidget *widget;

    int widget_height;

    widget = limenu->first_widget + limenu->sel_widget_idx;

    switch(mainmenu->event) {
        case KEY_UP:
        case 'k':
            widget_height = SWITCH_WIDGET_HEIGHT;
            if (limenu->sel_widget_idx > 0) {
                limenu->cury = limenu->cury - widget_height;
                /*assert((mainmenu.cury >= 0));*/
                limenu->sel_widget_idx--;
            }
            break;

        case KEY_DOWN:
        case 'j':
            widget_height = SWITCH_WIDGET_HEIGHT;
            if (limenu->sel_widget_idx < limenu->total_widgets - 1) {
                limenu->cury = limenu->cury + widget_height;
                /*assert((mainmenu.cury >= 0));*/
                limenu->sel_widget_idx++;
            }
            break;

        case KEY_RIGHT:
        case 'l':
            if (widget->bfocused == BISTABLE_STATE_OFF) {
                widget->bfocused = BISTABLE_STATE_ON;
            }
            break;

        case KEY_LEFT:
        case 'h':
            if (widget->bfocused == BISTABLE_STATE_ON) {
                widget->bfocused = BISTABLE_STATE_OFF;
            }
            break;

        case KEY_ESCAPE:
            break;
    }

    draw_sub_menu_lighting(MainWin, submenu);

    return NULL;
}

int draw_start_tui_app(void) {

	int event;
	struct MainMenu mainmenu;
    char *ip_addr;
    int ret;

    mainmenu.sel_sub_menu_idx = 0;
    mainmenu.total_sub_menus = sizeof(sub_menus) / sizeof(struct SubMenu);
    mainmenu.first_sub_menu = sub_menus;
    mainmenu.cury = 0;
    mainmenu.emcu_connected = 0;

    draw_top_win(app_name);
	draw_main_menu(&mainmenu);
    draw_bot_win(&mainmenu);

	wrefresh(TopWin);
	wrefresh(MainWin);
    wrefresh(BotWin);

	while ((event = wgetch(MainWin)) != 'q') {

        switch (event) {
            case KEY_UP:
            case 'k':
                if (mainmenu.sel_sub_menu_idx > 0) {
                    mainmenu.cury = mainmenu.cury - 3;
                    /*assert((mainmenu.cury >= 0));*/
                    mainmenu.sel_sub_menu_idx--;
                }
                break;
            case KEY_DOWN:
            case 'j':
                if (mainmenu.sel_sub_menu_idx < mainmenu.total_sub_menus - 1) {
                    mainmenu.cury = mainmenu.cury + 3;
                    mainmenu.sel_sub_menu_idx++;
                }
                break;
            case 'c':
                if (mainmenu.emcu_connected == 0) {
                    ip_addr = tcp_get_ip_addr_buf();
                    ret = tcp_connect_to_server(ip_addr, PORT, 1);
                    if (ret >= 0) {
                        mainmenu.emcu_connected = 1;
                    }
                }
                break;
            case KEY_RESIZE:
                resize_windows();
                draw_top_win(app_name);
                wrefresh(TopWin);
                break;
            case KEY_RETURN:
                draw_sub_menu(&mainmenu);
                draw_top_win(app_name);
                wrefresh(TopWin);
                break;
        }

		draw_main_menu(&mainmenu);
        wrefresh(MainWin);

        draw_bot_win(&mainmenu);
        wrefresh(BotWin);

	}

    return 0;

}

int draw_init_ncurses(void) {

	initscr();
	clear();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

#ifndef __DEBUG__

	if (has_colors())
		init_colorschemes();

	curs_set(0);

#endif

	create_windows();

	/*
	 * TODO: choose between arrowkeys or lightning speed escape
	 * capture, dumbass !!
	 * */
	keypad(MainWin, TRUE);
	ESCDELAY = 10;

    return 0;
}

int draw_deinit_ncurses(void) {

    endwin();
    return 0;

}
