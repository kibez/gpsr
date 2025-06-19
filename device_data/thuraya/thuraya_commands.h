//---------------------------------------------------------------------------

#ifndef thuraya_commandsH
#define thuraya_commandsH
#include <vector>
#include "thuraya_export.h"
//---------------------------------------------------------------------------
namespace VisiPlug { namespace Thuraya {

class THURAYA_EXPORT cfg
{
public:
	static const int param_count=0x20;
	static const int speed_count=0x2;

	int offset;
	int count;

	#pragma pack (1)
	union
	{
		unsigned char buf[param_count];
		struct
		{
			unsigned char :8;
			bool :2;    //������������ ���������� ����� ��� ������ �������� ������
			bool MNData:1;    //������������ ���������� ����� ��� ������ �������� ������
			bool ReplSN:1;    //�������� � ������, ���� ����� ����������� ����� �� � ����������������� �������
			bool ReplMN:1;    //�������� � ������ ��������� ������ �����������
			bool UseMN:1;     //��������� ����� �����������
			bool ReplPwd:1;   //�������� � ������ ��������� ������
			bool UsePwd:1;    //������������ �������� ������


			bool :5;
			bool PeriodPos:1; //��������� ����-������� ��������� � �������� PeriodPosT
			bool GPSoffen:1;  //��������� �������������� GPS-��������
			bool APINen:1;    //��������� ����������� ������������

			unsigned char APINpolar;   //���������� ��������� ������������
			unsigned char APINriseE;   //������������ ��� �������� �� 0 � 1
			unsigned char APINfallE;   //������������ ��� �������� �� 1 � 0
			unsigned char APINmutaE;   //�������, ������� �����������
			unsigned char APINdelaE;   //����� ��� �������� � ���������
			unsigned char APINmutaM;   //�������, ������� ���������
			unsigned char APINwakeM;   //����� ������ ��� ��������� GPS ��������

			unsigned char APINdelay;   //�������� ������������ ������������

			unsigned char OPINpolar;   //���������� �������� ����������
			unsigned char OPINpulsM;   //����� ��� ���������� �������� ����������
			unsigned char OPINwakeM;   //����� ������� ��� ��������� GPS ��������

			unsigned char OPINpulsT;   //����� ��� ���������� �������� ����������
			unsigned char GPSWakeupT;  //����� ���������� GPS
			unsigned char PeriodPosT;  //������ �������������� ������� ��������� 1���*7.765
			unsigned char TPTranBlkN;  //���������� ������ � ������ �������� �������� -1

			unsigned short TPDistance; //����������, ����� ������� ������������� ��������� ����� ��������

			unsigned char RetryMOCnt;  //���������� ������� ��� �������� SMS
			unsigned char RetryMODel;  //�������� ����� ��������� �������� �������� SMS

			unsigned char DConfThur;   // ����� ������������ ��� Thuraya

			unsigned char ThurGPST;    // ������ ������ ��������� ��� ������������� GPS-Thuraya

			unsigned char SPDincrAE;   //������������ ������������ ��� ���������� �������
			unsigned char SPDdecrAE;   //������������ ������������ ��� �������� �������� ���� �������

			unsigned char SpeedLim[speed_count]; //����� �������� � ������� ����� *1.852 ��/�
		};
	};
	#pragma pack ()

public:
	cfg();
	void* pack() const;
	bool unpack(const void* d);
	cfg& operator=(const cfg& val);
	bool operator==(const cfg& val) const;
	bool is_valid(int off) const;
};

class THURAYA_EXPORT rele
{
public:
	static const int count=8;
	static const int non_active=0;
	static const int active=1;
	static const int no_change=3;
	int v[8];

	rele();
	void* pack() const;
	bool unpack(const void* d);
	rele& operator=(const rele& val);
	bool operator==(const rele& val) const;
};


class THURAYA_EXPORT state
{
public:
	unsigned char input_change;
	unsigned char input;
	unsigned char output;
	unsigned char output_ctrl;
	unsigned char speed_limit_change;
	unsigned char speed_limit;
	unsigned char zone_change;
	unsigned char zone;

	state();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const state& val) const;
};


} }//namespace

#endif
