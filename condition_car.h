// ============================================================================
// condition_car.h_ - ����������� �� RAD Studio 10.3.3
// ������� ������� � ������� ��� ��������� �����������
// ============================================================================
#ifndef condition_carH
#define condition_carH

#include <System.hpp>
#include "car.h"
#include "condition/cond_cache.h"
#include "condition/cond_code.h"
#include "int2type.h"
#include "ud_envir.h"

class icar_condition_base;
typedef std::vector<unsigned char> data_t;

// ��������� �� ������� �������������� �������� ��� ��������� �������
// ������������ �� ����� ������ �������, ��� ���������� ������������ �������
class icar_condition_ctrl
{
public:
  // �������
  typedef cond_cache::trigger trigger;
  // �������
  typedef std::vector<trigger> triggers;

  //������ �������������� �������� ��� ��������� �������
  typedef std::map<int, icar_condition_base*, std::less<int> > conditions_t;
public:
  //������ �������������� �������� ��� ��������� �������
  conditions_t conditions;

  //��������� �������
  //@param env �����, �� ��� ����� ���������� �� ���� ������ ����� (SMS ��� GPRS)
  //@param ud �����, ������� ������ �� ����������
  //@param ress ���������� �������.
  //@return true ���� ���� �� ���� �������������� ������ ������ true
  //����� �������� ����� �� ���������� ������������ ��� �������
  //�� ������� ������� ������������ �� ����� ������ ����������� � ress
  virtual bool parse_condition_packet(const ud_envir& env, const data_t& ud, triggers& ress);

  // ���������� �� ������ ������� ��� ����������� ��������������� ������� �������
  void register_condition(icar_condition_base* val);
  
  // ���������� �� ������ ������� ��� ������������� ��������������� ������� �������
  void unregister_condition(icar_condition_base* val);

  // ���������� �� parse_condition_packet , ����� ������ ���� �������������� ��������
  virtual bool after_conditions_processed(const ud_envir& env, const data_t& ud, triggers& ress) { return false; }
};

// �������������� ������, ������� ���������� ������� ��� ��������� �������
class icar_condition_base
{
public:
  //��������� �������
  //@param env �����, �� ��� ����� ���������� �� ���� ������ ����� (SMS ��� GPRS)
  //@param ud �����, ������� ������ �� ����������
  //@param ress ���������� �������.
  //@return true ���� ���� �� ���� �������������� ������ ������ true
  //����� �������� ����� �� ���������� ������������ ��� �������
  //�� ������� ������� ������������ �� ����� ������ ����������� � ress
  virtual bool iparse_condition_packet(const ud_envir& env, const data_t& ud, std::vector<cond_cache::trigger>& ress) = 0;
  
  //������� ������������� �������
  virtual int get_condition_id() const = 0;
};

// ��������� ���������� �������
//@param _cond_id ������������� �������
template<int _cond_id>
class icar_condition : public icar_condition_base
{
public:
  const int cond_id;

  icar_condition() : cond_id(_cond_id) { ; }
  
private:
  // ������ ������, ����������� ������������� ������� � ������������� �������
  cond_cache::trigger get_condition_trigger()
  {
    cond_cache::trigger tr;
    icar* car = dynamic_cast<icar*>(this);
    if (car) tr.obj_id = car->obj_id;
    tr.cond_id = _cond_id;
    return tr;
  }
  
public:
  // ������ ������, ����������� ������������� ������� � ������������� �������
  // ���� ����� � ����� �������� ��������� �����
  cond_cache::trigger get_condition_trigger(const ud_envir& env)
  {
    cond_cache::trigger tr = get_condition_trigger();
    if (env.timestamp_valid) tr.datetime = env.timestamp;
    return tr;
  }

  // ���������� �� ������ ��������� ������� ��� ����������� �������
  void init()
  {
    icar_condition_ctrl* d = dynamic_cast<icar_condition_ctrl*>(this);
    if (d != nullptr) {  // RAD Studio 10.3.3: �������� ���������
      d->register_condition(this);
    }
  }

  // ���������� ����������
  int get_condition_id() const { return cond_id; }
};

#endif