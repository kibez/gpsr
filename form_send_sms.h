//---------------------------------------------------------------------------

#ifndef form_send_smsH
#define form_send_smsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TformSendSMS : public TForm
{
__published:	// IDE-managed Components
    TMemo *mem;
    TComboBox *cbPhone;
    TButton *bSend;
    TLabel *Label1;
private:	// User declarations
public:		// User declarations
    __fastcall TformSendSMS(TComponent* Owner);
    bool Execute();
};
//---------------------------------------------------------------------------
extern PACKAGE TformSendSMS *formSendSMS;
//---------------------------------------------------------------------------
#endif