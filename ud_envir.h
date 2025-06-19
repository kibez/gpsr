// ============================================================================
// ud_envir.h_ - ����������� �� RAD Studio 10.3.3
// ��������� ��������� ������ ������ �� ���������
// ============================================================================
#ifndef ud_envirH
#define ud_envirH

#include <System.hpp>
#include <string>
#include <time.h>

// ��������� ������ - ���������� ����� ��� ���������� ���������
struct ud_envir
{
    // ����� ����� �����, ���� number_valid
    std::string number;
    bool number_valid;

    // �����, ����� ����� ���� timestamp_valid
    // ����������� �������� �������� ������� SMS �� SMS �����
    time_t timestamp;
    bool timestamp_valid;

    // ip ����� ����� �����, ���� ip_valid
    unsigned char ip[4];
    bool ip_valid;
    unsigned short ip_port;
    bool ip_udp;

    ud_envir();
};

#endif