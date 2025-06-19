// ============================================================================
// car_auto_answer.h_ - Мигрировано на RAD Studio 10.3.3
// Система автоответов для устройств мониторинга
// ============================================================================
#ifndef car_auto_answerH
#define car_auto_answerH

#include <System.hpp>
#include "pdu.h"
#include <loki/Threads.h>
#include <vector>

typedef std::vector<unsigned char> data_t;

class car_auto_answer : public Loki::ClassLevelLockable<car_auto_answer>
{
    std::vector<data_t> pdu_list;
public:
    car_auto_answer();
    void insert_answer_pdu(const data_t& val);
    bool get_answer_pdu(data_t& val);
    virtual bool generate_ping() { return false; }
};

#endif