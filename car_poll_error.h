// ============================================================================
// car_poll_error.h_ - Мигрировано на RAD Studio 10.3.3
// Система управления ошибками опроса устройств
// ============================================================================
#ifndef car_poll_errorH
#define car_poll_errorH

#include <System.hpp>
#include <map>
#include <loki/NullType.h>
#include "poll_error/poll_error_code.h"

class icar_poll_error {
public:
    typedef std::pair<int, int> key;
    typedef std::map<key, Loki::NullType, std::less<key> > tree;

    typedef std::map<int, int, std::less<int> > itree;
    typedef std::map<int, bool, std::less<int> > btree;

public:
    tree  pr_er;
    itree er_pr;
    btree er_op;

    void set_priority(int error, int priority)
    {
        remove_error_from_priority(error);
        pr_er[key(priority, error)] = Loki::NullType();
        er_pr[error] = priority;
        er_op[error] = false;
    }

    void remove_error_from_priority(int error)
    {
        itree::iterator ii = er_pr.find(error);
        if (ii == er_pr.end()) return;
        pr_er.erase(key(ii->second, ii->first));
        er_pr.erase(ii);
    }

    void set_error(int error)
    {
        btree::iterator it = er_op.find(error);
        if (it == er_op.end()) return;
        it->second = true;
    }

    void reset_error(int error)
    {
        btree::iterator it = er_op.find(error);
        if (it == er_op.end()) return;
        it->second = false;
    }

    icar_poll_error()
    {
        int pr = 0;
        set_priority(ERROR_OBJECT_NOT_EXIST, ++pr);
        set_priority(ERROR_INFORMATOR_NOT_SUPPORT, ++pr);
        set_priority(ERROR_OBJECT_NOT_SUPPORT, ++pr);
        ++pr;
        set_priority(ERROR_OPERATION_TIMEOUT, ++pr);
        set_priority(POLL_ERROR_REMOTE_DEVICE_NO_ANSWER, ++pr);
        set_priority(POLL_ERROR_REMOTE_DEVICE_BUSY, ++pr);
        set_priority(POLL_ERROR_COMMUNICATION_ERROR, ++pr);
        set_priority(POLL_ERROR_PARSE_ERROR, ++pr);
        set_priority(POLL_ERROR_GPS_NO_FIX, ++pr);
        set_priority(ERROR_OPERATION_FAILED, ++pr);
        set_priority(ERROR_OPERATION_DENIED, ++pr);

        //всегда последними
        pr = 0x7FFFFFFF;
        set_priority(POLL_ERROR_NO_ERROR, pr);
        set_priority(ERROR_GPS_FIX, pr);
    }
};

#endif