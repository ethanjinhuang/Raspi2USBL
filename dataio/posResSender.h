/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-08-29 18:07:30
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:37:06
 * @FilePath: /Raspi2USBL/dataio/posResSender.h
 * @Description: nav result sender
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _POSRESSENDER_H_
#define _POSRESSENDER_H_

#include "../core/systeminfo.h"
#include "../general/typedef.h"
#include "../tool/ColorParse.h"
#include "absl/strings/str_format.h"
#include "dataverify.h"
#include <iomanip>
#include <iostream>
#include <string>

class PosResSender {
public:
    // PosResSender();
    PosResSender(SystemInfo &systemInfo);

    void init();

    void setPosResQueue(sfq::Safe_Queue<PositionResult> *navResQueue);

    std::string data2OutputString(PositionResult &data);

private:
    SystemInfo &systemInfo_;

    sfq::Safe_Queue<PositionResult> *navResQueue_ = nullptr;

    DATAVERIFY dataVerify_;
};

#endif // _POSRESSENDER_H_