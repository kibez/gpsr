// ============================================================================
// ireq_queue.h_ - Мигрировано на RAD Studio 10.3.3
// Интерфейс очереди запросов с приоритетами
// ============================================================================
#ifndef ireq_queueH
#define ireq_queueH

#include <System.hpp>
#include <loki/NullType.h>
#include <queue>
#include <time.h>
#include <algorithm>
#include <list>
#include <loki/Threads.h>
#include <map>


//#include <itree>
typedef std::map<int,int,std::less<int> > itree;
//

template<class Req>
class IReqQueue :
  public Loki::ClassLevelLockable<IReqQueue<Req> >
{
public:
  struct cmp_priority { bool operator()(const Req& a, const Req& b) const { return a.priority < b.priority; } };

  struct cmp_id
  {
    int val;
    cmp_id(int _val) : val(_val) {}
    bool operator()(const Req& b) const { return val == b.id; }
  };

  class pqueue : public std::priority_queue<Req, std::vector<Req>, cmp_priority>
  {
  public:
    typedef typename std::vector<Req>::iterator iterator;  // RAD Studio 10.3.3: typename
    std::vector<Req>& gc() { return this->c; }
    void erase(const Req& req)
    {
      iterator i = std::find(this->c.begin(), this->c.end(), req);
      if (i != this->c.end()) this->c.erase(i);
    }
  };

protected:
  pqueue lst;
  itree ids; // для быстрого поиска нового идентификатора
  unsigned int last_id;
public:

  IReqQueue() : last_id(0) {}

  unsigned int get_id()
  {
    unsigned int id = ++last_id;
    while (ids.find(id) != ids.end()) id++;
    return id;
  }

  void add(Req& req)
  {
    Lock lk;

    if (req.id == 0)
    {
      req.id = get_id();
      lst.push(req);
    }
    else
    {
      typename std::vector<Req>::iterator i;  // RAD Studio 10.3.3: typename
      unsigned int j;

      for (i = lst.gc().begin(), j = 0; i != lst.gc().end(); ++i, j++)
        if (i->id == req.id)
        {
          send_execute_seconds(req, execute_seconds(req, j));
          break;
        }
      if (i == lst.gc().end()) { send_no_such_request(req); return; }
    }
  }

  bool empty() { return lst.size() == 0; }  // RAD Studio 10.3.3: Исправлена логика

  bool get(Req& req)
  {
    Lock lk;
    for (typename std::vector<Req>::iterator i = lst.gc().begin(); i != lst.gc().end(); ++i)  // RAD Studio 10.3.3: typename
    {
      if (i->in_air) continue;
      i->in_air = true;
      i->timeout = 0;
      req = *i;
      return true;
    }
    return false;
  }

  void check_timeout()
  {
    Lock lk;
    time_t cur = time(nullptr);  // RAD Studio 10.3.3: nullptr
    for (typename std::vector<Req>::iterator it = lst.gc().begin(); it != lst.gc().end();)  // RAD Studio 10.3.3: typename
    {
      if (!it->in_air || it->timeout > cur) {
        ++it;
      }
      else
      {
        Req r = *it;
        it = lst.gc().erase(it);
        ids.erase(r.id);
        send_execute_timeout(r);
      }
    }
  }

  void remove(int id)
  {
    Lock lk;
    typename std::vector<Req>::iterator it = std::find_if(lst.gc().begin(), lst.gc().end(), cmp_id(id));  // RAD Studio 10.3.3: typename
    if (it != lst.gc().end()) lst.gc().erase(it);
  }

  virtual void send_execute_seconds(const Req& req, unsigned int seconds) { ; }
  virtual void send_execute_timeout(const Req& req) { ; }
  virtual void send_no_such_request(const Req& req) { ; }
  virtual unsigned int execute_seconds(const Req& req, unsigned int pos) = 0;
};

#endif