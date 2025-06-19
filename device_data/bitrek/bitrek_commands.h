
#ifndef bitrek_commandsH
#define bitrek_commandsH

#include <string>
#include <vector>

namespace VisiPlug { namespace Bitrek {

struct version_t
{
  std::string version;

  version_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const version_t& rhs) const;
};

struct track_t
{
  bool valid;
  char satellites;
  double lat, lon;
  short height;
  unsigned char speed;
  short direction;
  double utc_time;

  track_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const track_t& rhs) const;
};

struct delgps_t
{
  bool success;

  delgps_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const delgps_t& rhs) const;
};

struct io_t
{
  unsigned short d_inputs;
  unsigned char d_outputs;
  double analog1;
  double analog2;
  double power;
  double battary_power;

  io_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const io_t& rhs) const;
};

struct setouts_t
{
  unsigned char d_outputs;
  unsigned char d_outputs_values;

  setouts_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const setouts_t& rhs) const;
};

struct fwupdate_t
{
  std::string str;

  fwupdate_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const fwupdate_t& rhs) const;
};

int getTypeForParam(long id);

struct params_t
{
  bool set;
  std::vector<long> ids;
  std::vector<char> types;
  std::vector<char> data1;
  std::vector<long> data2;
  std::vector<std::string> strs;
  std::vector<unsigned long> data4;

  params_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const params_t& rhs) const;
};

}}

#endif
