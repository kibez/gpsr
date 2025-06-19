#ifndef obj_pantrack_customH
#define obj_pantrack_customH
#include "../device_data/pantrack/pantrack_commands.h"
#include "../device_data/mobitel/mobitel_commands.h"
#include <pkr_freezer.h>

//
//-------------команды-------------------------------------------------------
//
using namespace VisiPlug::Pantrack;
using VisiPlug::Mobitel::phone;

template<int cust_id,class Req=NullReq,class Res=NullReq>
class car_pantrack_accept : public icar_custom_packable<cust_id,Req,Res>
{
public:  // ИЗМЕНЕНО: сделали типы публичными
  typedef typename Req req_t;
  typedef typename Res res_t;

  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};


struct cc_pantrack_get_state : public icar_custom_packable<CURE_PANTRACK_GET_STATE,NullReq,state_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res);
};

struct cc_pantrack_reset : public icar_custom_simplex<CURE_PANTRACK_RESET>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_gomode : public car_pantrack_accept<CURE_PANTRACK_GOMODE,mode_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_signalization : public car_pantrack_accept<CURE_PANTRACK_SIGNALIZATION,mode_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_transmit_mode : public car_pantrack_accept<CURE_PANTRACK_TRANSMIT_MODE,mode_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_rele : public car_pantrack_accept<CURE_PANTRACK_RELE,mode_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_accumulator : public car_pantrack_accept<CURE_PANTRACK_ACCUMULATOR,mode_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_gprs_interval : public car_pantrack_accept<CURE_PANTRACK_GPRS_INTERVAL,interval_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_sms_interval : public car_pantrack_accept<CURE_PANTRACK_SMS_INTERVAL,interval_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_log : public car_pantrack_accept<CURE_PANTRACK_LOG>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_log_inetrval : public car_pantrack_accept<CURE_PANTRACK_LOG_INTERVAL,interval_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_read_log : public car_pantrack_accept<CURE_PANTRACK_READ_LOG>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_reset_log : public car_pantrack_accept<CURE_PANTRACK_RESET_LOG>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_server_ip : public car_pantrack_accept<CURE_PANTRACK_SERVER_IP,ip_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_apn : public car_pantrack_accept<CURE_PANTRACK_APN,apn_t>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

struct cc_pantrack_sms_center : public car_pantrack_accept<CURE_MOBITEL_SET_VIEWER_NUMBER,phone>
{
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

#endif

