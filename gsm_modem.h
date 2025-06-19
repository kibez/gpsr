#ifndef gsm_modemH
#define gsm_modemH

#include <windows.h>
#include <dutime/dutime.h>

#include "generic_modem.h"
#include <libpokr/libpokrexp.h>
#include "pkr_param.h"
#include "shareddev.h"

#include <string>
#include <algorithm>
#include <map>
#include <loki/NullType.h>
#include "ud_envir.h"

#include "igsm_modem.h"
#include <limits>

#include "pdu.h"

template<class Loger>
class GsmModem;

template<class Loger>
//class GsmModem : public GenericModem<Loger>
class GsmModem : public GenericModem<Loger>, public igsm_modem
{
public:
  typedef ::pdu pdu;
  typedef std::map<int,pdu,std::less<int> > pdu_list;

	class param{
	public:
    typename GenericModem<Loger>::param parent;
    std::string pin1;
    std::string puk1;
    std::string pin2;
    std::string puk2;
    std::string ph_sim_pin;
    std::string sca;

    int use_sca_timezone;//3 значения
		int sca_timezone;

    int wait_sms;
    int wait_sms_max;
    int atd_cycle_count;

    bool sms_over_gprs;

		param()
		{
      wait_sms=0;
      wait_sms_max=0;
      atd_cycle_count=ATD_CYCLE_COUNT;
      use_sca_timezone=0;
      sca_timezone=0;
      sms_over_gprs=false;
		}

    bool need_restart(const param& v)
    {
      return
        v.sca!=sca||
        v.pin1!=pin1||
        v.puk1!=puk1||
        v.pin2!=pin2||
				v.puk2!=puk2||
        v.ph_sim_pin!=ph_sim_pin||
        v.sms_over_gprs!=sms_over_gprs||
        parent.need_restart(v.parent);
    }
	};

	class param_check
  {
	public:
    bool pin1;
    bool puk1;
    bool pin2;
    bool puk2;
    bool ph_sim_pin;
    bool sca;

    bool use_sca_timezone;
		bool sca_timezone;

    bool wait_sms;
    bool wait_sms_max;
    bool atd_cycle_count;
    bool sms_over_gprs;

		param_check()
		{
      pin1=false;
      puk1=false;
      pin2=false;
      puk2=false;
      ph_sim_pin=false;
      sca=false;
      wait_sms=false;
      wait_sms_max=false;
      atd_cycle_count=false;
      use_sca_timezone=false;
      sca_timezone=false;
      sms_over_gprs=false;
		}
	};

	param def;
	param_check check;
	param val;
/*
private:
  bool parse_number(unsigned char* data,unsigned int len,std::string& phone);
*/
public:
  virtual bool parse_number(unsigned char* data,unsigned int len,std::string& phone) override;
  bool default_pdu_text_mode;

	~GsmModem(){;}

  GsmModem()
  {
    modem_type=HOST_MODEM_GENERIC;
    default_pdu_text_mode=false;
  }

	bool read_param(void* params,param& val);
	bool need_restart(param& v);
	void accept(param& v);
	bool open();
  bool initialize();
  bool autodetect();
  bool autodetect_end_line();
  bool autodetect_end_line_check();

  int flush_call(const std::string& phone);

/*
  int send_packet(const std::string phone,const std::vector<unsigned char> ud) override;
  int send_packet(const std::string phone,const std::vector<unsigned char> ud0,bool text_mode) override;
*/
virtual int send_packet(const std::string phone,const std::vector<unsigned char> ud) override;
virtual int send_packet(const std::string phone,const std::vector<unsigned char> ud0,bool text_mode) override;

  int storadge_count() const;
  bool switch_memory_storage(int read_mode=0,int write_mode=0);

  bool read_pdus(pdu_list& lst);
private:
  bool read_pdus_answer(pdu_list& lst);
public:

  bool delete_pdus(const pdu_list& lst);

  bool parse_pdu(const std::string& szRead,pdu& pd);

  void close(){/*if(is_open())write_command("ATH")*/;GenericModem<Loger>::close();}

  static ud_envir pdu2env(const pdu& pd);

  time_t sc_timestamp(const std::string& sct);

  bool is_7bit(unsigned char dcs);
};

template<class Loger>
bool GsmModem<Loger>::read_param(void* params,param& val)
{
  if(!GenericModem<Loger>::read_param(params,val.parent)) return false;
	PKR_GET_ONE("pin1",pin1)
	PKR_GET_ONE("puk1",puk1)
	PKR_GET_ONE("pin2",pin2)
	PKR_GET_ONE("puk2",puk2)
	PKR_GET_ONE("ph_sim_pin",ph_sim_pin)
	PKR_GET_ONE("sca",sca)

	PKR_GET_ONE("wait_sms",wait_sms)
	PKR_GET_ONE("wait_sms_max",wait_sms_max)
	PKR_GET_ONE("atd_cycle_count",atd_cycle_count)
	PKR_GET_ONE("sms_over_gprs",sms_over_gprs)

	PKR_GET_ONE("use_sca_timezone",use_sca_timezone)
  val.sca_timezone=std::numeric_limits<int>::max();
	PKR_GET_ONE("sca_timezone",sca_timezone)
  //04.12.2003 
  if(val.use_sca_timezone==2)val.sca_timezone=std::numeric_limits<int>::max();

	return true;
}

template<class Loger>
bool GsmModem<Loger>::need_restart(param& v)
{
  return GenericModem<Loger>::need_restart(v.parent)||is_open()&&val.need_restart(v);
}

template<class Loger>
void GsmModem<Loger>::accept(param& v)
{
	bool ch=val.need_restart(v);
  GenericModem<Loger>::accept(v.parent);
  val=v;
  if(ch)initialize();
}

template<class Loger>
bool GsmModem<Loger>::open()
{
  if(GenericModem<Loger>::open()&&initialize()) return true;
  close();
  return false;
}

template<class Loger>
bool GsmModem<Loger>::initialize()
{
  if(!is_open()) return false;
	if(!autodetect()) return false;

	if(modem_type==HOST_MODEM_WAVECOM_GPRS)end_line_prefix="\r";

  std::string szRead;
  std::string szCommand;

	//устанавливаем модем в режим без эха
	if(!write_command("ATE0")) return false;
	if(!read_answer(szRead)) return false;
	if(szRead=="ATE0"&&!read_answer(szRead)) return false;
//для телефона Nokia авторизованая команда
//	if(!is_ok(szRead)) return false;

  int au;
	if(modem_type!=HOST_MODEM_MITSUBISHI_TRIUM)
	while(1)
	{
		//что модему нужно для аутентификации
		if(!write_command("AT+CPIN?"))return false;
		if(!read_answer(szRead))return false;
		if(szRead=="AT+CPIN?"&&!read_answer(szRead))return false;

		//первая строчка что нужно
		if(szRead=="+CPIN: READY")au=au_ready;
		else if(szRead=="+CPIN: SIM PIN")au=au_pin1;
		else if(szRead=="+CPIN: SIM PUK")au=au_puk1;
		else if(szRead=="+CPIN: SIM PIN2")au=au_pin2;
		else if(szRead=="+CPIN: SIM PUK2")au=au_puk2;
		else if(szRead=="+CPIN: PH-SIM PIN")au=au_ph_sim_pin;
		else if(szRead=="+CPIN: BLOCKED")au=au_blocked;

		//вторая строчка OK
		if(modem_type!=HOST_MODEM_WAVECOM&&
			 modem_type!=HOST_MODEM_WAVECOM_GPRS&&
       modem_type!=HOST_MODEM_TECHFAITH)
		{
      //wavecom подобные модемы OK не отвечают
      read_answer(szRead);
//			if(!read_answer(szRead)) return false;
//			if(!is_ok(szRead)) return false;
		}

		//либо блокирован либо ответ не распознан
		if(au==au_blocked) return false;

		//готов к работе
		if(au==au_ready) break;

	  switch(au)
		{
		case au_pin1:
      szCommand="AT+CPIN=\""+val.pin1+"\"";
		  break;
		case au_puk1:
      szCommand="AT+CPIN=\""+val.puk1+"\"";
		  break;
		case au_pin2:
      szCommand="AT+CPIN=\""+val.pin2+"\"";
		  break;
		case au_puk2:
      szCommand="AT+CPIN=\""+val.puk2+"\"";
		  break;
		case au_ph_sim_pin:
      szCommand="AT+CPIN=\""+val.ph_sim_pin+"\"";
		  break;
		}

		if(!write_command(szCommand))return false;
		if(!read_answer(szRead)) return false;
		if(szRead==szCommand&&!read_answer(szRead)) return false;
		if(modem_type!=HOST_MODEM_ERICSSON_GM12&&!read_answer(szRead)) return false;
		if(!is_ok(szRead))return false;
	}

	if(modem_type==HOST_MODEM_NOKIA)
	{
		if(!write_command("ATE0")) return false;
		if(!read_answer(szRead)) return false;
		if(szRead=="ATE0"&&!read_answer(szRead)) return false;
		if(!is_ok(szRead)) return false;
	}

	//устанавливаем номер SMS-центра
  if(!val.sca.empty())
  {
    szCommand="AT+CSCA=\""+val.sca+"\"";
    if(!write_command(szCommand)) return false;
    if(!read_answer(szRead)) return false;
    if(!is_ok(szRead)) return false;
  }

	//говорим, что SMS кодируются в PDU режиме
	if(!write_command("AT+CMGF=0")) return false;
	if(!read_answer(szRead)) return false;
	if(!is_ok(szRead)) return false;

  switch(modem_type)
  {
  case HOST_MODEM_MITSUBISHI_TRIUM:
  case HOST_MODEM_ASCOM:
  case HOST_MODEM_SIEMENS_S45:
  case HOST_MODEM_SIEMENS_ME45:
  case HOST_MODEM_NOKIA:
    break;
  case HOST_MODEM_TECHFAITH:
		if(!write_command("AT+CNMI=2,0,0,0,0")) return false;
		if(!read_answer(szRead)) return false;
		if(!is_ok(szRead)) return false;
    break;
  default:
		if(!write_command("AT+CNMI=3,0")) return false;
		if(!read_answer(szRead)) return false;
		if(!is_ok(szRead)) return false;
	}

	if(modem_type==HOST_MODEM_ERICSSON_GM12)
	{
	  if(!write_command("AT*ECAM=1")) return false;
	  if(!read_answer(szRead)) return false;
    if(!is_ok(szRead)) return false;
	}

  if(val.sms_over_gprs)
	{
		if(modem_type==HOST_MODEM_SIEMENS_S45||
		   modem_type==HOST_MODEM_SIEMENS_ME45||
       modem_type==HOST_MODEM_WAVECOM_GPRS||
       modem_type==HOST_MODEM_TECHFAITH||
       modem_type==HOST_MODEM_SIMCOM300||
       modem_type==HOST_MODEM_SIEMENS_MC75)
    {
      //пытаемся переключиться в GPRS
      if(!write_command("AT+CGATT=1")) return false;
      if(!read_answer(szRead)&&!read_answer(szRead) ) return sleep(0);
      if(is_ok(szRead))
      {
        if(!write_command("AT+CGSMS=1")) return false; //GPRS предпочтительнее SMS
        if(!read_answer(szRead)&&!read_answer(szRead)&&!read_answer(szRead) ) return false;
      }
    }
  }

  return true;
}

template<class Loger>
bool GsmModem<Loger>::autodetect_end_line_check()
{
  std::string szRead;
	if(!write_command("AT"))return false;
  bool fsc=read_answer(szRead);
  if(fsc&&szRead=="AT")fsc=read_answer(szRead);
	if(!fsc||!is_ok(szRead))
  {
    //ещё одна попытка
	  if(!write_command("AT"))return false;
	  if(!read_answer(szRead))return false;
	  if(szRead=="AT"&&!read_answer(szRead)) return false;
  }

	return is_ok(szRead);
}

template<class Loger>
bool GsmModem<Loger>::autodetect_end_line()
{

  end_line_prefix="\r\n";
  if(autodetect_end_line_check()) return true;
	end_line_prefix="\r";
  if(autodetect_end_line_check()) return true;
  end_line_prefix="\n";
  if(autodetect_end_line_check()) return true;
	dbg_print("modem not found");
	return false;
}

template<class Loger>
bool GsmModem<Loger>::autodetect()
{
  std::string szRead;
  std::string szManufacturer;
  std::string szModel;

  if(!autodetect_end_line())return false;
	modem_type=HOST_MODEM_GENERIC;

	//устанавливаем модем в режим без эха
	if(!write_command("ATE0")) return false;
	if(!read_answer(szRead)) return false;
	if(szRead=="ATE0"&&!read_answer(szRead)) return false;
//для телефона Nokia авторизованая команда
//	if(!is_ok(szRead)) return false;

	if(!write_command("AT+CGMI"))return false;
	if(!read_answer(szRead))return false;
	if(szRead=="AT+CGMI"&&!read_answer(szRead))return false;
	if(is_error(szRead)) return false;
  szManufacturer=szRead;
	if(!read_answer(szRead))return false;
	if(!is_ok(szRead)) return true;

	if(!write_command("AT+CGMM"))return false;
	if(!read_answer(szRead))return false;
	if(szRead=="AT+CGMM"&&!read_answer(szRead))return false;
//для телефона Nokia авторизованая команда
	if(!is_error(szRead))
  {
    szModel=szRead;
	  if(!read_answer(szRead))return false;
	  if(!is_ok(szRead)) return true;
  }

	if(strncmp(szManufacturer.c_str(),"ERICSSON",sizeof("ERICSSON")-1)==0)
	{
		if(strncmp(szModel.c_str(),"6050102",sizeof("6050102")-1)==0) modem_type=HOST_MODEM_ERICSSON_GM12;
	}
	else if(strncmp(szManufacturer.c_str()," WAVECOM MODEM",sizeof(" WAVECOM MODEM")-1)==0)
  {
    if(strncmp(szModel.c_str()," MULTIBAND  900E  1800",sizeof(" MULTIBAND  900E  1800")-1)==0)
      modem_type=HOST_MODEM_WAVECOM_GPRS;
    else modem_type=HOST_MODEM_WAVECOM;
  }
  else if(strncmp(szManufacturer.c_str(),"Mitsubishi",sizeof("Mitsubishi")-1)==0)
  {
		if(strncmp(szModel.c_str(),"MT-Data-Outside",sizeof("MT-Data-Outside")-1)==0)
      modem_type=HOST_MODEM_MITSUBISHI_TRIUM;
  }
  else if(strncmp(szManufacturer.c_str(),"ascom",sizeof("ascom")-1)==0)
    modem_type=HOST_MODEM_ASCOM;
	else if(strncmp(szManufacturer.c_str(),"SIEMENS",sizeof("SIEMENS")-1)==0)
	{
		if(strncmp(szModel.c_str(),"ME45",sizeof("ME45")-1)==0) modem_type=HOST_MODEM_SIEMENS_ME45;
		else modem_type=HOST_MODEM_SIEMENS_S45;
	}
  else if(strncmp(szManufacturer.c_str(),"Nokia Mobile Phones",sizeof("Nokia Mobile Phones")-1)==0)
    modem_type=HOST_MODEM_NOKIA;
  else if(strncmp(szManufacturer.c_str(),"Benefon Oyj",sizeof("Benefon Oyj")-1)==0)
		modem_type=HOST_MODEM_BENEFON;
  else if(strncmp(szManufacturer.c_str(),"TECHFAITH MODEM",sizeof("TECHFAITH MODEM")-1)==0)
		modem_type=HOST_MODEM_TECHFAITH;
  else if(strncmp(szManufacturer.c_str(),"SIMCOM_Ltd",sizeof("SIMCOM_Ltd")-1)==0)
  {
    if(strncmp(szModel.c_str(),"SIMCOM_SIM300",sizeof("SIMCOM_SIM300")-1)==0) modem_type=HOST_MODEM_SIMCOM300;
  }
  else if(strncmp(szManufacturer.c_str(),"Cinterion",sizeof("Cinterion")-1)==0)
  {
    if(strncmp(szModel.c_str(),"MC75",sizeof("MC75")-1)==0) modem_type=HOST_MODEM_SIEMENS_MC75;
  }

  std::string mdm = "modem: " + szManufacturer + " " + szModel;

	switch(modem_type)
	{
	case HOST_MODEM_ERICSSON_GM12:dbg_print("modem: ERICSSON_GM12");break;
	case HOST_MODEM_WAVECOM:dbg_print("modem: WAVECOM");break;
	case HOST_MODEM_MITSUBISHI_TRIUM:dbg_print("modem: MITSUBISHI_TRIUM");break;
	case HOST_MODEM_ASCOM:dbg_print("modem: ASCOM");break;
	case HOST_MODEM_SIEMENS_S45:dbg_print("modem: SIEMENS_S45");break;
	case HOST_MODEM_SIEMENS_ME45:dbg_print("modem: SIEMENS_ME45");break;
	case HOST_MODEM_NOKIA:dbg_print("modem: NOKIA");break;
	case HOST_MODEM_WAVECOM_GPRS:dbg_print("modem: WAVECOM_GPRS");break;
	case HOST_MODEM_BENEFON:dbg_print("modem: BENEFON");break;
	case HOST_MODEM_TECHFAITH:dbg_print("modem: TECHFAITH");break;
	case HOST_MODEM_SIMCOM300:dbg_print("modem: SIMCOM_SIM300");break;
	case HOST_MODEM_SIEMENS_MC75:dbg_print("modem: Cinterion(Siemens) MC75");break;
	default:dbg_print(mdm.c_str());break;
	}

	return true;
}

template<class Loger>
int GsmModem<Loger>::flush_call(const std::string& phone)
{
	std::string szCommand;
  std::string szRead;
	bool ret;
  int i=0;
  int Error=0;

	szCommand=std::string("ATD")+phone+";";
	if(!write_command(szCommand)) goto inf_dev_error;

	if(modem_type==HOST_MODEM_ERICSSON_GM12)
	{
	  if(!read_answer(szRead)) goto inf_dev_error;
		if(is_ok(szRead))
		{
			char in_air=0;
			char idle=0;
			char busy=0;
			char connecting=0;
			for(i=0;i<val.atd_cycle_count;i++)
			{
				int call=0;
				int reason=0;
				ret=read_answer(szRead);
			  //что-то прочитали или таймаут
			  if(!is_open()) goto inf_dev_error;
				if(!ret) continue;

				if(strncmp(szRead.c_str(),"*ECAV: ",strlen("*ECAV: "))||
					 sscanf(szRead.c_str()+sizeof("*ECAV: ")-1,"%d%*1c%d",&call,&reason)!=2)
				{
					if(in_air) break;
					continue;
				}
				if(reason==0)idle=1;
				if(idle||in_air)break;

				if(reason==2)connecting=1;

				if(reason==3)in_air=1;
				if(reason==7){busy=1;break;}
			}
			if(in_air&&idle) goto exit;
			if(busy) goto remote_device_busy;
			if(in_air&&i==val.atd_cycle_count) goto communication_error;
			if(connecting) goto remote_device_no_answer;
			goto inf_dev_error;
		}
		else if(szRead=="NO CARRIER") goto remote_device_no_answer;
		else if(szRead=="BUSY") goto remote_device_busy;
		else goto inf_dev_error;
	}
	else
	{
		//время, которое отводится на то, чтобы модем сделал звонок
		for(i=0;i<val.atd_cycle_count;i++)
		{
			ret=read_answer(szRead);
			if(!is_open()) goto inf_dev_error;
			if(!ret) continue;

		  if(is_ok(szRead)||strncmp(szRead.c_str(),"CONNECT",strlen("CONNECT"))==0||
         szRead=="NO CARRIER")
			{
				read_answer(szRead);
				goto exit;
			}
		  else if(szRead=="NO ANSWER") goto remote_device_no_answer;
//		  else if(szRead=="NO CARRIER") goto communication_error;  может и не дождаться коннекта
		  else if(szRead=="BUSY") goto remote_device_busy;
		  else goto inf_dev_error;
		}
		goto remote_device_no_answer;
	}

exit:
  write_command("ATH");
  read_answer(szRead);
  write_command("ATH");
  read_answer(szRead);
  flush();
	return Error;
inf_dev_error:
  Error=POLL_ERROR_INFORMATOR_DEVICE_ERROR;
	goto exit;
remote_device_no_answer:
  Error=POLL_ERROR_REMOTE_DEVICE_NO_ANSWER;
	goto exit;
remote_device_busy:
	Error=POLL_ERROR_REMOTE_DEVICE_BUSY;
	goto exit;
communication_error:
  Error=POLL_ERROR_COMMUNICATION_ERROR;
	goto exit;
}

template<class Loger>
int GsmModem<Loger>::send_packet(const std::string phone,const std::vector<unsigned char> ud)
{
  return send_packet(phone,ud,default_pdu_text_mode);
}

template<class Loger>
int GsmModem<Loger>::send_packet(const std::string phone,const std::vector<unsigned char> ud0,bool text_mode)
{
  if(ud0.size()>140) return ERROR_INFORMATOR_NOT_SUPPORT;

	std::vector<unsigned char> ud;
	if(text_mode&&ud0.size())
	{
    unsigned char buf[512];
    std::fill(buf,buf+sizeof(buf),0);
		unsigned size=eight2seven(&ud0.front(),ud0.size(),buf);
    ud.insert(ud.end(),buf,buf+size);
	}else ud=ud0;

  std::vector<unsigned char> pdu;

	pdu.push_back(0);//Length of SCA
	pdu.push_back(0x11);//First Octet
	pdu.push_back(0);//Message Reference
  build_number(phone,pdu);
	pdu.push_back(0);//PID

  if(text_mode)pdu.push_back(0);//Coding shem
  else pdu.push_back(0xF6);//Coding shem

	pdu.push_back(0x00);//Validity Period
	pdu.push_back((unsigned char)ud0.size());
  pdu.insert(pdu.end(),ud.begin(),ud.end());

  char szPDU[1024];
	bin2hex(&*pdu.begin(),pdu.size(),szPDU);

  char szCommand[128];
  std::string szRead;

	sprintf(szCommand,"AT+CMGS=%d",pdu.size()-1);//length - SCA length
	if(!write_command(szCommand)) return POLL_ERROR_INFORMATOR_DEVICE_ERROR;
  if(!read_some()) return POLL_ERROR_INFORMATOR_DEVICE_ERROR;
  drop_first(buf_pos);

	szPDU[pdu.size()*2+1]=0;
	szPDU[pdu.size()*2]=26;
	if(!write(szPDU)) return POLL_ERROR_INFORMATOR_DEVICE_ERROR;
	read_answer(szRead);
  if(is_error_word(szRead)) return POLL_ERROR_COMMUNICATION_ERROR;
	read_answer(szRead);
  if(is_error_word(szRead)) return POLL_ERROR_COMMUNICATION_ERROR;

  if(params.local.log_sms)
	{
    std::string file_name=program_directory+"sms";CreateDirectory(file_name.c_str(),0l);
    file_name+="\\pout";CreateDirectory(file_name.c_str(),0l);
    file_name+="\\"+phone;CreateDirectory(file_name.c_str(),0l);
    file_name+="\\";

    Loger l;
    l.open_cur_time(file_name,"");
    if(ud0.size()) l.write(&ud0.at(0),ud0.size());
  }

	return POLL_ERROR_NO_ERROR;
}

template<class Loger>
int GsmModem<Loger>::storadge_count() const
{
  switch(modem_type)
  {
  case HOST_MODEM_SIEMENS_S45:return 3;
  case HOST_MODEM_MITSUBISHI_TRIUM: return 1;
  case HOST_MODEM_BENEFON: return 1;
  case HOST_MODEM_SIEMENS_ME45:return 1;
  case HOST_MODEM_TECHFAITH:return 1;
  case HOST_MODEM_GENERIC:return 1;
  case HOST_MODEM_SIMCOM300:return 1;
  case HOST_MODEM_SIEMENS_MC75:return 3;
	}
  return 2;
}

template<class Loger>
bool GsmModem<Loger>::switch_memory_storage(int read_mode,int write_mode)
{
	if(modem_type==HOST_MODEM_MITSUBISHI_TRIUM) return true;
  char* read="SM";
  char* write="SM";
  char szCommand[256];
  std::string szRead;

  if(modem_type==HOST_MODEM_ERICSSON_GM12||
     modem_type==HOST_MODEM_SIEMENS_S45||
     modem_type==HOST_MODEM_NOKIA||
     modem_type==HOST_MODEM_SIEMENS_MC75
     )
  {
    if(read_mode==1)read="ME";
    if(write_mode==1)write="ME";

    if(read_mode==2)read="MT";
    if(write_mode==2)read="MT";
  }
  else if(modem_type!=HOST_MODEM_ASCOM)
  {
    if(read_mode==1)read="BM";
    if(write_mode==1)read="BM";
  }

  sprintf(szCommand,"AT+CPMS=\"%s\",\"%s\"",read,write);

  if(!write_command(szCommand)) return false;
	//пропускаем +CPMS: xxxxx
	if(!read_answer(szRead)||is_error(szRead)) return false;
	//ловим OK
	if(!read_answer(szRead)||!is_ok(szRead)) return false;
	return true;
}

template<class Loger>
bool GsmModem<Loger>::read_pdus(pdu_list& lst)
{
  switch(modem_type)
  {
  case HOST_MODEM_MITSUBISHI_TRIUM:
  {
		if(!write_command("AT+CMGL=1"))return false;
    bool ret=read_pdus_answer(lst);
    if(!write_command("AT+CMGL=0")) return false;
    return ret&read_pdus_answer(lst);
  }
  default:
    if(!write_command("AT+CMGL=4")) return false;
    return read_pdus_answer(lst);
  }
}

template<class Loger>
bool GsmModem<Loger>::read_pdus_answer(pdu_list& lst)
{
	while(1)
  {
    std::string szRead;

		if(!read_answer(szRead)||is_error(szRead))return false;
		if(is_ok(szRead)||strncmp(szRead.c_str(),"+CMGL: ",strlen("+CMGL: ")) )break;

		int index,stat;
		if(sscanf(szRead.c_str()+strlen("+CMGL: "),"%d%*1c%d",&index,&stat)!=2) return false;

    if(!read_answer(szRead)) return false;

    pdu pd;
    pd.fresh=stat==0;
    pd.index=index;

    if(lst.find(index)==lst.end())
    {
		  if(!parse_pdu(szRead, pd))
      {
        dbg_print("coudn't parse pdu <<%s>>\n",szRead.c_str());
        pd.valid=false;
      }
      lst[index]=pd;
    }
  }
  return true;
}

template<class Loger>
bool GsmModem<Loger>::parse_pdu(const std::string& szRead, pdu& pd)
{
	unsigned char bpdu[512];
  const char* buf=szRead.c_str();
	int len=szRead.size()/2;
	int cur=0;
	bool enc7=false;

	unsigned int src_len=0;

	unsigned ud_len=0;
	unsigned char ud[512];

	unsigned i;

	hex2bin(buf,bpdu);

	if(len<13) return false;

	//пропускаем адрес SMS центра
	cur+=bpdu[cur]+1;
	if(cur>=len) return false;
	cur++; //пропускаем First Octet
	if(cur>=len) return false;

	//получаем адрес отправителя
	src_len=bpdu[cur];
	if(src_len%2)src_len=src_len/2+2;
	else src_len=src_len/2+1;
	cur++;

	if(!parse_number(bpdu+cur,src_len,pd.number)) return false;

	cur+=src_len;
	if(cur>=len) return false;

	cur++; //пропускаем PID
	//кодирование сообщения
	enc7=is_7bit(bpdu[cur]);
	cur++;

	std::string timestamp;
	if(!parse_gsm_number(bpdu+cur,7,timestamp)) return false;
  pd.timestamp=sc_timestamp(timestamp);
	cur+=7;//service center timestamp

	ud_len=bpdu[cur];
	cur++;

  int d = 0;
  if(bpdu[cur] <= 0x08 && bpdu[cur + 1] == 0x00)
  {
    int c = cur + 2;
    c += bpdu[c] + 1;

    pd.count = bpdu[c - 2];
    pd.part = bpdu[c - 1];
  }

	if(enc7)
	{
    if(pd.count > 1) d = 7;
		seven2eight(bpdu + cur, ud_len, (char*)ud);
		if(((int)ud_len) * 7 / 8 > len - cur) return false;
	}
  else
  {
    if(pd.count > 1) d = 6;
    pd.utf16 = true;
    memcpy(ud, bpdu + cur, ud_len);
  }
	if(pd.count == 1 && !pd.utf16) ud[ud_len] = 0;

  pd.ud = std::vector<unsigned char>(ud + d, ud + ud_len);

  if(params.local.log_sms)
	{
    std::string file_name=program_directory+"sms";CreateDirectory(file_name.c_str(),0l);
    file_name+="\\pin";CreateDirectory(file_name.c_str(),0l);
    file_name+="\\"+pd.number;CreateDirectory(file_name.c_str(),0l);
    file_name+="\\";

    Loger l;
    char* postf="";
    if(!pd.fresh)postf=".old";
    l.open_cur_time(file_name,postf);
    l.write(ud + d, ud_len - d);
  }

  return true;
}

template<class Loger>
bool GsmModem<Loger>::is_7bit(unsigned char dcs)
{
  switch(dcs>>4)
  {
  case 0:
  case 1:
  case 2:
  case 3:
		return (dcs&0xC)==0;
  case 0xF:
    return (dcs&0x04)==0;
  }
  return false;
}

template<class Loger>
bool GsmModem<Loger>::parse_number(unsigned char* data,unsigned int len,std::string& phone)
{
  if(len>20) return false;
  char str[128];
	char* cur=str;
	if(data[0]==0x91)
	{
		*cur='+';
		cur++;
	}
	for(unsigned int i=1;i<len;i++)
	{
		unsigned val=data[i];
		unsigned lo_digit=(val>>4)&0xF;
		unsigned hi_digit=val&0xF;

		if(lo_digit!=0xF&&lo_digit>9||hi_digit>9) return false;

		if(lo_digit==0xF)
		{
			sprintf(cur,"%1d",hi_digit);
			break;
		}
		else sprintf(cur,"%1d%1d",hi_digit,lo_digit);
		cur+=2;
	}

  phone=str;
  return true;
}

template<class Loger>
bool GsmModem<Loger>::delete_pdus(const pdu_list& lst)
{
  for(pdu_list::const_iterator i=lst.begin();i!=lst.end();++i)
	{
    char szCommand[32];
	  sprintf(szCommand,"AT+CMGD=%d",i->first);
	  write_command(szCommand);
    std::string szRead;
	  read_answer(szRead);
	}
  return true;
}

template<class Loger>
ud_envir GsmModem<Loger>::pdu2env(const pdu& pd)
{
  ud_envir env;
	if(pd.timestamp==0)
  {
		env.timestamp_valid=false;
    env.timestamp=bcb_time(0l);
  }
  else
  {
    env.timestamp_valid=true;
    env.timestamp=pd.timestamp;
  }
  env.number_valid=true;
  env.number=pd.number;
  return env;
}

template<class Loger>
time_t GsmModem<Loger>::sc_timestamp(const std::string& sct)
{
  if(val.sca_timezone==std::numeric_limits<int>::max()) return 0;

  tm t;
  // Relation to GMT. One unit is 15min. If MSB=1, value is negative.
  // Поэтому старший бит просто сбрасываем
  int timezone=0;
  if(sscanf(sct.c_str(),"%02d %02d %02d %02d %02d %02d %02d",
     &t.tm_year,&t.tm_mon,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec,&timezone)!=7) return time(0l);

	t.tm_year+=100;
  t.tm_mon--;

  if(val.use_sca_timezone) timezone=val.sca_timezone;
  else
  {
    bool tz_negative=timezone&0x80;
    timezone&=0x7F;
    if(tz_negative)timezone=-timezone;
    timezone*=15*60;
  }

  return localtime(t,timezone);
}

#endif