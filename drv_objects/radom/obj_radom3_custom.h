#ifndef obj_radom3_customH
#define obj_radom3_customH

class cc_radom3_set_viewer_number : public icar_custom<CURE_RADOM3_SET_VIEWER_NUMBER>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_radom3_switch_rele : public icar_custom<CURE_RADOM_SWITCH_RELE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
    {return internal_parse_custom_packet(ud,packet_id,res);}

  bool internal_parse_custom_packet(const std::vector<unsigned char>& ud,req_packet& packet_id,custom_result& res,bool check_type=true);
};

class cc_radom3_speed_limit : public icar_custom<CURE_RADOM3_SPEED_LIMIT>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_radom3_text_message : public icar_custom<CURE_RADOM3_TEXT_MESSAGE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

#endif
