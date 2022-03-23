#ifndef __EVENT_WATCHER_H__
#define __EVENT_WATCHER_H__

#include <thread>
#include <list>

#include <string>
#include <stdio.h>

#include <ev++.h>

//#define _LIBEV_CPP_
using namespace std;

namespace AW {

class EventWatcher {
public:
    static std::shared_ptr<EventWatcher> create();

    virtual ~EventWatcher();

    typedef enum Eventtype
    {
        EVENT_IO,
        EVENT_TIMER,
    }tEventType;

    class Listener{
    public:
        virtual ~Listener(){};
        virtual void onEvent(tEventType event_type, int event_src, void *priv) = 0;
    };

    int startWatcher();
    int stopWatcher();

    /* return io watcher id */
    int addIOEventWatcher(int fd, std::shared_ptr<Listener> listener, void *priv = nullptr);
    int deleteIOEventWatcher(int fd);

    /* return timer watcher id */
    int addTimerEventWatcher(float timeout, std::shared_ptr<Listener> listener, void *priv = nullptr);

private:
    EventWatcher();
private:
    // libev++
    struct ev_loop *m_main_loop;

    class event
    {
    public:
        event();
        ~event();
    };

    class io_event
    {
    public:
        static std::shared_ptr<io_event> create(struct ev_loop *loop, int fd, std::shared_ptr<Listener> listener, void *priv);
        ~io_event();

        int getIdentifier(){return m_identifier;};
    private:
        io_event(struct ev_loop *loop, int fd, std::shared_ptr<Listener> listener, void *priv);

    private:
        void ev_io_cb(ev::io &w, int revents);

        ev::io m_io;
        std::shared_ptr<Listener> m_listener;

        int m_identifier;
        void *m_priv;
    };

    class timer_event
    {
    public:
        static timer_event* create(struct ev_loop *loop, float timeout_s, std::shared_ptr<Listener> listener, void *priv);
        ~timer_event();

        int getIdentifier(){return m_identifier;};
    private:
        timer_event(struct ev_loop *loop, float timeout_s, std::shared_ptr<Listener> listener, void *priv);

    private:
        void ev_timer_cb(ev::timer &w, int revents);

        ev::timer m_timer;
        std::shared_ptr<Listener> m_listener;

        int m_identifier;
        void *m_priv;
    };

private:
    int add_io_event_watcher(int fd, std::shared_ptr<Listener> listener, void *priv);
    int add_timer_event_watcher(float timeout_us, std::shared_ptr<Listener> listener, void *priv);

    void ev_io_cb(ev::io &w, int revents);
    void sync_loop();
    void destory_loop();
private:
    std::thread m_Thread;
    void thread_loop();
private:
    int m_socket_paris[2];
    ev::io m_ctrl_io;

    std::list<std::shared_ptr<io_event>> m_io_list;


};/*class EventWatcher*/

} /*namespace AW*/

#endif /*__EVENT_WATCHER_H__*/
