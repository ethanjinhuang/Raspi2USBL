/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-08-29 23:30:54
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:36:06
 * @FilePath: /Raspi2USBL/dataio/posResSender.cpp
 * @Description: See posResSender.h
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#include "posResSender.h"

PosResSender::PosResSender(SystemInfo &systemInfo)
    : systemInfo_(systemInfo) {
    init();
}

void PosResSender::init() {
}

void PosResSender::setPosResQueue(sfq::Safe_Queue<PositionResult> *navResQueue) {
    navResQueue_ = navResQueue;
}

std::string PosResSender::data2OutputString(PositionResult &data) {
    constexpr absl::string_view outputformat1 = "%012.6f";
    constexpr absl::string_view outputformat2 = "%07.3f";
    std::string                 outputStr_;
    outputStr_.append("$USBL,,");
    outputStr_.append(absl::StrFormat(outputformat1, data.time));
    outputStr_.append(",");
    outputStr_.append(absl::StrFormat(outputformat1, data.position.x()));
    outputStr_.append(",");
    outputStr_.append(absl::StrFormat(outputformat1, data.position.y()));
    outputStr_.append(",");
    outputStr_.append(absl::StrFormat(outputformat1, data.position.z()));
    outputStr_.append(",");
    outputStr_.append(absl::StrFormat(outputformat1, data.tof));
    outputStr_.append(",");
    outputStr_.append(absl::StrFormat(outputformat2, data.doa));
    // outputStr_.append(",");
    // outputStr_.append(absl::StrFormat(outputformat_, ));
    // outputStr_.append(",");
    dataVerify_.addCheckSum(outputStr_);

    return outputStr_;
}