#include<stdio.h>
#include<stdlib.h>   // exit
#include<fcntl.h>    // O_WRONLY
#include<sys/stat.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>

#define MISC_PARTITION_PATH "/dev/by-name/misc"

/*
 * This is to set speed about count each time.
 * need to config time that update need to
 *
 * */
#define UPGRADE_TIME 300000  // 5 min
unsigned each_time = UPGRADE_TIME / 200;

struct progress_list_t {
	char misc_command[32];
	int  percentage;
};

/*
 * This list is a table about progress-percentage
 * if you have anthor progress , you can inset here.
 * percentage = img_size / all_img_size * 100 + last_percentage
 * */
struct progress_list_t progress_list[] = {
	{"upgrade_pre"   , 0},
	{"boot-recovery" , 30},
	{"upgrade_post"  , 70},
	{"upgrade_end"   ,100},
};
#define LIST_COUNT (sizeof(progress_list) /sizeof(struct progress_list_t))

struct bootloader_message {
    char command[32];
    char status[32];
    char version[32];
    //char reserved[1024];
};

/*
 * this function will read misc partition infor
 * */
static int read_misc_command(char *command)
{
	int fd;
	struct bootloader_message message;

	if (command == NULL) {
	    perror("command is NULL.\n");
	    return -1;
	}
	if ((fd = open(MISC_PARTITION_PATH, O_RDONLY | O_CREAT)) < 0) // Open a FIFO with write
	{
	    printf("Open %s Failed \n", MISC_PARTITION_PATH);
	    return -1;
	}
        if (read(fd, &message, sizeof(message)) < 0)  // Write to FIFO
        {
            printf("read %s Failed \n", MISC_PARTITION_PATH);
            close(fd);
	    return -1;
	}
	memcpy(command, &message.command, 32);
	close(fd);
	return 0;
}

int main()
{
    int fd, ret;
    int n, i, j;
    char buf[32];
    char a,b;
    float fraction = 0.0;
    char misc_command[32] = {0};
    char misc_command_last[32] = {0};

    printf("I am %d process.\n", getpid()); // Description process ID

    if((fd = open("/tmp/ota_progress", O_WRONLY | O_CREAT)) < 0) // Open a FIFO with write
    {
        perror("Open FIFO Failed");
        exit(1);
    }

    for(i=1; i<=200; ++i)
    {
	/*read misc infor*/
	ret = read_misc_command(misc_command);
	if (!ret){
		/*If the progress status was change, update percentage from list*/
		if (strncmp(misc_command_last, misc_command, 31)){
			for (j=0; j<LIST_COUNT; j++) {
				if (!strncmp(&progress_list[j].misc_command[0],
							   misc_command, 31)){
					if (i <= progress_list[j].percentage * 2)
						i = progress_list[j].percentage * 2;
				}
			}
			strcpy(misc_command_last, misc_command);
		}
	}
	/*If the count to 200, but status is not upgrade_end. hold to count and wait*/
	if ((i >= 200) && strncmp(misc_command_last, "upgrade_end", 12)){
		i = 199;
	}

	printf("Now the progress status:%s \n", misc_command_last);

        fraction = (float)i/200;

        n=sprintf(buf,"%6.3f",fraction);
        printf("Send message: %s\n", buf);
	lseek(fd, 0, SEEK_SET);
        if(write(fd, buf, n+1) < 0)  // Write to FIFO
        {
            perror("Write FIFO Failed");
            close(fd);
            exit(1);
        }
        usleep(each_time * 1000);
    }

    close(fd);  // Close the FIFO file
    return 0;
}

/*
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/stat.h>
int main()
{
     int fd;
     int len;
     char buf[1024];

     if(mkfifo("/tmp/ota_progress", 0666) < 0 && errno!=EEXIST) // Create a FIFO pipe
         perror("Create FIFO Failed");

     if((fd = open("/tmp/ota_progress", O_RDONLY)) < 0)  // Open FIFO with read
     {
         perror("Open FIFO Failed");
         exit(1);
     }

     while((len = read(fd, buf, 1024)) > 0) // Read FIFO pipe
         printf("Read message: %s\n", buf);

     close(fd);  // Close the FIFO file
     return 0;
}*/
