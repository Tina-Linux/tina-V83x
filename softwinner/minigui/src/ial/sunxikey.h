#ifndef GUI_IAL_SUNXIKEY_H

#define GUI_IAL_SUNXIKEY_H

/*
#define SUNXI_KEY_LEFT  139
#define SUNXI_KEY_RIGHT 217
#define SUNXI_KEY_MODE  115
#define SUNXI_KEY_OK    102
#define SUNXI_KEY_POWER 116
#define SUNXI_KEY_MENU	118
*/
#define KEYCODE_FILE "/etc/sunxi-keyboard.kl"

#define STR_UP		"UP"
#define STR_DOWN	"DOWN"
#define STR_MENU	"MENU"
#define STR_OK		"OK"
#define STR_POWER	"POWER"
#define STR_MODE	"MODE"
#define STR_RESET	"RESET"
#define STR_HOME	"HOME"


#define SUNXI_KEY_CNT 7

typedef struct sunxi_key_t
{
	int keyCode;
	char *name;
	int scancode;
}sunxi_key_t;

static sunxi_key_t key_set[SUNXI_KEY_CNT] = {
	{0, STR_UP,		SCANCODE_SUNXILEFT},
	{0, STR_DOWN,	SCANCODE_SUNXIRIGHT},
	{0, STR_MENU,	SCANCODE_SUNXIMENU},
	{0, STR_OK,		SCANCODE_SUNXIOK},
	{0, STR_POWER,	SCANCODE_SUNXIPOWER},
	{0, STR_MODE,	SCANCODE_SUNXIMODE},
	{0, STR_RESET,	SCANCODE_SUNXIRESET},
    {0, STR_HOME,	SCANCODE_SUNXIRETURN},        
};

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

BOOL    InitSUNXIKeyInput (INPUT* input, const char* mdev, const char* mtype);
void    TermSUNXIKeyInput (void);
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif
