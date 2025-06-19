// ============================================================================
// devlog.cpp - Мигрировано на RAD Studio 10.3.3
// Система логирования для устройств мониторинга
// ============================================================================
#include <System.hpp>
#include <vcl.h>
#pragma hdrstop
#include <Winapi.Windows.hpp>
#include <string>
#include "devlog.h"

DevLog::DevLog(const std::string& name, bool write_time, bool create_file)
{
    this->name = name;
    hFile = nullptr;  // RAD Studio 10.3.3: nullptr
    this->write_time = write_time;
    this->create_file = create_file;
    buf_mode = false;
    write_milliseconds = false;
    open();
}

DevLog::~DevLog()
{
    if (hFile == nullptr) return;  // RAD Studio 10.3.3: Проверка указателя
    
    try {
        fclose(hFile);
        hFile = nullptr;  // RAD Studio 10.3.3: Обнуление указателя
    }
    catch (...) {
        // RAD Studio 10.3.3: Безопасное закрытие файла
        hFile = nullptr;
    }
}

bool DevLog::open(const std::string& name)
{
    this->name = name;
    return open();
}

bool DevLog::open_cur_time(const std::string& pref, const std::string& post)
{
    write_time = false;
    create_file = true;
    
    SYSTEMTIME tm;
    GetLocalTime(&tm);
    
    char file[1024];
    sprintf_s(file, sizeof(file), "%02d%02d_%02d%02d%02d_%03d",  // RAD Studio 10.3.3: sprintf_s
              tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);

    std::string cur_name = pref + file;
    cur_name += post;

    int cnt = 0;
    FILE* f = nullptr;
    while (fopen_s(&f, cur_name.c_str(), "rb") == 0)  // RAD Studio 10.3.3: fopen_s
    {
        if (f != nullptr) {
            fclose(f);
            f = nullptr;
        }
        ++cnt;
        char tmp[256];
        sprintf_s(tmp, sizeof(tmp), "_%d", cnt);  // RAD Studio 10.3.3: sprintf_s
        cur_name = pref + file + tmp + post;
    }

    name = cur_name;
    return open();
}

void DevLog::set_buf_mode()
{
    buf_mode = true;
}

void DevLog::flush(bool save)
{
    if (!buf_mode) return;
    buf_mode = false;
    if (hFile == nullptr) return;  // RAD Studio 10.3.3: Проверка указателя
    
    if (save && !buffer.empty()) {  // RAD Studio 10.3.3: Проверка пустоты вектора
        fwrite(&buffer[0], 1, buffer.size(), hFile);  // RAD Studio 10.3.3: Безопасный доступ к данным
    }
    buffer.clear();
    if (save) fflush(hFile);
}

bool DevLog::open()
{
    if (hFile != nullptr) return true;  // RAD Studio 10.3.3: Проверка указателя
    if (name.empty()) return false;
    
    if (!create_file) {
        if (fopen_s(&hFile, name.c_str(), "rb") == 0) {  // RAD Studio 10.3.3: fopen_s
            if (hFile != nullptr) {
                fclose(hFile);
                hFile = nullptr;
            }
        }
    }
    
    if (hFile != nullptr || create_file) {  // RAD Studio 10.3.3: Проверка указателя
        if (fopen_s(&hFile, name.c_str(), "wb") != 0) {  // RAD Studio 10.3.3: fopen_s
            hFile = nullptr;
            return false;
        }
    }
    
    return hFile != nullptr;  // RAD Studio 10.3.3: Проверка указателя
}

void DevLog::close()
{
    if (hFile == nullptr) return;  // RAD Studio 10.3.3: Проверка указателя
    
    try {
        fclose(hFile);
        hFile = nullptr;  // RAD Studio 10.3.3: Обнуление указателя
    }
    catch (...) {
        // RAD Studio 10.3.3: Безопасное закрытие файла
        hFile = nullptr;
    }
}

void DevLog::write(const std::string& data)
{
    if (hFile == nullptr) return;  // RAD Studio 10.3.3: Проверка указателя
    
    SYSTEMTIME tm;
    GetLocalTime(&tm);
    
    if (write_time)
    {
        char time_buf[512];
        if (write_milliseconds)
            sprintf_s(time_buf, sizeof(time_buf), "%04d-%02d-%02d %02d:%02d:%02d:%03d ",  // RAD Studio 10.3.3: sprintf_s
                      tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
        else 
            sprintf_s(time_buf, sizeof(time_buf), "%04d-%02d-%02d %02d:%02d:%02d ",  // RAD Studio 10.3.3: sprintf_s
                      tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);

        if (buf_mode) 
            buffer.insert(buffer.end(), time_buf, time_buf + strlen(time_buf));
        else 
            fwrite(time_buf, 1, strlen(time_buf), hFile);
    }

    if (buf_mode) 
        buffer.insert(buffer.end(), data.c_str(), data.c_str() + data.size());
    else 
        fprintf(hFile, "%s", data.c_str());

    if (write_time)
    {
        const char* end_l = "\r\n";
        if (buf_mode)
            buffer.insert(buffer.end(), end_l, end_l + 2);
        else
            fprintf(hFile, "\r\n");
    }

    if (!buf_mode) fflush(hFile);
}

void DevLog::write(const char* data, int data_len)
{
    if (hFile == nullptr || data == nullptr || data_len <= 0) return;  // RAD Studio 10.3.3: Проверки
    
    SYSTEMTIME tm;
    GetLocalTime(&tm);
    
    if (write_time)
    {
        char time_buf[512];
        if (write_milliseconds)
            sprintf_s(time_buf, sizeof(time_buf), "%04d-%02d-%02d %02d:%02d:%02d:%03d ",  // RAD Studio 10.3.3: sprintf_s
                      tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
        else
            sprintf_s(time_buf, sizeof(time_buf), "%04d-%02d-%02d %02d:%02d:%02d ",  // RAD Studio 10.3.3: sprintf_s
                      tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);

        if (buf_mode)
            buffer.insert(buffer.end(), time_buf, time_buf + strlen(time_buf));
        else
            fwrite(time_buf, 1, strlen(time_buf), hFile);
    }

    if (buf_mode) 
        buffer.insert(buffer.end(), data, data + data_len);
    else 
        fwrite(data, 1, data_len, hFile);

    if (write_time)
    {
        const char* end_l = "\r\n";
        if (buf_mode)
            buffer.insert(buffer.end(), end_l, end_l + 2);
        else
            fprintf(hFile, "\r\n");
    }

    if (!buf_mode) fflush(hFile);
}