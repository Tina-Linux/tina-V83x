#include <stdio.h>
#include <stdlib.h>
#include "wifi_state_machine.h"

static int gwifi_machine_state = DISCONNECTED_STATE;

/* set wifi state */
int set_wifi_machine_state(tWIFI_MACHINE_STATE state)
{
	//printf("set_wifi_machine_state 0x%x\n", state);
    gwifi_machine_state = state;
}

/* get wifi state */
tWIFI_MACHINE_STATE get_wifi_machine_state()
{
	//printf("wifi machine state 0x%x\n", gwifi_machine_state);
    return gwifi_machine_state;
}
