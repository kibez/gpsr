#ifndef obj_dedal_customH
#define obj_dedal_customH
#include "../device_data/dedal/dedal_commands.h"
#include <pkr_freezer.h>
#include <set>

//
//-------------команды-------------------------------------------------------
//
using namespace VisiPlug::Dedal;

class custom_dedal : public icar_custom_base
{
public:
  const int req_id;
  const unsigned char main_cmd;
  std::set<unsigned char> addon_cmd;

  custom_dedal(int _r,unsigned char _m) : req_id(_r),main_cmd(_m){;}
  int get_custom_id() const {return req_id;}

  bool iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res){return false;}
  virtual bool parse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,custom_result& res)=0;

  inline bool is_cmd(unsigned char code)const
  {
    return code==main_cmd||addon_cmd.find(code)!=addon_cmd.end();
  }
};

template<class Req=NullReq,class Res=NullReq>
class custom_dedal_packable : public custom_dedal
{
public:
  //параметры
  typedef typename Req req_t;
  //результат
  typedef typename Res res_t;

  custom_dedal_packable(int _r,unsigned char _m) : custom_dedal(_r,_m){}

  //реализация интерфейса
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
  {
    return ibuild_custom_packet(req,packet_id,ud,t2t<req_t>());
  }

  //реализация интерфейса для команд с параметрами
  template<class T>
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud,t2t<T>)
  {
    const void* data=req.get_data();
    req_t v;
    if(!v.unpack(data))return false;
    return iibuild_custom_packet(v,packet_id,ud);
  }

  //реализация интерфейса для команд без параметров
  bool ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud,t2t<NullReq>)
  {
    req_t v;
    return iibuild_custom_packet(v,packet_id,ud);
  }

  //реализация интерфейса
  bool iis_custom_in(const custom_request& req)
  {
    return iis_custom_in(req,t2t<req_t>());
  }

  //реализация интерфейса для команд с параметрами
  template<class T>
  bool iis_custom_in(const custom_request& req,t2t<T>)
  {
    const void* data=req.get_data();
    req_t v;
    if(!v.unpack(data))return false;
    return unpack_custom_in(v);
  }

  //реализация интерфейса для команд без параметров
  bool iis_custom_in(const custom_request& req,t2t<NullReq>)
  {
    req_t v;
    return unpack_custom_in(v);
  }

  //реализация интерфейса
  bool parse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,custom_result& res)
  {
    return parse_custom_part(code,ud,res,t2t<res_t>());
  }

  //реализация интерфейса для команд с результатом
  template<class T>
  bool parse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,custom_result& res,t2t<T>)
  {
    res_t v;
    if(!iparse_custom_part(code,ud,v))return false;
    pkr_freezer fr(v.pack());
    res.set_data(fr.get());
    res.res_mask|=CUSTOM_RESULT_END;
    res.res_mask|=CUSTOM_RESULT_DATA;
    return true;
  }

  //реализация интерфейса для команд без результата
  bool parse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,custom_result& res,t2t<NullReq>)
  {
    res_t v;
    if(!iparse_custom_part(code,ud,v))return false;
    res.res_mask|=CUSTOM_RESULT_END;
    res.res_mask|=CUSTOM_RESULT_DATA;
    return true;
  }

  //построить пакет ud по параметрам req
  //true если пакет успешно построен
  virtual bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)=0;
  //разобрать пакет ud в результат res
  //true если пакет был успешно разобран
  virtual bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)=0;

  virtual bool unpack_custom_in(const req_t& req){return true;}
};


//--------------------------------------------------------------------------------


struct cc_dedal_get_state : public custom_dedal_packable<NullReq,state_t>
{
  cc_dedal_get_state() : custom_dedal_packable<NullReq,state_t>(CURE_DEDAL_GET_STATE,0x0)
  {
    addon_cmd.insert(0x7F);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_phone : public custom_dedal_packable<phone_number_t,phone_number_t>
{
  cc_dedal_phone() : custom_dedal_packable<phone_number_t,phone_number_t>(CURE_DEDAL_PHONE,0x4)
  {
    addon_cmd.insert(0x5);
    addon_cmd.insert(0x6);
    addon_cmd.insert(0x7);
    addon_cmd.insert(0x8);
    addon_cmd.insert(0x9);
    addon_cmd.insert(0xA);
    addon_cmd.insert(0xB);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_password : public custom_dedal_packable<password_t,password_t>
{
  cc_dedal_password() : custom_dedal_packable<password_t,password_t>(CURE_DEDAL_SET_PASSWORD,0xC)
  {
    addon_cmd.insert(0xD);
    addon_cmd.insert(0xE);
    addon_cmd.insert(0xF);
    addon_cmd.insert(0x10);
    addon_cmd.insert(0x11);
    addon_cmd.insert(0x12);
    addon_cmd.insert(0x13);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_events_mask_base : public custom_dedal_packable<events_mask_t,events_mask_t>
{
  const bool use_hole;
  cc_dedal_events_mask_base(int _r,unsigned char _m,bool _use_hole) : custom_dedal_packable<events_mask_t,events_mask_t>(_r,_m),use_hole(_use_hole){}

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_events_mask : public cc_dedal_events_mask_base
{
  cc_dedal_events_mask() : cc_dedal_events_mask_base(CURE_DEDAL_EVENTS_MASK,0x14,true)
  {
    addon_cmd.insert(0x15);
    addon_cmd.insert(0x16);
    addon_cmd.insert(0x17);
    addon_cmd.insert(0x18);
    addon_cmd.insert(0x1A);
    addon_cmd.insert(0x1B);
    addon_cmd.insert(0x1C);
  }
};

struct cc_dedal_log : public custom_dedal_packable<interval_t,interval_t>
{
  cc_dedal_log() : custom_dedal_packable<interval_t,interval_t>(CURE_DEDAL_LOG,0x1D){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_inputs : public custom_dedal_packable<input_delay_t,input_delay_t>
{
  cc_dedal_inputs() : custom_dedal_packable<input_delay_t,input_delay_t>(CURE_DEDAL_INPUTS,0x1E)
  {
    addon_cmd.insert(0x1F);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};


struct cc_dedal_outs : public custom_dedal_packable<outs_t,state_t>
{
  cc_dedal_outs() : custom_dedal_packable<outs_t,state_t>(CURE_DEDAL_OUTS,0x20)
  {
    addon_cmd.insert(0x21);
    addon_cmd.insert(0x22);
    addon_cmd.insert(0x7F);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_voice_phone : public cc_dedal_events_mask_base
{
  cc_dedal_voice_phone() : cc_dedal_events_mask_base(CURE_DEDAL_VOICE_PHONE,0x23,false)
  {
    addon_cmd.insert(0x24);
    addon_cmd.insert(0x25);
    addon_cmd.insert(0x26);
    addon_cmd.insert(0x27);
    addon_cmd.insert(0x28);
    addon_cmd.insert(0x29);
    addon_cmd.insert(0x2A);
  }
};

struct cc_dedal_guard : public custom_dedal_packable<interval_t,state_t>
{
  cc_dedal_guard() : custom_dedal_packable<interval_t,state_t>(CURE_DEDAL_ARM,0x2B)
  {
    addon_cmd.insert(0x7F);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_distance_track : public custom_dedal_packable<interval_t,interval_t>
{
  cc_dedal_distance_track() : custom_dedal_packable<interval_t,interval_t>(CURE_DEDAL_DISTANCE_TRACK,0x2C){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};


struct cc_dedal_zone : public custom_dedal_packable<zone_t,zone_t>
{
  cc_dedal_zone() : custom_dedal_packable<zone_t,zone_t>(CURE_DEDAL_ZONE,0x2D)
  {
    addon_cmd.insert(0x2E);
    addon_cmd.insert(0x2F);
    addon_cmd.insert(0x30);
    addon_cmd.insert(0x31);
    addon_cmd.insert(0x32);
    addon_cmd.insert(0x33);
    addon_cmd.insert(0x34);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_time_track : public custom_dedal_packable<interval_t,interval_t>
{
  cc_dedal_time_track() : custom_dedal_packable<interval_t,interval_t>(CURE_DEDAL_TIME_TRACK,0x35){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_sms_limit : public custom_dedal_packable<interval_t,interval_t>
{
  cc_dedal_sms_limit() : custom_dedal_packable<interval_t,interval_t>(CURE_DEDAL_SMS_LIMIT,0x36){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_reset_sms : public custom_dedal_packable<NullReq,state_t>
{
  cc_dedal_reset_sms() : custom_dedal_packable<NullReq,state_t>(CURE_DEDAL_RESET_SMS_LIMIT,0x37)
  {
    addon_cmd.insert(0x7F);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_gprs : public custom_dedal_packable<ip_t,ip_t>
{
  bool gprs_data_result;

  cc_dedal_gprs() : custom_dedal_packable<ip_t,ip_t>(CURE_DEDAL_GPRS,0x38)
  {
    gprs_data_result=false;
    addon_cmd.insert(0x7F);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool parse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,custom_result& res);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_apn : public custom_dedal_packable<string_value_t,string_value_t>
{
  cc_dedal_apn() : custom_dedal_packable<string_value_t,string_value_t>(CURE_DEDAL_APN,0x39){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_service_mode : public custom_dedal_packable<mode_t,state_t>
{
  cc_dedal_service_mode() : custom_dedal_packable<mode_t,state_t>(CURE_DEDAL_SERVICE_MODE,0x3A)
  {
    addon_cmd.insert(0x7F);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_radio_channel : public custom_dedal_packable<mode_t,state_t>
{
  cc_dedal_radio_channel() : custom_dedal_packable<mode_t,state_t>(CURE_DEDAL_RADIO_CHANNEL,0x3B)
  {
    addon_cmd.insert(0x7F);
  }
  

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_capture : public custom_dedal_packable<capture_t,state_t>
{
  cc_dedal_capture() : custom_dedal_packable<capture_t,state_t>(CURE_DEDAL_CAPTURE,0x3C)
  {
    addon_cmd.insert(0x7F);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_voice_limit : public custom_dedal_packable<interval_t,interval_t>
{
  cc_dedal_voice_limit() : custom_dedal_packable<interval_t,interval_t>(CURE_DEDAL_VOICE_LIMIT,0x3E){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_link_control : public custom_dedal_packable<interval_t,interval_t>
{
  cc_dedal_link_control() : custom_dedal_packable<interval_t,interval_t>(CURE_DEDAL_VOICE_LINK_CONTROL,0x3F){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_timestamp : public custom_dedal_packable<interval_t,interval_t>
{
  cc_dedal_timestamp() : custom_dedal_packable<interval_t,interval_t>(CURE_DEDAL_TIME_STAMP,0x01){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_time_shift : public custom_dedal_packable<interval_t,interval_t>
{
  cc_dedal_time_shift() : custom_dedal_packable<interval_t,interval_t>(CURE_DEDAL_TIME_SHIFT,0x02){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_serial : public custom_dedal_packable<NullReq,interval_t>
{
  cc_dedal_serial() : custom_dedal_packable<NullReq,interval_t>(CURE_DEDAL_SERIAL,0x03){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_service_call : public custom_dedal_packable<string_value_t,string_value_t>
{
  cc_dedal_service_call() : custom_dedal_packable<string_value_t,string_value_t>(CURE_DEDAL_SERIVCE_CALL,0x3D){}
  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_debug : public custom_dedal_packable<bin_t,state_t>
{
  cc_dedal_debug() : custom_dedal_packable<bin_t,state_t>(CURE_DEDAL_DEBUG,0x47)
  {
    addon_cmd.insert(0x7F);
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res);
};

struct cc_dedal_receive_log : public custom_dedal_packable<receive_log_t>
{
  static const unsigned int def_packet_execute=30;
  static const unsigned int def_max_packet_execute=300;

  unsigned int packet_execute;
  unsigned int max_packet_execute;

  //текщий пакет
  unsigned current_packet;
  //общее количество пакетов
  unsigned total_count;
  //максимальное время окончания
  unsigned end_time;

  //запрос выполняется
  bool in_air;

  //Обработаная запись в блоке
  unsigned current_fix;

  cc_dedal_receive_log() : custom_dedal_packable<receive_log_t>(CURE_DEDAL_RECEIVE_LOG,0x42)
  {
    current_packet=0;
    total_count=0;
    end_time=0;
    packet_execute=def_packet_execute;
    max_packet_execute=def_max_packet_execute;
    in_air=false;
    current_fix=0;


    addon_cmd.insert(0x7F);
  }

  //реализация интерфейса
  void ireset()
  {
    in_air=false;
    current_packet=0;
    total_count=0;
    end_time=0;
  }

  //реализация интерфейса
  bool iis_custom_self_timeout() const{return true;}

  //Вернуть примерное время окончания выполнения
  //вызывается из реализаций для устнаовки времеи окончания в структуре custom_result
  unsigned calculate_execute_time()
  {
    return (total_count-current_packet)*packet_execute;
  }

  //вызывается из реализации для продления таймаута
  void set_execute_timeout()
  {
    end_time=time(0l)+max_packet_execute;
  }

  //реализация интерфейса
  bool iis_custom_timeout(unsigned int& timeout) const
  {
    if(!in_air)return timeout==0;
    unsigned cur_time=time(0l);
    if(cur_time>=end_time) return true;
    timeout=end_time-cur_time;
    return false;
  }

  bool iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool parse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,custom_result& res);
  bool iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res){return false;}
};

#endif

