/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-06-15 20:14:27
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:35:18
 * @FilePath: /Raspi2USBL/dataio/dataverify.cpp
 * @Description: See dataverify.h
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#include "dataverify.h"

uint8_t DATAVERIFY::calCheckSum(void *pStart, uint32_t uSize, uint32_t Init, uint32_t Xor) {
    uint8_t  uCRCValue;
    uint8_t *pData = static_cast<uint8_t *>(pStart); // Explicitly cast
    uCRCValue      = Init;
    // Your checksum calculation logic goes here
    while (uSize--) {
        uCRCValue ^= *pData++;
    }
    return uCRCValue ^ Xor;
}

bool DATAVERIFY::verifyCheckSum(const std::string &data) {
    const char *p_buff = data.c_str();

    // find the position of "*"
    const char *p_asterisk = std::strstr(p_buff, "*");

    // If no "*" is found, or the position of "*" is before the end of the string, return false
    if (p_asterisk == nullptr || p_asterisk >= p_buff + data.size() - 2) {
        return false;
    }

    // Parse the received checksum, convert hex to integer
    char         *endptr;
    unsigned long receivedChecksum = std::strtoul(p_asterisk + 1, &endptr, 16);
    // if (*endptr != '\0' || endptr >= p_buff + data.size() - 2)
    // {
    //     return false;
    // }

    // Calculate the range of the checksum, from the second character to before "*"
    uint32_t cs_len = p_asterisk - (p_buff + 1);

    // Call calChecksum to calculate the checksum
    uint8_t calculatedChecksum = calCheckSum(const_cast<char *>(p_buff + 1), cs_len, 0, 0);

    // Return whether it matches
    return calculatedChecksum == receivedChecksum;
}

void DATAVERIFY::addCheckSum(std::string &data) {
    // check if the string is empty
    if (data.empty()) {
        return;
    }
    const char *p_buff = data.c_str();
    // calculate the length of the string
    uint32_t    cs_len = data.size() - 1;
    // call calChecksum to calculate the checksum
    uint8_t     calculatedChecksum = calCheckSum(const_cast<char *>(p_buff + 1), cs_len, 0, 0);
    // convert the checksum to string
    char        checksumStr[3];
    std::sprintf(checksumStr, "%02X", calculatedChecksum);
    // add "*" to the end of the string
    data += "*";
    // add the checksum to the end of the string
    data += checksumStr;
    // add "\r\n" to the end of the string
    data += "\r\n";
}
