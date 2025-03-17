#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>
#include<stdint.h>
#include<assert.h>

#include "draw.h"

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

static void *sub_menu_temp_handler(void *param);

static void *sub_menu_hume_handler(void *param);

static void *sub_menu_temp_handler(void *param) {
    return NULL;
}

static void *sub_menu_hume_handler(void *param) {
    return NULL;
}

static const struct SubMenu sub_menus[] = {
    {
        .title = "Temperature",
        .info = "Indoor and outdoor temperature",
        .handler = sub_menu_temp_handler,
        .param = NULL,
    },
    {
        .title = "Air Moisture",
        .info = "Indoor and outdoor air moisture",
        .handler = sub_menu_temp_handler,
        .param = NULL,
    },
    {
        .title = "Lighting",
        .info = "LDR data within the incubator",
        .handler = sub_menu_temp_handler,
        .param = NULL,
    },
    {
        .title = "Soil Moisture",
        .info = "Soil moisture at the base of each plant",
        .handler = sub_menu_temp_handler,
        .param = NULL,
    },
    {
        .title = "Reservoir Level",
        .info = "Water and fertilizer level",
        .handler = sub_menu_temp_handler,
        .param = NULL,
    },
    {
        .title = "Exhaust",
        .info = "Exhaust control",
        .handler = sub_menu_temp_handler,
        .param = NULL,
    },
    {
        .title = "Humidifier",
        .info = "Humidifier control",
        .handler = sub_menu_temp_handler,
        .param = NULL,
    },
};

static const char app_name[] = "Seed Incubator";

static const char *char_symbols[CHAR_END] = {
	[CHAR_CORNER_TOPLEFT]   = "┌",
	// [CHAR_CORNER_TOPLEFT]   = "╭",
	[CHAR_CORNER_TOPRIGHT]  = "┐",
	// [CHAR_CORNER_TOPRIGHT]  = "╮",
	[CHAR_CORNER_BOTRIGHT]  = "┘",
	// [CHAR_CORNER_BOTRIGHT]  = "╯",
	[CHAR_CORNER_BOTLEFT]   = "└",
	// [CHAR_CORNER_BOTLEFT]   = "╰",
	[CHAR_EDGE_TOP]         = "┬",
	[CHAR_EDGE_RIGHT]       = "┤",
	[CHAR_EDGE_BOT]         = "┴",
	[CHAR_EDGE_LEFT]        = "├",
	[CHAR_MINUS]            = "─",
	[CHAR_PIPE]             = "│",
	[CHAR_CROSS]            = "┼",
	[CHAR_DOT]              = "•"
};
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
        [ELEMENT_MENU_TITLE]             = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG) | A_BLINK,
        [ELEMENT_SUBMENU_TITLE_NORMAL]   = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG),
        [ELEMENT_SUBMENU_TITLE_SELECTED] = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG),
        [ELEMENT_SUBMENU_INFO_NORMAL]    = ColorPair(PAIR_SEC_WHITE_DEFAULT_BG),
        [ELEMENT_SUBMENU_INFO_SELECTED]  = ColorPair(PAIR_SEC_ACCENT_DEFAULT_BG),
        [ELEMENT_INFO_FRAME]             = ColorPair(PAIR_SEC_WHITE_DEFAULT_BG),
        [ELEMENT_INFO_TEXT]              = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG),
        [ELEMENT_INFO_VALUE]             = ColorPair(PAIR_PRI_WHITE_DEFAULT_BG) | A_BOLD,
        [ELEMENT_DOTS_NORMAL]            = ColorPair(PAIR_TER_WHITE_DEFAULT_BG),
        [ELEMENT_DOTS_SELECTED]          = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG),
        [ELEMENT_EMCU_CONNECTED]         = ColorPair(PAIR_PRI_ACCENT_DEFAULT_BG),
        [ELEMENT_EMCU_NOT_CONNECTED]     = ColorPair(PAIR_SEC_WHITE_DEFAULT_BG),


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
	wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_MENU_TITLE]);
	wprintw(win, "%s", title);

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

void draw_bot_win(struct MainMenu *menu, int emcu_connected) {

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

    if (emcu_connected == 1) {
        wmove(win, 0, xmax - sizeof(EMCU_CONNECTED_TEXT));
        wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_EMCU_CONNECTED]);
        wprintw(win, "%s", EMCU_CONNECTED_TEXT);
    } else {
        wmove(win, 0, xmax - sizeof(EMCU_NOT_CONNECTED_TEXT));
        wattron(win, color_schemes[SCHEME_DEFAULT][ELEMENT_EMCU_NOT_CONNECTED]);
        wprintw(win, "%s", EMCU_NOT_CONNECTED_TEXT);
    }

}

static void start_tui_app(void) {

	int event;
	struct MainMenu mainmenu;

    mainmenu.sel_sub_menu_idx = 0;
    mainmenu.total_sub_menus = sizeof(sub_menus) / sizeof(struct SubMenu);
    mainmenu.first_sub_menu = sub_menus;
    mainmenu.cury = 0;

    draw_top_win(app_name);
	draw_main_menu(&mainmenu);
    draw_bot_win(&mainmenu, 0);

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
            case KEY_RESIZE:
                resize_windows();
                draw_top_win(app_name);
                wrefresh(TopWin);
                break;
        }

		draw_main_menu(&mainmenu);
        wrefresh(MainWin);

        draw_bot_win(&mainmenu, 1);
        wrefresh(BotWin);

	}

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
	/*stretch_window(Mainwin);*/
	/*normal_mode();*/

    /*printf("hai\n");*/

    start_tui_app();

    /*wgetch(MainWin);*/

	endwin();

    printf("hai\n");

    return 0;
}
