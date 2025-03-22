enum SubCmd {
    SUB_CMD_UNKNOWN = 0,
    SUB_CMD_HMODE,
    SUB_CMD_CMODE,
    SUB_CMD_MONITOR,
    SUB_CMD_FANS,
    SUB_CMD_PELTIER,
    SUB_CMD_MUX,
    SUB_CMD_RBD,
    SUB_CMD_MONITOR_TEMP,
    SUB_CMD_MONITOR_HUME,
    SUB_CMD_MONITOR_LDR,
    SUB_CMD_MONITOR_SMS,
    SUB_CMD_MONITOR_USO,
    SUB_CMD_TUI,
    SUB_CMD_END,
};

#define SUB_CMD_STR_ENABLE   "enable"
#define SUB_CMD_STR_DISABLE  "disable"

/* TODO: remove this */
/*#define EXHAUST_HMODE 0*/
/*#define EXHAUST_CMODE 1*/

#define MAC_ADDR "1c:69:20:94:53:0c"
/*#define MAC_ADDR "88:79:7e:be:5e:e4"*/
#define PORT     10898
