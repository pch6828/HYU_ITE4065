#pragma once
#include <pthread.h>

template <typename T>
class ConcurrentQueue
{
private:
  struct QueueNode
  {
    T value;
    struct QueueNode *next;
    QueueNode()
    {
      this->next = nullptr;
    }
    QueueNode(T _value)
    {
      value = _value;
      next = nullptr;
    }
  };

  QueueNode *volatile head;
  QueueNode *volatile tail;

public:
  ConcurrentQueue()
  {
    head = tail = new QueueNode();
  }

  void push(T key)
  {
    QueueNode *newNode = new QueueNode(key);
    while (true)
    {
      QueueNode *last = tail;
      QueueNode *next = last->next;
      if (last != tail)
        continue;
      if (next != nullptr)
      {
        __sync_bool_compare_and_swap(&tail, last, next);
        continue;
      }
      if (__sync_bool_compare_and_swap(&last->next, nullptr, newNode))
      {
        __sync_bool_compare_and_swap(&tail, last, newNode);
        return;
      }
    }
  }

  bool pop(T &dest)
  {
    while (true)
    {
      QueueNode *first = head;
      QueueNode *next = first->next;
      QueueNode *last = tail;
      QueueNode *lastnext = last->next;
      if (first != head)
        continue;
      if (last == first)
      {
        if (lastnext == nullptr)
        {
          return false;
        }
        else
        {
          __sync_bool_compare_and_swap(&tail, last, lastnext);
          continue;
        }
      }
      if (nullptr == next)
        continue;
      T result = next->value;
      if (__sync_bool_compare_and_swap(&head, first, next))
      {
        first->next = nullptr;
        dest = result;
        return true;
      }
    }
  }
};