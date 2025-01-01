#include <windows.h>    //訪問和管理硬體設備（串口）
#include <stdio.h>

void UART_Init(const char* comPort, DWORD baudRate, HANDLE* hComm) {
    *hComm = CreateFileA(
        comPort,                     // COM 埠名稱 (如 "COM3")
        GENERIC_READ | GENERIC_WRITE, // 讀寫模式
        0,                            // 不共享
        NULL,                         // 無安全屬性
        OPEN_EXISTING,                // 必須是現有埠
        0,                            // 默認屬性
        NULL                          // 無模板文件
    );

    if (*hComm == INVALID_HANDLE_VALUE) {   //無法正常被創建或打開
        printf("Error opening COM port: %s\n", comPort);  //沒comport
        exit(EXIT_FAILURE);
    }

    // 配置串口
    DCB dcbSerialParams = { 0 };    // DCB（Device Control Block）是一個結構，用來存儲串口的配置參數
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);    // DCBlength 是 DCB 結構的長度，確保結構大小正確

    /*if (!GetCommState(*hComm, &dcbSerialParams)) {
        printf("Error getting COM state\n");
        CloseHandle(*hComm);    //
        exit(EXIT_FAILURE);     //關閉串口句柄並退出程序。
    }

    dcbSerialParams.BaudRate = baudRate; // 設置波特率
    dcbSerialParams.ByteSize = 8;        // 每字節 8 位
    dcbSerialParams.StopBits = ONESTOPBIT; // 1 個停止位
    dcbSerialParams.Parity = NOPARITY;  // 無校驗位

    if (!SetCommState(*hComm, &dcbSerialParams)) {    //*****
        printf("Error setting COM state\n");
        CloseHandle(*hComm);
        exit(EXIT_FAILURE);
    }

    // 設置超時
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;          // 讀取間隔超時
    timeouts.ReadTotalTimeoutConstant = 50;     // 總讀取超時
    timeouts.ReadTotalTimeoutMultiplier = 10;   // 每字節讀取超時
    timeouts.WriteTotalTimeoutConstant = 50;    // 總寫入超時
    timeouts.WriteTotalTimeoutMultiplier = 10;  // 每字節寫入超時

    if (!SetCommTimeouts(*hComm, &timeouts)) {    //*****
        printf("Error setting timeouts\n");
        CloseHandle(*hComm);    //關閉，釋放系統資源
        exit(EXIT_FAILURE);
    }*/

    printf("COM port %s initialized at %lu baud.\n", comPort, baudRate);
}

void UART_Send(HANDLE hComm, const char* data) {
    DWORD bytesWritten;
    if (!WriteFile(hComm, data, strlen(data), &bytesWritten, NULL)) {     //strlen(data) 不包含\0
        printf("Error writing to COM port\n");
    }
    else {
        printf("Sent: %s\n", data);
    }
}

void UART_Receive(HANDLE hComm, char* buffer, DWORD bufferSize) {
    DWORD bytesRead;
    if (!ReadFile(hComm, buffer, bufferSize - 1, &bytesRead, NULL)) {
        printf("Error reading from COM port\n");
    }
    else {
        // 寫入你的遊戲區 ******************************************

        if (buffer[0] == 'a') {  //模式A
            printf("A");

        }
        else if (buffer[0] == 'b') {  //模式B
           printf("B");

        }
        else if (buffer[0] == 'c') {  //模式C
            printf("C");

        }

        // 寫入你的遊戲區 ******************************************
        //buffer[bytesRead] = '\0'; // 添加字串結尾符
        /*if (buffer[0] != '\0') {
            printf("good\n");
            printf("Received: %s\n", buffer);
        }*/
    }
}

int main() {
    HANDLE hComm;   //HANDLE 專門設計來處理資源的標識符
    UART_Init("COM1", 9600, &hComm);    //初始化 UART
    char receiveBuffer[256] = "\0";     //清除空間
    char sentData[256];  // Buffer to hold user input
    //printf("Enter data to send: ");
    //scanf("%s", sentData);

    while (1) {
        // 傳送數據
        //UART_Send(hComm,"Hello, UART!\n");     // 傳送數據
        //UART_Send(hComm, sentData);     // 傳送數據
        UART_Receive(hComm, receiveBuffer, sizeof(receiveBuffer));  // 接收數據

        Sleep(100);   // 延遲 1 秒
    }
    CloseHandle(hComm); // 關閉 UART
    return 0;
}