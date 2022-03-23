#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <aw_softap_intf.h>

int help_msg()
{
    printf("******************************\n");
    printf("softap_down -help/-h: for usage details\n");
    printf("Usage:\n");
    printf("softap_down <\"1\">\n");
    printf("The slectable parameter <1> means to reload firmware or not\n");
    printf("******************************\n");
    return 0;
}

int main(int argc, char **argv)
{

    /*No argruments or only the one who is for help*/
    if((argc > 2) || (argc == 2 && (!strcmp(argv[1],"-help") || !strcmp(argv[1], "-h"))))
    {
	help_msg();
        return 0;
    }

    printf("***************************\n");
    printf("Start to shutdown hostapd!\n");
    printf("***************************\n");
/*no actual use for this demo, just provide the reader the right way for whole process for up and down softap.
DO NOT USE either aw_softap_init or aw_softap_deinit without the other one.
FOR COMPLETE PROCESS for up and dow softap, please refer to the demo: "softap_up_down_longtime_test".
*/
    aw_softap_init();

    if(argv[1] && argv[1][0] != '\0')
    {
        if(!strcmp(argv[1], "1"))
        {
	    printf("argv[1] is 1, reload fw for sta mode\n");
            aw_softap_reload_firmware("STA");
        }
	else
            printf("The other input! Do not reload fw\n");
    }
    else if(argc == 1)
    {
		printf("No argv[1], take default action to reload fw for sta mode\n");
#ifdef BCMDHD
		aw_softap_reload_firmware("STA");  //the default action is to reload fw, keep consistent with the older version.
#endif
    }
    else
    {
	printf("PARAM ERROR!\n");
	help_msg();
	return -1;
    }

    aw_softap_disable();

/*no actual use for this demo, just provide the reader the right way for whole process for up and down softap.
DO NOT USE either aw_softap_init or aw_softap_deinit without the other one.
FOR COMPLETE PROCESS for up and dow softap, please refer to the demo: "softap_up_down_longtime_test".
*/
    aw_softap_deinit();

    printf("***************************\n");
    printf("Shutdown Hostapd successed!\n");
    printf("***************************\n");

    return 0;
}
