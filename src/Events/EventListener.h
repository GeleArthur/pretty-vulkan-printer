#pragma once
#include <functional>

template<typename... Args>
class Event;

template<typename... Args>
class EventListener final
{
public:
    explicit EventListener(std::function<void(Args...)> function);
    ~EventListener();

    EventListener(const EventListener& other) noexcept;
    EventListener(EventListener&& other) noexcept;
    EventListener& operator=(const EventListener& other) noexcept;
    EventListener& operator=(EventListener&& other) noexcept;

    void call_function(Args&&... args);

private:
    template<typename... Args2>
    friend class Event;

    void set_event_internal(Event<Args...>* event);
    void remove_from_event_internal();

    std::function<void(Args...)> m_function;
    Event<Args...>*              m_event{};
};

// #include "Event.h"

template<typename... Args>
EventListener<Args...>::EventListener(std::function<void(Args...)> function)
    : m_function{ function }
{
}

template<typename... Args>
EventListener<Args...>::~EventListener()
{
    if (m_event != nullptr)
    {
        m_event->remove_listener(this);
    }
}

template<typename... Args>
EventListener<Args...>::EventListener(const EventListener& other) noexcept
    : m_function{ other.m_function }
    , m_event{ other.m_event }
{
    m_event->add_listener(this);
}

template<typename... Args>
EventListener<Args...>::EventListener(EventListener&& other) noexcept
    : m_function{ std::move(other.m_function) }
    , m_event{ other.m_event }
{
    if (m_event != nullptr)
    {
        m_event->add_listener(this);
        other.m_event->remove_listener(&other);
    }
    other.m_event = nullptr;
}
template<typename... Args>
EventListener<Args...>& EventListener<Args...>::operator=(const EventListener& other) noexcept
{
    if (m_event != nullptr)
    {
        m_function = other.m_function;
        m_event = other.m_event;
        m_event->add_listener(this);
    }

    return *this;
}

template<typename... Args>
EventListener<Args...>& EventListener<Args...>::operator=(EventListener&& other) noexcept
{
    if (this != &other)
    {
        m_function = std::move(other.m_function);
        m_event = other.m_event;
        if (m_event != nullptr)
        {
            other.m_event->remove_listener(&other);
            other.m_event->add_listener(this);
        }
    }

    return *this;
}

template<typename... Args>
void EventListener<Args...>::call_function(Args&&... args)
{
    m_function(std::forward<Args>(args)...);
}

template<typename... Args>
void EventListener<Args...>::set_event_internal(Event<Args...>* event)
{
    m_event = event;
}

template<typename... Args>
void EventListener<Args...>::remove_from_event_internal()
{
    m_event = nullptr;
}
