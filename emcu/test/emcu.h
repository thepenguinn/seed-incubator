enum SubCmd {
    SUB_CMD_UNKNOWN = 0,
    SUB_CMD_HMODE,
    SUB_CMD_CMODE,
    SUB_CMD_MONITOR,
    SUB_CMD_FANS,
    SUB_CMD_PELTIER,
    SUB_CMD_MUX,
    SUB_CMD_RBD,
    SUB_CMD_END,
};

#define SUB_CMD_STR_ENABLE   "enable"
#define SUB_CMD_STR_DISABLE  "disable"

#define EXHAUST_HMODE 0
#define EXHAUST_CMODE 1

#define MAC_ADDR "1c:69:20:94:53:0c"
/*#define MAC_ADDR "88:79:7e:be:5e:e4"*/
#define PORT     10898
