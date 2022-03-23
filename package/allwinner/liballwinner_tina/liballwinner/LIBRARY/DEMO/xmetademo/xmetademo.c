
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>

#include "cdx_config.h"
#include "log.h"
#include "xmetadataretriever.h"
#include "CdxTypes.h"



//* the main method.
int main(int argc, char** argv)
{
    AddVDPlugin();

	CEDARX_UNUSE(argc);
	CEDARX_UNUSE(argv);
	int ret;

    printf("\n");
    printf("******************************************************************************************\n");
    printf("* This program implements a simple player, you can type commands to control the player.\n");
    printf("* To show what commands supported, type 'help'.\n");
    printf("* Inplemented by Allwinner ALD-AL3 department.\n");
    printf("******************************************************************************************\n");

    if(argc < 2)
    {
	printf("Usage:\n");
	printf("demoretriver filename \n");
	return -1;
    }

    AwRetriever* demoRetriver;
    demoRetriver = AwRetrieverCreate();

    if(NULL == demoRetriver)
    {
	printf("create failed\n");
	return -1;
    }

    ret = AwRetrieverSetDataSource(demoRetriver, argv[1]);
    if(ret < 0)
    {
	printf("set datasource failed\n");
	return -1;
    }
    printf("AwRetrieverSetDataSource end");

	int width;
    AwRetrieverGetMetaData(demoRetriver, METADATA_VIDEO_WIDTH, &width);

    int height;
    AwRetrieverGetMetaData(demoRetriver, METADATA_VIDEO_HEIGHT, &height);

	int duration;
    AwRetrieverGetMetaData(demoRetriver, METADATA_DURATION, &duration);

    printf("get metadata: w(%d), h(%d), duration(%d)\n", width, height, duration);

	VideoFrame* videoFrame = NULL;
    videoFrame = AwRetrieverGetFrameAtTime(demoRetriver, 0);

    (void)videoFrame;
    AwRetrieverDestory(demoRetriver);

    printf("\n");
    printf("******************************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("******************************************************************************************\n");
    printf("\n");

	return 0;
}
