/*
 * Copyright (C) 2016 The AllWinner Project
 */

#define LOG_TAG "powerkey"

#include <tina_log.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "np_input.h"

int main(int argc, char **argv)
{
	np_input_init();
}
