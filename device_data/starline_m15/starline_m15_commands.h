
#ifndef starline_m15_commandsH
#define starline_m15_commandsH

#include <vector>
#include <string>
#include "starline_m15_export.h"
//---------------------------------------------------------------------------

namespace VisiPlug { namespace StarLine_M15 {

struct STARLINE_M15_EXPORT owner_num_t
{
  bool set;
  std::string number;

  owner_num_t();

  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const owner_num_t& other) const;
};

struct STARLINE_M15_EXPORT date_time_t
{
  bool set;
  double datetime; // 0.0 - ������ �����������

  date_time_t();

  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const date_time_t& other) const;
};

struct STARLINE_M15_EXPORT password_t
{
  std::string pwd;
  int mode; // 0 - �������; 1 - "+"; 2 - "-"

  password_t();

  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const password_t& other) const;
};

struct STARLINE_M15_EXPORT timers_t
{
  bool set;
  short timer_num; // 1 �� 2. ��� set==false (���������) ����������
  double datetime[2]; // 0.0 ���� �� ��������
  char days[2];       // ����� - ��.      0 - �������
  char hours[2];      // ����� - ������.   0 - �������
  char minutes[2];    // ����� - �������.  0 - �������

  enum format_t {fm_none = 0, fm_text, fm_link, fm_wait_cmd = 4};
  format_t format[2];

  timers_t();

  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const timers_t& other) const;
};

struct STARLINE_M15_EXPORT motion_sensor_t
{
  bool set;
  char on; // 0 - �������; 1 - ��������; 2 - ��������
  enum messages_t {m_none, m_sms, m_mon, m_sms_mon};
  messages_t messages;  // ��� ����������� ��� ���

  char time;     // ����������� � �� �������� ���
  char level;    // ��������� �������

  motion_sensor_t();

  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const motion_sensor_t& other) const;
};

struct STARLINE_M15_EXPORT state_mode_t
{
  bool normal;

  state_mode_t();

  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const state_mode_t& other) const;
};

struct STARLINE_M15_EXPORT mon_t
{
  bool set;
  char on;  // 0 - �������; 1 - "+"; 2 - "-"
  short time; // ����� � �������� 3..300

  mon_t();

  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const mon_t& other) const;
};

}} // namespace StarLine_M15 namespace VisiPlug

#endif 