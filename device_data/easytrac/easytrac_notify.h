//---------------------------------------------------------------------------
#ifndef easytrac_notifyH
#define easytrac_notifyH
#include "easytrac_export.h"
#include "easytrac_commands.h"
//---------------------------------------------------------------------------
namespace VisiPlug { namespace EasyTrac {

struct EASYTRAC_EXPORT geo_report_t : public common_t
{
  int group;
  double distance;

  geo_report_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const geo_report_t& rhs) const;
};

struct EASYTRAC_EXPORT key_report_t : public common_t
{
  std::string data;

	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const key_report_t& rhs) const;
};

struct /*KIBEZ EASYTRAC_EXPORT*/ button_report_t : public common_t
{
  int user_no;
  std::string rfid;

  button_report_t(){user_no=-1;}
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const button_report_t& rhs) const;
};

} }//namespace
#endif

