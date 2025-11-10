/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-08-19 23:09:32
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:51:15
 * @FilePath: /Raspi2USBL/dsp/tof.cpp
 * @Description:  See tof.h
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#include "tof.h"
#include <algorithm>

TOF::TOF(SystemInfo &systeminfo, ChannelSignalVector &refSignal)
    : SignalBase(systeminfo) {
    refSignal_       = refSignal;
    refSignalLength_ = refSignal_.signalLength;
    init();
}

TOF::TOF(SystemInfo &systeminfo, ChannelSignalEigenD &refSignal)
    : SignalBase(systeminfo)
    , refSignalLength_(refSignal.signalLength)
    , refSignalEigenD_(refSignal) {
    init();
}

void TOF::init() {
    maxIndex_.clear();
    maxIndex_.resize(systemInfo_.arrayInfo.arrayNum);
}

void TOF::calculateTOF(ChannelSignalVector &signal, std::vector<double> &tof) {
    if (!signal.isInit) {
        throw std::runtime_error("TOF::calculateTOF: signal is not initialized");
    }

    // resize the tof vector
    tof.resize(signal.channelNum);

    // flip the reference signal
    ChannelSignalVector refSiganl_fliplr = refSignal_;
    std::reverse(refSiganl_fliplr.channels[0].begin(), refSiganl_fliplr.channels[0].end());

    // matching filter
    ChannelSignalVector signal_conv;
    csvconv_valid(signal, refSiganl_fliplr, signal_conv);

    // find the max value
    for (int i = 0; i < signal.channelNum; ++i) {
        maxIndex_[i] = std::max_element(signal_conv.channels[i].begin(), signal_conv.channels[i].end()) -
                       signal_conv.channels[i].begin();
        // tof[i] = (double) maxIndex_[i] / systemInfo_.signalInfo.sampleRate;
        tof[i] = (double) maxIndex_[i] / systemInfo_.signalProcessInfo.referenceSignalFrequency;
    }

    // save the correlation result
    correlationResult_ = signal_conv;
}

void TOF::calculateTOF(ChannelSignalEigenD &signal, std::vector<double> &tof) {
    if (!signal.isInit) {
        throw std::runtime_error("TOF::calculateTOF: signal is not initialized");
    }

    // resize the tof vector
    tof.resize(signal.channelNum);

    // flip the reference signal
    ChannelSignalEigenD refSignal_fliplr = refSignalEigenD_;
    for (int i = 0; i < refSignalEigenD_.channelNum; ++i) {
        std::reverse(refSignal_fliplr.channels.row(i).data(),
                     refSignal_fliplr.channels.row(i).data() + refSignal_fliplr.signalLength);
    }

    // matching filter
    ChannelSignalEigenD signal_conv;
    csedconv_valid(signal, refSignal_fliplr, signal_conv);

    // find the max value
    for (int i = 0; i < signal.channelNum; ++i) {
        maxIndex_.push_back(
            std::distance(signal_conv.channels.row(i).data(),
                          std::max_element(signal_conv.channels.row(i).data(),
                                           signal_conv.channels.row(i).data() + signal_conv.signalLength)));
        // tof[i] = static_cast<double>(maxIndex_[i]) / systemInfo_.aiScanInfo.rate;、
        tof[i] = static_cast<double>(maxIndex_[i]) / systemInfo_.signalProcessInfo.referenceSignalFrequency;
    }

    // save the correlation result
    csed2csv(signal_conv, correlationResult_);
}