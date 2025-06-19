// ============================================================================
// fix_1024_weak_error.h_ - ����������� �� RAD Studio 10.3.3
// ����������� �������� 1024 ������ � GPS �����������
// ============================================================================
// ������ ����� ������ Boost
// ============================================================================
#ifndef FIX_1024_WEAK_ERRORH
#define FIX_1024_WEAK_ERRORH

#include <System.hpp>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <string>
#include "car.h"
#include "gps.h"

namespace fix1024weaks
{
    // ����� �� �� ���������
    static const int kWEAKS1024_DAYS = 1024 * 7;     // 7168 ����
    static const int kDELTA_DAYS = 1024 * 7 - 1000;  // 6168 ����
    
    // ��������� ��� ������� ������������� ���
    struct simple_date {
        int year, month, day;
        
        simple_date(int y, int m, int d) : year(y), month(m), day(d) {}
        
        // ����������� � ��� �� ����� (1 ������ 1900 = ���� 0)
        int to_days() const {
            int total_days = 0;
            
            // ��������� ��� �� ������ ����
            for (int y = 1900; y < year; y++) {
                total_days += is_leap_year(y) ? 366 : 365;
            }
            
            // ��������� ��� �� ������ ������ �������� ����
            for (int m = 1; m < month; m++) {
                total_days += days_in_month(m, year);
            }
            
            // ��������� ��� �������� ������
            total_days += day - 1;  // -1 ������ ��� ���� ���������� � 1
            
            return total_days;
        }
        
        static simple_date from_days(int days) {
            int year = 1900;
            int month = 1;
            int day = 1;
            
            // ������� ���
            while (days >= (is_leap_year(year) ? 366 : 365)) {
                days -= is_leap_year(year) ? 366 : 365;
                year++;
            }
            
            // ������� �����
            while (days >= days_in_month(month, year)) {
                days -= days_in_month(month, year);
                month++;
            }
            
            // ���������� ��� = ���� ������
            day = days + 1;
            
            return simple_date(year, month, day);
        }
        
        simple_date operator+(int days) const {
            return from_days(to_days() + days);
        }
        
        simple_date operator-(int days) const {
            return from_days(to_days() - days);
        }
        
        bool operator<(const simple_date& other) const {
            return to_days() < other.to_days();
        }
        
        bool operator>(const simple_date& other) const {
            return to_days() > other.to_days();
        }
        
    private:
        static bool is_leap_year(int year) {
            return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        }
        
        static int days_in_month(int month, int year) {
            static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            if (month == 2 && is_leap_year(year)) return 29;
            return days[month];
        }
    };
    
    static const simple_date kMAXD(2036, 1, 1);
    static const simple_date kMIND(2035, 8, 19); // ~~6 april 1999
    
    static std::string strNum(int num, const std::string& str = "") {
        std::stringstream sss;
        std::string strr;
        sss << num;
        strr = str + sss.str();
        return strr;
    }
    
    static void printDate(const simple_date& d, const std::string& delim) {
        AddMessage(delim);
        char date_str[32];
        sprintf(date_str, "%04d-%02d-%02d", d.year, d.month, d.day);
        AddMessage(date_str);
        AddMessage("\n");
    }
    
    static bool needFixDate() {
        return params.local.fix_gps_1024_weaks;
    }
    
    static bool isSpecialFlagSet() {
        return params.local.fix_gps_tezey_device;
    }
    
    static bool isInRange(simple_date& d) {
        return (d < kMAXD) && (d > kMIND);
    }
    
    static void fixDate(fix_data& f) {
        if (!needFixDate())
            return;
        if (f.year == 0)  // arts
            return;
            
        simple_date d99(f.year, f.month, f.day);
        
        if (isSpecialFlagSet()) {
            if (isInRange(d99)) {
                d99 = simple_date(1999, f.month, f.day);
            }
        }
        
        // �������� ������� ����
        time_t now = time(nullptr);
        std::tm* tm_now = localtime(&now);
        simple_date today(tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
        
        simple_date xday = today - kDELTA_DAYS;
        
        if (d99 < xday) {
            simple_date actualDate = d99 + kWEAKS1024_DAYS;
            f.year = actualDate.year;
            f.month = actualDate.month;
            f.day = actualDate.day;
        }
    }
    
    static void fixDate(time_t& DateTime) {
        if (!needFixDate())
            return;
            
        std::tm* tm_dt = localtime(&DateTime);
        simple_date d99(tm_dt->tm_year + 1900, tm_dt->tm_mon + 1, tm_dt->tm_mday);
        
        // �������� ������� ����
        time_t now = time(nullptr);
        std::tm* tm_now = localtime(&now);
        simple_date today(tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
        
        simple_date xday = today - kDELTA_DAYS;
        
        if (d99 < xday) {
            simple_date actualDate = d99 + kWEAKS1024_DAYS;
            std::tm* res = localtime(&DateTime);
            res->tm_year = actualDate.year - 1900;
            res->tm_mon = actualDate.month - 1;  // ����� ��� � ���������
            res->tm_mday = actualDate.day;       // ����� ��� � ���������
            DateTime = mktime(res);
        }
    }
    
    static void fixOne(icar_polling::fix_packet& pkt) {
        /******debug******/
        fix_data my_var = pkt.fix;
        /**************/
        fixDate(pkt.fix);
    }
    
    static void fixOne1(cond_cache::trigger& pkt) {
        fixDate(pkt.datetime);
    }
    
    static void fixIt(std::vector<icar_polling::fix_packet>& vfix) {
        /******debug code**********/
        int sizee = vfix.size();
        for (int i = 0; i < sizee; i++) {
            fix_data my_var_ = vfix[i].fix;
            int abc = 12312321;
        }
        /***************************/
        
        // ����� ��� � ��������� - ���������� int ������ size_t ��� �������������
        for (int c = 0; c < vfix.size(); c++) {
            fixOne(vfix[c]);
        }
    }
    
    static void fixIt(std::vector<cond_cache::trigger>& vfix) {
        std::for_each(vfix.begin(), vfix.end(), fixOne1);
    }
}

#endif