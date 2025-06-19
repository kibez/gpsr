#ifndef obj_mobitel_customH
#define obj_mobitel_customH

class cc_mobitel_set_viewer_number : public icar_custom_simplex<CURE_MOBITEL_SET_VIEWER_NUMBER>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_set_sms_center_number : public icar_custom_simplex<CURE_MOBITEL_SET_SMS_CENTER_NUMBER>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_log_save_time : public icar_custom_simplex<CURE_MOBITEL_SET_LOG_SAVE_TIME>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_set_password : public icar_custom_simplex<CURE_MOBITEL_SET_PASSWORD>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_zone : public icar_custom_simplex<CURE_MOBITEL_ZONE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_zone_current : public icar_custom_simplex<CURE_MOBITEL_ZONE_CURRENT>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_in_mask : public icar_custom_simplex<CURE_MOBITEL_IN_MASK>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_enable_guard : public icar_custom_simplex<CURE_MOBITEL_ENABLE_GUARD>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_enable_in : public icar_custom_simplex<CURE_MOBITEL_ENABLE_IN>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_switch_rele : public icar_custom_simplex<CURE_MOBITEL_SWITCH_RELE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_off : public icar_custom_simplex<CURE_MOBITEL_OFF>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_get_state : public icar_custom<CURE_MOBITEL_GET_STATE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

//
// Дополнения к протоколу
class cc_mobitel_set_zone_radius : public icar_custom_simplex<CURE_MOBITEL_SET_ZONE_RADIUS>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_set_log_distance : public icar_custom_simplex<CURE_MOBITEL_SET_LOG_DISTANCE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_mobitel_set_alert_sms_period : public icar_custom_simplex<CURE_MOBITEL_SET_ALERT_SMS_PERIOD>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

#endif

