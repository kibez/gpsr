#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "shareddev.h"
#include "gps.h"
#include <libpokr/libpokrexp.h>
#include <stdexcept>


#include <string>

// RAD Studio compatibility
#ifdef __BORLANDC__
    #include <vcl.h>
    #include <Forms.hpp>
#else
    // Modern RAD Studio headers
    #include <System.hpp>
    #include <System.SysUtils.hpp>
    #include <System.AnsiStrings.hpp>
    
    // Legacy compatibility functions
    inline char* strupr(char* str) { return _strupr(str); }
    inline char* strlwr(char* str) { return _strlwr(str); }
    inline time_t bcb_time(time_t* timer) { return time(timer); }
    
    // timegm implementation for Windows
    time_t timegm(struct tm* tm) { return _mkgmtime(tm); }
    
    // AnsiString/String compatibility
    typedef System::AnsiString AnsiString;
    typedef System::UnicodeString String;
    
    // Legacy string conversion functions
    inline AnsiString IntToStr(int value) {
        return System::Sysutils::IntToStr(value).c_str();
    }
    
    inline String WideString(const wchar_t* str, int len) {
        return String(str, len / sizeof(wchar_t));
    }
#endif

#include <boost/lexical_cast.hpp>

char* strnstr(char* src, int len, char* ptrn)
{
    int ptlen = 0;
    int i;

    if (len == 0) return nullptr;
    ptlen = strlen(ptrn);

    len -= ptlen;
    if (len <= 0) return nullptr;

    for (i = 0; i < len; i++)
        if (strncmp(src + i, ptrn, ptlen) == 0) return src + i;

    return nullptr;
}

unsigned char calculate_check_sum(const char* str, int len)
{
    int i;
    unsigned char res = 0;

    for (i = 0; i < len; i++)
        res ^= str[i];

    return res;
}

unsigned char calculate_simple_sum(const char* str, int len)
{
    int i;
    unsigned char res = 0;

    for (i = 0; i < len; i++)
        res += str[i];

    return res;
}

char* FoundContentInStreamB(char* pContent, unsigned long uContentSize, char* szStart, char* szEnd, unsigned int* uTokenSize)
{
    char* pCur = pContent;
    char* pEnd = nullptr;
    char* pStart = nullptr;
    unsigned long uStart = 0;
    unsigned long uEnd = 0;

    uStart = strlen(szStart);
    uEnd = strlen(szEnd);
    *uTokenSize = 0;

    if (pContent == nullptr) return nullptr;
    if (uContentSize < uStart || uContentSize < uEnd) return nullptr;

    pStart = nullptr;
    for (pCur = pContent; pCur <= pContent + uContentSize - uStart; pCur++)
        if (*pCur == *szStart)
            if (!memcmp(pCur, szStart, uStart))
            {
                pStart = pCur;
                break;
            }
    if (!pStart) return nullptr;

    pEnd = nullptr;
    for (pCur = pStart + uStart; pCur <= pContent + uContentSize - uEnd; pCur++)
        if (*pCur == *szEnd)
            if (!memcmp(pCur, szEnd, uEnd))
            {
                pEnd = pCur;
                break;
            }
    if (!pEnd) return nullptr;

    *uTokenSize = pEnd - pStart - uStart;
    return pStart + uStart;
}

char* FoundContentInStream(char* pContent, unsigned long uContentSize, char* szStart, char* szEnd, unsigned int* uTokenSize)
{
    char* pCur = pContent;
    char* pEnd = nullptr;
    char* pStart = nullptr;
    unsigned long uStart = 0;
    unsigned long uEnd = 0;

    uStart = strlen(szStart);
    uEnd = strlen(szEnd);
    *uTokenSize = 0;

    if (pContent == nullptr) { *uTokenSize = 0; return nullptr; }

    if (uContentSize < uStart || uContentSize < uEnd) return nullptr;

    pStart = nullptr;
    for (pCur = pContent; pCur <= pContent + uContentSize - uStart; pCur++)
        if (*pCur == *szStart)
            if (!memcmp(pCur, szStart, uStart))
            {
                pStart = pCur;
                break;
            }
    if (!pStart) return nullptr;

    pEnd = nullptr;
    for (pCur = pStart + uStart; pCur <= pContent + uContentSize - uEnd; pCur++)
        if (*pCur == *szEnd)
            if (!memcmp(pCur, szEnd, uEnd))
            {
                pEnd = pCur;
                break;
            }
    if (!pEnd) return nullptr;

    *uTokenSize = pEnd + uEnd - pStart;
    return pStart;
}

char* FoundContentInStream2(char* pContent, unsigned long uContentSize,
    char* szStart1, char* szEnd1,
    char* szStart2, char* szEnd2,
    unsigned int* uTokenSize)
{
    char* token1 = nullptr;
    int token1_len = 0;

    char* token2 = nullptr;
    int token2_len = 0;

    *uTokenSize = 0;

    token1 = FoundContentInStream(pContent, uContentSize, szStart1, szEnd1, (unsigned int*)&token1_len);
    if (token1 == nullptr) return nullptr;

    token2 = FoundContentInStream(pContent + token1_len, uContentSize - token1_len, szStart2, szEnd2, (unsigned int*)&token2_len);
    if (token2 == nullptr) return nullptr;

    if (token2 != token1 + token1_len) return nullptr;

    *uTokenSize = token1_len + token2_len;
    return token1;
}

void mobitel_time2fix(unsigned int val, fix_data& fix)
{
    unsigned int cur;

    cur = val / 60;
    fix.second = val - cur * 60;
    val = cur;

    cur = val / 60;
    fix.minute = val - cur * 60;
    val = cur;

    cur = val / 24;
    fix.hour = val - cur * 24;
    val = cur;

    cur = val / 32;
    fix.day = val - cur * 32;
    val = cur;

    cur = val / 13;
    fix.month = val - cur * 13;
    val = cur;

    fix.year = val;
    fix.year += 2000;

    fix.date_valid = true;
}

void parse_time2fix(double parse_time, fix_data& fix)
{
    fix.hour = (int)parse_time / 10000;
    fix.minute = ((int)parse_time % 10000) / 100;
    fix.second = parse_time - fix.hour * 10000 - fix.minute * 100;
}

void parse_date2fix(double parse_date, fix_data& fix)
{
    fix.day = (int)parse_date / 10000;
    fix.month = ((int)parse_date % 10000) / 100;
    fix.year = parse_date - fix.day * 10000 - fix.month * 100;
    fix.year += 2000;
}

void degree2nmea(fix_data& fix)
{
    double latitude = fix.latitude;
    double longitude = fix.longitude;

    fix.latitude = (long)latitude + (latitude - (long)latitude) / 100 * 60;
    fix.longitude = (long)longitude + (longitude - (long)longitude) / 100 * 60;
    fix.latitude *= 100;
    fix.longitude *= 100;
}

void nmea2degree(fix_data& fix)
{
    double latitude = fix.latitude;
    double longitude = fix.longitude;

    latitude /= 100.0;
    longitude /= 100.0;
    fix.latitude = (long)latitude + (latitude - (long)latitude) / 60.0 * 100.0;
    fix.longitude = (long)longitude + (longitude - (long)longitude) / 60.0 * 100.0;
}

double mobitel2nmea(double val)
{
    val /= 10000;
    double degmin = floor(val);
    double sec = val - degmin;
    double deg = floor(degmin / 60);
    double min = degmin - deg * 60;
    return deg * 100 + min + sec;
}

//
//  GPS packet parsers
//

int parseGGA(struct fix_data& fix, char* token)
{
    const char* cur = token;
    int token_len = 0;
    int check_sum;
    char north_char = 0;
    char east_char = 0;
    char data_valid = 0;
    double parse_time = 0;

    fix_data f = fix;

    token_len = strlen(token);

    if (token_len >= 2 && strcmp(token + token_len - 2, "\r\n") == 0)
    {
        token[token_len - 2] = 0;
        token_len -= 2;
    }

    if (token_len < 10 || token[token_len - 3] != '*') goto parse_error;
    sscanf(token + token_len - 2, "%x", &check_sum);
    if (calculate_check_sum(token + 1, token_len - 4) != check_sum) goto checksum;
    //Big alpha
    strupr(token);
    if (!strncmp(token, "$GPGGA", sizeof("$GPGGA")))goto parse_error;

    // Read time
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%lf", &parse_time) != 1) goto parse_error;

    // Read latitude
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%lf", &f.latitude) != 1) goto parse_error;

    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%1c", &north_char) != 1 ||
        !(north_char == 'N' || north_char == 'S')) goto parse_error;
    if (north_char == 'S')f.latitude = -f.latitude;

    // Read longitude
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%lf", &f.longitude) != 1) goto parse_error;

    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%1c", &east_char) != 1 ||
        !(east_char == 'E' || east_char == 'W')) goto parse_error;
    if (east_char == 'W')f.longitude = -f.longitude;

    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%1c", &data_valid) != 1) goto parse_error;
    if (!(data_valid == '1' || data_valid == '2')) goto fix_error;

    // Get number of satellites,skip it
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    // Get HDOP,skip it
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    // Get Altitude
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    else if (*cur != ',')
    {
        if (sscanf(cur, "%lf", &f.height) != 1) goto parse_error;
        f.height_valid = true;
    }

    parse_time2fix(parse_time, f);
    nmea2degree(f);
    fix = f;

    return ERROR_GPS_FIX;
fix_error:
    return POLL_ERROR_GPS_NO_FIX;
checksum:
    AddMessage("GGA - check sum incorrect");
parse_error:
    return POLL_ERROR_PARSE_ERROR;
}

double knots2km(double val) { return val * 1.852; }

int parseRMC(struct fix_data& fix, char* token)
{
    char* cur = token;
    int token_len = 0;
    int check_sum;
    char data_valid = 0;
    char north_char = 0;
    char east_char = 0;
    double parse_time = 0;

    fix_data f = fix;

    token_len = strlen(token);

    if (token_len >= 2 && strcmp(token + token_len - 2, "\r\n") == 0)
    {
        token[token_len - 2] = 0;
        token_len -= 2;
    }

    if (token_len < 10) goto parse_error;

    if (token[token_len - 3] != '*') goto parse_error;
    sscanf(token + token_len - 2, "%x", &check_sum);
    if (calculate_check_sum(token + 1, token_len - 4) != check_sum) goto checksum;

    //Big alpha
    strupr(token);
    if (!strncmp(token, "$GPRMC", sizeof("$GPRMC")))goto parse_error;

    // Read time
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0)goto parse_error;

    if (*cur != ',' && sscanf(cur, "%lf", &parse_time) != 1) goto parse_error;

    // Check is data valid?
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%1c", &data_valid) != 1) goto parse_error;
    if (data_valid != 'A') goto fix_error;
    if (parse_time == 0)goto parse_error;

    // Read latitude
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%lf", &f.latitude) != 1) goto parse_error;

    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%1c", &north_char) != 1 ||
        !(north_char == 'N' || north_char == 'S')) goto parse_error;
    if (north_char == 'S')f.latitude = -f.latitude;

    // Read longitude
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%lf", &f.longitude) != 1) goto parse_error;

    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%1c", &east_char) != 1 ||
        !(east_char == 'E' || east_char == 'W')) goto parse_error;
    if (east_char == 'W')f.longitude = -f.longitude;

    // Get SOG
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    else if (*cur != ',')
    {
        if (sscanf(cur, "%lf", &f.speed) != 1) goto parse_error;
        f.speed_valid = true;
        f.speed = knots2km(f.speed);
    }

    // Read COG
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    else if (*cur != ',')
    {
        if (sscanf(cur, "%lf", &f.course) != 1) goto parse_error;
        f.course_valid = true;
    }

    // Read date
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    else if (*cur != ',')
    {
        double date = 0;
        if (sscanf(cur, "%lf", &date) != 1) goto parse_error;
        parse_date2fix(date, f);
        f.date_valid = true;
    }

    parse_time2fix(parse_time, f);
    nmea2degree(f);
    fix = f;

    return ERROR_GPS_FIX;
fix_error:
    return POLL_ERROR_GPS_NO_FIX;
checksum:
    AddMessage("RMC - check sum incorrect");
parse_error:
    return POLL_ERROR_PARSE_ERROR;
}

int parseShortRMC(struct fix_data& fix, std::string token)
{
    std::string::size_type pos = token.find(',');
    if (pos == std::string::npos) return POLL_ERROR_PARSE_ERROR;
    token.insert(pos, ",A");
    token = "$GPRMC," + token;
    return parseRMC(fix, (char*)token.c_str());
}

int parseGLL(struct fix_data& fix, char* token)
{
    const char* cur = token;
    int token_len = 0;
    int check_sum;
    char north_char = 0;
    char east_char = 0;
    char data_valid = 0;
    double parse_time = 0;

    fix_data f = fix;

    token_len = strlen(token);

    if (token_len >= 2 && strcmp(token + token_len - 2, "\r\n") == 0)
    {
        token[token_len - 2] = 0;
        token_len -= 2;
    }

    if (token_len < 10 || token[token_len - 3] != '*') goto parse_error;
    sscanf(token + token_len - 2, "%x", &check_sum);
    if (calculate_check_sum(token + 1, token_len - 4) != check_sum) goto checksum;
    //Big alpha
    strupr(token);
    if (!strncmp(token, "$GPGLL", sizeof("$GPGLL")))goto parse_error;

    // Read latitude
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%lf", &f.latitude) != 1) goto parse_error;

    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%1c", &north_char) != 1 ||
        !(north_char == 'N' || north_char == 'S')) goto parse_error;
    if (north_char == 'S')f.latitude = -f.latitude;

    // Read longitude
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%lf", &f.longitude) != 1) goto parse_error;

    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%1c", &east_char) != 1 ||
        !(east_char == 'E' || east_char == 'W')) goto parse_error;
    if (east_char == 'W')f.longitude = -f.longitude;

    // Read time
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%lf", &parse_time) != 1) goto parse_error;

    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%1c", &data_valid) != 1) goto parse_error;
    if (!(data_valid == 'A')) goto fix_error;

    parse_time2fix(parse_time, f);
    nmea2degree(f);
    fix = f;

    return ERROR_GPS_FIX;
fix_error:
    return POLL_ERROR_GPS_NO_FIX;
checksum:
    AddMessage("GLL - check sum incorrect");
parse_error:
    return POLL_ERROR_PARSE_ERROR;
}

int parseVTG(struct fix_data& fix, char* token)
{
    char* cur = token;
    int token_len = 0;
    int check_sum;

    fix_data f = fix;

    token_len = strlen(token);

    if (token_len >= 2 && strcmp(token + token_len - 2, "\r\n") == 0)
    {
        token[token_len - 2] = 0;
        token_len -= 2;
    }

    if (token_len < 10 || token[token_len - 3] != '*') goto parse_error;
    sscanf(token + token_len - 2, "%x", &check_sum);
    if (calculate_check_sum(token + 1, token_len - 4) != check_sum) goto checksum;
    //Big alpha
    strupr(token);
    if (!strncmp(token, "$GPVTG", sizeof("$GPVTG")))goto parse_error;

    // Read COG
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    else if (*cur != ',')
    {
        if (sscanf(cur, "%lf", &f.course) != 1) goto parse_error;
        f.course_valid = true;
    }

    // Read COG orientation skip
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    // Read COG magnetic skip
    //
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    // Read COG magnetic orientation skip
    //
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    // Read SOG knots skip
    //
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    // Read SOG knots units skip
    //
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0) goto parse_error;
    // Get SOG
    cur = strchr(cur, ',');
    if (cur == nullptr) goto parse_error;
    else if (*(++cur) != 0 && *cur != '*')
    {
        if (sscanf(cur, "%lf", &f.speed) != 1) goto parse_error;
        f.speed_valid = true;
    }

    fix = f;
    return ERROR_GPS_FIX;
checksum:
    AddMessage("VTG - check sum incorrect");
parse_error:
    return POLL_ERROR_PARSE_ERROR;
}

int parseZDA(struct fix_data& fix, char* token)
{
    char* cur = token;
    int token_len = 0;
    int check_sum;
    int date_part;
    double parse_time;

    fix_data f = fix;

    token_len = strlen(token);

    if (token_len >= 2 && strcmp(token + token_len - 2, "\r\n") == 0)
    {
        token[token_len - 2] = 0;
        token_len -= 2;
    }

    if (token_len < 10 || token[token_len - 3] != '*') goto parse_error;
    sscanf(token + token_len - 2, "%x", &check_sum);
    if (calculate_check_sum(token + 1, token_len - 4) != check_sum) goto checksum;
    //Big alpha
    strupr(token);
    if (!strncmp(token, "$GPZDA", sizeof("$GPZDA")))goto parse_error;

    // Read Time
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%lf", &parse_time) != 1) goto parse_error;

    // Read day
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%d", &fix.day) != 1) goto parse_error;
    // Read month
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%d", &fix.month) != 1) goto parse_error;
    // Read year
    cur = strchr(cur, ',');
    if (cur == nullptr || *(++cur) == 0 || sscanf(cur, "%d", &fix.year) != 1) goto parse_error;
    fix.year += 2000;

    parse_time2fix(parse_time, f);
    nmea2degree(f);
    f.date_valid = true;
    fix = f;

    return ERROR_GPS_FIX;
checksum:
    AddMessage("ZDA - check sum incorrect");
parse_error:
    return POLL_ERROR_PARSE_ERROR;
}

void hex2bin(const char* str, unsigned char* bin)
{
    int len = strlen(str);
    int i;
    for (i = 0; i < len; i += 2)
    {
        int val;
        sscanf(str + i, "%2x", &val);
        bin[i / 2] = val;
    }
}

void bin2hex(const unsigned char* bin, int len, char* str)
{
    int i;
    for (i = 0; i < len; i++) snprintf(str + i * 2, 3, "%02X", bin[i]);
}

void seven2eight(const unsigned char* buf, unsigned ud_len, char* ud)
{
    int i;
    int buf_len = ud_len * 7 / 8;
    if (ud_len % 8)buf_len++;
    memset(ud, 0, ud_len);

    for (i = 0; i < buf_len; i++)
    {
        int pos = i * 8 / 7;
        int count = i % 7;
        int main_mask = (1 << (7 - count)) - 1;
        int second_count = 7 - count;
        int second_mask = (1 << (8 - second_count)) - 1;

        ud[pos] |= (buf[i] & main_mask) << count;
        ud[pos + 1] |= (buf[i] >> second_count) & second_mask;
    }
}

int eight2seven(const unsigned char* ud, unsigned ud_len, unsigned char* buf)
{
    unsigned i;
    unsigned buf_len = ud_len * 7 / 8;
    if (ud_len % 8)buf_len++;

    for (i = 0; i < buf_len; i++)
    {
        unsigned pos = i * 8 / 7;
        unsigned count = i % 7;

        unsigned main_mask = (1 << (7 - count)) - 1;
        unsigned second_count = 7 - count;
        unsigned second_mask = (1 << (8 - second_count)) - 1;

        buf[i] = (ud[pos] >> count) & main_mask;
        if (pos + 1 < ud_len)buf[i] |= (ud[pos + 1] & second_mask) << second_count;
    }

    return buf_len;
}

unsigned char tab_CRC_15[] = {
    0x00,0x00,0xC7,0x56,0x99,0x09,0x5E,0x5F,
    0x32,0x13,0xF5,0x45,0xAB,0x1A,0x6C,0x4C,
    0x64,0x26,0xA3,0x70,0xFD,0x2F,0x3A,0x79,
    0x56,0x35,0x91,0x63,0xCF,0x3C,0x08,0x6A,
    0xC8,0x4C,0x0F,0x1A,0x51,0x45,0x96,0x13,
    0xFA,0x5F,0x3D,0x09,0x63,0x56,0xA4,0x00,
    0xAC,0x6A,0x6B,0x3C,0x35,0x63,0xF2,0x35,
    0x9E,0x79,0x59,0x2F,0x07,0x70,0xC0,0x26,
    0x87,0x3D,0x40,0x6B,0x1E,0x34,0xD9,0x62,
    0xB5,0x2E,0x72,0x78,0x2C,0x27,0xEB,0x71,
    0xE3,0x1B,0x24,0x4D,0x7A,0x12,0xBD,0x44,
    0xD1,0x08,0x16,0x5E,0x48,0x01,0x8F,0x57,
    0x4F,0x71,0x88,0x27,0xD6,0x78,0x11,0x2E,
    0x7D,0x62,0xBA,0x34,0xE4,0x6B,0x23,0x3D,
    0x2B,0x57,0xEC,0x01,0xB2,0x5E,0x75,0x08,
    0x19,0x44,0xDE,0x12,0x80,0x4D,0x47,0x1B,
    0x0E,0x7B,0xC9,0x2D,0x97,0x72,0x50,0x24,
    0x3C,0x68,0xFB,0x3E,0xA5,0x61,0x62,0x37,
    0x6A,0x5D,0xAD,0x0B,0xF3,0x54,0x34,0x02,
    0x58,0x4E,0x9F,0x18,0xC1,0x47,0x06,0x11,
    0xC6,0x37,0x01,0x61,0x5F,0x3E,0x98,0x68,
    0xF4,0x24,0x33,0x72,0x6D,0x2D,0xAA,0x7B,
    0xA2,0x11,0x65,0x47,0x3B,0x18,0xFC,0x4E,
    0x90,0x02,0x57,0x54,0x09,0x0B,0xCE,0x5D,
    0x89,0x46,0x4E,0x10,0x10,0x4F,0xD7,0x19,
    0xBB,0x55,0x7C,0x03,0x22,0x5C,0xE5,0x0A,
    0xED,0x60,0x2A,0x36,0x74,0x69,0xB3,0x3F,
    0xDF,0x73,0x18,0x25,0x46,0x7A,0x81,0x2C,
    0x41,0x0A,0x86,0x5C,0xD8,0x03,0x1F,0x55,
    0x73,0x19,0xB4,0x4F,0xEA,0x10,0x2D,0x46,
    0x25,0x2C,0xE2,0x7A,0xBC,0x25,0x7B,0x73,
    0x17,0x3F,0xD0,0x69,0x8E,0x36,0x49,0x60,

    0x0B,0x52,0xCC,0x04,0x92,0x5B,0x55,0x0D,
    0x39,0x41,0xFE,0x17,0xA0,0x48,0x67,0x1E,
    0x6F,0x74,0xA8,0x22,0xF6,0x7D,0x31,0x2B,
    0x5D,0x67,0x9A,0x31,0xC4,0x6E,0x03,0x38,
    0xC3,0x1E,0x04,0x48,0x5A,0x17,0x9D,0x41,
    0xF1,0x0D,0x36,0x5B,0x68,0x04,0xAF,0x52,
    0xA7,0x38,0x60,0x6E,0x3E,0x31,0xF9,0x67,
    0x95,0x2B,0x52,0x7D,0x0C,0x22,0xCB,0x74,
    0x8C,0x6F,0x4B,0x39,0x15,0x66,0xD2,0x30,
    0xBE,0x7C,0x79,0x2A,0x27,0x75,0xE0,0x23,
    0xE8,0x49,0x2F,0x1F,0x71,0x40,0xB6,0x16,
    0xDA,0x5A,0x1D,0x0C,0x43,0x53,0x84,0x05,
    0x44,0x23,0x83,0x75,0xDD,0x2A,0x1A,0x7C,
    0x76,0x30,0xB1,0x66,0xEF,0x39,0x28,0x6F,
    0x20,0x05,0xE7,0x53,0xB9,0x0C,0x7E,0x5A,
    0x12,0x16,0xD5,0x40,0x8B,0x1F,0x4C,0x49,
    0x05,0x29,0xC2,0x7F,0x9C,0x20,0x5B,0x76,
    0x37,0x3A,0xF0,0x6C,0xAE,0x33,0x69,0x65,
    0x61,0x0F,0xA6,0x59,0xF8,0x06,0x3F,0x50,
    0x53,0x1C,0x94,0x4A,0xCA,0x15,0x0D,0x43,
    0xCD,0x65,0x0A,0x33,0x54,0x6C,0x93,0x3A,
    0xFF,0x76,0x38,0x20,0x66,0x7F,0xA1,0x29,
    0xA9,0x43,0x6E,0x15,0x30,0x4A,0xF7,0x1C,
    0x9B,0x50,0x5C,0x06,0x02,0x59,0xC5,0x0F,
    0x82,0x14,0x45,0x42,0x1B,0x1D,0xDC,0x4B,
    0xB0,0x07,0x77,0x51,0x29,0x0E,0xEE,0x58,
    0xE6,0x32,0x21,0x64,0x7F,0x3B,0xB8,0x6D,
    0xD4,0x21,0x13,0x77,0x4D,0x28,0x8A,0x7E,
    0x4A,0x58,0x8D,0x0E,0xD3,0x51,0x14,0x07,
    0x78,0x4B,0xBF,0x1D,0xE1,0x42,0x26,0x14,
    0x2E,0x7E,0xE9,0x28,0xB7,0x77,0x70,0x21,
    0x1C,0x6D,0xDB,0x3B,0x85,0x64,0x42,0x32
};

int crc_16(const unsigned char* data, int data_len)
{
    unsigned char Crc_Low = 0;
    unsigned char Crc_Hi = 0;
    int i;

    for (i = 0; i < data_len; i++)
    {
        unsigned char index;
        unsigned char gen0, gen1;
        unsigned char C_byte = data[i];
        index = C_byte ^ Crc_Low;
        gen1 = tab_CRC_15[index * 2];
        gen0 = tab_CRC_15[index * 2 + 1];
        Crc_Low = (gen1 ^ Crc_Hi);
        Crc_Hi = gen0;
    }

    return Crc_Hi * 0x100 + Crc_Low;
}

void* build_fix_result(int obj_id, const fix_data& fix)
{
    void* data = pkr_create_struct();
    if (!build_fix_result(data, obj_id, fix))
    {
        pkr_free_result(data);
        return nullptr;
    }
    return data;
}

bool build_fix_result(void*& data, int obj_id, const fix_data& fix)
{
    if (fix.speed > 1000)
    {
        AddMessage("invalid speed");
        return false;
    }

    double time = fix_time(fix);
    if (time == 0)
    {
        AddMessage("invalid fix time");
        return false;
    }

    data = pkr_add_int(data, "obj_id", obj_id);
    data = pkr_add_double(data, "datetime", time);
    data = pkr_add_double(data, "latitude", fix.latitude);
    data = pkr_add_double(data, "longitude", fix.longitude);

    if (fix.height_valid)data = pkr_add_double(data, "altitude", fix.height);
    if (fix.course_valid)data = pkr_add_double(data, "course", fix.course);
    if (fix.speed_valid)data = pkr_add_double(data, "speed", fix.speed);

    return true;
}

void fix2common_fix(const fix_data& from, VisiPlug::fix_data_t& to)
{
    to.datetime = fix_time(from);
    if (to.datetime == 0)to.fix_valid = false;

    to.latitude = from.latitude;
    to.longitude = from.longitude;

    to.altitude_valid = from.height_valid;
    to.altitude = from.height;
    to.course_valid = from.course_valid;
    to.course = from.course;
    to.speed_valid = from.speed_valid;
    to.speed = from.speed;
}

int max_day_in_month(int month, int year)
{
    switch (month)
    {
    case 0:
    case 2:
    case 4:
    case 6:
    case 7:
    case 9:
    case 11:
        return 31;
    case 3:
    case 5:
    case 8:
    case 10:
        return 30;
    }

    if ((year % 4) == 0)return 29;
    return 28;
}

void prev_day(tm& tt)
{
    --tt.tm_mday;
    if (tt.tm_mday > 0)return;
    --tt.tm_mon;

    if (tt.tm_mon < 0)
    {
        tt.tm_mon = 11;
        --tt.tm_year;
    }

    tt.tm_mday = max_day_in_month(tt.tm_mon, tt.tm_year);
}

void next_day(tm& tt)
{
    int max_day = max_day_in_month(tt.tm_mon, tt.tm_year);

    ++tt.tm_mday;
    if (tt.tm_mday <= max_day)return;
    tt.tm_mday = 1;

    ++tt.tm_mon;
    if (tt.tm_mon <= 11)return;
    tt.tm_mon = 0;
    ++tt.tm_year;
}

bool time_valid(time_t t)
{
    return t > 0 && t < bcb_time(0) + 3600 * 3;
}

double fix_time(const fix_data& fix)
{
    if (fix.utc_time_valid)
    {
        //время не корректно или в будующем на 3 часа
        if (time_valid(fix.utc_time))return fix.utc_time;
        return 0;
    }

    if (fix.date_valid)
    {
        tm tt;
        tt.tm_sec = (int)fix.second;
        tt.tm_min = fix.minute;
        tt.tm_hour = fix.hour;

        tt.tm_mday = fix.day;
        tt.tm_mon = fix.month - 1;
        tt.tm_year = fix.year - 1900;
        time_t t = timegm(&tt);

        if (!time_valid(t))return 0;
        return t + fix.second - (int)fix.second;
    }

    time_t cur_t = bcb_time(0);

    tm now_tt = *gmtime(&cur_t);
    now_tt.tm_sec = (int)fix.second;
    now_tt.tm_min = fix.minute;
    now_tt.tm_hour = fix.hour;
    time_t now_t = timegm(&now_tt);

    tm prev_tt = now_tt;
    prev_day(prev_tt);
    time_t prev_t = timegm(&prev_tt);

    tm next_tt = now_tt;
    next_day(next_tt);
    time_t next_t = timegm(&next_tt);

    time_t t = now_t;

    if (time_valid(prev_t) && (!time_valid(t) || labs(t - cur_t) > labs(prev_t - cur_t)))
        t = prev_t;

    if (time_valid(next_t) && (!time_valid(t) || labs(t - cur_t) > labs(next_t - cur_t)))
        t = next_t;

    if (!time_valid(t))return 0;
    return t + fix.second - (int)fix.second;
}

//KIBEZ int build_number(const std::string phone, std::vector<unsigned char>& vdata)
int build_number(std::string phone, std::vector<unsigned char>& vdata)
{
    if (phone.size() > 20)return ERROR_INFORMATOR_NOT_SUPPORT;

    int cur = 0;
    char data[128];
    unsigned char* pdata = (unsigned char*)(data + 2);
    const char* number = phone.c_str();

    if (number[0] == '+')
    {
        data[1] = (char)0x91;
        number++;
    }
    else data[1] = (char)0x81;

    for (cur = 0; number[cur]; cur++)
    {
        int digit;
        sscanf(number + cur, "%1d", &digit);
        if (cur % 2)pdata[cur / 2] = (pdata[cur / 2] & 0xF) + digit * 0x10;
        else pdata[cur / 2] = (pdata[cur / 2] & 0xF0) + digit;
    }
    if (cur % 2)pdata[cur / 2] = (pdata[cur / 2] & 0xF) + 0xF0;
    data[0] = (char)cur;

    int len = cur / 2 + cur % 2 + 2;
    vdata.insert(vdata.end(), data, data + len);
    return POLL_ERROR_NO_ERROR;
}

bool parse_gsm_number(unsigned char* data, unsigned int len, std::string& num)
{
    std::vector<char> vcur(len * 2 + 128);//если sprintf вдруг сломается
    char* cur = &*vcur.begin();

    for (unsigned int i = 0; i < len; i++)
    {
        unsigned val = data[i];
        unsigned lo_digit = (val >> 4) & 0xF;
        unsigned hi_digit = val & 0xF;

        if (lo_digit != 0xF && lo_digit > 9 || hi_digit > 9) return false;

        if (lo_digit == 0xF)
        {
            sprintf(cur, "%1d", hi_digit);
            break;
        }
        else sprintf(cur, "%1d%1d", hi_digit, lo_digit);
        cur += 2;
    }

    num = &*vcur.begin();
    return true;
}

//Используется в GSM модеме
time_t localtime(tm& t, int time_zone)
{
    t.tm_isdst = -1;
    mktime(&t);
    tm convtm = t;

    int tzone = time_zone + _timezone;

    int tsecond = tzone % 60;
    int thour = tzone / 3600;
    int tminute = (tzone - thour * 3600) / 60;

    convtm.tm_hour -= thour;
    convtm.tm_min -= tminute;
    convtm.tm_sec -= tsecond;
    convtm.tm_isdst = -1;
    return mktime(&convtm);
}

std::vector<unsigned char> gsm2iso_alphabet(const std::vector<unsigned char>& g)
{
    static unsigned char tbl[128] =
    {
        64,163,36,165,       232,233,249,236,      242,199,10,216,      248,13,197,229,
        0x10,95,0x12,0x13,   0x14,0x15,0x16,0x17,  0x18,0x19,0x1A,0x1B, 198,230,223,201,
        0x20,0x21,0x22,0x23, 164,0x25,0x26,0x27,   0x28,0x29,0x2A,0x2B, 0x2C,0x2D,0x2E,0x2F,
        0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37,  0x38,0x39,0x3A,0x3B, 0x3C,0x3D,0x3E,0x3F,

        161,0x41,0x42,0x43,  0x44,0x45,0x46,0x47,  0x48,0x49,0x4A,0x4B, 0x4C,0x4D,0x4E,0x4F,
        0x50,0x51,0x52,0x53, 0x54,0x55,0x56,0x57,  0x58,0x59,0x5A,196,  214,209,220,167,
        191,0x61,0x62,0x63,  0x64,0x65,0x66,0x67,  0x68,0x69,0x6A,0x6B, 0x6C,0x6D,0x6E,0x6F,
        0x70,0x71,0x72,0x73, 0x74,0x75,0x76,0x77,  0x78,0x79,0x7A,228,  246,241,252,224
    };

    std::vector<unsigned char> ret;

    for (std::vector<unsigned char>::const_iterator i = g.begin(); i != g.end(); ++i)
    {
        const unsigned char& c = *i;
        if (c >= sizeof(tbl))
        {
            ret.push_back(c);
            continue;
        }
        if (c == 27 && (i + 1) != g.end())
        {
            ++i;
            switch (*i)
            {
            case 10:ret.push_back(12); break;
            case 20:ret.push_back(94); break;
            case 40:ret.push_back(123); break;
            case 41:ret.push_back(125); break;
            case 47:ret.push_back(92); break;
            case 60:ret.push_back(91); break;
            case 61:ret.push_back(126); break;
            case 62:ret.push_back(93); break;
            case 64:ret.push_back(124); break;
            case 101:ret.push_back(164); break;
            default:
                ret.push_back(27);
                --i;
            }
        }
        else ret.push_back(tbl[c]);
    }

    return ret;
}

std::vector<unsigned char> iso2gsm_alphabet(const std::vector<unsigned char>& g)
{
    static unsigned char tbl[256] =
    {
        0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,  0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F,
        0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17,  0x18,0x19,0x1A,0x1B, 0x1C,0x1D,0x1E,0x1F,
        0x20,0x21,0x22,0x23, 2   ,0x25,0x26,0x27,  0x28,0x29,0x2A,0x2B, 0x2C,0x2D,0x2E,0x2F,
        0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37,  0x38,0x39,0x3A,0x3B, 0x3C,0x3D,0x3E,0x3F,

        0   ,0x41,0x42,0x43, 0x44,0x45,0x46,0x47,  0x48,0x49,0x4A,0x4B, 0x4C,0x4D,0x4E,0x4F,
        0x50,0x51,0x52,0x53, 0x54,0x55,0x56,0x57,  0x58,0x59,0x5A,0x5B, 0x5C,0x5D,0x5E,0x11,
        0x60,0x61,0x62,0x63, 0x64,0x65,0x66,0x67,  0x68,0x69,0x6A,0x6B, 0x6C,0x6D,0x6E,0x6F,
        0x70,0x71,0x72,0x73, 0x74,0x75,0x76,0x77,  0x78,0x79,0x7A,0x7B, 0x7C,0x7D,0x7E,0x7F,

        0x80,0x81,0x82,0x83, 0x84,0x85,0x86,0x87,  0x88,0x89,0x8A,0x8B, 0x8C,0x8D,0x8E,0x8F,
        0x90,0x91,0x92,0x93, 0x94,0x95,0x96,0x97,  0x98,0x99,0x9A,0x9B, 0x9C,0x9D,0x9E,0x9F,
        0xA0,0x40,0xA2,1   , 0x24,3   ,0xA6,0x5f,  0xA8,0xA9,0xAA,0xAB, 0xAC,0xAD,0xAE,0xAF,
        0xB0,0xB1,0xB2,0xB3, 0xB4,0xB5,0xB6,0xB7,  0xB8,0xB9,0xBA,0xBB, 0xBC,0xBD,0xBE,0x60,

        0xC0,0xC1,0xC2,0xC3, 0x5b,0x0E,0x1C,9   ,  0xC8,0x1f,0xCA,0xCB, 0xCC,0xCD,0xCE,0xCF,
        0xD0,0x5d,0xD2,0xD3, 0xD4,0xD5,0x5c,0xD7,  0x0B,0xD9,0xDA,0xDB, 0x5e,0xDD,0xDE,0x1e,
        0x7f,0xE1,0xE2,0xE3, 0x7b,0x0F,0x1d,0xE7,  4   ,5   ,0xEA,0xEB, 7   ,0xED,0xEE,0xEF,
        0xF0,0x7d,8   ,0xF3, 0xF4,0xF5,0x7c,0xF7,  0x0c,6   ,0xFA,0xFB, 0x7e,0xFD,0xFE,0xFF,
    };

    std::vector<unsigned char> ret;

    for (std::vector<unsigned char>::const_iterator i = g.begin(); i != g.end(); ++i)
    {
        const unsigned char& c = *i;
        switch (c)
        {
        case 12:
        case 94:
        case 123:
        case 125:
        case 92:
        case 91:
        case 126:
        case 93:
        case 124:
        case 164:
            ret.push_back(27);
        }

        switch (c)
        {
        case 12:ret.push_back(10); break;
        case 94:ret.push_back(20); break;
        case 123:ret.push_back(40); break;
        case 125:ret.push_back(41); break;
        case 92:ret.push_back(47); break;
        case 91:ret.push_back(60); break;
        case 126:ret.push_back(61); break;
        case 93:ret.push_back(62); break;
        case 124:ret.push_back(64); break;
        case 164:ret.push_back(101); break;
        default:
            ret.push_back(tbl[c]);
        }
    }

    return ret;
}

//
//  bin_reader
//
void bin_reader::read(unsigned char* dst, unsigned count)
{
    if (m_pos + count > data.size())
    {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "bin_reader::read(): invalid position=%u count=%u data.size()=%u",
                m_pos, count, (unsigned int)data.size());
        throw std::runtime_error(error_msg);
    }

    std::copy(data.begin() + m_pos, data.begin() + m_pos + count, dst);
    m_pos += count;
}
