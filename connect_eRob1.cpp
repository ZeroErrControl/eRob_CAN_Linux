//
// Created by zero on 24-8-16.
//

#include <iostream>
#include <cstring>
#include <unistd.h>
#include "ECanVci.h"  // Include the header file provided by Gcan Technology
// Interface card type definition
#define eRob 3
// Function call return status values
#define STATUS_OK 1
#define STATUS_ERR 0
#define CAN1 0
#define CAN2 1
bool
m_connect=false; /* Device start flag
false: indicates that the device is not started or has been closed
true: indicates that the device has started and can send and receive data normally */
DWORD DeviceType = eRob; // Device type USBCAN2 dual channel
DWORD DeviceInd = 0; // Device index 0, there is only one USBCAN

/*
 * COB_messages in absolute motion mode
 * Instruction message reference https://www.zeroerr.cn/download/index.html
 */
unsigned char cob_Position_mode[6] = {0x00, 0x4E, 0x00, 0x00, 0x00, 0x03};
unsigned char cob_ABSTargetPosition_mode[6] = {0x00, 0x8D, 0x00, 0x00, 0x00, 0x00};
unsigned char cob_setACC[6] = {0x00, 0x88, 0x00, 0x00, 0x75, 0x30};       // set acc = 10000 counts/s
unsigned char cob_set_dec_ACC[6] = {0x00, 0x89, 0x00, 0x00, 0x75, 0x30};  // set dec acc = 10000 counts/s
unsigned char cob_set_target_Vel[6] = {0x00, 0x8A, 0x00, 0x00, 0x27, 0x10};  // set target velocity = 10000 counts/s
unsigned char cob_enable[6] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x01};          // enable motor motion
unsigned char cob_setRELPosition[6] = {0x00, 0x87, 0x00, 0x00, 0x00, 0x00};  // set relative position
unsigned char cob_setABSTargetPosition[6] = {0x00, 0x86, 0x00, 0x08, 0x00, 0x00}; // set absolute position
unsigned char cob_startRunning[2] = {0x00, 0x83};                          // start running
unsigned char cob_read_actual_Vel[4] = {0x00, 0x05, 0x00, 0x01};    // read the actual velocity
unsigned char cob_read_current_Pos[2] = {0x00, 0x02};    // read the current position
unsigned char cob_read_Motorcurrent[2] = {0x00, 0x08};    // read the Motor current
unsigned char cob_stopRunning[2] = {0x00, 0x84};    // read the Motor current
unsigned char cob_disable[6] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};  // disable motor motion

void copyDataToSendObject(uint8_t* sendObj, uint8_t* cob_message, size_t length) {
    for (size_t i = 0; i < length; i++) {
        sendObj[i] = cob_message[i];
        // printf("sendObj[%d] = 0x%02x\n", i, sendObj[i]);
    }
}

int main() {
    // 1. Initialize the USBCAN device
    if (OpenDevice(eRob, 0, 0) != 1) {
        std::cerr << "Failed to open USBCAN device!" << std::endl;
        return -1;
    }
    std::cout << "USBCAN device opened successfully." << std::endl;

    // 2. Configure the CAN channel
    INIT_CONFIG config;
    config.AccCode = 0;
    config.AccMask = 0xFFFFFFFF;
    config.Filter = 1;  // Receive all frames
    config.Timing0 = 0x00;  // Baud rate 1000kbps
    config.Timing1 = 0x14;
    config.Mode = 0;  // Normal mode

    if (InitCAN(eRob, 0, 0, &config) != 1) {
        std::cerr << "CAN channel initialization failed!" << std::endl;
        CloseDevice(eRob, 0);
        return -1;
    }
    std::cout << "CAN channel initialized successfully." << std::endl;

    // 3. Start the CAN channel
    if (StartCAN(eRob, 0, 0) != 1) {
        std::cerr << "Failed to start CAN channel!" << std::endl;
        CloseDevice(eRob, 0);
        return -1;
    }
    std::cout << "CAN channel started successfully." << std::endl;

    // 4. Create a CAN frame to send
    CAN_OBJ sendObj;
    std::memset(&sendObj, 0, sizeof(sendObj));
    sendObj.ID = 0x641;  // Standard frame ID
    sendObj.SendType = 0;
    sendObj.RemoteFlag = 0;  // Data frame
    sendObj.ExternFlag = 0;  // Standard frame
    sendObj.DataLen = 2;  // Data length
    copyDataToSendObject(sendObj.Data, cob_startRunning, sendObj.DataLen);  // Add the message

    // 5. Send the CAN frame
    Transmit(eRob, 0, 0, &sendObj, 1);

    // 6. Receive CAN frames
    CAN_OBJ recvObj[2500];
    int len = Receive(eRob, 0, 0, recvObj, 2500, 1000);
    if (len > 0) {
        std::cout << "Received " << len << " CAN frames:" << std::endl;
        for (int i = 0; i < len; ++i) {
            std::cout << "Frame ID: 0x" << std::hex << recvObj[i].ID << std::endl;
            std::cout << "Data: ";
            for (int j = 0; j < recvObj[i].DataLen; ++j) {
                std::cout << "0x" << std::hex << static_cast<int>(recvObj[i].Data[j]) << " ";
            }
            std::cout << std::endl;
        }
    } else {
        std::cerr << "Failed to receive CAN frames or no data available." << std::endl;
    }

    // 7. Close the device
    // CloseDevice(eRob, 0);

    return 0;
}
