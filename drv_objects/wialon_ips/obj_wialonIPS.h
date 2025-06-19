//---------------------------------------------------------------------------
#ifndef obj_wialon_IPSH
#define obj_wialon_IPSH
//---------------------------------------------------------------------------
#include <map>

#include <vector>
#include <cstring>  // Для strtok, atoi, atof
#include <cstdlib>  // Для `atoi`, `atof`
#include <iostream> // Для `cout`


#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_wialonIPS_tool.h"
#include "obj_wialonIPS_condition.h"
#include "obj_wialonIPS_custom.h"

#define WIPS_MIN_PACKET_SIZE 3

namespace wialonIPS
{


//
typedef std::vector<unsigned char> data_t;

// Структура для хранения одного сообщения
struct ParsedMessage
{
    char date[7];
    char time[7];
    double lat;
    char lat_dir;
    double lng;
    char lng_dir;
    int speed;
    int course;
    int height;
    int sats;
    double hdop;
/*
    std::string field12;
    std::string field13;
    std::string field14;
    std::string field15;
*/
    void print() const {
  //      std::cout << "Date/Time: " << WialonUtils::formatTime(data_time) << std::endl;
        std::cout << "Latitude: " << lat << " " << lat_dir
                  << ", Longitude: " << lng << " " << lng_dir << std::endl;
        std::cout << "Speed: " << speed << " km/h, Course: " << course
                  << "°, Height: " << height << " m" << std::endl;
        std::cout << "Sats: " << sats << ", HDOP: " << hdop << std::endl;
/*
        std::cout << "Field12: " << field12 << ", Field13: " << field13
                  << ", Field14: " << field14 << ", Field15: " << field15 << std::endl;
*/
    }

    void clean()/* const*/ {
        memset((void*)date, 0, sizeof(date));
        memset((void*)time, 0, sizeof(time));
        //
        lat = 0.0;
        lat_dir = 0;
        lng = 0.0;
        lng_dir = 0;
        speed = 0;
        course = 0;
        height = 0;
        sats = 0;
        hdop = 0.0;
    }

    ParsedMessage() { clean(); }
};

// Класс для разбора данных "пакета с черным ящиком"
class BlackBoxParser {
private:
    //
    inline double parse_nmea_coord(const char* s)
    {
        std::string str(s);
        size_t dot_pos = str.find_last_of('.');

        if (dot_pos == std::string::npos) return 0.0;

        std::string deg_str = str.substr(0, dot_pos - 2);
        std::string min_str = str.substr(dot_pos - 2);

        return atof(deg_str.c_str()) + atof(min_str.c_str()) / 60.0;
    }

// Разделение строки на подстроки
std::vector<char*> splitString(const char* input, char delimiter)
{
    std::vector<char*> result;
    if (!input || !*input)
        return result;

    // Создаем копию строки для безопасного разделения
    size_t len = strlen(input);
    char* buffer = new char[len + 1];
    strcpy(buffer, input);

    // Запоминаем начало буфера для последующего освобождения
    char* bufferStart = buffer;

    // Обрабатываем строку вручную, без strtok
    while (*buffer) {
        // Запоминаем начало текущего токена
        char* tokenStart = buffer;

        // Ищем конец текущего токена (следующий разделитель или конец строки)
        while (*buffer && *buffer != delimiter) {
            buffer++;
        }

        // Если дошли до разделителя, заменяем его на '\0'
        if (*buffer == delimiter) {
            *buffer = '\0';
            buffer++; // Переходим к следующему символу
        }

        // Копируем текущий токен
        char* tokenCopy = new char[strlen(tokenStart) + 1];
        strcpy(tokenCopy, tokenStart);
        result.push_back(tokenCopy);
    }

    // Если последний символ в исходной строке был разделитель, добавляем пустую строку
    if (len > 0 && input[len-1] == delimiter) {
        char* emptyStr = new char[1];
        emptyStr[0] = '\0';
        result.push_back(emptyStr);
    }

    // Освобождаем буфер
    delete[] bufferStart;

    return result;
}

// Функция для освобождения памяти выделенной для результата
void freeStringVector(std::vector<char*>& parts)
{
    // Освобождаем память каждой подстроки
    for (size_t i = 0; i < parts.size(); i++) {
        delete[] parts[i];
    }
    
    // Очищаем вектор
    parts.clear();
}

  // Разбор одного сообщения
  signed char parseMessage(char* message, ParsedMessage& msg)
  {
        msg.clean(); // Заполняем нулями
        //
        std::vector<char*> fields = splitString(message, ';');
        //
        if (fields.size() < 10) {
            std::cout << "Ошибка: некорректное количество полей!" << std::endl;
            return /*msg*/-1;
        }

        // DATA /TIME
        strcpy(msg.date, fields[0]); // 180325
        strcpy(msg.time, fields[1]); // 080533
        //POS
        // Широта: 5027.4530
        msg.lat = parse_nmea_coord(fields[2]);
        msg.lat_dir = fields[3][0];

        // Долгота: 03024.4342
        msg.lng = parse_nmea_coord(fields[4]);
        msg.lng_dir = fields[5][0];

        //ADD
        msg.speed = atoi(fields[6]);
        msg.course = atoi(fields[7]);
        msg.height = atoi(fields[8]);
        msg.sats = atoi(fields[9]);
//??        msg.hdop = atof(fields[10]);
/*
        // Проверка "NA" > "NULL"
        msg.field12 = (strcmp(fields[11], "NA") == 0 || fields[11][0] == '\0') ? "NULL" : fields[11];
        msg.field13 = (strcmp(fields[12], "NA") == 0 || fields[12][0] == '\0') ? "NULL" : fields[12];
        msg.field14 = (strcmp(fields[13], "NA") == 0 || fields[13][0] == '\0') ? "NULL" : fields[13];
        msg.field15 = (strcmp(fields[14], "NA") == 0 || fields[14][0] == '\0') ? "NULL" : fields[14];
*/

         // ВАЖНО: Освобождаем память
        freeStringVector(fields);


        return 1;
  }

public:

//std::vector<ParsedMessage>&

    int count_packet(const data_t& rawData)
    {
        if (rawData.empty()) return 0;
        return std::count(rawData.begin(), rawData.end(), '|') + 1;
    }

    unsigned char parseData(const data_t& rawData, std::vector<ParsedMessage>& messages)
    {
//        std::vector<ParsedMessage> messages;

// ParsedMessage messages__[10];

        // Преобразуем `data_t` в C-строку
        char* buffer = new char[rawData.size() + 1];
        memcpy(buffer, &rawData[0], rawData.size());
        buffer[rawData.size()] = '\0';


/*
//////////////////////
char* rawData__ =
        "140325;094450;5027.4531;N;03024.4271;E;0;194;206;17;0.69;NA;NA;;NA;|"
        "140325;094750;5027.4531;N;03024.4271;E;0;194;206;17;0.66;NA;NA;;NA;|"
        "140325;095350;5027.4531;N;03024.4271;E;0;194;206;19;0.60;NA;NA;;NA;";
//!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

        // Разделяем по `|`
        std::vector<char*> messageParts = splitString(/*rawData__*/ buffer, '|');

        if(messageParts.size())
        {
            // Разбираем каждое сообщение
            for (size_t i = 0; i < messageParts.size(); i++)
            {
              char* one_msg = messageParts[i];
              ParsedMessage pars_one_msg;
              if(!parseMessage(one_msg, pars_one_msg) )
              {
//ERROR PARSE!!!
               continue;
              }
              messages.push_back(pars_one_msg);
            }
        }
        else
        {
              ParsedMessage pars_one_msg;
              if(parseMessage(buffer, pars_one_msg) )
              {
                  messages.push_back(pars_one_msg);
              }

        }

        delete[] buffer;

//auto zzzz = messages.size();

        return messages.size();
    }
};

//GPT--------------------------------------------




class car_wialonIPS :
  public car_gsm,  //Относиться к классу GSM устройств
  public icar_polling_packet, //опрос происходит пакетами
//  public icar_pdu_kind, //SMS отсылать ввиде текста
//KIBEZ_UDP  public icar_udp,  //устройство поддерживает UDP
  public icar_need_answer,  //У него есть синхро-пакет, на который надо ответить немедленно
  public icar_tcp,

//Интеллитрак идентифицируется 10 значным числом, но т.к. оно не влезает в int
//то говорим что это строка
  public icar_string_identification,
//Интеллитрак имеет пароль для выполнения комманд  
  public icar_password,

//Интеллитрак имеет набор комманд
  public icar_custom_ctrl,
//классы реализации комманд  
  public cc_get_state,
  public cc_alarm,
  public cc_track,
  public cc_inf,
  public cc_geo,
  public cc_life,
  public cc_log,
  public cc_reset,
  public cc_power_off,

//Интеллитрак имеет набор событий
  public icar_condition_ctrl,
  public co_common,
  public co_geo
{
private:
  mutable bool is_connected;

public:

  enum GpsMask{gm_latlon,gm_alt,gm_angle,gm_speed,gm_sat,gm_loc_cell,gm_signal_quality,gm_operator_code};
  enum ParamId{pi_battery=1,pi_usb=2,pi_live_time=5,pi_HDOP=20,pi_VDOP=21,pi_PDOP=22};
  enum Priority{pr_track,pr_periodic,pr_alarm};

  struct record_t
  {
    unsigned char priority;
    fix_packet fix;
//$$$$$    common_t evt;

    bool fix_present;

    record_t()
    {
      priority=0;
      fix_present=false;
    }
  };

  typedef std::map<std::string,std::string> strings_t;

protected:

  TcpConnectWPtr accepted_stream;
  std::string imei_pattern;
  BlackBoxParser parser;

//  bool is_synchro_packet(const data_t& data) const;

//  static unsigned short get_crc(data_t::const_iterator from,data_t::const_iterator to);
	//
	bool parse_short_data_packet(ParsedMessage& par_msg,fix_packet& fix_pak);
	//Получаем пароль из пакета
	static std::string get_passwd(const data_t& data);
	static std::vector<std::string> split(const data_t& data, unsigned char delimiter);

	static std::string extractNumber(const data_t& data);
	// Функция поиска следующего пакета, возвращает data_t
	static data_t findNextPacket(const data_t& data, size_t& start);
	static void trimByPattern(data_t& data);

public:
  car_wialonIPS(int dev_id=OBJ_DEV_WIALON_IPS/*OBJ_DEV_INTELLITRACK*/,int obj_id=0);
  //функция для регистрации комманд
  void register_custom();
  //функция для регистрации событий
  void register_condition();

  virtual ~car_wialonIPS(){;}
  //интерфейс для клонирования с другим obj_id
  virtual car_wialonIPS* clone(int obj_id=0)
        {return new car_wialonIPS(OBJ_DEV_WIALON_IPS/*OBJ_DEV_INTELLITRACK*/,obj_id);}
  //Реализация интерфейса icar_polling_packet
  void build_poll_packet(data_t& ud,const req_packet& packet_id);
  //Реализация интерфейса icar_polling
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);
  bool parse_string_fix_packet(const data_t& ud,std::vector<fix_packet>& vfix);

  bool need_answer(data_t& data,const ud_envir& env,data_t& answer) const;

  static bool common_need_answer(data_t& ud,const ud_envir& env,data_t& answer,icar* current_car);


//KIBEZ_UDP  ident_t is_my_udp_packet(const data_t& data) const;
  void split_tcp_stream(std::vector<data_t>& packets);



  //Реализация интерфейса icar. В этой функции распаковываются параметры
  //Необходимо вызвать ВСЕ!!! базовые классы, которые имеют эту функцию
  //для того чтобы они могли распаковать свои параметры
	bool params(void* param)
  {
     return car_gsm::params(param)&&
           icar_string_identification::params(param)&&
           icar_password::params(param)&&
//KIBEZ_UDP           icar_udp::params(param)&&
           icar_tcp::params(param);
  }

  virtual const char* get_device_name() const;

  void update_state(const data_t& data,const ud_envir& env);
  bool is_my_connection(const TcpConnectPtr& tcp_ptr) const;
  bool is_gsm_alphabet() const{return true;}
  bool imei_present(const data_t& ud);
  static void parse_strings(const data_t& ud,std::string& header,strings_t& ret);

public:
  static const char* endl;

//KIBEZ  virtual void parse_fix(const data_t& ud,std::vector<record_t>& records);
  void parse_string_fix(const strings_t& strs,record_t& rec);

void save_fix_data(const fix_data& fix, int error_code);


};

}//namespace
#endif
