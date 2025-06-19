//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include <stdlib.h>

#include "common_commands.h"

#include <pokrpak/pack.h>

namespace VisiPlug {

//--------fix_data_t---------------------------------------------------------
fix_data_t::fix_data_t()
{
  fix_valid=false;
  datetime=0;
  latitude=0;
  longitude=0;
  altitude_valid=false;
  altitude=0;
  course_valid=false;
  course=0;
  speed_valid=false;
  speed=0;
}

void* fix_data_t::pack() const
{
  void* d=pkr_create_struct();
  if(!fix_valid)return d;

  PKR_ADD_ITEM(datetime);
  PKR_ADD_ITEM(latitude);
  PKR_ADD_ITEM(longitude);

	if(altitude_valid)PKR_ADD_ITEM(altitude);
	if(course_valid)PKR_ADD_ITEM(course);
	if(speed_valid)PKR_ADD_ITEM(speed);

  return d;
}

bool fix_data_t::unpack(const void* d)
{
  fix_valid=PKR_CORRECT(datetime)&&PKR_CORRECT(latitude)&&PKR_CORRECT(longitude);
  if(!fix_valid)return true;

	if(!PKR_UNPACK(datetime)||!PKR_UNPACK(latitude)||!PKR_UNPACK(longitude))return false;

  altitude_valid=PKR_CORRECT(altitude);
  if(altitude_valid&&!PKR_UNPACK(altitude))return false;

  course_valid=PKR_CORRECT(course);
  if(course_valid&&!PKR_UNPACK(course))return false;

  speed_valid=PKR_CORRECT(speed);
  if(speed_valid&&!PKR_UNPACK(speed))return false;

  return true;
}

bool fix_data_t::operator==(const fix_data_t& rhs) const
{
  return
    fix_valid==rhs.fix_valid&&
    datetime==rhs.datetime&&
    latitude==rhs.latitude&&
    longitude==rhs.longitude&&
    altitude_valid==rhs.altitude_valid&&
    altitude==rhs.altitude&&
    course_valid==rhs.course_valid&&
    course==rhs.course&&
    speed_valid==rhs.speed_valid&&
    speed==rhs.speed;
}

//---------------------------------------------------------------------------
string_t::string_t()
{
  set=true;
}

void* string_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(val);
	return d;
}

bool string_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(val);
}

bool string_t::operator==(const string_t& rhs) const
{
	return
		set==rhs.set&&
		val==rhs.val;
}

//---------------------------------------------------------------------------
universal_command_t::universal_command_t()
{
  wait_answer=true;
}

void* universal_command_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(val);
	PKR_ADD_ITEM(wait_answer);
	return d;
}

bool universal_command_t::unpack(const void* d)
{
  return
    PKR_UNPACK(val)&&
    PKR_UNPACK(wait_answer);
}

bool universal_command_t::operator==(const universal_command_t& rhs) const
{
	return
		val==rhs.val&&
		wait_answer==rhs.wait_answer;
}

//---------------------------------------------------------------------------

void* enable_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	return d;
}

bool enable_t::unpack(const void* d){return PKR_UNPACK(set)&&PKR_UNPACK(enable);}

//---------------------------------------------------------------------------

integer_t::integer_t()
{
  set=true;
  val=0;
}

void* integer_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(val);
	return d;
}

bool integer_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(val);
}

bool integer_t::operator==(const integer_t& rhs) const
{
	return
		set==rhs.set&&
		val==rhs.val;
}

//---------------------------------------------------------------------------
bin_data_t::bin_data_t()
{
  set=true;
}

void* bin_data_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(val);
	return d;
}

bool bin_data_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(val);
}

bool bin_data_t::operator==(const bin_data_t& rhs) const
{
	return
		set==rhs.set&&
		val==rhs.val;
}


//---------------------------------------------------------------------------
static wchar_t get_next_utf8(const unsigned char *pc)
{
	return (*pc)&0x3f;
}

static void utf8_to_wchar(std::vector<wchar_t>& dst, const char *src, size_t len)
{
	if(len==0) return;

  const unsigned char* pc=(const unsigned char*)src;
  const unsigned char* last=pc+len;
  int b1;

  while (pc<last)
	{
		b1=*pc++;

		// Determine whether we are dealing
		// with a one-, two-, three-, or four-
		// byte sequence.
		if ((b1 & 0x80) == 0)
		{
			// 1-byte sequence: 000000000xxxxxxx = 0xxxxxxx
			dst.push_back((wchar_t)b1);
		}
		else if((b1 & 0xe0)==0xc0)
		{
			// 2-byte sequence: 00000yyyyyxxxxxx = 110yyyyy 10xxxxxx
			wchar_t val=(static_cast<wchar_t>(b1 & 0x1f)<<6)|get_next_utf8(pc++);
			dst.push_back( val  );
		}
		else if ((b1 & 0xf0) == 0xe0)
		{
			// 3-byte sequence: zzzzyyyyyyxxxxxx = 1110zzzz 10yyyyyy 10xxxxxx
			wchar_t b2 = get_next_utf8(pc++);
			wchar_t b3 = get_next_utf8(pc++);
			wchar_t val=(static_cast<wchar_t>(b1 & 0x0f)<<12) | (b2<<6) | b3;
			dst.push_back( val );
		}
		else if((b1&0xf8)==0xf0)
		{
			// 4-byte sequence: 11101110wwwwzzzzyy + 110111yyyyxxxxxx
			//     = 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx
			// (uuuuu = wwww + 1)
			int b2 = get_next_utf8(pc++);
			int b3 = get_next_utf8(pc++);
			int b4 = get_next_utf8(pc++);
			dst.push_back( (wchar_t)(0xd800 |
               ((((b1 & 0x07) << 2) | ((b2 & 0x30) >> 4) - 1) << 6) |
               ((b2 & 0x0f) << 2) |
               ((b3 & 0x30) >> 4)) );
			dst.push_back( (wchar_t)(0xdc|((b3&0x0f)<<6)|b4) );
		}
  }
}

std::string utf82str(const std::string& val)
{
	if(val.empty())return val;
	std::vector<wchar_t> wbuf;
	utf8_to_wchar(wbuf,val.c_str(),val.size());
	if(wbuf.empty())return val;
	std::vector<char> buf(wbuf.size()*2);
  std::string old_locale=setlocale(LC_CTYPE,0);
  setlocale(LC_CTYPE,"Russian_Russia.1251");
	size_t ret=wcstombs(&*buf.begin(),&*wbuf.begin(),wbuf.size());
  setlocale(LC_CTYPE,old_locale.c_str());
	if(ret==-1)return val;
	return std::string(buf.begin(),buf.begin()+ret);
}

static void wchar_to_utf8(std::vector<char>& ret,const wchar_t* wstr)
{
  const wchar_t *pc = wstr;
  for(unsigned int c = *pc; c ; c = *(++pc))
  {
    if (c < (1 << 7))ret.push_back(c);
    else if (c < (1 << 11))
    {
      ret.push_back((c >> 6) | 0xc0);
      ret.push_back((c & 0x3f) | 0x80);
    }
    else if (c < (1 << 16))
    {
      ret.push_back((c >> 12) | 0xe0);
      ret.push_back(((c >> 6) & 0x3f) | 0x80);
      ret.push_back((c & 0x3f) | 0x80);
    }
    else if (c < (1 << 21))
    {
      ret.push_back((c >> 18) | 0xe0);
      ret.push_back(((c >> 12) & 0x3f) | 0x80);
      ret.push_back(((c >> 6) & 0x3f) | 0x80);
      ret.push_back((c & 0x3f) | 0x80);
    }
  }
}

std::string str2utf8(const std::string& val)
{
	if(val.empty())return val;

	std::vector<wchar_t> wbuf(val.size()+1);

  std::string old_locale=setlocale(LC_CTYPE,0);
  setlocale(LC_CTYPE,"Russian_Russia.1251");

  size_t r=mbstowcs(&*wbuf.begin(),val.c_str(),val.size());

  setlocale(LC_CTYPE,old_locale.c_str());

	if(!r)return val;
  
  std::vector<char> utf_buf;
  wchar_to_utf8(utf_buf,&*wbuf.begin());
	return std::string(utf_buf.begin(),utf_buf.end());
}


}//namespace


