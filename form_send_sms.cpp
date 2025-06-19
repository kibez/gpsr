//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "form_send_sms.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TformSendSMS *formSendSMS;
//---------------------------------------------------------------------------
__fastcall TformSendSMS::TformSendSMS(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
bool TformSendSMS::Execute()
{
  return ShowModal()==mrOk;
}