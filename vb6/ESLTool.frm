VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Begin VB.Form FrmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "ESLTool"
   ClientHeight    =   3090
   ClientLeft      =   150
   ClientTop       =   435
   ClientWidth     =   5535
   Icon            =   "ESLTool.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   206
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   369
   StartUpPosition =   2  'CenterScreen
   Begin VB.PictureBox PictBox 
      Appearance      =   0  'Flat
      AutoRedraw      =   -1  'True
      AutoSize        =   -1  'True
      BackColor       =   &H80000005&
      BorderStyle     =   0  'None
      ForeColor       =   &H80000008&
      Height          =   2235
      Index           =   4
      Left            =   120
      ScaleHeight     =   149
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   248
      TabIndex        =   20
      Top             =   120
      Visible         =   0   'False
      Width           =   3720
   End
   Begin VB.PictureBox PictBox 
      Appearance      =   0  'Flat
      AutoRedraw      =   -1  'True
      AutoSize        =   -1  'True
      BackColor       =   &H80000005&
      BorderStyle     =   0  'None
      ForeColor       =   &H80000008&
      Height          =   2235
      Index           =   3
      Left            =   120
      ScaleHeight     =   149
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   248
      TabIndex        =   19
      Top             =   120
      Visible         =   0   'False
      Width           =   3720
   End
   Begin VB.PictureBox PictBox 
      Appearance      =   0  'Flat
      AutoRedraw      =   -1  'True
      AutoSize        =   -1  'True
      BackColor       =   &H80000005&
      BorderStyle     =   0  'None
      ForeColor       =   &H80000008&
      Height          =   2235
      Index           =   2
      Left            =   120
      ScaleHeight     =   149
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   248
      TabIndex        =   18
      Top             =   120
      Visible         =   0   'False
      Width           =   3720
   End
   Begin VB.PictureBox PictBox 
      Appearance      =   0  'Flat
      AutoRedraw      =   -1  'True
      AutoSize        =   -1  'True
      BackColor       =   &H80000005&
      BorderStyle     =   0  'None
      ForeColor       =   &H80000008&
      Height          =   2235
      Index           =   1
      Left            =   120
      ScaleHeight     =   149
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   248
      TabIndex        =   17
      ToolTipText     =   "Click in picture to set update position"
      Top             =   120
      Visible         =   0   'False
      Width           =   3720
   End
   Begin VB.Frame FrameRight 
      BorderStyle     =   0  'None
      Caption         =   "Frame1"
      Height          =   2895
      Left            =   3960
      TabIndex        =   1
      Top             =   120
      Width           =   1695
      Begin VB.CheckBox CheckDither 
         Caption         =   "Dithering"
         Height          =   255
         Left            =   120
         TabIndex        =   16
         Top             =   1680
         Width           =   1335
      End
      Begin VB.CheckBox CheckPreview 
         Caption         =   "Preview"
         Height          =   255
         Left            =   120
         TabIndex        =   15
         Top             =   1920
         Value           =   1  'Checked
         Width           =   1335
      End
      Begin VB.CheckBox CheckColor 
         Caption         =   "Color ESL"
         Height          =   255
         Left            =   120
         TabIndex        =   14
         Top             =   1440
         Width           =   1335
      End
      Begin VB.CommandButton CmdUpdate 
         Caption         =   "Update ESL"
         Height          =   495
         Left            =   120
         TabIndex        =   5
         Top             =   2280
         Width           =   1335
      End
      Begin VB.ComboBox ComboPage 
         Height          =   315
         ItemData        =   "ESLTool.frx":10CA
         Left            =   480
         List            =   "ESLTool.frx":10CC
         TabIndex        =   4
         Text            =   "0"
         Top             =   0
         Width           =   975
      End
      Begin VB.TextBox TxtPosX 
         Height          =   285
         Left            =   240
         MaxLength       =   3
         TabIndex        =   3
         Text            =   "0"
         Top             =   720
         Width           =   615
      End
      Begin VB.TextBox TxtPosY 
         Height          =   285
         Left            =   240
         MaxLength       =   3
         TabIndex        =   2
         Text            =   "0"
         Top             =   1080
         Width           =   615
      End
      Begin VB.Label Label3 
         Caption         =   "Position from top-left:"
         Height          =   255
         Left            =   0
         TabIndex        =   9
         Top             =   480
         Width           =   1575
      End
      Begin VB.Label Label4 
         Caption         =   "X:"
         Height          =   255
         Left            =   0
         TabIndex        =   8
         Top             =   780
         Width           =   255
      End
      Begin VB.Label Label5 
         Caption         =   "Y:"
         Height          =   255
         Left            =   0
         TabIndex        =   7
         Top             =   1140
         Width           =   255
      End
      Begin VB.Label Label6 
         Caption         =   "Page:"
         Height          =   255
         Left            =   0
         TabIndex        =   6
         Top             =   60
         Width           =   495
      End
   End
   Begin MSComDlg.CommonDialog comdlg 
      Left            =   0
      Top             =   0
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      CancelError     =   -1  'True
      Filter          =   "Graphic Files (*.bmp;*.gif;*.jpg)|*.bmp;*.gif;*.jpg|"
   End
   Begin VB.Timer Timer1 
      Interval        =   1000
      Left            =   480
      Top             =   0
   End
   Begin VB.PictureBox PictBox 
      Appearance      =   0  'Flat
      AutoRedraw      =   -1  'True
      AutoSize        =   -1  'True
      BackColor       =   &H80000005&
      BorderStyle     =   0  'None
      ForeColor       =   &H80000008&
      Height          =   2235
      Index           =   0
      Left            =   120
      ScaleHeight     =   149
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   248
      TabIndex        =   0
      ToolTipText     =   "Click in picture to set update position"
      Top             =   120
      Width           =   3720
   End
   Begin VB.Frame FrameBot 
      BorderStyle     =   0  'None
      Caption         =   "Frame1"
      Height          =   975
      Left            =   120
      TabIndex        =   10
      Top             =   2025
      Width           =   5250
      Begin VB.TextBox Text1 
         Height          =   285
         Left            =   0
         MaxLength       =   17
         TabIndex        =   11
         Top             =   240
         Width           =   2175
      End
      Begin VB.Label Label1 
         Caption         =   "ESL barcode data:"
         Height          =   255
         Left            =   0
         TabIndex        =   13
         Top             =   0
         Width           =   1455
      End
      Begin VB.Label Label2 
         Caption         =   "Status: Ready"
         Height          =   255
         Left            =   0
         TabIndex        =   12
         Top             =   720
         Width           =   5295
      End
   End
   Begin VB.Menu menu_file 
      Caption         =   "&File"
      Begin VB.Menu menu_openimg 
         Caption         =   "&Open image"
         Shortcut        =   ^O
      End
   End
End
Attribute VB_Name = "FrmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Public BlasterPresent As Boolean
Dim PixelsBWR() As Byte
Dim PixelsBWRD() As Byte

Function GetPLID(Barcode As String) As Byte()
    On Error GoTo ErrHandler
    
    Dim PLIDValue As Double
    Dim RetData(4) As Byte
    
    PLIDValue = Mid(Barcode, 3, 5) + (Mid(Barcode, 8, 5) * 65536)
    
    RetData(0) = (PLIDValue \ 256) And 255
    RetData(1) = PLIDValue And 255
    RetData(2) = (PLIDValue \ 65536 \ 256) And 255
    RetData(3) = (PLIDValue \ 65536) And 255
    
    GetPLID = RetData
    Exit Function
    
ErrHandler:
    MsgBox "The barcode is invalid.", vbExclamation
    RetData(0) = 255
    RetData(1) = 255
    RetData(2) = 255
    RetData(3) = 255
    GetPLID = RetData
End Function

Sub RecordRLERun(ByVal RunCount As Long)
    Dim Bits(16) As Integer
    Dim B As Integer, i As Integer
    
    ' Convert to binary and count required bits
    i = 0
    While RunCount > 0
        Bits(i) = RunCount And 1
        RunCount = RunCount \ 2
        i = i + 1
    Wend
    
    ' Unary-code the value length - 1
    For B = 0 To i - 1 - 1
        ArrayAppendByte Compressed, 0
    Next B
    ' Write bits
    If i > 0 Then
        For B = 0 To i - 1
            ArrayAppendByte Compressed, CByte(Bits(i - B - 1))
        Next B
    End If
End Sub

Sub ArrayAppendByte(arr() As Byte, ByVal v As Byte)
    Dim NewSize As Double
    
    NewSize = UBound(arr) + 1
    ReDim Preserve arr(NewSize)
    arr(NewSize) = v
End Sub

Private Sub CheckColor_Click()
    UpdateView
End Sub

Private Sub CheckDither_Click()
    UpdateView
End Sub

Private Sub CheckPreview_Click()
    UpdateView
End Sub

Private Sub UpdateView()
    Dim c As Integer
    
    For c = 0 To 4
        PictBox(c).Visible = False
    Next c
    ' 0: Original
    ' 1: BW
    ' 2: BWR
    ' 3: BW dithered
    ' 4: BWR dithered
    
    If CheckPreview.Value = vbChecked Then
        If CheckDither.Value = vbChecked Then
            If CheckColor.Value = vbChecked Then
                PictBox(4).Visible = True
            Else
                PictBox(3).Visible = True
            End If
        Else
            If CheckColor.Value = vbChecked Then
                PictBox(2).Visible = True
            Else
                PictBox(1).Visible = True
            End If
        End If
    Else
        PictBox(0).Visible = True
    End If
End Sub

Private Sub CmdUpdate_Click()
    'Dim Pixels(307200) As Boolean  ' 640x480
    Dim RunPixel As Byte
    Dim RunCount As Long, BitsPerFrame As Integer
    Dim ImgWidth As Integer, ImgHeight As Integer
    Dim ParamFrame As frame_t, DataFrame As frame_t
    Dim PLID() As Byte
    Dim CompressionType As Byte
    Dim DMPage As Byte
    Dim image_bit_data() As Byte
    Dim Dot As Double, SizeRaw As Double, SizeCompressed As Double, DataSize As Double
    Dim R As Integer, G As Integer, B As Integer
    Dim Px As Integer, Py As Integer, c As Double, i As Double, fr As Integer, bi As Integer
    Dim v As Integer, checksum As Integer
    Dim Padding As Integer, PaddedDataSize As Double, DataFrameCount As Integer
    
    FrameCount = 0
    
    ' Check barcode validity
    If Len(Text1.Text) <> 17 Then
        MsgBox "The barcode data must be exactly 17 characters.", vbExclamation
        Exit Sub
    End If
    checksum = 0
    For c = 1 To 16
        checksum = checksum + Asc(Mid(Text1.Text, c, 1))
    Next c
    If Trim(Str(checksum Mod 10)) <> Mid(Text1.Text, 17, 1) Then
        MsgBox "The barcode is invalid.", vbExclamation
        Exit Sub
    End If

    ' Get PLID from barcode string
    PLID = GetPLID(Text1.Text)
    If PLID(0) = 255 And PLID(1) = 255 And PLID(2) = 255 And PLID(3) = 255 Then Exit Sub

    DMPage = ComboPage.ListIndex
    
    ImgWidth = PictBox(0).Width
    ImgHeight = PictBox(0).Height
    SizeRaw = CDbl(ImgWidth) * CDbl(ImgHeight)

    Label2.Caption = "Encoding image, please wait..."
    DoEvents
    
    ' Select between dithered data or not
    If CheckDither.Value = vbChecked Then
        image_bit_data = PixelsBWRD
    Else
        image_bit_data = PixelsBWR
    End If
    
    ' Select between color data or not
    If CheckColor.Value = vbChecked Then
        DataSize = CDbl(ImgWidth) * ImgHeight * 2
    Else
        DataSize = CDbl(ImgWidth) * ImgHeight
    End If
        
    ' Try some RLE compression
    ReDim Compressed(0)
    RunPixel = image_bit_data(0)
    RunCount = 1
    Compressed(0) = RunPixel
    For c = 1 To DataSize
        If image_bit_data(c) = RunPixel Then
            RunCount = RunCount + 1
        Else
            RecordRLERun RunCount
            RunCount = 1
            RunPixel = image_bit_data(c)
        End If
    Next c
    
    If RunCount > 1 Then RecordRLERun RunCount
    
    SizeCompressed = UBound(Compressed)
    
    ' Decide on compression or not
    If SizeCompressed < SizeRaw Then
        'print("Compression ratio: %.1f%%" % (100 - ((size_compressed * 100) / float(size_raw))))
        image_bit_data = Compressed
        CompressionType = 2
        DataSize = SizeCompressed
    Else
        'Print ("Compression ratio suxx, using raw data")
        CompressionType = 0
        'DataSize = SizeRaw
    End If
    
    ' Pad data to multiple of bits_per_frame
    BitsPerFrame = 20 * 8
    Padding = BitsPerFrame - (DataSize Mod BitsPerFrame)
    ReDim Preserve image_bit_data(UBound(image_bit_data) + Padding)
    For c = 0 To Padding - 1
        image_bit_data(DataSize + c) = 0
    Next c

    PaddedDataSize = DataSize + Padding
    DataFrameCount = PaddedDataSize \ BitsPerFrame
    
    If PaddedDataSize > 65535 Or DataFrameCount > 255 Then
        MsgBox "The data size to transmit (" & Trim(Str(DataFrameCount)) & " frames) exceeds what the IR protocol can support (256)." & vbCrLf & "Please transmit your image in multiple smaller blocks with different positions.", vbExclamation
        Exit Sub
    End If

    ' Wake up frame
    StackFrame make_ping_frame(PLID, 200)

    ' Parameters frame
    ParamFrame = make_mcu_frame(PLID, 5)
    frame_append_word ParamFrame, DataSize \ 8      ' Byte count
    frame_append_byte ParamFrame, 0                 ' Unused
    frame_append_byte ParamFrame, CompressionType
    frame_append_byte ParamFrame, DMPage
    frame_append_word ParamFrame, CDbl(ImgWidth)
    frame_append_word ParamFrame, CDbl(ImgHeight)
    frame_append_word ParamFrame, CDbl(Val(TxtPosX.Text))
    frame_append_word ParamFrame, CDbl(Val(TxtPosY.Text))
    frame_append_word ParamFrame, &H0   ' Keycode
    frame_append_byte ParamFrame, &H88  ' 0x80 = update, 0x08 = set base page
    frame_append_word ParamFrame, &H0   ' Enabled pages (bitmap)
    frame_append_word ParamFrame, 0
    frame_append_word ParamFrame, 0
    terminate_frame ParamFrame, 1
    StackFrame ParamFrame
    
    ' Data frames
    i = 0
    For fr = 0 To DataFrameCount - 1
        DataFrame = make_mcu_frame(PLID, &H20)
        frame_append_word DataFrame, CByte(fr)
        For c = 0 To 20 - 1
            v = 0
            ' Bit string to byte
            For bi = 0 To 8 - 1
                v = v * 2
                v = v + image_bit_data(i + bi)
            Next bi
            frame_append_byte DataFrame, CByte(v)
            i = i + 8
        Next c
        terminate_frame DataFrame, 1
        StackFrame DataFrame
    Next fr
    
    ' Refresh frame
    StackFrame make_refresh_frame(PLID)
    
    ' DEBUG
    'Dim concat As String
    'Open "d:\out.txt" For Output As #1
    'For fr = 0 To FrameCount - 1
    '    concat = ""
    '    For B = 0 To Frames(fr).Size - 1
    '        concat = concat & HexPad(Frames(fr).Data(B)) & " "
    '    Next B
    '    Print #1, concat
    'Next fr
    'Close #1
    
    TransmitESLBlaster
    
    MsgBox "Transmit done. Please allow a few seconds for the ESL to refresh.", vbInformation
    
    CommClose 7
    SetBlasterPresent False
End Sub

Sub TransmitESLBlaster()
    Dim BufferStr As String
    Dim StrRead As String
    Dim fr As Integer, c As Integer, i As Integer, Repeats As Integer
    Dim DataSize As Integer, BufferLen As Integer
    
    Timer1.Enabled = False
    CmdUpdate.Enabled = False
    
    ' DEBUG
    'Open "d:\out_eslb.txt" For Binary As #1
    
    i = 1
    For fr = 0 To FrameCount - 1
        DataSize = Frames(fr).Size - 2
        Repeats = Frames(fr).Data(DataSize) + (Frames(fr).Data(DataSize + 1) * 256)
        Label2.Caption = "Transmitting frame " & fr & "/" & FrameCount - 1 & ", length " & DataSize & ", repeated " & Repeats & " times, please wait..."
        
        BufferLen = Frames(fr).Size - 2
        
        BufferStr = "L"  ' Load
        
        BufferStr = BufferStr + Chr(BufferLen) ' Data size
        BufferStr = BufferStr + Chr(30)
        BufferStr = BufferStr + Chr(Repeats And 255)
        BufferStr = BufferStr + Chr(Repeats \ 256)
        For c = 0 To BufferLen - 1
            BufferStr = BufferStr + Chr(Frames(fr).Data(c))
        Next c
        
        ' For ESL Blaster only
        BufferStr = BufferStr + "T"  ' Transmit
        
        ' DEBUG
        'Put #1, , BufferStr
        
        CommWrite 7, BufferStr   ' Fire !
        
        Do
            CommRead 7, StrRead, 1
            If InStr(1, StrRead, "K") Then Exit Do
            DoEvents
        Loop
    Next fr
    
    ' DEBUG
    'Close #1
    
    CmdUpdate.Enabled = True
    Timer1.Enabled = True
End Sub

Function HexPad(ByVal v As Byte) As String
    HexPad = Hex(v)
    If Len(HexPad) = 1 Then HexPad = "0" & HexPad
End Function

Private Sub Form_Load()
    Dim c As Integer
    
    SetBlasterPresent False
    
    For c = 0 To 15
        ComboPage.AddItem "Page " & c
    Next c
    ComboPage.ListIndex = 0
    
    PictBox(0).Line (0, 0)-(PictBox(0).Width, PictBox(0).Height), vbBlack
    PictBox(0).Line (PictBox(0).Width, 0)-(0, PictBox(0).Height), vbBlack
    ProcessImage
    UpdateView
    
    Text1.Text = Left(INIRead("ESLTool", "LastPLID", "ESLTool.ini"), 17)
End Sub

Private Sub Form_Resize()
    FrameRight.Left = FrmMain.ScaleWidth - FrameRight.Width
    FrameBot.Top = FrmMain.ScaleHeight - FrameBot.Height
End Sub

Private Sub Form_Unload(Cancel As Integer)
    If BlasterPresent = True Then CommClose 7
    INIWrite "ESLTool", "LastPLID", Text1.Text, "ESLTool.ini"
End Sub

Function Max(ByVal a As Integer, ByVal B As Integer)
    If a >= B Then
        Max = a
    Else
        Max = B
    End If
End Function

Private Sub AddRatio(Pixel As RGB, ByRef Err As RGB, ratio As Single)
    If Pixel.R > -16384 And Pixel.R < 16384 Then Pixel.R = Pixel.R + Err.R * ratio
    If Pixel.G > -16384 And Pixel.G < 16384 Then Pixel.G = Pixel.G + Err.G * ratio
    If Pixel.B > -16384 And Pixel.B < 16384 Then Pixel.B = Pixel.B + Err.B * ratio
End Sub

Public Sub ProcessImage()
    Dim x As Integer, y As Integer
    Dim w As Integer, h As Integer
    Dim PixelsRGB() As RGB
    Dim PixelErr As RGB
    Dim CCDC As Long
    Dim SOB As Long
    Dim RGBLong As Long
    Dim S As Long
    Dim B As Integer, G As Integer, R As Integer
    Dim dB As Integer, dG As Integer, dR As Integer
    Dim cbw As Long, cbwr As Long
    Dim DataSize As Long, i As Long, j As Long
    
    Label2.Caption = "Converting image, please wait..."
    DoEvents
    
    w = PictBox(0).Width
    h = PictBox(0).Height
    DataSize = CLng(w) * h
    
    FrmMain.Width = Max(w + 160, 350) * 15
    FrmMain.Height = Max(h + 130, 250) * 15
    
    CCDC = CreateCompatibleDC(PictBox(0).hdc)
    SOB = SelectObject(CCDC, PictBox(0).Picture)
    
    ReDim PixelsRGB(0 To w - 1, 0 To h - 1)
    'ReDim PixelsBWR(0 To (w - 1) * 2, 0 To (h - 1) * 2)
    'ReDim PixelsBWRD(0 To (w - 1) * 2, 0 To (h - 1) * 2)
    ReDim PixelsBWR(0 To (DataSize * 2))
    ReDim PixelsBWRD(0 To (DataSize * 2))
    
    i = 0
    j = DataSize
    For y = 0 To h - 1
        For x = 0 To w - 1
            RGBLong& = GetPixel(CCDC, x, y)
            B = RGBLong \ 65536
            G = (RGBLong And &HFF00&) \ 256
            R = RGBLong And &HFF&
            PixelsRGB(x, y).R = R
            PixelsRGB(x, y).G = G
            PixelsRGB(x, y).B = B
            
            S = R + G + B
            
            ' No dithering pass
            If S > 3 * 127 Then
                ' White
                PixelsBWR(i) = 1
                PixelsBWR(j) = 1
                cbw = vbWhite
                cbwr = vbWhite
            ElseIf R > (G + B) / 2 Then
                ' Red
                PixelsBWR(i) = 0
                PixelsBWR(j) = 0
                cbw = vbBlack
                cbwr = vbRed
            Else
                PixelsBWR(i) = 0
                PixelsBWR(j) = 1
                cbw = vbBlack
                cbwr = vbBlack
            End If
            
            PictBox(1).PSet (x, y), cbw
            PictBox(2).PSet (x, y), cbwr
            
            i = i + 1
            j = j + 1
        Next x
    Next y
    
    i = 0
    j = DataSize
    For y = 0 To h - 1
        For x = 0 To w - 1
            B = PixelsRGB(x, y).B
            G = PixelsRGB(x, y).G
            R = PixelsRGB(x, y).R
            
            S = CLng(R) + G + B
            
            If S > 3 * 127 Then
                ' White
                PixelErr.R = R - 255
                PixelErr.G = G - 255
                PixelErr.B = B - 255
                PixelsBWRD(i) = 1
                PixelsBWRD(j) = 1
                cbw = vbWhite
                cbwr = vbWhite
            ElseIf R > (G + B) / 2 Then
                ' Red
                PixelErr.R = R - 255
                PixelErr.G = G
                PixelErr.B = B
                PixelsBWRD(i) = 0
                PixelsBWRD(j) = 0
                cbw = vbBlack
                cbwr = vbRed
            Else
                PixelErr.R = R
                PixelErr.G = G
                PixelErr.B = B
                PixelsBWRD(i) = 0
                PixelsBWRD(j) = 1
                cbw = vbBlack
                cbwr = vbBlack
            End If
            
            If (x + 1 < w) Then AddRatio PixelsRGB(x + 1, y), PixelErr, 7# / 16
            If (x - 1 >= 0) And (y + 1 < h) Then AddRatio PixelsRGB(x - 1, y + 1), PixelErr, 3# / 16
            If (y + 1 < h) Then AddRatio PixelsRGB(x, y + 1), PixelErr, 5# / 16
            If (x + 1 < w) And (y + 1 < h) Then AddRatio PixelsRGB(x + 1, y + 1), PixelErr, 1# / 16
            
            PictBox(3).PSet (x, y), cbw
            PictBox(4).PSet (x, y), cbwr
            
            i = i + 1
            j = j + 1
        Next x
    Next y
    
    Label2.Caption = "Ready"
End Sub

Private Sub menu_openimg_Click()
    On Error GoTo errchk
    Dim w As Integer, h As Integer, c As Integer
    
    Dim ratio As Single
    Dim Resized As Boolean
    
    comdlg.ShowOpen
    
    PictBox(0).Picture = LoadPicture(comdlg.FileName)
    
    w = PictBox(0).Width
    h = PictBox(0).Height
    ratio = w / h
    
    If PictBox(0).Width > 640 Then
        Resized = True
        w = 640
        h = 640 / ratio
    End If
    If PictBox(0).Height > 480 Then
        Resized = True
        h = 480
        w = 480 * ratio
    End If
    
    ' Make sure image dimensions are multiples of 4
    If w Mod 4 <> 0 Or h Mod 4 <> 0 Then
        MsgBox "The image was resized to make its dimensions multiples of 4.", vbInformation
        w = (w \ 4) * 4
        h = (h \ 4) * 4
    End If
    
    For c = 0 To 4
        PictBox(c).Width = w
        PictBox(c).Height = h
    Next c
            
    If Resized = True Then MsgBox "The image was resized to fit into 640*480.", vbInformation
    
    ProcessImage
    
    Exit Sub
errchk:
    ' Ignore cancel
    If Err.Number <> cdlCancel And Err.Number <> 53 Then Err.Raise Err.Number
End Sub

Private Sub Timer1_Timer()
    Dim StrComPort As String
    Dim c As Integer
    
    If BlasterPresent = False Then
        Label2.Caption = "Searching for ESL Blaster..."
        
        ' Search for ESL Blaster on COM1 to COM10
        For c = 1 To 10
            StrComPort = "COM" & c
            If CommOpen(7, StrComPort, "baud=57600 parity=N data=8 stop=1") = 0 Then
                If TestBlasterPresence = True Then
                    Label2.Caption = "ESL Blaster found on " & StrComPort
                    SetBlasterPresent True
                    Exit For
                Else
                    CommClose 7
                End If
            End If
        Next c
        
        If c = 11 Then
            CommClose 7
            SetBlasterPresent False
        End If
    Else
        ' Check that ESL Blaster is still here
        If TestBlasterPresence = False Then
            SetBlasterPresent False
            CommClose 7
        End If
    End If
End Sub

Sub SetBlasterPresent(ByVal v As Boolean)
    BlasterPresent = v
    CmdUpdate.Enabled = v
End Sub

Function TestBlasterPresence() As Boolean
    Dim RefTime As Double
    Dim StrRead As String
    
    StrRead = Space(20)
    
    TestBlasterPresence = False
    CommWrite 7, "?"
    RefTime = Timer
    Do
        CommRead 7, StrRead, 10
        If InStr(1, StrRead, "ESLBlaster") Then
            TestBlasterPresence = True
            Exit Do
        End If
        DoEvents
    Loop Until Timer > RefTime + 0.5
End Function
