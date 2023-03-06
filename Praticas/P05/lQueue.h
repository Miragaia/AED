//
// Tomás Oliveira e Silva, AED, November 2021
//
// Generic queue (First In First Out) implementation based on an linked list
//

#ifndef _AED_lQueue_
#define _AED_lQueue_

#include <cassert>

template <typename T>
class lQueue
{
  private:
    int cur_size;
  public:
    lQueue(void)
    {
      assert(n >= 10 && n <= 1000000);
      max_size = n;
      cur_size = 0;
      data = new T[n];
    }
    ~lQueue(void)
    {
      delete[] data;
      max_size = 0;
      cur_size = 0;
      data = nullptr;
    }
    void clear(void)
    {
      cur_size = 0;
    }
    int size(void) const
    {
      return cur_size;
    }
    int is_full(void) const
    {
      return (cur_size == max_size) ? 1 : 0;
    }
    int is_empty(void) const
    {
      return (cur_size == 0) ? 1 : 0;
    }
    void enqueue(T &v)
    {
      assert(cur_size < max_size);
      data[cur_size++] = v;
    }
    T dequeue(void)
    {
      assert(cur_size > 0);
      T v = data[0];
      for(int i = 1;i < cur_size;i++)
        data[i - 1] = data[i];
      cur_size--;
      return v;
    }
    T peek(void)
    {
      assert(cur_size > 0);
      return data[0];
    }
};

#endif

