#ifndef obj_benefon_box_customH
#define obj_benefon_box_customH

struct cc_benefon_outs : icar_custom<CURE_BENEFON_OUTS>
{
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res);
};


#endif

