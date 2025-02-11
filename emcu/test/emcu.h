enum SubCmd {
    SUB_CMD_UNKNOWN = 0,
    SUB_CMD_HMODE,
    SUB_CMD_CMODE,
    SUB_CMD_MONITOR,
    SUB_CMD_FANS,
    SUB_CMD_PELTIER,
    SUB_CMD_END,
};

#define SUB_CMD_STR_ENABLE   "enable"
#define SUB_CMD_STR_DISABLE  "disable"

#define EXHAUST_HMODE 0
#define EXHAUST_CMODE 1
