//
// Created by zero on 24-8-20.
//
#include <iostream>
#include <cstdint>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <vector>
#include "ECanVci.h"  // Include the Gcan Technology header

// Interface card type definition
#define eRob 3

// Function call return status values
#define STATUS_OK 1
#define STATUS_ERR 0

#define CAN1 0
#define CAN2 1

DWORD DeviceType = eRob; // Device type USBCAN2 Dual Channel
DWORD DeviceInd = 0;     // Device index 0, only one USBCAN

// Function to convert a hex string to a byte
uint8_t hexStringToByte(const std::string& hexStr) {
    return static_cast<uint8_t>(std::stoul(hexStr, nullptr, 16));
}

// Function to copy data into CAN_OBJ structure
void copyDataToSendObject(CAN_OBJ& sendObj, const std::vector<uint8_t>& data) {
    for (size_t i = 0; i < data.size(); i++) {
        sendObj.Data[i] = data[i];
    }
    sendObj.DataLen = data.size();
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: eRobCAN_sendCOB <CAN ID> {<Data>}\n";
        std::cout << "Example: ./eRobCAN_sendCOB 0x641 “{01 00 00 00 00 01}”\n"<<std::endl;
        return -1;
    }

    // Parse CAN ID
    std::string canIDStr = argv[1];
    DWORD canID = std::stoul(canIDStr, nullptr, 16);
    printf("CAN ID: %s\n", canIDStr.c_str());


    // Parse Data
    std::string dataStr = argv[2];
    dataStr = dataStr.substr(1, dataStr.length() - 2); // Remove curly braces
    std::stringstream ss(dataStr);
    std::string byteStr;
    std::vector<uint8_t> data;

    while (ss >> byteStr) {
        data.push_back(hexStringToByte(byteStr));
    }

    // 1. Initialize USBCAN device
    if (OpenDevice(eRob, 0, 0) != 1) {
        std::cerr << "Failed to open USBCAN device!" << std::endl;
        return -1;
    }
    std::cout << "USBCAN device opened successfully." << std::endl;

    // 2. Configure CAN channel
    INIT_CONFIG config;
    config.AccCode = 0;
    config.AccMask = 0xFFFFFFFF;
    config.Filter = 1;    // Receive all frames
    config.Timing0 = 0x00; // Baud rate 1000kbps
    config.Timing1 = 0x14;
    config.Mode = 0;      // Normal mode

    if (InitCAN(eRob, 0, 0, &config) != 1) {
        std::cerr << "Failed to initialize CAN channel!" << std::endl;
        CloseDevice(eRob, 0);
        return -1;
    }
    std::cout << "CAN channel initialized successfully." << std::endl;

    // 3. Start CAN channel
    if (StartCAN(eRob, 0, 0) != 1) {
        std::cerr << "Failed to start CAN channel!" << std::endl;
        CloseDevice(eRob, 0);
        return -1;
    }
    std::cout << "CAN channel started successfully." << std::endl;

    // 4. Create a CAN frame to send
    CAN_OBJ sendObj;
    std::memset(&sendObj, 0, sizeof(sendObj));
    sendObj.ID = canID; // Standard frame ID
    sendObj.SendType = 0;
    sendObj.RemoteFlag = 0; // Data frame
    sendObj.ExternFlag = 0; // Standard frame

    // Copy data to send object

    copyDataToSendObject(sendObj, data);

    // 5. Transmit CAN frame
    if (Transmit(eRob, 0, 0, &sendObj, 1) != 1) {
        std::cerr << "Failed to send CAN frame!" << std::endl;
    } else {
        std::cout << "CAN frame sent successfully." << std::endl;
    }
    // Print Data and DataLen
    std::cout << "sendObj.DataLen: " << static_cast<int>(sendObj.DataLen) << std::endl;

    // 6. Receive CAN frame (optional, can be adjusted based on needs)
    CAN_OBJ recvObj[2500];
    int len = Receive(eRob, 0, 0, recvObj, 2500, 1000);
    if (len > 0) {
        std::cout << "Received " << len << " CAN frames:" << std::endl;
        for (int i = 0; i < len; ++i) {
            std::cout << "Frame ID: 0x" << std::hex << recvObj[i].ID << std::endl;
            std::cout << "Data: ";
            for (int j = 0; j < recvObj[i].DataLen; ++j) {
                std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')<< static_cast<int>(recvObj[i].Data[j]) << " ";
            }
            std::cout << std::endl;
            // Process the position data (assuming the data is in the first 4 bytes of the response)
            if (recvObj[i].DataLen >= 4) {
                int32_t data = 0;  // 初始化
                std::cout<<static_cast<int32_t>(recvObj[i].Data[3]) <<std::endl;
                data = (static_cast<int32_t>(recvObj[i].Data[3]) << 0) +
                                  (static_cast<int32_t>(recvObj[i].Data[2]) << 8) +
                                  (static_cast<int32_t>(recvObj[i].Data[1]) << 16) +
                                  (static_cast<int32_t>(recvObj[i].Data[0]) << 24);
                std::cout << (int)data << std::endl;

                switch (sendObj.Data[1]) {
                    case 0x02:
                        std::cout << "Current Position: " << std::dec << data << std::endl;  // 十进制显示
                    break;
                    case 0x08:
                        std::cout << "Motor Current: " << std::dec << data <<" mA"<< std::endl;  // 十进制显示
                    break;
                    case 0x05:
                        std::cout << "Actual Velocity: " << std::dec << data << "dec/s"<<std::endl;  // 十进制显示
                    break;
                    default:
                        std::cout << "Unhandled Data[1] value: 0x" << std::hex << static_cast<int>(sendObj.Data[1]) << std::endl;
                    break;
                }

                //
            }
        }

    } else {
        std::cerr << "Failed to receive CAN frame or no data." << std::endl;
    }

    // 7. Close device
    CloseDevice(eRob, 0);

    return 0;
}
