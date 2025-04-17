#include "DestructorQueue.h"

void DestructorQueue::add_to_queue(const std::function<void()>& function)
{
    m_destruction_functions.push_back(function);
}

void DestructorQueue::destroy_and_clear()
{
    for (auto iter = m_destruction_functions.rbegin(); iter != m_destruction_functions.rend();
         ++iter)
        (*iter)();

    m_destruction_functions.clear();
}

DestructorQueue::~DestructorQueue()
{
    destroy_and_clear();
}
