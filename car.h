// ============================================================================
// 1. car.h - Базовые классы устройств
// ============================================================================
#ifndef carH
#define carH

#include <System.hpp>
#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>

#include "pkr_param.h"
#include "shareddev.h"
#include "req_queue.h"
#include "car_poll_error.h"
#include "condition_car.h"
#include "comport.h"
#include "log_callback.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

struct ud_envir;
class dirreq;
class custreq;

typedef std::vector<unsigned char> data_t;

//  Абстрактная машина имеет тип железа,
//  идентификатор и название.
class icar{
private:
    icar(const icar&);
    void operator=(const icar&);
public:

    class busy{
    private:
        icar* owner;

        void set_owner(icar* p);
        void release_owner();
    public:
        busy(){owner=0l;}
        busy(icar* p){owner=0l;set_owner(p);}
        void operator=(icar* p){set_owner(p);}
        ~busy(){release_owner();}
    };
private:
    std::string build_log_name() const;

public:
    const int dev_id;

    unsigned int busy_count;
    HANDLE busy_handle;
    HANDLE should_die;

    int obj_id;
    std::string name;
    fix_data fix;

    icar(int id, int _obj_id=0);

    virtual ~icar();

    void create_die();
    void mark_die();

    virtual icar* clone(int obj_id=0)=0l;
    virtual bool params(void* param){return pkr_get_one(param,"object_name",1,name)==0;}
    virtual const char* get_device_name() const=0l;

    virtual void save_out_log(const data_t& ud) const;
    virtual void save_in_log(const data_t& ud) const;
};

class icar_net
{
public:
    std::vector<std::string> net_address;
    unsigned int current;

    icar_net(){current=0;}
    virtual bool params(void* param);

    std::string get_net_address();
    void set_next_net_address();
    bool contain(const std::string& name);
};

class icar_net_persistant
{
public:
    int polling_interval;
    icar_net_persistant(){polling_interval=0;}
    virtual bool params(void* param){return pkr_get_one(param,"polling_interval",1,polling_interval)==0;}
};

class TFixProxy;

class icar_polling
{
    struct pec{
        int remote_device_no_answer;
        int communication_error;
        int parse_error;
        int gps_no_fix;
        pec();
    };
public:
    int polling_time;
    pec poll_error;
    DWORD last_tick;

    class fix_packet
    {
    public:
        fix_data fix;
        req_packet packet_id;
        int error;
        fix_packet() : error(0){;}
    };

    icar_polling()
    {
        polling_time=0;
        last_tick=GetTickCount();
    }

    virtual bool params(void* param){return pkr_get_one(param,"polling_time",1,polling_time)==0;}
    virtual bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)=0l;
    virtual bool need_use_fix_proxy(const data_t& ud,const ud_envir& env, TFixProxy*& fix_proxy) {return false;}
    virtual bool parse_from_fix_proxy(const std::string& answer, fix_packet& vfix) {return false;}
    virtual std::string get_fix_proxy_prefix() {return "";}
};

class icar_polling_flush_call{};

class icar_polling_packet{
public:
    virtual void build_poll_packet(data_t& ud,const req_packet& packet_id)=0l;
};

// Aborigen 03.10.12
// Кожен прийнятий пакет має свій номер
class icar_inpacket_identification
{
private:

protected:
    unsigned int inpacket_identificator;
    bool b_is_inpacket_identificator;

public:
    icar_inpacket_identification() : b_is_inpacket_identificator(false), inpacket_identificator(0){}
    virtual bool is_inpacket_identificator(const data_t& ud){return false;}
    virtual data_t extract_inpacket_identificator(const data_t& ud)=0l;
    unsigned int get_inpacket_identificator() const
    {
        if(b_is_inpacket_identificator)
            return inpacket_identificator;
        else
            return (unsigned int)-1;
    }
};

// End Aborigen

//каждый отправляемый пакет имеет свой номер
class icar_packet_identification{
public:
    unsigned int packet_identificator;
    icar_packet_identification(){packet_identificator=0;}
    virtual unsigned int get_packet_identificator(){return ++packet_identificator;}
    virtual bool is_use_packet_identificator(const dirreq& req) const{return true;}
    virtual bool is_use_packet_identificator(const custreq& req) const{return true;}
};

//каждое устройство идентифицируется целым числом
class icar_int_identification{
public:
    unsigned int dev_instance_id;
    icar_int_identification(){dev_instance_id=0;}
    virtual bool params(void* param){return pkr_get_one(param,"dev_instance_id",1,dev_instance_id)==0;}
};

//каждое устройство идентифицируется строкой
class icar_string_identification{
public:
    std::string dev_instance_id;
    virtual bool params(void* param){return pkr_get_one(param,"dev_instance_id",1,dev_instance_id)==0;}
};

//логин на доступ к устройству
class icar_login{
public:
    std::string dev_login;
    virtual bool params(void* param){return pkr_get_one(param,"dev_login",0,dev_login)==0;}
};

//пароль на доступ к устройству
class icar_password{
public:
    std::string dev_password;
    virtual bool params(void* param){return pkr_get_one(param,"dev_password",1,dev_password)==0;}
};

//некоторые устройства воспринимают SMS
//только ввиде текста
class icar_pdu_kind
{
public:
    virtual bool send_pdu_text() const{return true;}
    //Пока все устройства не протестированы стоит false
    virtual bool is_gsm_alphabet() const{return false;}
};

//лог считывается из файла
//
class icar_save_log_file
{
public:
    virtual bool autodetectable(){return true;}
    virtual bool hard_autodetect(){return true;}
    virtual bool autodetect_string(FILE* f,std::string& id){return false;}
    virtual bool autodetect_int(FILE* f,unsigned int& id){return false;}
    virtual void save(FILE* f,log_callback_t& callback)=0l;

};

//лог считывается из COM порта
//
class icar_save_com_port
{
public:
    virtual bool autodetectable(){return true;}
    virtual bool hard_autodetect(){return true;}
    virtual bool need_autodetect_context(){return false;}
    virtual bool autodetect_string(com_port_t& f,std::string& id){return false;}
    virtual bool autodetect_int(com_port_t& f,unsigned int& id){return false;}
    virtual void save(com_port_t& f,log_callback_t& callback)=0l;
    virtual bool open(com_port_t& f)=0l;
};

class car_gsm :
    public icar_polling,
    public icar_poll_error,
    public icar_net,
    public icar
{
public:
    enum out_t{ot_sms,ot_gprs,ot_preffer_gprs};
public:
    car_gsm::out_t udp_command_out;

    car_gsm(int dev_id,int obj_id=0);
    virtual bool params(void* param);

    virtual ~car_gsm(){;}
    virtual car_gsm* clone(int obj_id=0)=0l;

    virtual void update_state(const data_t& data,const ud_envir& env);
    virtual bool is_synchro_packet(const data_t& data) const{return false;}

    bool can_send_sms(){return get_udp_command_out()!=ot_gprs;}
protected:
    virtual out_t get_udp_command_out() const {return udp_command_out;}
};

struct icar_udp
{
    enum ident_t{ud_my,ud_not_my,ud_undeterminate};

    std::string ip_addr;
    unsigned char ip[4];
    int udp_port;
    int online_timeout;
    time_t timeout;
    car_gsm::out_t udp_command_out;
    bool static_ip;

    icar_udp();

    virtual bool params(void* param);
    bool contain(const unsigned char* _ip,int port) const{return (static_ip||is_online())&&memcmp(ip,_ip,sizeof(ip))==0&&udp_port==port;}
    virtual ident_t update_udp_online_state(const data_t& data);
    bool can_send();
    bool find_my_dynamic(const unsigned char* _ip,int _port,const data_t& data);

    inline bool is_online() const{return online_timeout==0||timeout>time(0);}
    virtual ident_t is_my_udp_packet(const data_t& data) const {return ud_undeterminate;}
protected:
    virtual car_gsm::out_t get_udp_command_out() const {return udp_command_out;}
};

typedef bool (*common_answer_f)(data_t& ud,const ud_envir& env,data_t& answer,icar* current_car);

class icar_need_answer
{
public:
    virtual bool need_answer(data_t& data,const ud_envir& env,data_t& answer) const =0l;
    virtual common_answer_f get_common_answer_functor() const{return 0;}
};

class TcpConnect;
typedef boost::shared_ptr<TcpConnect> TcpConnectPtr;
typedef boost::weak_ptr<TcpConnect> TcpConnectWPtr;

class icar_tcp
{
public:
    int online_timeout;
    car_gsm::out_t udp_command_out;
private:
    TcpConnectWPtr tcp_ptr;
public:
    icar_tcp();

    virtual bool params(void* param);

    TcpConnectPtr get_tcp() const{return tcp_ptr.lock();}
    virtual void set_tcp(const TcpConnectPtr& _tcp_ptr);
    virtual void split_tcp_stream(std::vector<data_t>& packets)=0;
    virtual bool is_my_connection(const TcpConnectPtr& tcp_ptr) const=0;
    bool can_send();
};

class icar_station_error
{
public:
};

#endif