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
    /*element_toggle_button_frame,*/
    /*element_toggle_button_text,*/
    /*element_toggle_button_inactive_state,*/
    /*element_toggle_button_active_state,*/
	ELEMENT_END,
};

int draw_init_ncurses(void);
