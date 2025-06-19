// ============================================================================
// car_auto_answer.cpp - ����������� �� RAD Studio 10.3.3
// ������� ����������� ��� ��������� �����������
// ============================================================================

#pragma hdrstop
#include <System.hpp>
#include "car_auto_answer.h"

car_auto_answer::car_auto_answer()
{
    // ����������� �� ���������
}

void car_auto_answer::insert_answer_pdu(const data_t& val)
{
    Lock lk;  // ���������� ��� ������������������
    pdu_list.push_back(val);
}

bool car_auto_answer::get_answer_pdu(data_t& val)
{
    Lock lk;  // ���������� ��� ������������������
    
    if (pdu_list.empty()) return false;
    
    val = *pdu_list.begin();
    pdu_list.erase(pdu_list.begin());
    return true;
}