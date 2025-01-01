<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()>
Partial Class Form1
    Inherits System.Windows.Forms.Form

    'Form 覆寫 Dispose 以清除元件清單。
    <System.Diagnostics.DebuggerNonUserCode()>
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    '為 Windows Form 設計工具的必要項
    Private components As System.ComponentModel.IContainer

    '注意: 以下為 Windows Form 設計工具所需的程序
    '可以使用 Windows Form 設計工具進行修改。
    '請勿使用程式碼編輯器進行修改。
    <System.Diagnostics.DebuggerStepThrough()>
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container()
        Me.SerialPort1 = New System.IO.Ports.SerialPort(Me.components)
        Me.ListBox2 = New System.Windows.Forms.ListBox()
        Me.outputbox1 = New System.Windows.Forms.TextBox()
        Me.Label1 = New System.Windows.Forms.Label()
        Me.clearout = New System.Windows.Forms.Button()
        Me.back = New System.Windows.Forms.Button()
        Me.out = New System.Windows.Forms.Button()
        Me.clearin = New System.Windows.Forms.Button()
        Me.GroupBox1 = New System.Windows.Forms.GroupBox()
        Me.Timer1 = New System.Windows.Forms.Timer(Me.components)
        Me.Label2 = New System.Windows.Forms.Label()
        Me.Timer2 = New System.Windows.Forms.Timer(Me.components)
        Me.outputbox2 = New System.Windows.Forms.TextBox()
        Me.Label3 = New System.Windows.Forms.Label()
        Me.ListBox1 = New System.Windows.Forms.ListBox()
        Me.SuspendLayout()
        '
        'SerialPort1
        '
        Me.SerialPort1.DtrEnable = True
        Me.SerialPort1.PortName = "COM2"
        Me.SerialPort1.RtsEnable = True
        '
        'ListBox2
        '
        Me.ListBox2.BackColor = System.Drawing.Color.FromArgb(CType(CType(192, Byte), Integer), CType(CType(255, Byte), Integer), CType(CType(255, Byte), Integer))
        Me.ListBox2.FormattingEnabled = True
        Me.ListBox2.ItemHeight = 16
        Me.ListBox2.Location = New System.Drawing.Point(93, 233)
        Me.ListBox2.Margin = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.ListBox2.Name = "ListBox2"
        Me.ListBox2.Size = New System.Drawing.Size(391, 180)
        Me.ListBox2.TabIndex = 64
        '
        'outputbox1
        '
        Me.outputbox1.Font = New System.Drawing.Font("新細明體", 14.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(136, Byte))
        Me.outputbox1.Location = New System.Drawing.Point(132, 324)
        Me.outputbox1.Margin = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.outputbox1.Multiline = True
        Me.outputbox1.Name = "outputbox1"
        Me.outputbox1.Size = New System.Drawing.Size(296, 57)
        Me.outputbox1.TabIndex = 66
        '
        'Label1
        '
        Me.Label1.AutoSize = True
        Me.Label1.BackColor = System.Drawing.Color.FromArgb(CType(CType(192, Byte), Integer), CType(CType(255, Byte), Integer), CType(CType(255, Byte), Integer))
        Me.Label1.Font = New System.Drawing.Font("新細明體", 21.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(136, Byte))
        Me.Label1.Location = New System.Drawing.Point(141, 257)
        Me.Label1.Margin = New System.Windows.Forms.Padding(4, 0, 4, 0)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(93, 38)
        Me.Label1.TabIndex = 65
        Me.Label1.Text = "輸出"
        '
        'clearout
        '
        Me.clearout.BackColor = System.Drawing.Color.Silver
        Me.clearout.Location = New System.Drawing.Point(987, 208)
        Me.clearout.Margin = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.clearout.Name = "clearout"
        Me.clearout.Size = New System.Drawing.Size(150, 67)
        Me.clearout.TabIndex = 67
        Me.clearout.Text = "清除輸出"
        Me.clearout.UseVisualStyleBackColor = False
        '
        'back
        '
        Me.back.BackColor = System.Drawing.Color.Silver
        Me.back.Location = New System.Drawing.Point(987, 283)
        Me.back.Margin = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.back.Name = "back"
        Me.back.Size = New System.Drawing.Size(150, 67)
        Me.back.TabIndex = 68
        Me.back.Text = "返回"
        Me.back.UseVisualStyleBackColor = False
        '
        'out
        '
        Me.out.BackColor = System.Drawing.Color.Silver
        Me.out.Location = New System.Drawing.Point(987, 357)
        Me.out.Margin = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.out.Name = "out"
        Me.out.Size = New System.Drawing.Size(150, 67)
        Me.out.TabIndex = 69
        Me.out.Text = "送出"
        Me.out.UseVisualStyleBackColor = False
        '
        'clearin
        '
        Me.clearin.BackColor = System.Drawing.Color.Silver
        Me.clearin.Location = New System.Drawing.Point(987, 432)
        Me.clearin.Margin = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.clearin.Name = "clearin"
        Me.clearin.Size = New System.Drawing.Size(150, 67)
        Me.clearin.TabIndex = 70
        Me.clearin.Text = "清除輸入"
        Me.clearin.UseVisualStyleBackColor = False
        '
        'GroupBox1
        '
        Me.GroupBox1.BackColor = System.Drawing.Color.White
        Me.GroupBox1.Location = New System.Drawing.Point(676, 145)
        Me.GroupBox1.Margin = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.GroupBox1.Name = "GroupBox1"
        Me.GroupBox1.Padding = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.GroupBox1.Size = New System.Drawing.Size(506, 401)
        Me.GroupBox1.TabIndex = 71
        Me.GroupBox1.TabStop = False
        '
        'Timer1
        '
        Me.Timer1.Enabled = True
        '
        'Label2
        '
        Me.Label2.AutoSize = True
        Me.Label2.Font = New System.Drawing.Font("新細明體", 26.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(136, Byte))
        Me.Label2.Location = New System.Drawing.Point(204, 109)
        Me.Label2.Margin = New System.Windows.Forms.Padding(4, 0, 4, 0)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(140, 46)
        Me.Label2.TabIndex = 72
        Me.Label2.Text = "Label2"
        '
        'Timer2
        '
        Me.Timer2.Enabled = True
        Me.Timer2.Interval = 1
        '
        'outputbox2
        '
        Me.outputbox2.Font = New System.Drawing.Font("新細明體", 14.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(136, Byte))
        Me.outputbox2.Location = New System.Drawing.Point(132, 499)
        Me.outputbox2.Margin = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.outputbox2.Multiline = True
        Me.outputbox2.Name = "outputbox2"
        Me.outputbox2.Size = New System.Drawing.Size(296, 57)
        Me.outputbox2.TabIndex = 75
        '
        'Label3
        '
        Me.Label3.AutoSize = True
        Me.Label3.BackColor = System.Drawing.Color.FromArgb(CType(CType(192, Byte), Integer), CType(CType(255, Byte), Integer), CType(CType(255, Byte), Integer))
        Me.Label3.Font = New System.Drawing.Font("新細明體", 21.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(136, Byte))
        Me.Label3.Location = New System.Drawing.Point(141, 432)
        Me.Label3.Margin = New System.Windows.Forms.Padding(4, 0, 4, 0)
        Me.Label3.Name = "Label3"
        Me.Label3.Size = New System.Drawing.Size(93, 38)
        Me.Label3.TabIndex = 74
        Me.Label3.Text = "輸入"
        '
        'ListBox1
        '
        Me.ListBox1.BackColor = System.Drawing.Color.FromArgb(CType(CType(192, Byte), Integer), CType(CType(255, Byte), Integer), CType(CType(255, Byte), Integer))
        Me.ListBox1.FormattingEnabled = True
        Me.ListBox1.ItemHeight = 16
        Me.ListBox1.Location = New System.Drawing.Point(93, 408)
        Me.ListBox1.Margin = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.ListBox1.Name = "ListBox1"
        Me.ListBox1.Size = New System.Drawing.Size(391, 180)
        Me.ListBox1.TabIndex = 73
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(9.0!, 16.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(1200, 600)
        Me.Controls.Add(Me.outputbox2)
        Me.Controls.Add(Me.Label3)
        Me.Controls.Add(Me.ListBox1)
        Me.Controls.Add(Me.Label2)
        Me.Controls.Add(Me.clearin)
        Me.Controls.Add(Me.out)
        Me.Controls.Add(Me.back)
        Me.Controls.Add(Me.clearout)
        Me.Controls.Add(Me.outputbox1)
        Me.Controls.Add(Me.Label1)
        Me.Controls.Add(Me.ListBox2)
        Me.Controls.Add(Me.GroupBox1)
        Me.Margin = New System.Windows.Forms.Padding(4, 4, 4, 4)
        Me.Name = "Form1"
        Me.Text = "默契模式"
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents ListBox2 As ListBox
    Friend WithEvents outputbox1 As TextBox
    Friend WithEvents Label1 As Label
    Friend WithEvents clearout As Button
    Friend WithEvents back As Button
    Friend WithEvents out As Button
    Friend WithEvents clearin As Button
    Friend WithEvents GroupBox1 As GroupBox
    Friend WithEvents Timer1 As Timer
    Protected Friend WithEvents SerialPort1 As IO.Ports.SerialPort
    Friend WithEvents Label2 As Label
    Friend WithEvents Timer2 As Timer
    Friend WithEvents outputbox2 As TextBox
    Friend WithEvents Label3 As Label
    Friend WithEvents ListBox1 As ListBox
End Class
