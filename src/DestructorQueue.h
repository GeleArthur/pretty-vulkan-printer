#pragma once

#include <deque>
#include <forward_list>
#include <functional>

class DestructorQueue final
{
  public:
    void add_to_queue(const std::function<void()>& function);
    void destroy_and_clear();
    ~DestructorQueue();

  private:
    std::deque<std::function<void()>> m_destruction_functions;
};
