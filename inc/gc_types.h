/**
 * Specific types used in guard controller project
 */

enum sk_button {
	BTN_UP,
	BTN_DOWN,
	BTN_MID,
	BTN_LEFT,
	BTN_RIGHT
};

typedef enum sk_button sk_button;

enum states {
	M_PUT_ON_GUEARD,
	M_SENSOR_STAT,
	M_DISPLAY_LOG,
	M_TIME_DATE,
	M_SERVICE
};

typedef enum states states;

// Final State Machine for main menu
static const states FMS_menu[5][3] = {
	[M_PUT_ON_GUEARD][BTN_UP] 	= M_PUT_ON_GUEARD,
	[M_PUT_ON_GUEARD][BTN_DOWN] = M_SENSOR_STAT,
	[M_PUT_ON_GUEARD][BTN_MID] 	= M_PUT_ON_GUEARD,

	[M_SENSOR_STAT][BTN_UP] 	= M_PUT_ON_GUEARD,
	[M_SENSOR_STAT][BTN_DOWN] 	= M_DISPLAY_LOG,
	[M_SENSOR_STAT][BTN_MID] 	= M_SENSOR_STAT,

	[M_DISPLAY_LOG][BTN_UP] 	= M_SENSOR_STAT,
	[M_DISPLAY_LOG][BTN_DOWN]	= M_TIME_DATE,
	[M_DISPLAY_LOG][BTN_MID] 	= M_DISPLAY_LOG,

	[M_TIME_DATE][BTN_UP]		= M_DISPLAY_LOG,
	[M_TIME_DATE][BTN_DOWN]		= M_SERVICE,
	[M_TIME_DATE][BTN_MID] 		= M_TIME_DATE,

	[M_SERVICE][BTN_UP]			= M_TIME_DATE,
	[M_SERVICE][BTN_DOWN]		= M_SERVICE,
	[M_SERVICE][BTN_MID] 		= M_SERVICE
};

enum sensor_status{
	SS_DS, // door sensor
	SS_GS  // gas sensor
	// SS_MS
};

typedef enum sensor_status sensor_status;

static const sensor_status FMS_sensor_status[2][2] = {
	[SS_DS][BTN_UP] 	= SS_DS,
	[SS_DS][BTN_DOWN]	= SS_GS,
	[SS_GS][BTN_UP]		= SS_DS,
	[SS_GS][BTN_DOWN]	= SS_GS
};

enum service_menu_state{
	SM_ADD_PIN,
	SM_REMOVE_PIN,
	SM_CHANGE_SPIN
	// SM_ERASE_LOG_DATA
};

typedef enum service_menu_state service_menu_state;

static const service_menu_state FMS_service_menu[3][2] = {
	[SM_ADD_PIN][BTN_UP] 		= SM_ADD_PIN,
	[SM_ADD_PIN][BTN_DOWN] 		= SM_REMOVE_PIN,
	[SM_REMOVE_PIN][BTN_UP] 	= SM_ADD_PIN,
	[SM_REMOVE_PIN][BTN_DOWN] 	= SM_CHANGE_SPIN,
	[SM_CHANGE_SPIN][BTN_UP] 	= SM_REMOVE_PIN,
	[SM_CHANGE_SPIN][BTN_DOWN] 	= SM_CHANGE_SPIN
};


union pin_t{
	uint32_t pin;
	int8_t num[4];
};

typedef union pin_t pin_t;

union date_format_t{
	uint32_t date;
	int8_t num[6];
};

typedef union date_format_t date_format_t;


typedef void (*menu_func_t)(void);


enum time_format {
	TF_HOUR_T 	= 0,
	TF_HOUR_U 	= 1,
	TF_MINUTE_T = 3,
	TF_MINUTE_U = 4,
	TF_SECOND_T = 6,
	TF_SECOND_U = 7,
	TF_YEAR_T	= 8,
	TF_YEAR_U	= 9,
	TF_MONTH_T	= 11,
	TF_MONTH_U 	= 12,
	TF_DAY_T	= 14,
	TF_DAY_U	= 15
};
