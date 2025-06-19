// ============================================================================
// custom_car.h_ - ����������� �� RAD Studio 10.3.3
// ������� ���������������� ������ ��� ���������
// ============================================================================
#ifndef custom_carH
#define custom_carH

#include <System.hpp>
#include "car.h"
#include "custom_request/custom_request_code.h"
#include "custom_request/custom_data.h"
#include <int2type.h>
#include <type2type.h>
#include "pokr/dbgprn.h"

#include <vector>

class icar_custom_base;

// ��������� �� ������� �������������� �������� ��� ���������� �������
// ������������ �� ����� ������ �������, ��� ���������� ������������ ���������� �������
class icar_custom_ctrl
{
public:
  //���� ���� �� ��������������, � �������� ��������� �������
  typedef std::pair<req_packet, custom_result> cust_value;
  //���������� ��������
  typedef std::vector<cust_value> cust_values;

  //������ �������������� �������� ��� ���������� ��������
  typedef std::map<int, icar_custom_base*, std::less<int> > customs_t;
public:
  //������ �������������� �������� ��� ���������� ��������
  customs_t customs;

  //��� ����������, ������� ����� ��������� ��������, �������� �� ���������
  virtual void custom_reset();

  //��������� ������
  //@param req ������������ ������, �� �������� ���� ��������� ��������
  //@param packet_id ������������� ������, �� ����������, ������� ������������ ������������� ������� �������� ��� � �����
  //@param ud ������, ������� ������ ���������� ���������� �������
  //@return true ���� ������ ��� ������� ��������
  //��������� ���� ������������� ������ ��������������� ���� �������, � �������� ��� ��� ���������� ������� 
  virtual bool build_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud);
  
  //��������� �����
  //@param ud �����, ������� ������ �� ����������
  //@param ress ���������� �������. ������������ ����� ���� ���������
  //@return true ���� ���� �� ���� �������������� ������ ������ true
  //����� �������� ����� �� ���������� ������������ ��� ��������
  //�� ������� ������� ��������� ����� ������ ��������� ��������� � ������� true
  virtual bool parse_custom_packet(const data_t& ud, const ud_envir& env, cust_values& ress);
  
  // true ���� ��������� ����������
  virtual bool support(const custom_request& req);
  
  // ������ rq ��� ���� � ��� �� ������ � air_rq
  // �� ��������� ���������� false
  virtual bool same_in_air(const custom_request& rq, const custom_request& air_rq);
  
  // tru�, ���� �� ������� ��������� �����-�� ���������
  virtual bool is_custom_in(const custom_request& req);

  // ���������� �� ������ ������� ��� ����������� ��������������� ������� �������
  void register_custom(icar_custom_base* val);
  
  // ���������� �� ������ ������� ��� ������������� ��������������� ������� �������
  void unsregister_custom(icar_custom_base* val);

  //true ���� �������� ���� ������������ ���� ���� ����������
  virtual bool is_custom_self_timeout(int cust_id) const;
  
  //true ���� ���� ���� ���������� �������
  //���� false � � timeout ������������ ��������� ����� ���������� ������� � ��������
  virtual bool is_custom_timeout(int cust_id, unsigned int& timeout) const;

  //true, ���� ������ �������������
  virtual bool is_out_multi_packet(int cust_id) const;
  
  //��������� ��������� ����� �������
  //true ���� �������
  //���� ���������� false �� �����
  virtual bool build_custom_next_part(int cust_id, data_t& ud);
};

// �������������� ������ ��� ���������� ����� �������
class icar_custom_base
{
public:
  void init(icar_custom_ctrl* d)
  {
    if (d != nullptr) {  // RAD Studio 10.3.3: �������� ���������
      d->register_custom(this);
    }
  }

  //��� ����������, ������� ����� ��������� ��������,
  //��� �� ����� ��������
  virtual void ireset() {}

  //�������� ������������� ��������
  virtual int get_custom_id() const = 0;
  
  //��������� ������
  //@param req ������������ ������, �� �������� ���� ��������� ��������
  //@param packet_id ������������� ������, �� ����������, ������� ������������ ������������� ������� �������� ��� � �����
  //@param ud ������, ������� ������ ���������� ���������� �������
  //@return true ���� ������ ��� ������� ��������
  virtual bool ibuild_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud) = 0;
  
  //��������� �����
  //@param ud �����, ������� ������ �� ����������
  //@param req ������������ ������, �� �������� ���� ��������� ��������
  //@param packet_id ������������� ������, �� ����������, ������� ������������ ������������� �������,
  // �������������� ��� ���������
  //@param res ��������� ���������� �������
  //@return true ���� ������ ��� ������� ��������
  //����� �������� ����� �� ���������� ������������ ��� ��������
  //�� ������� ������� ��������� ����� ������ ��������� res � ������� true
  virtual bool iparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, custom_result& res) = 0;
  
  // true ���� ��������� ����������
  virtual bool isupport(const custom_request& req) { return true; }
  
  // ������ rq ��� ���� � ��� �� ������ � air_rq
  // �� ��������� ���������� false
  virtual bool isame_in_air(const custom_request& rq, const custom_request& air_rq) { return false; }
  
  // tru�, ���� �� ������� ��������� �����-�� ���������
  virtual bool iis_custom_in(const custom_request& req) { return true; }

  //true ���� �������� ���� ������������ ���� ���� ����������
  virtual bool iis_custom_self_timeout() const { return false; }
  
  //true ���� ���� ���� ���������� �������
  //���� false � � timeout ������������ ��������� ����� ���������� ������� � ��������
  //����� ����� ����� ������ ��� ������� � ����������� �������� ������ ����������
  virtual bool iis_custom_timeout(unsigned int& timeout) const { return true; }

  //true, ���� ������ �������������
  virtual bool iis_out_multi_packet() const { return false; }
  
  //��������� ��������� ����� �������
  //true ���� �������
  //���� ���������� false �� �����
  virtual bool ibuild_custom_next_part(data_t& ud) { return false; }
};

//��������� ����������
//@param cust_id ������������� ��������
template<int cust_id>
class icar_custom : public icar_custom_base
{
public:
  const int req_id;
  icar_custom() : req_id(cust_id) { ; }
  int get_custom_id() const { return req_id; }
};

//��������� ���������� � ����������� ���������
//@param cust_id ������������� ��������
template<int cust_id>
class icar_custom_self_timeout : public virtual icar_custom<cust_id>
{
public:
  //����������� ����� ���������� ������ �� ���������
  static const unsigned int def_packet_execute = 30;
  //����������� ����� ���������� ������
  unsigned int packet_execute;
  //������������ ����� ���������� ������ �� ���������
  static const unsigned int def_max_packet_execute = 300;
  //������������ ����� ���������� ������
  unsigned int max_packet_execute;

  //������ �����
  unsigned current_packet;
  //����� ���������� �������
  unsigned total_count;
  //������������ ����� ���������
  unsigned end_time;

  //������ �����������
  bool in_air;

  icar_custom_self_timeout() :
    current_packet(),
    total_count(),
    packet_execute(def_packet_execute),
    max_packet_execute(def_max_packet_execute),
    in_air() {}

  //���������� ����������
  void ireset()
  {
    in_air = false;
    current_packet = 0;
    total_count = 0;
    end_time = 0;
  }

  //���������� ����������
  bool iis_custom_self_timeout() const { return true; }

  //������� ��������� ����� ��������� ����������
  //���������� �� ���������� ��� ��������� ������ ��������� � ��������� custom_result
  unsigned calculate_execute_time()
  {
    return (total_count - current_packet) * packet_execute;
  }

  //���������� �� ���������� ��� ��������� ��������
  void set_execute_timeout()
  {
    end_time = time(nullptr) + max_packet_execute;  // RAD Studio 10.3.3: nullptr
  }

  //���������� ����������
  bool iis_custom_timeout(unsigned int& timeout) const
  {
    if (!in_air) return timeout == 0;
    unsigned cur_time = time(nullptr);  // RAD Studio 10.3.3: nullptr
    if (cur_time >= end_time) return true;
    timeout = end_time - cur_time;
    return false;
  }
};

//��������� ���������� ��� �������, �� ������� �� �������� �������� ������
//@param cust_id ������������� ��������
template<int cust_id>
class icar_custom_simplex : public icar_custom<cust_id>
{
  bool iparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, custom_result& res) { return false; }
  bool iis_custom_in(const custom_request& req) { return false; }
};

//��������� ��������� �������
//������������ ��� ������ ����, ��� ������� �� ����� ���������� ��� ����������
struct NullReq {};

//��������� ���������� � ���������� ���������� � ���������� �������
//@param cust_id ������������� ��������
//@param Req ���������, ����������� ��������� �������. NullReq ���� ��� ����������
//@param Res ���������, ����������� ��������� �������. NullReq ���� ��� ����������
template<int cust_id, class Req = NullReq, class Res = NullReq>
class icar_custom_packable : public icar_custom<cust_id>
{
public: //KIBEZ
  //���������
  typedef Req req_t;  // RAD Studio 10.3.3: ������ typename
  //���������
  typedef Res res_t;  // RAD Studio 10.3.3: ������ typename

  //���������� ����������
  bool ibuild_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud)
  {
    return ibuild_custom_packet(req, packet_id, ud, t2t<req_t>());
  }

  //���������� ���������� ��� ������ � �����������
  template<class T>
  bool ibuild_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud, t2t<T>)
  {
    const void* data = req.get_data();
    req_t v;

#ifdef BUILD_CUSTOM_PACKET_LOG
    dbg_print("#### Unpack command data ... ####");
#endif

    if (!v.unpack(data))
    {
#ifdef BUILD_CUSTOM_PACKET_LOG
      dbg_print("#### Unpack command data FAIL ####");
#endif
      return false;
    }

#ifdef BUILD_CUSTOM_PACKET_LOG
    dbg_print("#### Build command packet... ####");
#endif

    bool ret = iibuild_custom_packet(v, packet_id, ud);

#ifdef BUILD_CUSTOM_PACKET_LOG
    if (ret)
    {
      dbg_print("#### Command OK ####");
      dbg_print("#### ud: '%s' ####", ud.begin());
    }
    else
      dbg_print("#### Command FAIL ####");
#endif

    return ret;
  }

  //���������� ���������� ��� ������ ��� ����������
  bool ibuild_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud, t2t<NullReq>)
  {
    req_t v;
    return iibuild_custom_packet(v, packet_id, ud);
  }

  //���������� ����������
  bool iparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, custom_result& res)
  {
    return iparse_custom_packet(ud, env, packet_id, res, t2t<res_t>());
  }

  //���������� ���������� ��� ������ � �����������
  template<class T>
  bool iparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, custom_result& res, t2t<T>)
  {
    res_t v;
    if (!iiparse_custom_packet(ud, env, packet_id, v)) return false;
    pkr_freezer fr(v.pack());
    res.set_data(fr.get());
    res.res_mask |= CUSTOM_RESULT_END;
    res.res_mask |= CUSTOM_RESULT_DATA;
    return true;
  }

  //���������� ���������� ��� ������ ��� ����������
  bool iparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, custom_result& res, t2t<NullReq>)
  {
    res_t v;
    if (!iiparse_custom_packet(ud, env, packet_id, v)) return false;
    res.res_mask |= CUSTOM_RESULT_END;
    res.res_mask |= CUSTOM_RESULT_DATA;
    return true;
  }

  //��������� ����� ud �� ���������� req
  //true ���� ����� ������� ��������
  virtual bool iibuild_custom_packet(const req_t& req, const req_packet& packet_id, data_t& ud) = 0;
  
  //��������� ����� ud � ��������� res
  //true ���� ����� ��� ������� ��������
  virtual bool iiparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, res_t& res) = 0;
};

#endif