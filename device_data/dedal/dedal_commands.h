//---------------------------------------------------------------------------

#ifndef dedal_commandsH
#define dedal_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "dedal_export.h"
#include "../common_commands.h"

namespace VisiPlug { namespace Dedal {

enum AlarmMode{am_off=0,am_on=10,am_silent_arm=11,am_silent=12,am_zone_on=13,am_zone_off=14};

enum EventsKind
{
  ek_input1,//�������� ��. ���-��� 1
  ek_door,//�������� �����
  ek_trunk,//�������� �������-��
  ek_cowl,//�������� ������
  ek_drawing_off,//����. ��� �����-���
  ek_drawing_on,//���. ��� ��������
  ek_info,//������� ������ ������ (INFO)
  ek_alarm,//������� ������� ������(ALARM)
  ek_critical_batery,//����������� ���-��� ������ �������.
  ek_no_gps,//����� GPS
  ek_gsm,//����/����� �� ��-�� GSM
  ek_main_power_lost,//������� �� ������-��� �����. �������
  ek_sms_limit,//����� SMS
  ek_no_module,//����� ���. ����� � ��� �������
  ek_block_off,//����. ����������
  ek_block_on,//�����. ����������
  ek_panic,//��������� ������ "������"
  ek_ignition,//��������� ������-���
  ek_blow,//�������� ������� �����
  ek_move,//�������� ������� ��������
  ek_low_signal,//������� ��������
  ek_account_limit,//����� �����
  ek_work_button,//������� ����. ����-�� ���. �������.
  ek_red_button,//������� ������� ������ ������
  ek_alaram_on,//��������� ���. ���-����
  ek_alarm_off,//���������� ���. �������
  ek_arm_on,//����. ���. ������
  ek_arm_off,//���. ������ ������
  ek_capture_off,//����. ������ �����-��
  ek_capture_on,//�����. ������ ��-�����
  ek_service_off,//����. ������ ��-����������
  ek_service_on,//��������� ������ ������.
  ek_imm_off,//���������� IMM.
  ek_imm_label,//��������� IMM �� ���-��
  ek_imm_door,//��������� IMM �� ���-��
  ek_no_pult_off,//���������� ������ ����������� ������
  ek_no_pult_on,//��������� ������ ����������� ������
  ek_anticarjack_off,//���������� ANTICARJACK
  ek_anticarjack_on,//��������� ANTICARJACK
  ek_phone_alert,//������� GSM(� ���. ��������)
  ek_park_search,//���. ������ � ��������
  ek_zone_out,//����� �� ������� ����-������ ��� ����
  ek_zone_in,//����� � ������������� ����
  ek_get_state,//������ ���������
  ek_gsm_error,//������ GSM ������
  ek_sms_timer,//������ SMS
  ek_distance,//������� ���������
  ek_low_battery//������ ������� ����������
};

enum GuardState
{
  gs_code_rele,//������� ����
  gs_out2,//��� 2
  gs_out1,//��� 1
  gs_input1,//������ 1
  gs_zone,//������ � ��������� ����
  gs_anticarjack,//Anticarjack
  gs_imm_door,//������������ �� �����
  gs_imm_label,//������������ �� �����
  gs_no_pult,//����� �����������
  gs_service,//����� ��������
  gs_silent,//����� ������
  gs_capture,//����� �������
  gs_alert,//�������
  gs_external_power,//������� �������
  gs_ignition,//���������
  gs_guard//����� ������
};


//���������
struct DEDAL_EXPORT state_t : public VisiPlug::fix_data_t
{
  unsigned events_low;
  unsigned events_hi;

  double battery_power;
  bool module_ok;
  bool gsm_ok;
  bool gps_ok;

  unsigned short guard_state;
  unsigned char analog1;
  unsigned char analog2;
  unsigned char tahometer;
  unsigned char speedometer;
  

  state_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const state_t& rhs) const;
};

//����� ��������
struct DEDAL_EXPORT phone_number_t
{
  bool set;
  int index;
  std::string number;

  phone_number_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const phone_number_t& rhs) const;
};

//������
struct DEDAL_EXPORT password_t
{
  bool set;
  int index;
  //-1 ������������ ������ � ���������� ��. ���������� 
  int old_password;
  int password;

  password_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const password_t& rhs) const;
};

//����� �������
struct DEDAL_EXPORT events_mask_t
{
  bool set;
  int index;
  unsigned int low_bits;
  unsigned short high_bits;

  events_mask_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const events_mask_t& rhs) const;
};

//������������ ��������
struct DEDAL_EXPORT interval_t
{
  bool set;
  int interval;
  interval_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const interval_t& rhs) const;
};

//������������ ��������
struct DEDAL_EXPORT string_value_t
{
  bool set;
  std::string val;
  string_value_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const string_value_t& rhs) const;
};

//������������ ��������
struct DEDAL_EXPORT input_delay_t
{
	enum DT{dt_nochange,dt_one,dt_zero,dt_invert};

  bool set;
  int index;
  DT delay_type;
  int interval;
  input_delay_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const input_delay_t& rhs) const;
};

//�����
struct DEDAL_EXPORT outs_t
{
  int index;
  bool enable;
  outs_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const outs_t& rhs) const;
};

//����������\�������� �����
struct DEDAL_EXPORT zone_t
{
	enum ZC{zc_off,zc_in,zc_out,zc_cross};

  bool set;
  int index;
  double latitude;
  double longitude;
  unsigned char radius;
  ZC kind;

  zone_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const zone_t& rhs) const;
};

//����������\�������� �����
struct DEDAL_EXPORT mode_t
{
  bool enable;

  mode_t(){enable=false;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const mode_t& rhs) const{return enable==rhs.enable;}
};

//IP �����
struct DEDAL_EXPORT ip_t
{
  bool set;
  bool enable;
	unsigned char ip[4];
  unsigned short port;
  unsigned char interval;

  ip_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const ip_t& rhs) const;
};

//APN
struct DEDAL_EXPORT apn_t
{
  std::string apn;
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const apn_t& rhs) const{return apn==rhs.apn;}
};

struct DEDAL_EXPORT capture_t: public interval_t
{
  capture_t()
  {
    interval=21;
  }
};

struct DEDAL_EXPORT bin_t
{
  std::vector<unsigned char> data;
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const bin_t& rhs) const{return data==rhs.data;}
};

struct DEDAL_EXPORT receive_log_t
{
	bool all;
	int start_time;
	int stop_time;
  unsigned short record_count;

	receive_log_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const receive_log_t& rhs) const;
};

} }//namespace
#endif

