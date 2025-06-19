//---------------------------------------------------------------------------
#ifndef function_queueH
#define function_queueH
#include <loki/Functor.h>
#include <vector>
#include <loki/Threads.h>
//---------------------------------------------------------------------------
class function_queue : public Loki::ObjectLevelLockable<function_queue>
{
public:
  typedef Loki::Functor<void> f_t;
  typedef std::vector<f_t> fs_t;
private:
  fs_t items;
public:
  void add(const f_t& val);
  bool get(f_t& val);
};


#endif
