#ifndef device_gsm_udpH
#define device_gsm_udpH
#include "infdev.h"
#include "gsm_udp.h"
#include "sms_queue.h"
#include "doublethread.h"
#include "function_queue.h"

typedef infdev<DEV_GSM_UDP,GsmTcp,car_gsm,CarContainer,DevLog,DoubleThread> gsm_ip_base;

class GsmIpDevice : public gsm_ip_base , public SmsReqQueue
{
public:
  struct poll_env
  {
    int obj_id;
    dir_req_queue_type::Req req;
    bool as_direct;
    int ret;

		poll_env() : obj_id(0),as_direct(false),ret(0) {}
  };

  struct cust_env
	{
    int obj_id;
    cust_req_queue_type::Req req;
    int ret;

    cust_env() : obj_id(0),ret(0) {}
  };

  typedef std::map<int,common_answer_f> common_answers_t;
  typedef gsm_ip_base parent_t;
private:
  function_queue in_functions;
  function_queue out_functions;
  common_answers_t common_answers;

  void validate_common_answers();
  bool get_common_answer(data_t& ud,const ud_envir& env,data_t& answer,car_type *car);
public:
  GsmIpDevice();
  ~GsmIpDevice();

  void poll_routine();
  void do_read_data();
  void do_read_udp_data();
  void do_read_tcp_data();
  void do_read_connection(const TcpConnectPtr& con_ptr);

	bool read_messages_inmem();
  void process_ud_list(carrier_type::pdu_list& lst);
  void process_ud(car_type& car,const std::vector<unsigned char>& ud,const ud_envir& env);
  void process_udp(const unsigned char* ip,int port,car_type& car,const std::vector<unsigned char>& ud);
  void do_custom(car_type* car,const cust_req_queue_type::Req& req);

  void do_flush_call(poll_env &env,const std::string& phone);
  void do_poll_packet(poll_env& env,const std::string& phone,const std::vector<unsigned char>& ud,bool text_mode);
  void poll_answer(poll_env& env);

  void thread_tick();
  void thread1_tick();

  void sms_send_routine();

  void do_read_modem_data();

  void do_custom_packet(cust_env &env,const std::string& phone,const std::vector<unsigned char>& ud,bool text_mode);

  bool need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer);

  void process_auto_answer();
  void process_udp_auto_answer();

	bool sms_queue_ready() const{return carrier_type::is_open()&&val.use_modem;}

  car_type* find_dynamic_ip(const unsigned char* ip,int port,const std::vector<unsigned char>& ud);
  car_type* get_car_by_ip(const char* ip,int port);

  car_type* get_car_by_connect(const TcpConnectPtr& con_ptr);
  void process_tcp(car_type& car,const TcpConnectPtr& con_ptr,const std::vector<unsigned char>& ud);

	bool device_params(void* param);
  bool objects_params(void* param);
};


#endif