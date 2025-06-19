object formSendSMS: TformSendSMS
  Left = 192
  Top = 107
  Width = 401
  Height = 279
  ActiveControl = cbPhone
  Caption = #1054#1090#1087#1088#1072#1074#1082#1072' SMS'
  Color = clBtnFace
  Font.Charset = RUSSIAN_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  DesignSize = (
    393
    252)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 6
    Width = 48
    Height = 13
    Caption = #1058#1077#1083#1077#1092#1086#1085':'
  end
  object mem: TMemo
    Left = 0
    Top = 43
    Width = 393
    Height = 209
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 1
  end
  object cbPhone: TComboBox
    Left = 64
    Top = 6
    Width = 153
    Height = 21
    ItemHeight = 13
    TabOrder = 0
  end
  object bSend: TButton
    Left = 224
    Top = 6
    Width = 75
    Height = 21
    Caption = #1054#1090#1087#1088#1072#1074#1082#1072
    ModalResult = 1
    TabOrder = 2
  end
end