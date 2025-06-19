//---------------------------------------------------------------------------

#ifndef save_log_dialogH
#define save_log_dialogH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.Dialogs.hpp>
#include "log_callback.h"
#include "carcontainer.h"

#include <loki/Functor.h>
#include <vector>
#include <map>

#define WM_AFTER_START (WM_USER + 1)
//---------------------------------------------------------------------------
class TSaveLogDialog : public TForm
{
__published:	// IDE-managed Components
    TBevel *bevDownLine;
    TButton *bBack;
    TButton *bNext;
    TButton *bCancel;
    TPanel *pBase;
    TPanel *pHardwareList;
    TListBox *lbHardwareList;
    TLabel *Label1;
    TPanel *pSrcKind;
    TLabel *Label2;
    TRadioButton *rbSrcFile;
    TRadioButton *rbSrcCOMPort;
    TPanel *pSelectFile;
    TLabel *Label3;
    TOpenDialog *open_files;
    TPanel *pSelectCOMPort;
    TLabel *Label4;
    TComboBox *cbCOMPort;
    TPanel *pSelectObj;
    TLabel *Label5;
    TListBox *lbSelectObj;
    TPanel *pProgress;
    TProgressBar *Progress;
    TLabel *lProgress;
    TLabel *lFile;
    TLabel *lFileName;
    TPanel *pSelectDestination;
    TLabel *Label7;
    TSaveDialog *save_file;
    TOpenDialog *open_log_file;
    TPanel *pSelectObject;
    TLabel *Label6;
    TListBox *lbSelectObject;
    TCheckBox *cbSaveToServer;
    TCheckBox *cbSaveToFile;
    void __fastcall bBackClick(TObject *Sender);
    void __fastcall bNextClick(TObject *Sender);
    void __fastcall bCancelClick(TObject *Sender);
    void __fastcall lbHardwareListClick(TObject *Sender);
    void __fastcall rbSrcFileClick(TObject *Sender);
    void __fastcall cbCOMPortSelect(TObject *Sender);
    void __fastcall lbSelectObjectClick(TObject *Sender);
    
private:	// User declarations
    typedef Loki::Functor<bool> onend;
    typedef Loki::Functor<void> onbegin;
    
    class key
    {
    public:
        TPanel* panel;
        onend end;
        onbegin begin;
        static void fake_begin() { ; }
        static bool fake_end() { return true; }
        key(TPanel* p = 0l, onend e = onend(key::fake_end), onbegin b = onbegin(key::fake_begin)) 
        { 
            panel = p; 
            begin = b; 
            end = e; 
        }
    };

private:	// User declarations
    std::vector<key> steps;
    int step;

    void set_next_enabled(bool val, TPanel *p = 0l);

    void add_step(int i, key k);
    void remove_step(int i, TPanel* p);
    int find_step(TPanel* p, int from = 0);

    void next();
    void back();
    void end();
    void cancel();
    void go_to(int new_step);

    void move_top_panels();
    void check_step_buttons();
    void show_page(TPanel* val);

    void bSelectDestination();
    void bHardwareList();
    void bSrcKind();
    void bSelectFile();
    void bSelectCOMPort();
    void bProgress();
    void bProgressLog();
    void bSelectObject();

    bool eSelectDestination();
    bool eHardwareList();
    bool eSrcKind();
    bool eSelectCOMPort();
    bool eSelectFile();
    bool eSelectObject();

    void checkHardwareList();
    void checkSrcKind();
    void checkCOMPort();

    void ibProgress();

    void ProgressCallback(double part);
    void ProgressOneMessage(const char* val);
    void SaveResult(void* result);
    void SaveCondition(void* result);

    int autodetect_com_int_car(icar_save_com_port* car_com, iCarContainer* car_cont);
    int autodetect_com_string_car(icar_save_com_port* car_com, iCarContainer* car_cont);
    
private:
    void do_synchronize(bool fix);
    void save_dat_fix(FILE* f);
    void save_dat_cond(FILE* f);

public:		// User declarations
    int dev_id;
    int obj_id;
    int file_index;
    com_port_t* comp;
    FILE* fimport;
    log_callback_t callback;
    FILE* fo;
    FILE* foc;

    __fastcall TSaveLogDialog(TComponent* Owner);
    __fastcall ~TSaveLogDialog();
    int execute();
    int execute_archive();

    void __fastcall OnAfterStart(TMessage& Message);
    
    BEGIN_MESSAGE_MAP
        VCL_MESSAGE_HANDLER(WM_AFTER_START, TMessage, OnAfterStart)
    END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TSaveLogDialog *SaveLogDialog;
//---------------------------------------------------------------------------
#endif