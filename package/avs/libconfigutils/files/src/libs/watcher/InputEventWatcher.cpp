#define TAG "InputEventWatcher"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>

#include "watcher/InputEventWatcher.h"


static int get_input_event_classpath(const char *name, char *class_path)
{
    char dirname[] = "/sys/class/input";
    char buf[256];
    int res;
    DIR *dir;
    struct dirent *de;
    int fd = -1;
    int found = 0;

    dir = opendir(dirname);
    if (dir == NULL)
        return -1;

    while((de = readdir(dir))) {
        if (strncmp(de->d_name, "input", strlen("input")) != 0) {
            continue;
        }

        sprintf(class_path, "%s/%s", dirname, de->d_name);
        snprintf(buf, sizeof(buf), "%s/name", class_path);

        fd = open(buf, O_RDONLY);
        if (fd < 0) {
            continue;
        }
        if ((res = read(fd, buf, sizeof(buf))) < 0) {
            close(fd);
            continue;
        }
        buf[res - 1] = '\0';
        if (strcmp(buf, name) == 0) {
            found = 1;
            close(fd);
            break;
        }

        close(fd);
        fd = -1;
    }
    closedir(dir);
    if (found) {
        return 0;
    }else {
        *class_path = '\0';
        return -1;
    }
}

static int get_input_event_fd(const char *name)
{
    char class_path[30];
    int ret = get_input_event_classpath(name,class_path);
    if(ret < 0){
        printf("get %s class path failed!",name);
        return -1;
    }
    int len = strlen(class_path);
    sprintf(class_path, "/dev/input/event%c", class_path[len - 1]);

    //open event
    int fd = open(class_path, O_RDONLY);
    if (fd < 0) {
        printf("can not open device %s !(%s)",class_path,strerror(errno));
        return -1;
    }
    return fd;
}

namespace AW {

std::shared_ptr<InputEventWatcher> InputEventWatcher::create(std::shared_ptr<EventWatcher> watcher,
                                                             const std::string name)
{
    auto inputwatcher = std::shared_ptr<InputEventWatcher>(new InputEventWatcher(watcher, name));
    if(inputwatcher->initialize()) {
        return inputwatcher;
    }
    return nullptr;
}

void InputEventWatcher::release()
{
    m_watcher->deleteIOEventWatcher(m_fd);
    close(m_fd);
}

bool InputEventWatcher::initialize()
{
    m_fd = get_input_event_fd(m_name.data());
    if(m_fd < 0) return false;

    m_watcher->addIOEventWatcher(m_fd, shared_from_this());
}

void InputEventWatcher::onEvent(EventWatcher::tEventType event_type, int event_src, void *priv)
{
    if(event_type == EventWatcher::EVENT_IO) {
        struct input_event event;
        read(event_src, &event, sizeof(event));

        auto iter = m_listeners.find(event.code);
        if(iter == m_listeners.end()) return;

        for(auto listener : iter->second) {
            listener->onInputEvent(event);
        }
    }
}

void InputEventWatcher::addKeyCodeListener(int code, std::shared_ptr<Listener> listener)
{
    auto iter = m_listeners.find(code);
    if(iter == m_listeners.end()){
        m_listeners[code] = {listener};
        printf("KeyCodeListener add here, key code: %d\n", code);
    }else{
        iter->second.insert(listener);
        printf("TODO:: test, add here2\n");
    }
}

void InputEventWatcher::removeKeyCodeListener(int code, std::shared_ptr<Listener> listener)
{
    auto iter = m_listeners.find(code);
    if(iter != m_listeners.end()){
        //printf("iter->second.size()= %d\n", iter->second.size());
        if(iter->second.size() == 1) m_listeners.erase(iter);
        else iter->second.erase(listener);
    }
}

}
