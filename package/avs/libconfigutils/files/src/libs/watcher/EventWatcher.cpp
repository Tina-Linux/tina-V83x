

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stddef.h>

#include <ev++.h>

#include <iostream>
#include <atomic>

#include "watcher/EventWatcher.h"

static std::atomic<int> g_timer_id{0};

namespace AW {

std::shared_ptr<EventWatcher> EventWatcher::create()
{
    return std::shared_ptr<EventWatcher>(new EventWatcher());
}

EventWatcher::EventWatcher()
{
    m_main_loop = ev_loop_new (EVBACKEND_EPOLL | EVFLAG_NOENV);
    //main_loop = ev_default_loop(0);

    if (!m_main_loop){
        perror ("no epoll found here, maybe it hides under your chair");
        throw;
    }
    ev_set_userdata(m_main_loop, this);
}

EventWatcher::~EventWatcher()
{
    ev_loop_destroy(m_main_loop); //TODO
}

int EventWatcher::startWatcher()
{
    if( socketpair(AF_UNIX, SOCK_STREAM, 0, m_socket_paris) == -1 ){
        printf("create unnamed socket pair failed:%s\n",strerror(errno) );
        return -1;
    }

    m_ctrl_io.set(m_main_loop);
    m_ctrl_io.set<EventWatcher, &EventWatcher::ev_io_cb>(this);
    m_ctrl_io.start(m_socket_paris[0], ev::READ);

    m_Thread = std::thread(&EventWatcher::thread_loop, this);

    return -1;
}

int EventWatcher::stopWatcher()
{
    if(!m_io_list.empty()) {
        printf("io watcher is no empty, wait!\n");
        return -1;
    }

    destory_loop();

    if (m_Thread.joinable()) {
        m_Thread.join();
    }

    return 0;
}

int EventWatcher::addIOEventWatcher(int fd, std::shared_ptr<Listener> listener, void *priv)
{
    int ret = add_io_event_watcher(fd, listener, priv);
    if(ret < 0) return ret;

    sync_loop();

    return 0;
}

int EventWatcher::deleteIOEventWatcher(int fd)
{
    for (std::list<std::shared_ptr<io_event>>::iterator it = m_io_list.begin(); it != m_io_list.end();)  {
        if((*it)->getIdentifier() == fd){
            m_io_list.erase(it);
            break;
        }
    }
    sync_loop();

    return 0;
}

int EventWatcher::addTimerEventWatcher(float timeout_s, std::shared_ptr<Listener> listener, void *priv)
{
    int ret = add_timer_event_watcher(timeout_s, listener, priv);
    if(ret < 0) return ret;

    sync_loop();
    return 0;
}

void EventWatcher::ev_io_cb(ev::io &w, int revents)
{
    if(w.fd == m_socket_paris[0]){
        char x;
        recv(w.fd, &x, 1, 0);
        if(x == 'x') {
            shutdown(m_socket_paris[0], SHUT_RDWR);
            shutdown(m_socket_paris[1], SHUT_RDWR);
            close(m_socket_paris[0]);
            close(m_socket_paris[1]);
            ev_break (m_main_loop);
            w.stop();
            printf("exit io %c\n", x);
        }
    }
}

void EventWatcher::thread_loop()
{
    ev_run(m_main_loop, 0);
}

int EventWatcher::add_io_event_watcher(int fd, std::shared_ptr<Listener> listener, void *priv)
{
    std::shared_ptr<io_event> io = io_event::create(m_main_loop, fd, listener, priv);
    m_io_list.push_back(io);
    return 0;
}

int EventWatcher::add_timer_event_watcher(float timeout_s, std::shared_ptr<Listener> listener, void *priv)
{
    timer_event *timer = timer_event::create(m_main_loop, timeout_s, listener, priv);

    return 0;
}

void EventWatcher::sync_loop()
{
    if(m_Thread.get_id() == std::this_thread::get_id()) {
        //printf("no need to sync\n");
        return;
    }
    char x = 's';
    send(m_socket_paris[1], &x, 1, 0);
}

void EventWatcher::destory_loop()
{
    char x = 'x';
    send(m_socket_paris[1], &x, 1, 0);
}

/****************************************************************************************/
std::shared_ptr<EventWatcher::io_event> EventWatcher::io_event::create(struct ev_loop *loop, int fd, std::shared_ptr<Listener> listener, void *priv)
{
    return std::shared_ptr<io_event>(new io_event(loop, fd, listener, priv));
}

EventWatcher::io_event::io_event(struct ev_loop *loop, int fd, std::shared_ptr<Listener> listener, void *priv)
{
    m_listener = listener;
    m_identifier = fd;
    m_priv = priv;

    m_io.set(loop);
    m_io.set<io_event, &io_event::ev_io_cb>(this);
    m_io.start(fd, ev::READ);
}

EventWatcher::io_event::~io_event()
{
    //printf("io_event: %d disconstructed!\n", m_identifier);
    m_io.stop();
}

void EventWatcher::io_event::ev_io_cb(ev::io &w, int revents)
{
    if(m_listener != nullptr)
        m_listener->onEvent(EVENT_IO, w.fd, m_priv);
}

EventWatcher::timer_event* EventWatcher::timer_event::create(struct ev_loop *loop, float timeout_s, std::shared_ptr<Listener> listener, void *priv)
{
    return new timer_event(loop, timeout_s, listener, priv);
}

EventWatcher::timer_event::timer_event(struct ev_loop *loop, float timeout_s, std::shared_ptr<Listener> listener, void *priv)
{
    m_listener = listener;
    m_identifier = g_timer_id++;
    m_priv = priv;

    m_timer.set(loop);
    m_timer.set<timer_event, &timer_event::ev_timer_cb>(this);
    m_timer.start(timeout_s, 0);
}

EventWatcher::timer_event::~timer_event()
{
    //printf("timer_event disconstructed!\n");
    m_timer.stop();
}

void EventWatcher::timer_event::ev_timer_cb(ev::timer &w, int revents)
{
    if(m_listener != nullptr)
        m_listener->onEvent(EVENT_TIMER, m_identifier, m_priv);

    delete this;
}

}
