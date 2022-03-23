// #include <stdarg.h>
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

#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include "watcher/EventWatcher.h"
#include "threading/Executor.h"
#include "Element.h"

using namespace AW;

class unit_test_eventwatch : public EventWatcher::Listener
{
public:
    void onEvent(EventWatcher::tEventType event_type, int event_src, void *priv) {
        printf("event coming: %s, index: %d ", (event_type == EventWatcher::EVENT_IO ? "IO":"TIMER"), event_src);
        if(event_type == EventWatcher::EVENT_IO) {
            char buf[100];
            bzero(buf, 100);
            read(event_src, buf, 100);
            printf("read: %s", buf);
        }
        printf("\n");
    }

};

int main(int argc, char *argv[]) {

    auto listener = std::shared_ptr<EventWatcher::Listener>(new unit_test_eventwatch());
    auto watcher = EventWatcher::create();

    watcher->startWatcher();
    watcher->addIOEventWatcher(0, listener);
    watcher->addTimerEventWatcher(1, listener);
    watcher->addTimerEventWatcher(2, listener);
    watcher->addTimerEventWatcher(3, listener);

    sleep(10);
    watcher->deleteIOEventWatcher(0);

    watcher->stopWatcher();

    while(1);
}
