// ============================================================================
// 3. devlog.h - Система логирования
// ============================================================================
#ifndef devlogH
#define devlogH

#include <System.hpp>
#include <stdio.h>
#include <string>
#include <vector>

class NullLog {
public:
    bool write_time;
    bool write_milliseconds;
    bool create_file;
    NullLog(const std::string& name = std::string(), bool _write_time = true, bool _create_file = false) : create_file(_create_file), write_time(_write_time) { write_milliseconds = false; }
    bool open(const std::string& name) { return true; }
    void write(const std::string& data) {;}
    void write(const char* data, int data_len) {;}
    void write(const unsigned char* data, int data_len) {;}
    void close() {;}
};

class DevLog
{
private:
    FILE* hFile;
    std::string name;
    std::string file;
public:
    bool write_time;
    bool create_file;
    bool write_milliseconds;
private:

    DevLog(const DevLog& r);
    DevLog& operator =(const DevLog& r);

    std::vector<char> buffer;
    bool buf_mode;
public:
    DevLog(const std::string& name = std::string(), bool write_time = true, bool create_file = false);
    ~DevLog();

    bool open(const std::string& name);
    bool open_cur_time(const std::string& pref, const std::string& post);
    bool open();
    void close();

    void set_buf_mode();
    void flush(bool save = false);

    void write(const std::string& data);
    void write(const char* data, int data_len);

    inline void write(const unsigned char* data, int data_len) { write((const char*)data, data_len); }
};

#endif