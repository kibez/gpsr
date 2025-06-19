// ============================================================================
// sms_queue.h_ - Мигрировано на RAD Studio 10.3.3
// Очередь SMS запросов для устройств мониторинга
// ============================================================================
#ifndef sms_queueH
#define sms_queueH

#include <System.hpp>
#include "ireq_queue.h"
#include <vector>
#include <string>

class smsreq
{
public:
    unsigned int id;
    std::vector<unsigned char> data;
    std::string phone;
    bool txt_mode;
    int priority;
    bool in_air;
    int timeout;
    
    smsreq() : id(), priority(), in_air(), timeout(), txt_mode(true) {}
};

class SmsReqQueue : public IReqQueue<smsreq>
{
public:
    typedef smsreq Req;
    typedef IReqQueue<smsreq> req_queue_type;

    static const unsigned int def_sms_execute_seconds = 4000;
public:
    unsigned int sms_execute_seconds;

    SmsReqQueue() : sms_execute_seconds(def_sms_execute_seconds) {}

    unsigned int execute_seconds(const Req& req, unsigned int pos) { return sms_execute_seconds * pos + 1; }
    virtual bool sms_queue_ready() const { return true; }
};

#endif