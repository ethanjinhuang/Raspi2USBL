/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-09-16 23:18:59
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:50:39
 * @FilePath: /Raspi2USBL/dsp/thread_agc.h
 * @Description: Recevied signal AGC and send to DAC to control the amplification gain
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _THREAD_AGC_H_
#define _THREAD_AGC_H_

#include "../dataio/serialDriver.h"
#include "../tool/ColorParse.h"
#include "../tool/SafeQueue.hpp"
#include "signalProcess.h"

#include <chrono>
#include <thread>

class ThreadAGC {
public:
    explicit ThreadAGC(SystemInfo &systeminfo, sfq::Safe_Queue<double> *agcDataque);
    ThreadAGC()  = default;
    ~ThreadAGC() = default;

    void init();

    bool setSerialPort(SerialDriver *serialPort);
    bool setAGCQueue(sfq::Safe_Queue<double> *agcDataque);

    std::string creatDACCommand(double voltageValue);

    bool sendDACCommand(double voltageValue);

    bool checkVoltageValue(double voltageValue);

    void processAGC();

    void creatThread_agcProcess();

    void joinThread_agcProcess();

    void closeThread_agcProcess();

private:
    bool enableThread_agcProcess_ = false;
    bool isSerialPortSet_         = false;
    bool isAGCQueueSet_           = false;

    SystemInfo               systemInfo_;
    SerialDriver            *serial_     = nullptr;
    sfq::Safe_Queue<double> *agcDataque_ = nullptr;

    // thread
    std::thread thread_agcProcess_;

    // gain parameter
    double initGainValue_ = 0.0;
    double gainValue_     = 0.0;
    double gainMin_       = 0.0;
    double gainMax_       = 0.0;
    double gainStep_      = 0.0;
};

#endif // _THREAD_AGC_H_