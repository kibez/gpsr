#ifndef obj_thuraya_customH
#define obj_thuraya_customH

class cc_thuraya_set_viewer_number : public icar_custom<CURE_MOBITEL_SET_VIEWER_NUMBER>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_thuraya_set_password : public icar_custom<CURE_MOBITEL_SET_PASSWORD>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};


class cc_thuraya_set_data_number : public icar_custom<CURE_THURAYA_SET_DATA_NUMBER>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_thuraya_get_cfg : public icar_custom<CURE_THURAYA_GET_CFG>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_thuraya_switch_rele : public icar_custom<CURE_THURAYA_SWITCH_RELE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_thuraya_stream_fix_mode : public icar_custom<CURE_THURAYA_STREAM_FIX_MODE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_thuraya_log_transmit : public icar_custom<CURE_THURAYA_LOG_TRANSMIT>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_thuraya_set_zone : public icar_custom<CURE_THURAYA_SET_ZONE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_thuraya_voice_session : public icar_custom<CURE_THURAYA_VOICE_SESSION>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_thuraya_cfg : public icar_custom<CURE_THURAYA_CFG>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_thuraya_get_state : public icar_custom<CURE_THURAYA_GET_STATE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_thuraya_log_erase : public icar_custom<CURE_THURAYA_LOG_ERASE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};
#endif
