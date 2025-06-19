//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <string>
#include "logpass_dialog.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TLogpassDialog *LogpassDialog;
//---------------------------------------------------------------------

bool ask_login_password(std::string& login, std::string& password)
{
    bool ret = false;
    if (LogpassDialog->ShowModal() == mrOk)
    {
        login = AnsiString(LogpassDialog->edLogin->Text).c_str();
        password = AnsiString(LogpassDialog->edPassword->Text).c_str();
        ret = true;
    }

    LogpassDialog->edLogin->Text = L"";
    LogpassDialog->edPassword->Text = L"";
    return ret;
}

//---------------------------------------------------------------------
__fastcall TLogpassDialog::TLogpassDialog(TComponent* AOwner)
    : TForm(AOwner)
{
}
//---------------------------------------------------------------------