#ifndef obj_benefon_customH
#define obj_benefon_customH

class cc_benefon_acknowlege : public icar_custom_simplex<CURE_BENEFON_ACKNOWLEGE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_benefon_get_history : public virtual icar_custom<CURE_BENEFON_GET_HISTORY>,
                               public icar_custom_self_timeout<CURE_BENEFON_GET_HISTORY>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

class cc_benefon_condition_activate : public icar_custom_simplex<CURE_BENEFON_CONDITION_ACTIVATE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_benefon_easy_track_minute : public icar_custom_simplex<CURE_BENEFON_EASY_TRACK_MINUTE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_benefon_easy_track_second : public icar_custom_simplex<CURE_BENEFON_EASY_TRACK_SECOND>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_benefon_easy_track_stop : public icar_custom_simplex<CURE_BENEFON_EASY_TRACK_STOP>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_benefon_complex_track : public icar_custom_simplex<CURE_BENEFON_COMPLEX_TRACK>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_benefon_route_sending : public icar_custom_simplex<CURE_BENEFON_ROUTE_SENDING>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_benefon_route_activate : public icar_custom_simplex<CURE_BENEFON_ROUTE_ACTIVATE>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_benefon_set_cfg : public icar_custom_simplex<CURE_BENEFON_SET_CFG>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
};

class cc_benefon_get_cfg : public icar_custom<CURE_BENEFON_GET_CFG>
{
public:
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};

#endif

