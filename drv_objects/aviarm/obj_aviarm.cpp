#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "obj_aviarm.h"
#include "shareddev.h"
#include "gps.h"
#include <libpokr/libpokrexp.h>
#include "locale_constant.h"

car_gsm* create_car_aviarm(){return new car_aviarm;}

const char* car_aviarm::get_device_name() const
{
	return sz_objdev_aviarm;
}

void car_aviarm::build_poll_packet(std::vector<unsigned char>& _ud,const req_packet& packet_id)
{
	std::vector<unsigned char> ud;
	int pass=0;
	sscanf(dev_password.c_str(),"%x",&pass);
	pass&=0xFFFFFF;
	ud.push_back(pass&0xFF);
	ud.push_back((pass&0xFF00)>>8);
	ud.push_back((pass&0xFF0000)>>16);
	bin2txt(ud,_ud);
}

bool car_aviarm::parse_fix_packet(const data_t& _ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
	std::vector<unsigned char> ud,ud0;
	ud0=gsm2iso_alphabet(_ud);
	txt2bin(ud0,ud);
	if(ud.size()<3+sizeof(coordinates))return false;
	unsigned id=ud[0]+ud[1]*0x100;
	if(id!=dev_instance_id||ud[2]!=0xFF)return false;

	const coordinates* cr=reinterpret_cast<const coordinates*>(&ud[3]);
	cr=cr;

	fix_packet fix_pak;
	fix_data& f=fix_pak.fix;
	int &error=fix_pak.error;

	error=ERROR_GPS_FIX;

	f.date_valid=true;
	f.day=cr->month_day;
	f.month=cr->month;
	f.year=cr->year+2000;

	f.hour=cr->hour;
	f.minute=cr->minute;
	f.second=cr->second;

	f.latitude=(cr->lat[0]+0x100*cr->lat[1]+0x10000*cr->lat[2])/60000.0;
	f.longitude=(cr->lon[0]+0x100*cr->lon[1]+0x10000*cr->lon[2])/60000.0;

	if(cr->speed&0x8000)f.latitude=-f.latitude;
	if(cr->speed&0x0400)f.longitude=-f.longitude;

	f.speed=knots2km((cr->speed&0x3FF)/10.0);
	f.speed_valid=true;

	f.height=(cr->altitude[0]+cr->altitude[1]*0x100+cr->altitude[2]*0x10000)/10.0;
	f.height_valid=true;

	f.course=cr->curs/10.0;
	f.course_valid=true;

	if(((cr->hardware_state>>6)&3)!=2)error=POLL_ERROR_GPS_NO_FIX;
	else if(fix_time(f)==0)error=POLL_ERROR_PARSE_ERROR;
	vfix.push_back(fix_pak);
	return true;
}

void car_aviarm::bin2txt(const std::vector<unsigned char>& bin,std::vector<unsigned char>& text)
{
	unsigned sz=bin.size()*4/3;
	if(bin.size()%3)++sz;
	text.resize(sz);
	for(unsigned i=0;i<sz;i++)
	{
		unsigned bin_ind=i*3/4;
		unsigned char bin_lo=bin[bin_ind];
		unsigned char bin_hi=0;
		if(bin_ind+1<bin.size())bin_hi=bin[bin_ind+1];
		unsigned char val=0;
		switch(i%4)
		{
		case 0:val=bin_lo&0x3F;break;
		case 1:val=((bin_lo>>6)&0x3)+((bin_hi&0xF)<<2);break;
		case 2:val=((bin_lo>>4)&0xF)+((bin_hi&0x3)<<4);break;
		case 3:val=(bin_lo>>2)&0x3F;break;
		}
		text[i]=bin2txt(val);
	}
}

void car_aviarm::txt2bin(const std::vector<unsigned char>& text,std::vector<unsigned char>& bin)
{
	unsigned bsz=text.size()*3/4;
	if(text.size()%4)++bsz;
	bin.resize(bsz);
	std::fill(bin.begin(),bin.end(),0);

	for(unsigned i=0;i<text.size();i++)
	{
		unsigned bin_ind=i*3/4;
		unsigned char& bin_lo=bin[bin_ind];
		unsigned char& bin_hi=bin[bin_ind+1];
		unsigned char val=txt2bin(text[i]);
		switch(i%4)
		{
		case 0:bin_lo|=val&0x3F;break;
		case 1:bin_lo|=(val&0x3)<<6;bin_hi|=(val>>2)&0xF;break;
		case 2:bin_lo|=(val&0xF)<<4;bin_hi|=(val>>4)&0x3;;break;
		case 3:bin_lo|=(val&0x3F)<<2;break;
		}
	}
	if(text.size()%4)bin.resize(bsz-1);
}

unsigned char car_aviarm::bin2txt(unsigned char b)
{
	b+=0x30;
	if(b>0x5A)b+=6;
	return b;
}

unsigned char car_aviarm::txt2bin(unsigned char x)
{
	x-=0x30;
	if(x>0x2A)x-=6;
	return x;
}

