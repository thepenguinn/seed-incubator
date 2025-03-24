/*
 * pins for input multiplexer board
 * */
#define MUX_CLK_PIN  GPIO_NUM_33
#define MUX_DATA_PIN GPIO_NUM_25
#define MUX_EN_PIN   GPIO_NUM_26
#define MUX_READ_PIN GPIO_NUM_32

#define MUX_ADDR_LDR_OFFSET  0

#define MUX_ADDR_SMS_OFFSET  3

#define MUX_ADDR_DHT_OFFSET  7

#define MUX_ADDR_USO_OFFSET  9

/*
 * pins for relay control board
 * */

#define RBD_CLK_PIN  GPIO_NUM_27
#define RBD_DATA_PIN GPIO_NUM_14

/*
 * pins for main control board
 * */

#define MAIN_SCLK_PIN GPIO_NUM_23
#define MAIN_DATA_PIN GPIO_NUM_22
#define MAIN_OCLK_PIN GPIO_NUM_21

#define LIGHT_0_PIN  GPIO_NUM_18
#define LIGHT_1_PIN  GPIO_NUM_19

#define HUMIDIFIER_PIN GPIO_NUM_21

#define ENABLE_12V_PIN GPIO_NUM_22
