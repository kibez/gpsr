//---------------------------------------------------------------------------
#ifndef radio_trace_notifyH
#define radio_trace_notifyH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "radio_trace_export.h"
#include "../common_commands.h"

namespace VisiPlug { namespace RadioTrace {

enum EvtType
{
  //������ ���������� ��������� ��������� ����� ��������� ������
  ev_first_fix=1,
  //��� ������ �� GPS  ���������
  ev_gps_no_data,
  //GPS �������� ����� �������� ������
  ev_gps_have_data,
  //����������� ���������� ������� ������� GPS ���������
  ev_gps_no_power,
  //���������� ������� ������� GPS ��������� ����� � �����
  ev_gps_have_power,
  //��� ����������� ������� ����������� ��������� ��������� ����� 8.5 ���
  ev_fix_no_change,
  //GPS �������� ����� ������� ����������
  ev_fix_have_change,
  //��� ����������� ����������� ��������� ��������� ����� 18 �����
  ev_fix_long_no_change,
  //��������� ������� ������
  ev_restart,
  //����� ������� ����������� ������ �� ����
  ev_data_transmited,
  //������ ������ �������������, �������� ������ ����� �������
  ev_memory_overflow,
  //������� ��������� ���������� �� ����� ������� �� �������
  ev_stop_fix_failed,
  //������� ����������� ������ ��� ����������� ���������
  ev_tow_vehice,
  //��������� ������� ������
  ev_no_power,
  //������� �������� 1
  ev_input1_on=20,
  //�������� �������� 1
  ev_input1_off,
  //������� �������� 2
  ev_input2_on,
  //�������� �������� 2
  ev_input2_off,
  //������� �������� 3
  ev_input3_on,
  //�������� �������� 3
  ev_input3_off,
  //������� �������� 4
  ev_input4_on,
  //�������� �������� 4
  ev_input4_off
};

enum RstType
{
  //�� ���� �������
  rs_other=0,
  //�������� �������
  rs_power_on=1,
  //�������� ������
  rs_WDT,
  //��� �������� �������
  rs_no_frequency,
  //����� RST ���������������
  rs_RST,
  //�� ������� �������
  rs_command=0xf,
};

static const unsigned input_count=4;

//���������
struct RADIO_TRACE_EXPORT state_t : public VisiPlug::fix_data_t
{
  unsigned char inputs;
  inline bool input_active(unsigned i) const{return (inputs&(1<<i))!=0;}

  bool no_gps_data;
  bool no_radio_net;
  bool reserve_energy;
  bool gps_data_valid;
  bool invalid_loger_memory;
  bool gps_valid;
  unsigned char energy_state;

  state_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const state_t& rhs) const;
};


//������� �����
struct RADIO_TRACE_EXPORT base_t
{
  int event_id;
  std::string event_str;

  base_t();
	virtual void* pack() const;
	virtual bool unpack(const void* d);
	bool operator==(const base_t& rhs) const;
};

struct RADIO_TRACE_EXPORT power_t : public base_t
{
  double power;

  power_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const power_t& rhs) const;
};

struct RADIO_TRACE_EXPORT power_sat_t : public power_t
{
  int satellite_count;

  power_sat_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const power_sat_t& rhs) const;
};

struct RADIO_TRACE_EXPORT transmit_t : public base_t
{
  int data_count;

  transmit_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const transmit_t& rhs) const;
};

struct RADIO_TRACE_EXPORT restart_t : public base_t
{
  int reason;

  restart_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const restart_t& rhs) const;
};

} }//namespace


#endif
