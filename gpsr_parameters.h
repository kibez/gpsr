//---------------------------------------------------------------------------

#ifndef gpsr_parametersH
#define gpsr_parametersH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Dialogs.hpp>
//#include <Vcl.Spin.hpp>  // Для TSpinEdit
#include "gps.h"
#include "ures.hpp"
//#include <Vcl.Samples.Spin.hpp>


//---------------------------------------------------------------------------
class TformParamters : public TForm
{
__published:	// IDE-managed Components
    TButton *OKBtn;
    TButton *CancelBtn;
    TPageControl *pc;
    TTabSheet *tsCommon;
    TTabSheet *tsConnection;
    TTabSheet *tsAuth;
    TLabel *Label1;
    TLabel *Label2;
    TEdit *edStationName;
    /*TSpinEdit*/TEdit *edInformatorId;  // Заменен TNumberEdit на TEdit
    TCheckBox *cbAutoUpdateCfg;
    TGroupBox *gbLog;
    TCheckBox *cbLogProgramMessage;
    TCheckBox *cbLogDeviceExchange;
    TCheckBox *cbLogSMS;
    TCheckBox *cIP;
    TCheckBox *cbLogSynchroPacket;
    TCheckBox *cbLogObject;
    TCheckBox *cbWriteDeviceExchangeTime;
    TLabel *Label3;
    TLabel *Label4;
    TEdit *edServerHost;
    /*TSpinEdit*/TEdit *edServerPort;  // Заменен TNumberEdit на TSpinEdit
    TRadioButton *rbAskLogin;
    TRadioButton *rbUseFile;
    TEdit *edFileName;
    TButton *bSelectFile;
    TButton *bExport;
    TOpenDialog *open;
    void __fastcall rbAskLoginClick(TObject *Sender);
    void __fastcall bSelectFileClick(TObject *Sender);
    void __fastcall bExportClick(TObject *Sender);
    void __fastcall edInformatorIdClick(TObject *Sender);
    void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
    void __fastcall cbLogObjectClick(TObject *Sender);
    void __fastcall edFileNameChange(TObject *Sender);
private:	// User declarations
    bool program_fill;
    
    void fill();
    void set_cfg();
    void enable();
    
public:
    cfg_t cfg;
    __fastcall TformParamters(TComponent* Owner);
    bool show();
};
//---------------------------------------------------------------------------
extern PACKAGE TformParamters *formParamters;
//---------------------------------------------------------------------------
#endif