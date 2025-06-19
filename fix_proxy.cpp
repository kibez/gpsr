// ============================================================================
// fix_proxy.cpp - Мигрировано на RAD Studio 10.3.3
// Исправленная версия для работы с WinSock
// ============================================================================

#include <System.hpp>
#pragma hdrstop
#include "fix_proxy.h"
#include "pokr\dbgprn.h"

#pragma comment(lib, "ws2_32.lib")

std::vector<TFixProxy*> TFixProxy::FDeleteLeter = std::vector<TFixProxy*>();

TFixProxy::TFixProxy(const std::string& packet, const char* addr, unsigned short port) :
    TThread(true),
    tcp_socket(INVALID_SOCKET),
    host(addr),
    port(port),
    FPacket(packet.c_str())
{
    while (FDeleteLeter.size())
    {
        TFixProxy* fp = FDeleteLeter.front();
        delete fp;
        FDeleteLeter.erase(FDeleteLeter.begin());
    }
}

__fastcall TFixProxy::~TFixProxy()
{
    if (tcp_socket != INVALID_SOCKET)
    {
        closesocket(tcp_socket);
        tcp_socket = INVALID_SOCKET;
    }
}

void TFixProxy::send(const std::string& prefix)
{
    FPrefix = prefix;
    Resume();
}

// Функция для резолва имени хоста в IP
unsigned long ResolveHostname(const std::string& hostname)
{
    // Сначала пробуем как IP адрес
    unsigned long addr = inet_addr(hostname.c_str());
    if (addr != INADDR_NONE)
        return addr;

    // Если не IP, то резолвим как hostname
    hostent* host_entry = gethostbyname(hostname.c_str());
    if (host_entry)
        return *((unsigned long*)host_entry->h_addr_list[0]);

    return INADDR_NONE;
}

// Функция для чтения строки до \r\n (аналог ReadLn)
std::string ReadLine(SOCKET sock, int timeout_ms = 30000)
{
    std::string result;
    char ch;
    bool found_cr = false;

    // Установка таймаута
    DWORD timeout = timeout_ms;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    while (true)
    {
        int bytes = recv(sock, &ch, 1, 0);
        if (bytes <= 0)
            break;

        if (ch == '\r')
        {
            found_cr = true;
            continue;
        }

        if (ch == '\n' && found_cr)
            break;

        if (found_cr && ch != '\n')
        {
            result += '\r';  // Добавляем \r если после него не \n
            found_cr = false;
        }

        result += ch;
    }

    return result;
}

void __fastcall TFixProxy::Execute(void)
{
    // Инициализация WinSock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        dbg_print("TFixProxy::Execute> WSAStartup failed");
        FDeleteLeter.push_back(this);
        return;
    }

    try
    {
        // Создание сокета
        tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (tcp_socket == INVALID_SOCKET)
        {
            dbg_print("TFixProxy::Execute> socket creation failed: %d", WSAGetLastError());
            WSACleanup();
            FDeleteLeter.push_back(this);
            return;
        }

        // Установка таймаута подключения
        DWORD timeout = 30000; // 30 секунд
        setsockopt(tcp_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

        // Настройка адреса сервера
        sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        // Резолв хоста
        server_addr.sin_addr.s_addr = ResolveHostname(host);
        if (server_addr.sin_addr.s_addr == INADDR_NONE)
        {
            dbg_print("TFixProxy::Execute> hostname resolution failed for: %s", host.c_str());
            closesocket(tcp_socket);
            tcp_socket = INVALID_SOCKET;
            WSACleanup();
            FDeleteLeter.push_back(this);
            return;
        }

        // Подключение
        if (connect(tcp_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
        {
            dbg_print("TFixProxy::Execute> connect failed: %d", WSAGetLastError());
            closesocket(tcp_socket);
            tcp_socket = INVALID_SOCKET;
            WSACleanup();
            FDeleteLeter.push_back(this);
            return;
        }

        // Отправка данных (аналог WriteLn - добавляем \r\n)
        std::string send_data = FPrefix + FPacket + "\r\n";
        int total_sent = 0;
        int data_length = send_data.length();

        while (total_sent < data_length)
        {
            int bytes_sent = ::send(tcp_socket, send_data.c_str() + total_sent,
                                  data_length - total_sent, 0);
            if (bytes_sent == SOCKET_ERROR)
            {
                dbg_print("TFixProxy::Execute> send failed: %d", WSAGetLastError());
                break;
            }
            total_sent += bytes_sent;
        }

        if (total_sent == data_length)
        {
            // Чтение ответа (аналог ReadLn)
            FAnswer = ReadLine(tcp_socket);

            // Закрытие соединения (аналог Disconnect)
            shutdown(tcp_socket, SD_BOTH);
            closesocket(tcp_socket);
            tcp_socket = INVALID_SOCKET;
            WSACleanup();

            if (!FAnswer.empty())
            {
                dbg_print("TFixProxy::Execute> RESULT: %s", FAnswer.c_str());
                Synchronize(answed);
                return; //КРИТИЧНО: возврат БЕЗ добавления в FDeleteLeter!
            }
            else
            {
                dbg_print("TFixProxy::Execute> no response received");
                // Продолжаем выполнение к FDeleteLeter.push_back(this)
            }
        }

        // Закрытие при ошибке отправки
        shutdown(tcp_socket, SD_BOTH);
        closesocket(tcp_socket);
        tcp_socket = INVALID_SOCKET;
    }
    catch (...)
    {
        dbg_print("TFixProxy::Execute> Exception occurred");
        if (tcp_socket != INVALID_SOCKET)
        {
            closesocket(tcp_socket);
            tcp_socket = INVALID_SOCKET;
        }
    }

    WSACleanup();
    FDeleteLeter.push_back(this); //Добавление только при ОШИБКЕ
}

void __fastcall TFixProxy::answed()
{
    SFixParams fp(FCar, FPacket, FAnswer, FFixPacket);
    if (FCb) FCb(fp);
    FDeleteLeter.push_back(this);
}
