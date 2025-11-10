/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-03 14:21:32
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:26:53
 * @FilePath: /Raspi2USBL/daq/daqTypeDefine.h
 * @Description: define daq config and type
 * @             2024-08-18 using ChannelSignalVector replace std::vector<double>
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _DAQTYPEDEFINE_H_
#define _DAQTYPEDEFINE_H_

#include "../config/defineconfig.h"
#include "../general/typedef.h"
#include "../tool/SafeQueue.hpp"
#include "signalGenerator.h"
#include "uldaq.h"
#include "utility.h"
#include <iostream>
#include <stdlib.h>
#include <vector>

#define MAX_DEV_COUNT           100
#define MAX_STR_LENGTH          64
#define MAX_SCAN_OPTIONS_LENGTH 256

typedef struct AIScanEventParameters {
    // Data Buffer
    int                                   bufferSize;    // data buffer size
    double                               *buffer;        // data buffer
    sfq::Safe_Queue<ChannelSignalVector> *dataQueue;     // data queue
    sfq::Safe_Queue<ChannelSignalVector> *dataSaveQueue; // data save queue
    sfq::Safe_Queue<ChannelSignalVector> *dataSendQueue; // data send queue

    // Scan Parameter
    int    lowChan;  // first Channel
    int    highChan; // last Channel
    double rate;     // Channel Sample num per second
    double interval; // scan interval

#ifdef _DAQAI_MCC1608FSPLUS_
    DaqDeviceHandle daqDeviceHandle;
    AiInputMode     aiInputMode;
    Range           range;
    int             samplesPerChannel;
    // double rate;
    ScanOption      scanOptions;
    AInScanFlag     flag;
#endif // _DAQAI_MCC1608FSPLUS_
} AIScanEventParameters;

typedef struct AIScanInfo {
    int    lowChan;           // * [S] first Channel
    int    highChan;          // * [S] last Channel
    int    samplesPerChannel; // * [S] Channel cumulate Sample num
    double rate;              // * [S] Channel Sample num per second

    AInScanFlag  flags;       // * [S] Data is returned with scaling and calibration factors applied control
    ScanOption   scanOption;  // * [S] Scan Option
    DaqEventType eventTypes;  // * [S] Event Type
    int          availableSampleCount = 0; // * [S] Event Trigger sample num

    // process parameter
    int    duration; // * [S] scan duration
    double interval; // * [S] scan interval
} AIScanInfo;

typedef struct AOScanInfo {
    int    lowChan;           // * [S] first Channel
    int    highChan;          // * [S] last Channel
    int    samplesPerChannel; // * [S] Channel cumulate Sample num
    double rate;              // * [S] Channel Sample num per second

    AOutScanFlag flags;       // * [S] Data is returned with scaling and calibration factors applied control
    ScanOption   scanOption;  // * [S] Scan Option
    // DaqEventType eventTypes;        // * [S] Event Type
    // int availableSampleCount = 0;   // * [S] Event Trigger sample num

    // process parameter
    int duration; // * [S] scan duration
} AOScanInfo;

typedef struct SignalInfo {
    int                         partialSignalNum;
    double                      sampleRate;
    std::vector<SIGNAL_PARTIAL> signalPartial;
} SignalInfo;

#endif // _DAQTYPEDEFINE_H_