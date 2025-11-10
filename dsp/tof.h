/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-08-19 15:04:46
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:53:01
 * @FilePath: /Raspi2USBL/dsp/tof.h
 * @Description: Time of flight calculation
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _TOF_H_
#define _TOF_H_

#include "signalBase.h"

class TOF : public SignalBase {
public:
    TOF(SystemInfo &systeminfo, ChannelSignalVector &refSignal);
    TOF(SystemInfo &systeminfo, ChannelSignalEigenD &refSignal);
    ~TOF() = default;

    void init();
    void calculateTOF(ChannelSignalVector &signal, std::vector<double> &tof);
    void calculateTOF(ChannelSignalEigenD &signal, std::vector<double> &tof);

    ChannelSignalVector getCorrelationResult() {
        return correlationResult_;
    }

    std::vector<int> getMaxIndex() {
        return maxIndex_;
    }

private:
    int                 refSignalLength_;
    ChannelSignalVector correlationResult_;
    std::vector<int>    maxIndex_;
    ChannelSignalVector refSignal_;
    ChannelSignalEigenD refSignalEigenD_;
};

#endif // _TOF_H_