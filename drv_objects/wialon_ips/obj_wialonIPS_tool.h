//---------------------------------------------------------------------------
#ifndef obj_wialon_tool_IPSH
#define obj_wialon_tool_IPSH
//---------------------------------------------------------------------------

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <ctime>
#include <sstream>
#include <algorithm>

// В ответ на:
//
//#define CONNECT_OK_PK_ANSWER_ "#AP#\r\n"
//#define PK_SYNC_ANSWER_SZ (sizeof(PK_SYNC_ANSWER) - 1)  // 4 (без `\0`)



/*
//====== Пакет логина
#define LOGIN_OK_ANSWER "#AP#1\r\n"
#define LOGIN_OK_ANSWER_SZ 7

#define LOGIN_ERR_LOGIN_ANSWER "#AP#0\r\n"
#define LOGIN_ERR_LOGIN_ANSWER_SZ 7
#define LOGIN_ERR_PASSWD_ANSWER "#AP#01\r\n"
#define LOGIN_ERR_PASSWD_ANSWER_SZ 8

//
#define PK_PING_ANSWER "#AP#\r\n"
#define PK_PING_ANSWER_SZ 6

//===== Сокращенный пакет с данными
//пакет успешно зафиксировался
#define PK_SDATA_OK_ANSWER "#ASD#1\r\n"
#define PK_SDATA_OK_ANSWER_SZ 8
//ошибка структуры пакета
#define PK_SDATA_ERR_STRUCT_ANSWER "#ASD#-1\r\n"
#define PK_SDATA_ERR_STRUCT_ANSWER_SZ 9
//некорректное время
#define PK_SDATA_ERR_TIME_ANSWER "#ASD#0\r\n"
#define PK_SDATA_ERR_TIME_ANSWER_SZ 8
//ошибка получения координат
#define PK_SDATA_ERR_POS_ANSWER "#ASD#10\r\n"
#define PK_SDATA_ERR_POS_ANSWER_SZ 9
//ошибка получения скорости, курса или высоты
#define PK_SDATA_ERR_ALLPARAM_ANSWER "#ASD#11\r\n"
#define PK_SDATA_ERR_ALLPARAM_ANSWER_SZ 9
// ошибка получения количества спутников
#define PK_SDATA_ERR_SAT_COUNT_ANSWER "#ASD#12\r\n"
#define PK_SDATA_ERR_SAT_COUNT_ANSWER_SZ 9

//================= Пакет с данными

//пакет успешно зафиксировался
#define PK_DATA_OK_ANSWER "#AD#1\r\n"
#define PK_DATA_OK_ANSWER_SZ 7
//ошибка структуры пакета
#define PK_DATA_ERR_STRUCT_ANSWER "#AD#-1\r\n"
#define PK_DATA_ERR_STRUCT_ANSWER_SZ 8
//некорректное время
#define PK_DATA_ERR_TIME_ANSWER "#AD#0\r\n"
#define PK_DATA_ERR_STRUCT_ANSWER_SZ 7
//ошибка получения координат
#define PK_DATA_ERR_POS_ANSWER "#AD#10\r\n"
#define PK_DATA_ERR_POS_ANSWER_SZ 8
//ошибка получения скорости, курса или высоты
#define PK_DATA_ERR_MANY_PARAM_ANSWER "#AD#11\r\n"
#define PK_DATA_ERR_MANY_PARAM_ANSWER_SZ 8
// ошибка получения количества спутников
#define PK_DATA_ERR_POS_ANSWER "#AD#12\r\n"
#define PK_DATA_ERR_POS_ANSWER_SZ 8
// ошибка получения inputsили outputs
#define PK_DATA_ERR_INOUT_ANSWER "#AD#13\r\n"
#define PK_DATA_ERR_INOUT_ANSWER_SZ 8
// ошибка получения adc
#define PK_DATA_ERR_ADC_ANSWER "#AD#14\r\n"
#define PK_DATA_ERR_ADC_ANSWER_SZ 8
// ошибка получения дополнительных параметров
#define PK_SDATA_ERR_EXT_PARAM_ANSWER "#AD#15\r\n"
#define PK_SDATA_ERR_EXT_PARAM_ANSWER_SZ 8
*/

typedef std::vector<unsigned char> data_t;

class WialonUtils
{
public:
    // Определение типа пакета через enum
    enum PacketType
    {
        UNKNOWN = 0,
        LOGIN = 1,
        SHORT_DATA = 2,
        DATA = 3,
        BLACKBOX = 4, //Blackbox packetv
        PING = 5,
        TXT_MSG = 6, //Text message
        BOOT_DATA = 7, // boot
        CONFIG_DATA = 8, // config
    };


static std::time_t convertToTimeZone(const std::string& datetimeStr, const char* timezone)
{
    if (datetimeStr.length() != 13) return -1; // Проверка длины строки "HHMMSS DDMMYY"

    std::tm tmStruct = {};

    int hh, mm, ss, dd, mon, yy;
    sscanf(datetimeStr.c_str(), "%2d%2d%2d %2d%2d%2d", &hh, &mm, &ss, &dd, &mon, &yy);

    yy += 2000;
    tmStruct.tm_hour = hh;
    tmStruct.tm_min = mm;
    tmStruct.tm_sec = ss;
    tmStruct.tm_mday = dd;
    tmStruct.tm_mon = mon - 1;
    tmStruct.tm_year = yy - 1900;

    // Устанавливаем часовой пояс
    putenv(timezone);
    tzset();

    return std::mktime(&tmStruct);
}

// Преобразуем в локальное время (учитывает часовой пояс!)
//std::time_t now = std::time(NULL);  // Получаем текущее время (UTC)

static std::string formatTime(std::time_t timeVal)
{
    std::tm* localTime = std::localtime(&timeVal);
    char buffer[20];

    if (localTime) {
        std::strftime(buffer, sizeof(buffer), "%H:%M:%S %d-%m-%Y", localTime);
        return std::string(buffer);
    }
    return "Ошибка!";
}



    static std::string getDateTime(std::time_t* dt_val, char* dt_tpl, bool is_localtime)
    {
      std::tm* tmStruct;
//std::time_t now = std::time(NULL);
        if(is_localtime)
          tmStruct = std::localtime(dt_val);  // с учётом часового пояса
        else
            tmStruct = std::gmtime(/*&now*/dt_val);
        char buffer[7];
        std::strftime(buffer, sizeof(buffer), dt_tpl/*"%d%m%y"*/, tmStruct);
        return std::string(buffer);
    }


    static std::string getCurrentDT(char* dt_tpl, bool is_localtime)
    {
      std::time_t now = std::time(NULL);
      return getDateTime(&now, dt_tpl, is_localtime);
    }


    // Функция получения текущего времени в формате HHMMSS
    static std::string getCurrentTimeHHMMSS(bool is_localtime)
    {
return getCurrentDT("%H%M%S", is_localtime);
    }

    // Функция получения текущей даты в формате DDMMYY
    static std::string getCurrentDateDDMMYY(bool is_localtime)
    {
return getCurrentDT("%d%m%y", is_localtime);
    }

    // Функция получения текущего времени в формате Wialon IPS
    static std::string getCurrentTime()
    {
return getCurrentDT("%Y-%m-%d %H:%M:%S", false);
    }

    // Функция для определения типа пакета
    static PacketType getPacketType(const std::string& prefix)
    {
        static std::map<std::string, PacketType> packetMap;
        if (packetMap.empty()) {
            packetMap["L"] = LOGIN;
            packetMap["SD"] = SHORT_DATA;
            packetMap["D"] = DATA;
            packetMap["P"] = PING;
            packetMap["B"] = BLACKBOX;
            packetMap["M"] = TXT_MSG;
            packetMap["US"] = BOOT_DATA; //
            packetMap["UC"] = CONFIG_DATA; // config / boot
        }
        //
        std::map<std::string, PacketType>::const_iterator it = packetMap.find(prefix);
        return (it != packetMap.end()) ? it->second : UNKNOWN;
    }

//##$#$#$#$#####################

    // Функция обработки пакета: возвращает данные без префикса и записывает тип пакета
    static data_t extractPacket(const data_t& input, PacketType& type)
    {
        size_t start = std::find(input.begin(), input.end(), '#') - input.begin();
        if (start >= input.size() - 1) {
            type = UNKNOWN;
            return data_t();
        }
        size_t secondHash = std::find(input.begin() + start + 1, input.end(), '#') - input.begin();
        if (secondHash >= input.size()) {
            type = UNKNOWN;
            return data_t();
        }

        std::string prefix(input.begin() + start + 1, input.begin() + secondHash);
        type = getPacketType(prefix);

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/* OK
size_t secondHash__ = std::find(input.begin() + start + 1,
                            input.end(), '\r') - input.begin();
size_t secondHash_2_ = std::find(input.begin() + start + 1,
                            input.end(), '\n') - input.begin();
*/
        const unsigned char CRLF[] = {'\r', '\n'};
        std::vector<unsigned char>::const_iterator it = std::search(
            input.begin() + start + 1, input.end(),
            CRLF, CRLF + 2
        );

        size_t endPos = (it != input.end()) ? (it - input.begin()) : std::string::npos;

        data_t packet_data = data_t(input.begin() + secondHash + 1, input.begin() + endPos);
        if (endPos >= input.size()) {
            return data_t();
        }
        return packet_data;
    }

    // Функция проверки типа пакета
    static bool isPacketType(PacketType expectedType, const data_t& packet) {
        PacketType detectedType;
        extractPacket(packet, detectedType);
        return detectedType == expectedType;
    }
};
/*
int main() {
    const char rawData1[] = "#SD#NA;NA;53.9000;27.5667;60;90;200;5\r\n";
    const char rawData2[] = "#D#1627468274;53.9001;27.5668;61;91;201;6\r\n";
    const char rawData3[] = "#P#\r\n";

    const char rawData3[] = "#L#123456789012345;2222\r\n";

    data_t data1(rawData1, rawData1 + sizeof(rawData1) - 1);
    data_t data2(rawData2, rawData2 + sizeof(rawData2) - 1);
    data_t data3(rawData3, rawData3 + sizeof(rawData3) - 1);

    WialonUtils::PacketType type1, type2, type3;
    data_t extracted1 = WialonUtils::extractPacket(data1, type1);
    data_t extracted2 = WialonUtils::extractPacket(data2, type2);
    data_t extracted3 = WialonUtils::extractPacket(data3, type3);

    std::cout << "Packet 1 is SD: " << WialonUtils::is_typeisPacketType(WialonUtils::SHORT_DATA, data1) << std::endl;
    std::cout << "Packet 2 is DATA: " << WialonUtils::is_typeisPacketType(WialonUtils::DATA, data2) << std::endl;
    std::cout << "Packet 3 is PING: " << WialonUtils::is_typeisPacketType(WialonUtils::PING, data3) << std::endl;

    std::cout << "Current Time (HHMMSS): " << WialonUtils::getCurrentTimeHHMMSS() << std::endl;
    std::cout << "Current Date (DDMMYY): " << WialonUtils::getCurrentDateDDMMYY() << std::endl;

    return 0;
}
*/

#endif
