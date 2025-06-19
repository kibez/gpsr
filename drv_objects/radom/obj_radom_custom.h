#ifndef obj_radom_customH
#define obj_radom_customH

class car_radom;

class cc_radom_switch_rele : public icar_custom<CURE_RADOM_SWITCH_RELE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
    {return internal_parse_custom_packet(ud,packet_id,res);}
  bool internal_parse_custom_packet(const std::vector<unsigned char>& ud,req_packet& packet_id,custom_result& res,bool check_type=true);
};

class cc_radom_cfg : public icar_custom<CURE_RADOM_CFG>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
    {return internal_parse_custom_packet(ud,packet_id,res);}
  bool internal_parse_custom_packet(const std::vector<unsigned char>& ud,req_packet& packet_id,custom_result& res,bool check_type=true);
};

class cc_radom_alert_reset : public icar_custom<CURE_RADOM_ALERT_RESET>
{
  car_radom& parent;
public:
  cc_radom_alert_reset(car_radom& _parent) : parent(_parent){}
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_radom_get_state : public icar_custom<CURE_RADOM_GET_STATE>
{
  car_radom& parent;
public:
  cc_radom_get_state(car_radom& _parent) : parent(_parent){}
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

#endif

