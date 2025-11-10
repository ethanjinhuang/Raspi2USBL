/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-04 16:26:22
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:39:16
 * @FilePath: /Raspi2USBL/dataio/serialDriver.h
 * @Description: 2024-09-16 add HEX read and write
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef SERIAL_DRIVER_H
#define SERIAL_DRIVER_H

#include <cstdint>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <termio.h>
#include <thread>
#include <unistd.h>

#include "../config/defineconfig.h"
class SerialDriver {
public:
    SerialDriver(const std::string &serial_port_name, std::string &baudrate);
    SerialDriver()                                  = default;
    ~SerialDriver()                                 = default;
    SerialDriver(const SerialDriver &a)             = delete;
    SerialDriver &operator=(const SerialDriver &a)  = delete;
    SerialDriver(SerialDriver &&a)                  = delete;
    SerialDriver &operator=(const SerialDriver &&a) = delete;

    void        open(const std::string &serial_port_name, const std::string &baudrate);
    bool        isOpen() const;
    // read
    bool        readByte(uint8_t &byte);
    bool        readHexByte(std::string &hexString);
    int         readBytes(uint8_t *buffer, const unsigned int packetsize);
    int         readHexBytes(std::string &hexString, const unsigned int packetsize);
    // int readString(char* buffer, const unsigned int buffer_size);
    std::string readString();
    // write
    bool        writeByte(uint8_t byte);
    bool        writeHexString(const std::string &hex_string);
    bool        flush();
    void        close();

private:
    int            file_handle_;
    struct termios config_;
};

#endif // SERIAL_DRIVER_H