#include "DestructorQueue.h"

void DestructorQueue::add_to_queue(std::function<void()>&& function)
{
    m_destruction_functions.push_back(std::move(function));
}

void DestructorQueue::destroy_and_clear()
{
    for (auto iter = m_destruction_functions.rbegin(); iter != m_destruction_functions.rend(); ++iter)
        (*iter)();

    m_destruction_functions.clear();
}

DestructorQueue::~DestructorQueue()
{
    destroy_and_clear();
}
