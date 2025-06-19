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
			bool :2;    //Использовать сохранённый номер для режима передачи данных
			bool MNData:1;    //Использовать сохранённый номер для режима передачи данных
			bool ReplSN:1;    //отвечать в случае, если номер отправителя задан не в интернацианальном формате
			bool ReplMN:1;    //отвечать в случае неверного номера отправителя
			bool UseMN:1;     //проверять номер отправителя
			bool ReplPwd:1;   //отвечать в случае неверного пароля
			bool UsePwd:1;    //использовать проверку пароля


			bool :5;
			bool PeriodPos:1; //Разрешить авто-отсылку координат с периодом PeriodPosT
			bool GPSoffen:1;  //Разрешить автовыключение GPS-приёмника
			bool APINen:1;    //Разрешить срабатыване сигнализации

			unsigned char APINpolar;   //Полярность контактов сигнализации
			unsigned char APINriseE;   //Сигнализация при переходе из 0 в 1
			unsigned char APINfallE;   //Сигнализация при переходе из 1 в 0
			unsigned char APINmutaE;   //Сигналы, которые блокируются
			unsigned char APINdelaE;   //Маска для сигналов с задержкой
			unsigned char APINmutaM;   //Сигналы, которые блокируют
			unsigned char APINwakeM;   //Маска входов для включение GPS приёмника

			unsigned char APINdelay;   //Задержка срабатывания сигнализации

			unsigned char OPINpolar;   //Полярность сигналов управления
			unsigned char OPINpulsM;   //Маска для импульсных сигналов управления
			unsigned char OPINwakeM;   //Маска выходов для включение GPS приёмника

			unsigned char OPINpulsT;   //Маска для импульсных сигналов управления
			unsigned char GPSWakeupT;  //Время готовности GPS
			unsigned char PeriodPosT;  //Период автоматической отсылки координат 1мин*7.765
			unsigned char TPTranBlkN;  //Количество блоков в пакете реальное значение -1

			unsigned short TPDistance; //Расстояние, через которое откладывается следующая точка маршрута

			unsigned char RetryMOCnt;  //Количество попыток при отправке SMS
			unsigned char RetryMODel;  //Задержка перед очередной попыткой отправки SMS

			unsigned char DConfThur;   // Флаги конфигурации для Thuraya

			unsigned char ThurGPST;    // Период опроса координат при использовании GPS-Thuraya

			unsigned char SPDincrAE;   //Срабатывание сигнализации при привышении предела
			unsigned char SPDdecrAE;   //Срабатывание сигнализации при снижении скорости ниже предела

			unsigned char SpeedLim[speed_count]; //Лимит скорости в морских узлах *1.852 км/ч
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
