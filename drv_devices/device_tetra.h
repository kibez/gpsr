//---------------------------------------------------------------------------

#ifndef device_tetraH
#define device_tetraH
#include "infdev.h"
#include "dirreq_queue.h"
//#include "obj_tetra.h"
#include "../drv_objects/tetra/obj_tetra.h"

#include "nulldev.h"
#include <sockcpp/pubinc/sock_inet.h>
#include <pair_comparator.h>

namespace Tetra
{


class TetraDevice : public infdev<DEV_TETRA,NullDev,car_tetra>
{
public:
  typedef infdev<DEV_TETRA,NullDev,car_tetra> parent_t;
  typedef std::vector< std::pair<unsigned,int> > dev2obj_t;
  typedef stdext::first_less_pr<unsigned,int> dev2obj_pr;

  struct fix_t : public icar_polling::fix_packet
  {
    int obj_id;
    fix_t()
    {
      obj_id=0;
    }

    inline bool operator<(const fix_t& rhs) const{return obj_id<rhs.obj_id;}
    inline bool operator<(int id) const{return obj_id<id;}
  };

  typedef std::vector<fix_t> fixes_t;

  enum PduType{pt_register_ind,pt_register_rsp,pt_data_ind,pt_report_ind,pt_status_ind};

  struct header_t
  {
    static const unsigned char magic_value=0xa5;

    unsigned char magic;
    unsigned char counter;
    unsigned short length;
    unsigned char pdu;

    header_t()
    {
      magic=magic_value;
      counter=0;
      length=1;
    }

    virtual ~header_t(){}

    virtual void validate_length(){length=1;}

    virtual void unpack(bin_reader& pk);
    virtual void pack(bin_writer& pk) const;
  };

  struct register_ind : public header_t
  {
    unsigned int host_ssi;

    register_ind()
    {
      pdu=pt_register_ind;
      host_ssi=0;
      length=5;
    }

    void validate_length(){length=5;}
    virtual void unpack(bin_reader& pk);
    virtual void pack(bin_writer& pk) const;
  };

  struct register_status : public header_t
  {
    unsigned char status;

    register_status()
    {
      pdu=pt_register_rsp;
      status=0;
      length=2;
    }

    void validate_length(){length=2;}
    virtual void unpack(bin_reader& pk);
    virtual void pack(bin_writer& pk) const;
  };


  struct data_ind : public header_t
  {
    unsigned int src_ssi;
    unsigned int dst_ssi;
    unsigned char protocol_ident;
    unsigned char message_refference;
    unsigned char area_selection;
    unsigned char validity_period;
    unsigned int forward_address;
    unsigned char flags;
    unsigned short length_bits;
    unsigned short length_bytes;
    unsigned char coding_scheme;
    std::vector<unsigned char> user_data;

    data_ind()
    {
      pdu=pt_data_ind;
      src_ssi=0;
      dst_ssi=0;
      protocol_ident=0;
      message_refference=0;
      area_selection=0;
      validity_period=0;
      forward_address=0;
      flags=0;
      length_bits=8;
      length_bytes=1;
      length=23;
      coding_scheme=0;
    }

    void validate_length()
    {
      if(protocol_ident!=10)
      {
        length_bytes=user_data.size()+1;
        length=23+user_data.size();
      }
      else
      {
        length_bytes=user_data.size();
        length=22+user_data.size();
      }

      //Может быть непаврное к-во бит
      if((length_bits%8)==0||length_bits/8!=length_bytes-1)
        length_bits=length_bytes*8;
    }

    virtual void unpack(bin_reader& pk);
    virtual void pack(bin_writer& pk) const;
  };

  struct report_ind : public header_t
  {
    unsigned int src_ssi;
    unsigned int dst_ssi;
    unsigned char protocol_ident;
    unsigned char message_refference;
    unsigned char area_selection;
    unsigned char delivery_status;

    report_ind()
    {
      pdu=pt_report_ind;
      src_ssi=0;
      dst_ssi=0;
      protocol_ident=0;
      message_refference=0;
      area_selection=0;
      delivery_status=0;
      length=13;
    }

    void validate_length()
    {
      length=13;
    }

    virtual void unpack(bin_reader& pk);
    virtual void pack(bin_writer& pk) const;
  };

  struct status_ind : public header_t
  {
    unsigned int src_ssi;
    unsigned int dst_ssi;
    unsigned short status_value;

    status_ind()
    {
      pdu=pt_status_ind;
      src_ssi=0;
      dst_ssi=0;
      status_value=0;
    }

    void validate_length()
    {
      length=11;
    }

    virtual void unpack(bin_reader& pk);
    virtual void pack(bin_writer& pk) const;
  };

  struct bingps_t
  {
    unsigned minute;
    unsigned second;
    double latitude;
    double longitude;
    unsigned char accuracy;
    unsigned char additional_info;

    bingps_t()
    {
      minute=0;
      second=0;
      latitude=0.0;
      accuracy=0;
      additional_info=0;
    }

    virtual void unpack(bin_reader& pk);
  };

  struct lip_short
  {
    unsigned char type;
    unsigned char time_elapsed;
    unsigned longitude;
    unsigned latitude;
    unsigned char hdop;
    unsigned char velocity;
    unsigned char direction;
    bool type_of_add_data;
    unsigned reason_for_sending;
    void unpack(const unsigned char* data);
  };

  struct lip_long
  {
    unsigned char type;
    unsigned char extension_type;
    unsigned char time_type;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    unsigned char location_shape;
    unsigned longitude;
    unsigned latitude;
    unsigned char hdop;
    unsigned altitude;
    unsigned char velocity_type;
    unsigned char velocity;
    unsigned char direction;
    bool ack;
    bool type_of_add_data;
    unsigned char reason_for_sending;
    unsigned short status;
    bool status_valid;
    void unpack(const unsigned char* data,unsigned len);
  };

  enum ReasonForSend
  {
    rs_on=0,
    rs_off=1,
    rs_emergency=2,
    rs_push_to_talk=3,
    rs_status=4,
    rs_transmit_inhibit_on=5,
    rs_transmit_inhibit_off=6,
    rs_tmo_on=7,
    rs_dmo_on=8,
    rs_enter_service=9,
    rs_loss_service=10,
    rs_cell_reselect=11,
    rs_low_battery=12,
    rs_connected_car_kit=13,
    rs_disconnected_car_kit=14,
    rs_ask_config=15,
    rs_arrival_destination=16,
    rs_arrival_location=17,
    rs_aproach_location=18,
    rs_sds_type1=19,
    rs_user=20,
    rs_no_gps=21,
    rs_have_gps=22,
    rs_leave_point=23,
    rs_ambience_listening_call=24,
    rs_temporary_reporting=25,
    rs_normal_reporting=26,
    rs_immediate_location_request=32,
    rs_time_track=129,
    rs_distance_track=130
  };

private:
  std::string gateway_host;
  int gateway_port;
  int gateway_id;
  SockCpp::inet_address_t addr;
  SockCpp::tcp_socket_t connect;
  std::vector<unsigned char> buffer;
  dev2obj_t d2o;
  DevLog olog;
  DevLog ilog;
  unsigned char cur_counter;

  static const DWORD sync_send_time=60000;
  static const DWORD sync_timeout=180000;
  static const unsigned int sync_address=999999;
  bool auth_success;
  DWORD last_send_time;
  DWORD last_receive_time;



  void build_dev2obj();
  void save_fix(const fixes_t& fixes);
  void process_packets();
  bool process_registration(bin_reader& wr);
  void process_data_ind(bin_reader& wr,fixes_t& fixes,icar_condition_ctrl::triggers& conditions);
  void process_report_ind(bin_reader& wr,fixes_t& fixes);
  void process_status_ind(bin_reader& wr);

  void do_receive();
  void do_send(const unsigned char* data,unsigned len);
  int do_send_car(car_type* car,data_ind& pk);
  int do_poll(car_type* car,fix_data& fix,const dir_req_queue_type::Req& req);

  void process_fix(const data_ind& pk,int obj_id,fixes_t& fixes,icar_condition_ctrl::triggers& conditions);
  void process_short_fix(const data_t& ud,int obj_id,fixes_t& fixes,icar_condition_ctrl::triggers& conditions);
  void process_lip_fix(const data_t& ud,int obj_id,fixes_t& fixes,icar_condition_ctrl::triggers& conditions);
  void process_short_lip_fix(const data_t& ud,int obj_id,fixes_t& fixes,icar_condition_ctrl::triggers& conditions);
  void process_long_lip_fix(const data_t& ud,int obj_id,fixes_t& fixes,icar_condition_ctrl::triggers& conditions);
  void process_lip_condition(int obj_id,int reason,int status,bool status_valid,fix_t& f,icar_condition_ctrl::triggers& conditions);



public:
  TetraDevice();

  void do_read_data();
  void do_custom(car_type* car,const cust_req_queue_type::Req& req);
	bool device_params(void* param);

  bool device_open();
  void device_close();
  bool device_is_open();
};

}//namespace
#endif