/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-02-25 15:55:54
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2024-08-29 18:11:01
 * @FilePath: /Raspi2USBL/dataio/dataverify.h
 * @Description: class for data verify, including check checksum and add checksum
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _DATAVERIFY_H_
#define _DATAVERIFY_H_

#include <cstdint>
#include <cstring>
#include <string>

#include "../config/defineconfig.h"

class DATAVERIFY {
public:
    // calculate checksum
    uint8_t calCheckSum(void *pStart, uint32_t uSize, uint32_t Init, uint32_t Xor);
    // verify checksum
    bool    verifyCheckSum(const std::string &data);
    // add checksum
    void    addCheckSum(std::string &data);
};

#endif // _DATAVERIFY_H_