// ============================================================================
// fix_proxy.h - Мигрировано на RAD Studio 10.3.3
// Прокси для исправления данных GPS устройств (WinSock версия)
// ============================================================================
#ifndef fix_proxyH
#define fix_proxyH

#include <System.hpp>
#include <string>
#include <System.Classes.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>  // Для дополнительных функций TCP/IP
#include "car.h"
#include <vector>

class car_gsm;

class TFixProxy : public TThread
{
public:
    TFixProxy(const std::string& packet, const char* addr, unsigned short port);
    __fastcall ~TFixProxy();

    void send(const std::string& prefix = "");

    struct SFixParams
    {
        car_gsm* car;
        const std::string& packet;
        const std::string& answer;
        icar_polling::fix_packet& fixPacket;

        SFixParams(car_gsm* _car, const std::string& _packet,
                   const std::string& _answer, icar_polling::fix_packet& _fixPacket) :
            car(_car), packet(_packet), answer(_answer), fixPacket(_fixPacket)
        {}
    };

    typedef void __fastcall (__closure *TFixProxyCallback)(SFixParams& fixParams);

    void setCar(car_gsm* car) { FCar = car; }
    void setCallback(TFixProxyCallback cb) { FCb = cb; }

    icar_polling::fix_packet& getFixPacket() { return FFixPacket; }

private:
    SOCKET tcp_socket;
    std::string host;
    unsigned short port;
    icar_polling::fix_packet FFixPacket;
    std::string FPrefix;
    std::string FPacket;
    std::string FAnswer;

    car_gsm* FCar;
    TFixProxyCallback FCb;

    static std::vector<TFixProxy*> FDeleteLeter;

    void __fastcall Execute(void);
    void __fastcall answed();
};

#endif
