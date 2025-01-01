Partial Class Form1
    Private data123 As String = ""                                                                                          '初始設空白
    Dim receive As String = ""

    Dim button(8, 8) As Button

    Dim txreg As String = ""
    Dim rxreg As String = ""

    Dim data1 As String = ""
    Dim data As String = ""

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        SerialPort1.Open()
        For y As Integer = 1 To 4
            For x As Integer = 1 To 3
                button(y, x) = Me.Controls("button" & y)
                button(y, x) = New Button
                button(y, x).Name = "button" & (y - 1) * 8 + x + 4
                button(y, x).Width = 50                                         '寬
                button(y, x).Height = 50                                        '高
                button(y, x).Font = New Font("新細明體", 15)
                button(y, x).BackColor = Drawing.Color.Silver
                button(y, x).Tag = y & x
                Me.Controls.Add(button(y, x))                                    '將按鈕控制項加入主窗口
                button(y, x).Location = New Point(x * 50 + 450, y * 55 + 103)
                AddHandler button(y, x).MouseClick, AddressOf button_click
                Dim btn As Button = button(y, x)
                btn.BringToFront()
            Next
        Next

        SerialPort1.DtrEnable = True
        SerialPort1.RtsEnable = True

        button(1, 1).Text = "7"
        button(1, 2).Text = "8"
        button(1, 3).Text = "9"

        button(2, 1).Text = "4"
        button(2, 2).Text = "5"
        button(2, 3).Text = "6"

        button(3, 1).Text = "1"
        button(3, 2).Text = "2"
        button(3, 3).Text = "3"

        button(4, 1).Text = "a"
        button(4, 2).Text = "b"
        button(4, 3).Text = "c"

    End Sub
    Private Sub button_click(ByVal sender As Object, ByVal e As MouseEventArgs) '鍵盤矩陣
        Dim row As Integer = (sender.Tag \ 10) ' 计算行号 /
        Dim col As Integer = (sender.Tag Mod 10) ' 计算列号

        txreg = row & col

        Select Case txreg.ToString
            Case "11" : data = 7
            Case "12" : data = 8
            Case "13" : data = 9
            Case "21" : data = 4
            Case "22" : data = 5
            Case "23" : data = 6
            Case "31" : data = 1
            Case "32" : data = 2
            Case "33" : data = 3
            Case "41" : data = "a"
            Case "42" : data = "b"
            Case "43" : data = "c"
        End Select

        If data1.Length < 4 Then
            data1 = data1 & data
        Else
            data1 = data1.Substring(data1.Length - 3) & data '向左移寫法
            'data1 = data & data1.Substring(0, data1.Length - 1) 向右移寫法
        End If
        outputbox1.Text = data1
        data = ""
    End Sub

    Private Sub clearout_Click(sender As Object, e As EventArgs) Handles clearout.Click
        data1 = ""
        data = ""
        Label1.Text = "輸出"
        outputbox1.Clear()
    End Sub
    Private Sub clearin_Click(sender As Object, e As EventArgs) Handles clearin.Click
        rxreg = ""
        Label3.Text = "輸入"
        outputbox2.Clear()
    End Sub
    Private Sub back_Click(sender As Object, e As EventArgs) Handles back.Click
        If data1.Length > 0 Then                                                                                                                                                                                  '只要>0就會返回
            data1 = data1.Substring(0, data1.Length - 1)
            outputbox1.Text = data1
        End If
    End Sub

    Private Sub out_Click(sender As Object, e As EventArgs) Handles out.Click
        'SerialPort1.Write("x")
        SerialPort1.Write(outputbox1.Text)
        outputbox1.Text = ""
        data1 = ""
    End Sub
    Private Sub SerialPort1_DataReceived(sender As Object, e As IO.Ports.SerialDataReceivedEventArgs) Handles SerialPort1.DataReceived
        rxreg = SerialPort1.ReadExisting
    End Sub

    Private Sub Timer1_Tick(sender As Object, e As EventArgs) Handles Timer1.Tick
        'If rxreg.Length = 6 Then
        If Mid(rxreg, 1, 1) = "x" Then
            Label3.Text = "選擇結果"

            Dim rxreg1 As String = Mid(rxreg, 2, 1)                         '抓收到長度的第2個只抓一個
            Dim rxreg2 As String = Mid(rxreg, 3, 1)                         '抓收到長度的第3個只抓一個
            Dim rxreg3 As String = Mid(rxreg, 4, 1)                         '抓收到長度的第4個只抓一個
            Dim rxreg4 As String = Mid(rxreg, 5, 1)                         '抓收到長度的第5個只抓一個
            Dim rxreg5 As String = Mid(rxreg, 6, 1)                         '抓收到長度的第6個只抓一個

            outputbox2.Text = rxreg1
            'outputbox2.Text = rxreg1 + rxreg2 + rxreg3 + rxreg4 + rxreg5
            'rxreg = ""
            'rxreg1 = ""                                                 '一定清除
            'rxreg2 = ""                                                 '一定清除
            'rxreg3 = ""                                                 '一定清除
            'rxreg4 = ""                                                 '一定清除
            'rxreg5 = ""                                                 '一定清除

        End If
        'End If
    End Sub


    'Private Sub Timer1_Tick(sender As Object, e As EventArgs) Handles Timer1.Tick
    '    If rxreg.Length = 6 Then
    '        If Mid(rxreg, 1, 1) = "x" Then
    '            Label1.Text = "回傳結果"
    '
    '            Dim rxreg1 As String = Mid(rxreg, 2, 1)                         '抓收到長度的第2個
    '            Dim rxreg2 As String = Mid(rxreg, 3, 1)                         '抓收到長度的第3個
    '            Dim rxreg3 As String = Mid(rxreg, 4, 1)                         '抓收到長度的第4個
    '            Dim rxreg4 As String = Mid(rxreg, 5, 1)                         '抓收到長度的第5個
    '            Dim rxreg5 As String = Mid(rxreg, 6, 1)                         '抓收到長度的第6個
    '
    '            outputbox.Text = rxreg1 + rxreg2 + rxreg3 + rxreg4 + rxreg5
    '            rxreg = ""
    '            rxreg1 = ""                                                 '一定清除
    '            rxreg2 = ""                                                 '一定清除
    '            rxreg3 = ""                                                 '一定清除
    '            rxreg4 = ""                                                 '一定清除
    '            rxreg5 = ""                                                 '一定清除
    '
    '        End If
    '    End If
    '    If rxreg.Length = 9 Then
    '        If Mid(rxreg, 1, 1) = "x" Then
    '            Label1.Text = "回傳結果"
    '
    '            Dim rxreg1 As String = Mid(rxreg, 2, 1)                         '抓收到長度的第2個
    '            Dim rxreg2 As String = Mid(rxreg, 3, 1)                         '抓收到長度的第3個
    '            Dim rxreg3 As String = Mid(rxreg, 4, 1)                         '抓收到長度的第4個
    '            Dim rxreg4 As String = Mid(rxreg, 5, 1)                         '抓收到長度的第5個
    '            Dim rxreg5 As String = Mid(rxreg, 6, 1)                         '抓收到長度的第6個
    '            Dim rxreg6 As String = Mid(rxreg, 7, 1)                         '抓收到長度的第4個
    '            Dim rxreg7 As String = Mid(rxreg, 8, 1)                         '抓收到長度的第5個
    '            Dim rxreg8 As String = Mid(rxreg, 9, 1)                         '抓收到長度的第6個
    '
    '            outputbox.Text = rxreg1 + rxreg2 + rxreg3 + rxreg4 + rxreg5 + rxreg6 + rxreg3 + rxreg7 + rxreg8
    '
    '            rxreg = ""                                                      '一定清除
    '            rxreg1 = ""                                                 '一定清除
    '            rxreg2 = ""                                                 '一定清除
    '            rxreg3 = ""                                                 '一定清除
    '            rxreg4 = ""                                                 '一定清除
    '            rxreg5 = ""                                                 '一定清除
    '            rxreg6 = ""                                                 '一定清除
    '            rxreg7 = ""                                                 '一定清除
    '            rxreg8 = ""                                                 '一定清除
    '        End If
    '    End If
    '
    '    If rxreg = "R" Then
    '        Label1.Text = "輸入數字"
    '        outputbox.Text = ""
    '        rxreg = ""
    '    End If
    'End Sub

    Dim PortOK As Integer = 0
    Dim PortReady As Integer = 0



    'Private Sub Timer2_Tick(sender As Object, e As EventArgs) Handles Timer2.Tick
    '    For Each sp As String In My.Computer.Ports.SerialPortNames
    '        If sp = "COM4" Then
    '            PortReady = 1
    '        Else
    '            PortReady = 0
    '        End If
    '    Next
    '    If PortReady = 1 Then
    '        If PortOK = 0 Then
    '            SerialPort1.Open()
    '            PortOK = 1
    '        End If
    '    Else
    '        If PortOK = 1 Then
    '            SerialPort1.Close()
    '            PortOK = 0
    '        End If
    '    End If
    '    If SerialPort1.IsOpen Then
    '        Label2.Text = "open"
    '    ElseIf Not SerialPort1.IsOpen Then
    '        Label2.Text = "close"
    '    End If
    'End Sub
End Class