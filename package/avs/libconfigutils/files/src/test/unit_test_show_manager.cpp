#include <gtest/gtest.h>

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
// #include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <endian.h>
#include <iostream>

#include <chrono>
#include <mutex>
#include <condition_variable>

#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include <ugpio/ugpio.h>
#include "utils/JsonUtils.h"
#include "watcher/EventWatcher.h"
#include "ledshow/ShowManager.h"

using namespace AW;



int main(int argc, char *argv[]) {

    auto watcher = EventWatcher::create();
    auto manager = ShowManager::create(watcher, nullptr);

    watcher->startWatcher();

    manager->enableShow(Profile::LISTENING, ProfileFlag::REPLACE);
    sleep(5);
    manager->enableShow(Profile::THINKING, ProfileFlag::REPLACE);
    sleep(5);
    manager->enableShow(Profile::SPEAKING, ProfileFlag::REPLACE);
    sleep(5);
    manager->enableShow(Profile::MUTE, ProfileFlag::REPLACE);
    sleep(5);
    manager->enableShow(Profile::IDLE, ProfileFlag::REPLACE);
    while(1);
}
