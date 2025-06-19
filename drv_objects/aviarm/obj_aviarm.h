//---------------------------------------------------------------------------
#ifndef obj_aviarmH
#define obj_aviarmH
#include "car.h"

class car_aviarm :
  public car_gsm,
  public icar_pdu_kind,
  public icar_password,

	public icar_polling_packet,
	public icar_int_identification
{
public:
#pragma pack(1)
	struct coordinates
	{
		unsigned second:6;
		unsigned minute:6;
		unsigned hour:5;
		unsigned month_day:5;
		unsigned month:4;
		unsigned year:6;


		unsigned char lat[3];
		unsigned char lon[3];

		unsigned short speed;
		unsigned char state[3];
		unsigned char hardware_state;

		unsigned char altitude[3];
		unsigned short curs;
		unsigned char digit_out;
		unsigned char digit_in;
		unsigned char analog_in1;
		unsigned char analog_in2;
		unsigned char tahometer;
		unsigned char speedometer;
	};
#pragma pack()
public:
	car_aviarm(int dev_id=OBJ_DEV_AVIARM,int obj_id=0) : car_gsm(dev_id,obj_id) {}

	virtual ~car_aviarm(){;}
	virtual car_aviarm* clone(int obj_id=0){return new car_aviarm(OBJ_DEV_AVIARM,obj_id);}
  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	virtual bool params(void* param)
  {
    return car_gsm::params(param)&&
					 icar_int_identification::params(param)&&
					 icar_password::params(param);
	}

	virtual const char* get_device_name() const;

public:
	static void bin2txt(const std::vector<unsigned char>& bin,std::vector<unsigned char>& text);
	static void txt2bin(const std::vector<unsigned char>& text,std::vector<unsigned char>& bin);
	static unsigned char bin2txt(unsigned char b);
	static unsigned char txt2bin(unsigned char x);
};

#endif


