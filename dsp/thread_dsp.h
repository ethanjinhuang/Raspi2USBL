/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-08-25 00:39:39
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-09-17 17:05:33
 * @FilePath: /RaspiUSBL/dsp/thread_dsp.h
 * @Description: See thread_dsp.h
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */
#ifndef _THREAD_DSP_H_
#define _THREAD_DSP_H_

#include "../tool/SafeQueue.hpp"
#include "signalProcess.h"
#include <chrono>
#include <thread>

class ThreadDSP {
public:
    explicit ThreadDSP(SystemInfo &systeminfo, ChannelSignalVector &refSignal,
                       sfq::Safe_Queue<ChannelSignalVector> &dataque);
    ~ThreadDSP() = default;

    void init();

    // create thread for dsp process
    void creatThread_dspProcess();
    // join thread for dsp process
    void joinThread_dspProcess();
    // close thread for dsp process
    void closeThread_dspProcess();

    // dsp process function
    void dspProcess();

    // set output queue
    // set position result queue
    void setPosResQueue(sfq::Safe_Queue<PositionResult> *posResQueue);
    // set AGC power queue
    void setAGCQueue(sfq::Safe_Queue<double> *acgQueue);
    // set signal TOF result output queue
    void setSignalTOFQueue(sfq::Safe_Queue<std::vector<double>> *signalTOFQueue);
    // set signal correlation result output queue
    void setSignalCorrelationQueue(sfq::Safe_Queue<ChannelSignalVector> *signalCorrelationQueue);
    // set signal side amplitude spectrum output queue
    void setSignalSideAmpSpecQueue(sfq::Safe_Queue<ChannelSignalVector> *signalSideAmpSpecQueue);
    // set beam pattern output queue
    void setBeamPatternQueue(sfq::Safe_Queue<Eigen::MatrixXd> *beamPatternQueue);

private:
    SystemInfo                           &systemInfo_;
    ChannelSignalVector                  &refSignal_;
    sfq::Safe_Queue<ChannelSignalVector> &signalQueue_;

    // process object
    SignalProcess *signalProcess_;

    // process temp data
    ChannelSignalVector signalInput_;
    double              tofOutput_;
    double              doaOutput_;
    PositionResult      positionResult_;
    double              agcPower_;
    std::vector<double> tofResult_;
    ChannelSignalVector correlationResult_;
    ChannelSignalVector signalSideAmpSpec_;
    Eigen::MatrixXd     beamPattern_;

    // thread
    std::thread thread_dspProcess_;

    // output queue
    sfq::Safe_Queue<PositionResult>      *posResQueue_            = nullptr;
    sfq::Safe_Queue<double>              *acgQueue_               = nullptr;
    sfq::Safe_Queue<std::vector<double>> *signalTOFQueue_         = nullptr;
    sfq::Safe_Queue<ChannelSignalVector> *signalCorrelationQueue_ = nullptr;
    sfq::Safe_Queue<ChannelSignalVector> *signalSideAmpSpecQueue_ = nullptr;
    sfq::Safe_Queue<Eigen::MatrixXd>     *beamPatternQueue_       = nullptr;

    // status flag
    bool enableThread_dspProcess_ = false;
};

#endif // _THREAD_DSP_H_