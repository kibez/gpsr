//---------------------------------------------------------------------------

#ifndef logpass_dialogH
#define logpass_dialogH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <string>
//---------------------------------------------------------------------------
class TLogpassDialog : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TLabel *Label1;
    TLabel *Label2;
    TButton *OKBtn;
    TButton *CancelBtn;
    TEdit *edLogin;
    TEdit *edPassword;
private:	// User declarations
public:		// User declarations
    __fastcall TLogpassDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TLogpassDialog *LogpassDialog;
//---------------------------------------------------------------------------

// Global function for authentication
bool ask_login_password(std::string& login, std::string& password);

//---------------------------------------------------------------------------
#endif