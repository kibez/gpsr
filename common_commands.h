//---------------------------------------------------------------------------

#ifndef common_commandsH
#define common_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "common_device_export.h"

namespace VisiPlug {

//Состояние
struct COMMON_DEVICE_EXPORT fix_data_t
{
  bool fix_valid;
  int datetime;
  double latitude;
  double longitude;

  bool altitude_valid;
  double altitude;

  bool course_valid;
  double course;

  bool speed_valid;
  double speed;

  fix_data_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const fix_data_t& rhs) const;
};

struct COMMON_DEVICE_EXPORT string_t
{
	bool set;
  std::string val;

	string_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const string_t& rhs) const;
};

struct COMMON_DEVICE_EXPORT universal_command_t
{
  std::string val;
  bool wait_answer;
  
	universal_command_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const universal_command_t& rhs) const;
};

struct /* KIBEZ COMMON_DEVICE_EXPORT*/ enable_t
{
	bool set;
	bool enable;

	enable_t(){set=true;enable=false;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const enable_t& rhs) const{return set==rhs.set&&enable==rhs.enable;}
};


struct COMMON_DEVICE_EXPORT enable_true_t : public enable_t
{
	enable_true_t(){enable=true;}
};


struct COMMON_DEVICE_EXPORT integer_t
{
	bool set;
  int val;

	integer_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const integer_t& rhs) const;
};


struct COMMON_DEVICE_EXPORT bin_data_t
{
	bool set;
  std::vector<unsigned char> val;

	bin_data_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const bin_data_t& rhs) const;
};


COMMON_DEVICE_EXPORT std::string str2utf8(const std::string& val);
COMMON_DEVICE_EXPORT std::string utf82str(const std::string& val);

}//namespace
#endif

