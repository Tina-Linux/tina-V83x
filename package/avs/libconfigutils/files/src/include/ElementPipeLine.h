#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "Element.h"

class ElementPipeLine : public Element::Listener
{
public:
    ElementPipeLine(std::shared_ptr<EventWatcher> watcher,
                    std::shared_ptr<Executor> executor) :
                        m_watcher{watcher}, m_executor{executor} {};
    virtual ~ElementPipeLine() = default;

    virtual int start();
    virtual int stop();

private:
    void onElementStart(Element *element);
    void onElementFinished(Element *element);

protected:
    void addElement(std::shared_ptr<Element> element);
    void deleteElement(std::shared_ptr<Element> element);

private:


private:
    std::shared_ptr<EventWatcher> m_watcher;
    std::shared_ptr<Executor> m_executor;

    std::atomic<bool> m_active;

    std::list<std::shared_ptr<Element>> m_elemet_list;
};

#endif /*__ELEMENT_H__*/
