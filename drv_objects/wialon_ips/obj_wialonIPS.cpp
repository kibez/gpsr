//KIBEZ 24.03.2025
#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_wialonIPS.h"
#include "obj_wialonIPS_tool.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <limits>
#include <ctype.h>
#include <boost/format.hpp>
#include "tcp_server.h"
#include <boost/lexical_cast.hpp>
#include <boost\tokenizer.hpp>


//функция потом будет использоваться для подключения к сустройствам ст. наблюдения
car_gsm* create_car_wialonIPS(){return new wialonIPS::car_wialonIPS;}

namespace wialonIPS
{

const char* car_wialonIPS::endl="\r";

//инициализируем базовые классы
car_wialonIPS::car_wialonIPS(int dev_id,int obj_id)
 : car_gsm(dev_id,obj_id),
 co_common(*this),
 co_geo(*this)
{
  icar_tcp::online_timeout=300;
  register_custom();
  register_condition();
  is_connected = false;
}

//вернуть имя устройства
const char* car_wialonIPS::get_device_name() const
{
  return sz_objdev_wialonIPS;
}

//Получаем пароль из пакета
std::string car_wialonIPS::get_passwd(const data_t& data)
{
    std::string str(data.begin(), data.end()); // Преобразуем data_t в std::string
    size_t start = str.find(';'); // Ищем точку с запятой

    if (start == std::string::npos) return ""; // Если ';' нет – вернуть пусто

    start += 1; // Пропускаем ';'
    size_t end = str.find("\r\n", start); // Ищем конец числа

    return str.substr(start, end - start); // Вырезаем код
}

//
std::vector<std::string> car_wialonIPS::split(const data_t& data, unsigned char delimiter)
{
    std::vector<std::string> result;
    std::string temp;

    for (size_t i = 0; i < data.size(); i++) {
        if (data[i] == delimiter) {
            result.push_back(temp);
            temp = ""; // Очищаем строку
        } else {
            temp += static_cast<char>(data[i]); // Преобразуем в char
        }
    }

    // Добавляем последнюю подстроку, если она не пустая
    if (!temp.empty()) {
        result.push_back(temp);
    }

    return result;
}

// Функция поиска следующего пакета, возвращает data_t
data_t car_wialonIPS::findNextPacket(const data_t& data, size_t& start)
{
    if (start >= data.size()) return data_t(); // Если буфер пуст, вернуть пустой пакет

    // Ищем начало пакета: "#"
    unsigned char* found = (unsigned char*)std::memchr(&data[start], '#', data.size() - start);
    if (!found) return data_t();

    size_t packetStart = found - &data[0];

    // Ищем конец пакета: "\r\n"
    unsigned char* foundEnd = (unsigned char*)std::memchr(&data[packetStart], '\r', data.size() - packetStart);
    if (!foundEnd) return data_t(); // Конец не найден

    size_t end = (foundEnd - &data[0]) + 2; // Учитываем `\n`

    // Обновляем start для следующего поиска
    start = end;

    // Возвращаем найденный пакет
    return data_t(data.begin() + packetStart, data.begin() + end);
}

//
void car_wialonIPS::trimByPattern(data_t& data)
{
    const unsigned char pattern[] = {'\r', '\n'};
    const size_t pattern_size = sizeof(pattern) / sizeof(pattern[0]);

    data_t::iterator it = std::search(data.begin(), data.end(), pattern, pattern + pattern_size);
    if (it != data.end()) {
        data.erase(it, data.end());  // Обрезаем всё после \r\n
    }
}

//
std::string car_wialonIPS::extractNumber(const data_t& data) 
{
    std::string str(data.begin(), data.end()); // Преобразуем data_t в std::string
    size_t start = str.find("#L#");
    if (start == std::string::npos) return ""; // Если #L# не найдено, вернуть пустую строку

    start += 3; // Пропускаем "#L#"
    size_t end = str.find(';', start);
    if (end == std::string::npos) return ""; // Если ';' не найден, вернуть пустую строку

    return str.substr(start, end - start); // Вырезаем число
}

//------------------------------------------------------------

//У него есть синхро-пакет, на который надо ответить немедленно
bool car_wialonIPS::need_answer(data_t& data,const ud_envir& env,data_t& answer) const
{
  if(!env.ip_valid)return false;
	//
  WialonUtils::PacketType th_packet_type;
  data_t extracted_data = WialonUtils::extractPacket(data, th_packet_type);

  // Eto paket na soedinenie!!!!!!
  if (th_packet_type == WialonUtils::LOGIN)
  {
    // Получаем пароль из пакета
    std::string data_passwd = get_passwd(data);

    // Poluchaem s servera parametri objrkta
    const car_wialonIPS* car_obj = dynamic_cast<const car_wialonIPS*>(this);
    car_wialonIPS* th_car_obj = const_cast<car_wialonIPS*>(car_obj);
    //
    std::string serv_dev_passwd = th_car_obj->dev_password;

    //Проверяем правильность пароля
    if(data_passwd == serv_dev_passwd)
    {
      // Вставляем строку ответа на CONNECT
      const char* str_ok = "#AL#1\r\n"; //Пароль ОК
      answer.assign(str_ok, str_ok + 7);
      is_connected = true;

OutputDebugStringA("========> is_connected = true;");

      return true;
    }
    //ERRROR CONNECT
    // Вставляем строку ответа на CONNECT

    const char* str_err_passwd = "#AL#01\r\n"; //Пароль не правильный
    answer.assign(str_err_passwd, str_err_passwd + 8);

OutputDebugStringA("========> is_connected = str_err_passwd;");

    return false;
  }

  //PING packet
  if (th_packet_type == WialonUtils::PING)
  {
    const char* ping_answer = /*PK_PING_ANSWER;*/ "#AP#\r\n"; //
    answer.assign(ping_answer, ping_answer + /*PK_PING_ANSWER_SZ*/6);


OutputDebugStringA("========> PK_PING_ANSWER");


    return true;
  }

   // SHORT_DATA
  if (th_packet_type == WialonUtils::SHORT_DATA)
  {
OutputDebugStringA("========> PK_SHORT_DATA_ANSWER");

    const char* sync_answer = /*PK_DATA_OK_ANSWER;*/"#ASD#1\r\n";
    answer.assign(sync_answer, sync_answer + 8);

    return false;
  }
  // DATA
  if (th_packet_type == WialonUtils::DATA)
  {
OutputDebugStringA("========> PK_DATA_ANSWER");

    const char* sync_answer = /*PK_DATA_OK_ANSWER;*/ "#ASD#1\r\n";
    answer.assign(sync_answer, sync_answer + 8);

    return false;
  }

  // BLACKBOX
  if (th_packet_type == WialonUtils::BLACKBOX)
  {
    //!!! PEREDAVAT SODERGIMOE ~~~BEZ PREFIKSA   #B#

    // vipolnaet POLNUY razborku
    // rezultat v VEKTORE  ParsedMessage-ey !!!
    size_t size_count = parser.count_packet(extracted_data);

/*
    std::vector<ParsedMessage> messages = parser.parseData(extracted_data);//data);
    size_t size_count = messages.size();
*/
    //
    static char buffer[100];
    unsigned int res_length = sprintf(buffer, "#AB#%u\r\n",
            static_cast<unsigned int>(size_count));
    //
    const char* sync_answer = buffer;
    answer.assign(sync_answer, sync_answer + res_length);

OutputDebugStringA(sync_answer);
OutputDebugStringA("========> PK_BLACKBOX_ANSWER");

    // Важно вернуть FALSE если необходимо получить
    // эти-же данные в parse_fix_packet

//return false;
return true;


  }

// 140325;094450;5027.4531;N;03024.4271;E;0;194;206;17;0.69;NA;NA;;NA;|
// 140325;094750;5027.4531;N;03024.4271;E;0;194;206;17;0.66;NA;NA;;NA;|
// 140325;095350;5027.4531;N;03024.4271;E;0;194;206;19;0.60;NA;NA;;NA;|
// 140325;095950;5027.4531;N;03024.4271;E;0;194;206;20;0.66;NA;NA;;NA;|
// 140325;100250;5027.4531;N;03024.4271;E;0;194;206;19;0.66;NA;NA;;NA;|
// 140325;100250;5027.4531;N;03024.4271;E;0;194;206;19;0.66;NA;NA;;NA;|
// 140325;110850;5027.4531;N;03024.4271;E;0;194;206;21;0.57;NA;NA;;NA;


  // TXT_MSG
  if (th_packet_type == WialonUtils::TXT_MSG)
  {
    // “1” – soobchenie PRINATO
    const char* sync_answer = "#AM#1\r\n"; //
    answer.assign(sync_answer, sync_answer + 7);

    return false;
  }


  return false;
}

//
void car_wialonIPS::split_tcp_stream(std::vector<data_t>& packets)
{
  TcpConnectPtr con=get_tcp();
  if(!con)return;
  data_t& buf=con->read_buffer;

  const unsigned preamble=0;
  const unsigned char* bpreamble=reinterpret_cast<const unsigned char*>(&preamble);
  const unsigned char* epreamble=bpreamble+4;

  // в цикле разбираем полученый блок (несколько пакетов)
  size_t start = 0;

  while (start < buf.size())
  {
    size_t prevStart = start; // Запоминаем позицию до поиска

    data_t packet = findNextPacket(buf, start);
    if (packet.empty()) break;

    // Выводим найденный пакет
    packets.push_back(packet);

    // Заполняем нулями только обработанную часть данных
    std::memset(&buf[prevStart], 0, start - prevStart);
  }
}

//
bool car_wialonIPS::is_my_connection(const TcpConnectPtr& tcp_ptr) const
{
    data_t& buf=tcp_ptr->read_buffer;
    if(accepted_stream.lock()==tcp_ptr)return true;
    //
    if(buf.size()<dev_instance_id.size()+sizeof(unsigned short))return false;
    //
    std::string th_dev_id = extractNumber(buf);
    //
    bool is_my_dev_id = std::equal(dev_instance_id.begin(),
                          dev_instance_id.end(), th_dev_id.begin());
    //
    return is_my_dev_id;
}

//Реализация интерфейса icar_polling_packet
//строим команду на запрос координат и записываем её в ud
void car_wialonIPS::build_poll_packet(data_t& ud,const req_packet& packet_id)
{
  const char* req="FIX?";
  ud.insert(ud.end(),req,req+4);
}


#include <vcl.h>
#include <stdio.h>

double ConvertToUTC_Double(const String& dateStr, const String& timeStr, int timeZoneOffset) {
    try {
        // Разбираем дату (ddmmyy -> день, месяц, год)
        int day = StrToInt(dateStr.SubString(1, 2));
        int month = StrToInt(dateStr.SubString(3, 2));
        int year = 2000 + StrToInt(dateStr.SubString(5, 2)); // Добавляем 2000

        // Разбираем время (hhmmss -> часы, минуты, секунды)
        int hour = StrToInt(timeStr.SubString(1, 2));
        int minute = StrToInt(timeStr.SubString(3, 2));
        int second = StrToInt(timeStr.SubString(5, 2));

        // Создаём объект TDateTime (локальная дата)
        TDateTime localDateTime = EncodeDate(year, month, day) + EncodeTime(hour, minute, second, 0);

        // Конвертируем в UTC вручную (отнимаем смещение)
        double utcDateTime = localDateTime.Val - (double)timeZoneOffset / 24.0;

        return utcDateTime;
    }
    catch (...) {
        return 0.0; // Ошибка обработки
    }
}
/*
// Пример вызова
void __fastcall TForm1::Button1Click(TObject *Sender) {
    int timeZoneOffset = 3; // Например, UTC+3 (Москва)
    double utcValue = ConvertToUTC_Double("140325", "094450", timeZoneOffset);
    ShowMessage("UTC (double): " + FloatToStr(utcValue));
}
*/

bool car_wialonIPS::parse_short_data_packet(ParsedMessage& par_msg,fix_packet& fix_pak)
{

/*
std::time_t zz1 = par_msg.data_time;
std::string formattedTime = WialonUtils::formatTime(zz1);
*/

	fix_data& f=fix_pak.fix;
	int &error=fix_pak.error;

//!!!! 	error=ERROR_GPS_FIX; //POLL_ERROR_GPS_NO_FIX

//std::time_t zz1 = msg.data_time;
//std::string formattedTime = WialonUtils::formatTime(zz1);

//////////////////////////////////////////////
    char* date = par_msg.date;
    char* time = par_msg.time;
    double lat = par_msg.lat;
    char lat_dir = par_msg.lat_dir;
    double lng = par_msg.lng;
    char lng_dir = par_msg.lng_dir;
    int speed = par_msg.speed;
    int course = par_msg.course;
    int height = par_msg.height;
    int sats = par_msg.sats;
    double hdop = par_msg.hdop;
////////////////////////////////////////////////////

	f.date_valid=true;

    // Разбираем строку по формату "DDMMYY"
    sscanf(par_msg.date, "%2u%2u%2u", &f.day, &f.month, &f.year);
if(f.year < 2000)
f.year += 2000;
	//
    sscanf(par_msg.time, "%2u%2u%2lf", &f.hour, &f.minute, &f.second);

 
//ESLI budut problemi so vremenem - poprpbpvat ispolzovat!!!!!! 
 int timeZoneOffset = 2; //
 f.utc_time = ConvertToUTC_Double(date, time, timeZoneOffset);
 f.utc_time_valid=false;//true;



/*
	f.latitude=(cr->lat[0]+0x100*cr->lat[1]+0x10000*cr->lat[2])/60000.0;
	f.longitude=(cr->lon[0]+0x100*cr->lon[1]+0x10000*cr->lon[2])/60000.0;

  f.latitude=pk.lat/600000.0;
  f.longitude=pk.lng/600000.0;
*/

    f.latitude= par_msg.lat; //  /600000.0;
//    char lat_dir = par_msg.lat_dir;
  f.longitude=par_msg.lng; //   /600000.0;
//    char lon_dir = par_msg.lon_dir;


	f.speed=par_msg.speed; //knots2km((cr->speed&0x3FF)/10.0)
	f.speed_valid=true;

	f.height=par_msg.height; //(cr->altitude[0]+cr->altitude[1]*0x100+cr->altitude[2]*0x10000)/10.0;
	f.height_valid=true;

	f.course=par_msg.course;  //cr->curs/10.0;
	f.course_valid=true;
	
//SDELAT OBRABOTKU OSHIBOK!!!!!
	
/*
	if(((cr->hardware_state>>6)&3)!=2)error=POLL_ERROR_GPS_NO_FIX;
	else if(fix_time(f)==0)error=POLL_ERROR_PARSE_ERROR;
*/

return true;
}

// Добавьте в класс car_wialonIPS
void car_wialonIPS::save_fix_data(const fix_data& fix, int error_code)
{
  // Создаем результат напрямую
  void* result = ::build_fix_result(obj_id, fix);
  if (result) {
    // Отправляем напрямую, минуя проверку запроса "в воздухе"
    ::inc_data(result, 0);
    pkr_free_result(result);
  }
}

//Реализация интерфейса icar_polling
bool car_wialonIPS::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
//!!! + Chasovoi POYS!!! +   GMT+2

// Current Time (HHMMSS)  105720
std::string wialon_time =  WialonUtils::getCurrentTimeHHMMSS(true);
// Current Date (DDMMYY)  170325
std::string wialon_date = WialonUtils::getCurrentDateDDMMYY(true);

  WialonUtils::PacketType th_packet_type;
  data_t extracted_data = WialonUtils::extractPacket(ud, th_packet_type);

  // SHORT_DATA
  if (th_packet_type == WialonUtils::SHORT_DATA)
  {
    std::vector<ParsedMessage> messages;
    signed char parse_res_cod = parser.parseData(extracted_data, messages);
    size_t size_count = messages.size();

    if(size_count != 1) return false;


//!!! тут уже нужно отправлять данные НА СЕРВЕР!!!


        // TUT formiruem i vozvrachaen VISICAR-u resultat!!!!
        fix_packet fix_pak;

        // Устанавливаем правильный код ошибки
        fix_pak.error = ERROR_GPS_FIX;

        bool parse_res = parse_short_data_packet(messages[0], fix_pak);
        //
        if(!parse_res) return false;

        //
        vfix.push_back(fix_pak);
       // Используем наш метод с прямой отправкой (as_direct = true)
        save_fix_data(fix_pak.fix, fix_pak.error);

    return !vfix.empty();

  }

  // DATA
  if (th_packet_type == WialonUtils::DATA)
  {
    return true;
  }

 // BLACKBOX
  if (th_packet_type == WialonUtils::BLACKBOX)
  {
    // PEREDAVAT SODERGIMOE ~~~BEZ PREFIKSA   #B#
    // vipolnaet POLNUY razborku
    // rezultat v VEKTORE  ParsedMessage-ey !!!
    std::vector<ParsedMessage> messages;
    unsigned char res_parse_cod = parser.parseData(extracted_data/*data*/, messages);
    size_t size_count = messages.size();

    // V messages uge RAZOBRANOE soobchenie!!!!
    for (size_t i = 0; i < size_count; i++)
    {
        ParsedMessage msg = messages[i];

        // TUT formiruem i vozvrachaen VISICAR-u resultat!!!!
        fix_packet fix_pak;

        // Устанавливаем правильный код ошибки
        fix_pak.error = ERROR_GPS_FIX;

        bool parse_res = parse_short_data_packet(msg, fix_pak);
        //
        if(!parse_res) continue;

        //
        vfix.push_back(fix_pak);
       // Используем наш метод с прямой отправкой (as_direct = true)
        save_fix_data(fix_pak.fix, fix_pak.error);
    }

    return !vfix.empty();

  }
  // BOOT_DATA
  if (th_packet_type == WialonUtils::BOOT_DATA)
  {
    return true;
  }

  // CONFIG_DATA
  if (th_packet_type == WialonUtils::CONFIG_DATA)
  {
    return true;
  }

return true;
/*
/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

  if(imei_present(ud))
    return parse_string_fix_packet(ud,vfix);

  std::vector<record_t> rec;
  parse_fix(ud,rec);

  for(unsigned i=0;i<rec.size();i++)
    if(rec[i].fix_present)vfix.push_back(rec[i].fix);


  return !vfix.empty();
*/
}

bool car_wialonIPS::parse_string_fix_packet(const data_t& ud,std::vector<fix_packet>& vfix)
{
  strings_t strs;
  std::string header;
  parse_strings(ud,header,strs);

  record_t rec;
  parse_string_fix(strs,rec);
  if(!rec.fix_present)return false;
  vfix.push_back(rec.fix);
  return true;
}

/* KIBEZ
void car_wialonIPS::parse_fix(const data_t& ud,std::vector<record_t>& records)
{
return;
}
*/

void car_wialonIPS::parse_string_fix(const strings_t& strs,record_t& rec)
{
  strings_t::const_iterator it;
  fix_data& fix=rec.fix.fix;
//*$$$  common_t& evt=rec.evt;

  it=strs.find("Fix");
//...........................
}

void car_wialonIPS::update_state(const data_t& data,const ud_envir& env)
{
/* KIBEZ_UDP
  if(!env.ip_valid||!env.ip_udp)
    icar_udp::timeout=0;
*/
  if(!env.ip_valid||env.ip_udp)
  {
    TcpConnectPtr c=icar_tcp::get_tcp();
    if(c)c->close();
  }

  if(env.ip_valid&&!env.ip_udp)
  {
    TcpConnectPtr c=icar_tcp::get_tcp();
    if(c)
    {
      if(icar_tcp::online_timeout!=0)c->expired_time=time(0)+icar_tcp::online_timeout;
      else c->expired_time=0;
    }
  }

}

//
bool car_wialonIPS::imei_present(const data_t& ud)
{
  return std::search(ud.begin(),ud.end(),imei_pattern.begin(),imei_pattern.end())!=ud.end();
}

//
void car_wialonIPS::parse_strings(const data_t& ud,std::string& header,strings_t& ret)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("\r\n", "");
  tokenizer tok(reinterpret_cast<const char*>(ud.begin()),reinterpret_cast<const char*>(ud.end()),sep);

  header.clear();
  ret.clear();
  tokenizer::iterator ei=tok.end();
  bool add_header=true;


  for(tokenizer::iterator i=tok.begin();i!=ei;++i)
  {
    const std::string& s=*i;
    size_t ps=s.find(':');
    if(ps==s.npos)
    {
      if(add_header)
      {
        if(!header.empty())header+=" ";
        header+=s;
      }
      continue;
    }

    add_header=false;
    std::string key(s.begin(),s.begin()+ps);
    std::string value(s.begin()+ps+1,s.end());

    if((key=="Time"||key=="Date")&&i!=tok.end())
    {
      ++i;
      value+=" "+*i;
    }


    ret[key]=value;
  }
}

/*
void* car_wialonIPS::pack_trigger(const fix_data& fix)
{
  pkr_freezer fr(build_fix_result(obj_id, fix));
  if(fr.get() == 0l) fr = pkr_create_struct();
  // Можно добавить дополнительные поля при необходимости, например:
  // fr.replace(pkr_add_int(fr.get(), "extra_field", some_value));
  return fr.replace(0l);
}
*/

//
//-------------custom packet----------------------------------------------------
//

void car_wialonIPS::register_custom()
{
  static_cast<cc_get_state*>(this)->init(this);
  static_cast<cc_alarm*>(this)->init(this);
  static_cast<cc_track*>(this)->init(this);
  static_cast<cc_inf*>(this)->init(this);
  static_cast<cc_geo*>(this)->init(this);
  static_cast<cc_life*>(this)->init(this);
  static_cast<cc_log*>(this)->init(this);
  static_cast<cc_reset*>(this)->init(this);
  static_cast<cc_power_off*>(this)->init(this);
}


//------------------------------------------------------------------------------
bool cc_get_state::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  return true;
}

bool cc_get_state::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return true;
}

//------------------------------------------------------------------------------
bool cc_alarm::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  return true;
}

bool cc_alarm::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  return true;
}

//------------------------------------------------------------------------------
bool cc_track::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  return true;
}

bool cc_track::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  return true;
}

//------------------------------------------------------------------------------
bool cc_inf::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  return true;
}

bool cc_inf::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  return true;
}

//------------------------------------------------------------------------------
bool cc_geo::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  return true;
}

bool cc_geo::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  return true;
}

//------------------------------------------------------------------------------
bool cc_life::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  return true;
}

bool cc_life::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  return true;
}

//------------------------------------------------------------------------------
bool cc_log::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud)
{
  return true;
}

//------------------------------------------------------------------------------
bool cc_reset::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud)
{
  return true;
}

//------------------------------------------------------------------------------
bool cc_power_off::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud)
{
  return true;
}

//
//-------------condition packet----------------------------------------------
//

void car_wialonIPS::register_condition()
{
  static_cast<co_common*>(this)->init();
  static_cast<co_geo*>(this)->init();
}

bool co_common::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
    return true;
}

bool co_geo::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  return true;
}

}//namespace

