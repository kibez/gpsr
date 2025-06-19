object SaveLogDialog: TSaveLogDialog
  Left = 571
  Top = 424
  ActiveControl = bNext
  Anchors = [akLeft, akTop, akRight]
  BorderStyle = bsDialog
  Caption = #1048#1084#1087#1086#1088#1090' '#1082#1086#1086#1088#1076#1080#1085#1072#1090
  ClientHeight = 199
  ClientWidth = 385
  Color = clBtnFace
  Font.Charset = RUSSIAN_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  ShowHint = True
  DesignSize = (
    385
    199)
  PixelsPerInch = 96
  TextHeight = 13
  object bevDownLine: TBevel
    Left = 0
    Top = 157
    Width = 385
    Height = 2
    Anchors = [akLeft, akRight, akBottom]
    Shape = bsBottomLine
  end
  object bBack: TButton
    Left = 126
    Top = 163
    Width = 79
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '<'#1042#1077#1088#1085#1091#1090#1100#1089#1103
    TabOrder = 1
    OnClick = bBackClick
  end
  object bNext: TButton
    Left = 205
    Top = 163
    Width = 79
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = #1044#1072#1083#1077#1077'>'
    TabOrder = 2
    OnClick = bNextClick
  end
  object bCancel: TButton
    Left = 299
    Top = 163
    Width = 79
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = #1054#1090#1084#1077#1085#1072
    ModalResult = 2
    TabOrder = 3
    OnClick = bCancelClick
  end
  object pBase: TPanel
    Left = 0
    Top = 0
    Width = 385
    Height = 155
    Anchors = [akLeft, akTop, akRight, akBottom]
    BevelOuter = bvNone
    TabOrder = 0
    DesignSize = (
      385
      155)
    object pHardwareList: TPanel
      Left = 0
      Top = 500
      Width = 385
      Height = 155
      Anchors = [akLeft, akTop, akRight, akBottom]
      BevelOuter = bvNone
      TabOrder = 0
      object Label1: TLabel
        Left = 8
        Top = 2
        Width = 73
        Height = 13
        Caption = #1054#1073#1086#1088#1091#1076#1086#1074#1072#1085#1080#1077
      end
      object lbHardwareList: TListBox
        Left = 8
        Top = 21
        Width = 268
        Height = 129
        ItemHeight = 13
        TabOrder = 0
        OnClick = lbHardwareListClick
      end
    end
    object pSrcKind: TPanel
      Left = 0
      Top = 0
      Width = 385
      Height = 155
      Anchors = [akLeft, akTop, akRight, akBottom]
      BevelOuter = bvNone
      TabOrder = 1
      object Label2: TLabel
        Left = 9
        Top = 6
        Width = 116
        Height = 13
        Caption = #1053#1086#1089#1080#1090#1077#1083#1100' '#1080#1085#1092#1086#1088#1084#1072#1094#1080#1080
      end
      object rbSrcFile: TRadioButton
        Left = 16
        Top = 32
        Width = 89
        Height = 17
        Caption = #1060#1072#1081#1083
        Checked = True
        TabOrder = 0
        TabStop = True
        OnClick = rbSrcFileClick
      end
      object rbSrcCOMPort: TRadioButton
        Left = 16
        Top = 64
        Width = 89
        Height = 17
        Caption = 'COM '#1087#1086#1088#1090
        TabOrder = 1
        OnClick = rbSrcFileClick
      end
    end
    object pSelectFile: TPanel
      Left = 0
      Top = 500
      Width = 385
      Height = 155
      Anchors = [akLeft, akTop, akRight, akBottom]
      BevelOuter = bvNone
      TabOrder = 2
      object Label3: TLabel
        Left = 9
        Top = 6
        Width = 329
        Height = 13
        Caption = #1042#1080#1073#1077#1088#1080#1090#1077' '#1092#1072#1081#1083#1099', '#1089' '#1082#1086#1090#1086#1088#1099#1084#1080' '#1073#1091#1076#1091#1090' '#1080#1084#1087#1086#1088#1090#1080#1088#1086#1074#1072#1085#1099' '#1082#1086#1086#1088#1076#1080#1085#1072#1090#1099
      end
    end
    object pSelectCOMPort: TPanel
      Left = 0
      Top = 500
      Width = 385
      Height = 155
      Anchors = [akLeft, akTop, akRight, akBottom]
      BevelOuter = bvNone
      TabOrder = 3
      object Label4: TLabel
        Left = 9
        Top = 6
        Width = 98
        Height = 13
        Caption = #1059#1082#1072#1078#1080#1090#1077' COM '#1087#1086#1088#1090
      end
      object cbCOMPort: TComboBox
        Left = 8
        Top = 24
        Width = 145
        Height = 21
        Style = csDropDownList
        TabOrder = 0
        OnSelect = cbCOMPortSelect
      end
    end
    object pSelectObj: TPanel
      Left = 0
      Top = 500
      Width = 385
      Height = 155
      Anchors = [akLeft, akTop, akRight, akBottom]
      BevelOuter = bvNone
      TabOrder = 4
      object Label5: TLabel
        Left = 8
        Top = 8
        Width = 87
        Height = 13
        Caption = #1042#1080#1073#1077#1088#1080#1090#1077' '#1086#1073#1098#1077#1082#1090
      end
      object lbSelectObj: TListBox
        Left = 8
        Top = 24
        Width = 153
        Height = 129
        ItemHeight = 13
        TabOrder = 0
      end
    end
    object pProgress: TPanel
      Left = 0
      Top = 500
      Width = 385
      Height = 155
      Anchors = [akLeft, akTop, akRight, akBottom]
      BevelOuter = bvNone
      TabOrder = 5
      object lProgress: TLabel
        Left = 8
        Top = 111
        Width = 3
        Height = 13
      end
      object lFile: TLabel
        Left = 8
        Top = 2
        Width = 32
        Height = 13
        Caption = #1060#1072#1081#1083':'
      end
      object lFileName: TLabel
        Left = 0
        Top = 18
        Width = 379
        Height = 47
        AutoSize = False
        WordWrap = True
      end
      object Progress: TProgressBar
        Left = 8
        Top = 135
        Width = 369
        Height = 16
        Max = 10000
        TabOrder = 0
      end
    end
    object pSelectDestination: TPanel
      Left = 0
      Top = 500
      Width = 385
      Height = 155
      Anchors = [akLeft, akTop, akRight, akBottom]
      BevelOuter = bvNone
      TabOrder = 6
      object Label7: TLabel
        Left = 9
        Top = 6
        Width = 56
        Height = 13
        Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100':'
      end
      object cbSaveToServer: TCheckBox
        Left = 16
        Top = 32
        Width = 97
        Height = 17
        Caption = #1085#1072' '#1089#1077#1088#1074#1077#1088
        Checked = True
        State = cbChecked
        TabOrder = 0
      end
      object cbSaveToFile: TCheckBox
        Left = 16
        Top = 64
        Width = 97
        Height = 17
        Caption = #1074' '#1092#1072#1081#1083
        Checked = True
        State = cbChecked
        TabOrder = 1
      end
    end
    object pSelectObject: TPanel
      Left = 0
      Top = 500
      Width = 385
      Height = 155
      Anchors = [akLeft, akTop, akRight, akBottom]
      BevelOuter = bvNone
      TabOrder = 7
      object Label6: TLabel
        Left = 8
        Top = 2
        Width = 46
        Height = 13
        Caption = #1054#1073#1098#1077#1082#1090#1099
      end
      object lbSelectObject: TListBox
        Left = 8
        Top = 21
        Width = 268
        Height = 129
        ItemHeight = 13
        TabOrder = 0
        OnClick = lbSelectObjectClick
      end
    end
  end
  object open_files: TOpenDialog
    Options = [ofHideReadOnly, ofAllowMultiSelect, ofEnableSizing]
    Title = #1042#1080#1073#1077#1088#1080#1090#1077' '#1092#1072#1081#1083#1099
    Left = 40
    Top = 160
  end
  object save_file: TSaveDialog
    DefaultExt = 'dat'
    Filter = '('#1040#1088#1093#1080#1074' *.dat)|*.dat'
    Title = #1042#1099#1073#1077#1088#1080#1090#1077' '#1092#1072#1081#1083
    Left = 72
    Top = 160
  end
  object open_log_file: TOpenDialog
    DefaultExt = 'dat'
    Filter = '('#1040#1088#1093#1080#1074' *.dat)|*.dat'
    Title = #1042#1080#1073#1077#1088#1080#1090#1077' '#1092#1072#1081#1083#1099
    Left = 8
    Top = 160
  end
end
