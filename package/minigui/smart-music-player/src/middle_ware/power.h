#ifndef _POWER_LIB_H_
#define  _POWER_LIB_H_

typedef enum _POWER_LEVEL_T {
    POWER_LEVEL_0,
    POWER_LEVEL_1,
    POWER_LEVEL_2,
    POWER_LEVEL_3,
    POWER_LEVEL_4,
    POWER_LEVEL_5,
    POWER_LEVEL_
} power_level_t;

typedef enum _POWER_TYPE_T {
    POWER_BAT_ONLY, POWER_CHARGER_LINK, POWER_PC_LINK, POWER_
} power_type_t;

int power_get_battery_level(void);

int power_is_charging(void);

int power_is_low(void);

#endif
