object formGpsLogDate: TformGpsLogDate
  Left = 227
  Top = 108
  BorderStyle = bsDialog
  Caption = #1048#1084#1087#1086#1088#1090' '#1082#1086#1086#1088#1076#1080#1085#1072#1090
  ClientHeight = 139
  ClientWidth = 309
  Color = clBtnFace
  ParentFont = True
  OldCreateOrder = True
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 237
    Height = 13
    Caption = #1054#1073#1085#1072#1088#1091#1078#1077#1085' NMEA '#1087#1072#1082#1077#1090', '#1085#1077' '#1089#1086#1076#1077#1088#1078#1072#1097#1080#1081' '#1076#1072#1090#1099
  end
  object OKBtn: TButton
    Left = 148
    Top = 104
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
  end
  object CancelBtn: TButton
    Left = 228
    Top = 104
    Width = 75
    Height = 25
    Cancel = True
    Caption = #1054#1090#1084#1077#1085#1072
    ModalResult = 2
    TabOrder = 1
  end
  object rbSkipNoDate: TRadioButton
    Left = 16
    Top = 32
    Width = 193
    Height = 17
    Caption = #1048#1075#1085#1086#1088#1080#1088#1086#1074#1072#1090#1100' '#1087#1072#1082#1077#1090#1099' '#1073#1077#1079' '#1076#1072#1090#1099
    Checked = True
    TabOrder = 2
    TabStop = True
    OnClick = rbUseDateClick
  end
  object rbUseDate: TRadioButton
    Left = 16
    Top = 64
    Width = 119
    Height = 17
    Caption = #1059#1082#1072#1079#1072#1090#1100' UTC '#1076#1072#1090#1091
    TabOrder = 3
    OnClick = rbUseDateClick
  end
  object date: TDateTimePicker
    Left = 152
    Top = 62
    Width = 129
    Height = 21
    CalAlignment = dtaLeft
    Date = 38131.5002021528
    Time = 38131.5002021528
    DateFormat = dfShort
    DateMode = dmComboBox
    Kind = dtkDate
    ParseInput = False
    TabOrder = 4
  end
end