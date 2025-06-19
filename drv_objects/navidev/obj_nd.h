#ifndef obj_ndH
#define obj_ndH
#include "obj_intellitrack.h"
#include "obj_nd_condition.h"
#include "obj_nd_custom.h"

namespace Nd
{
using Intellitrac::common_t;

#pragma pack(1)
struct bin_pack_t
{
  unsigned dev_id;
  unsigned datetime_direction;
  int lon;
  int lat;
  unsigned altitude:14;
  unsigned speed:10;
  unsigned char report;
  unsigned char inputs;
  unsigned short analog1;
  unsigned short analog2;
};

struct message_header_t
{
  static const unsigned char header_sig=0x02;
  unsigned char header;
  unsigned short length;
  unsigned char cmd;
};

struct message_t : public message_header_t
{
  static const unsigned char cmd_sig=0x03;
  unsigned id;
  unsigned char total;
  unsigned char number;
};

struct message_ack_t : public message_header_t
{
  static const unsigned char cmd_sig=0x01;
  unsigned char orig_cmd;
  unsigned id;
  unsigned char total;
  unsigned char number;
};

#pragma pack()

class car_nd :
  public Intellitrac::car_intellitrack,

  public cc_at_command,
  public cc_track,
  public cc_traff,
  public cc_sim_conf,
  public cc_update_server,
  public cc_auto_cmd,
  public cc_stopped_filter,
  public cc_send_sms,
  public cc_gsm_conf,
  public cc_dev_state,
  public cc_roaming,
  public cc_mdt,
  public cc_imei,
  public cc_off,
  public cc_update_fw,

  public co_nd,
  public co_mdt
{
protected:
  static void find_message_packet(data_t::iterator& msg_beg,data_t::iterator& msg_end);
  static bool is_msg_packet(data_t::iterator msg_beg,data_t::iterator& msg_end);
public:
  car_nd(int dev_id=OBJ_DEV_ND,int obj_id=0);
  virtual ~car_nd(){;}

  void register_custom();
  void register_condition();

  virtual car_nd* clone(int obj_id=0){return new car_nd(OBJ_DEV_ND,obj_id);}
  virtual const char* get_device_name() const;

  void split_tcp_stream(std::vector<data_t>& packets);
  bool need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const;
public:
};

}//namespace

#endif

