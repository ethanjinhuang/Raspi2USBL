/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-08-25 00:39:39
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-09-17 17:08:26
 * @FilePath: /RaspiUSBL/dsp/thread_dsp.cpp
 * @Description: See thread_dsp.h
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */
#include "thread_dsp.h"
#include "../tool/ColorParse.h"

ThreadDSP::ThreadDSP(SystemInfo &systeminfo, ChannelSignalVector &refSignal,
                     sfq::Safe_Queue<ChannelSignalVector> &dataque)
    : systemInfo_(systeminfo)
    , refSignal_(refSignal)
    , signalQueue_(dataque) {
    init();
}

void ThreadDSP::init() {
    enableThread_dspProcess_ = true;
    signalProcess_           = new SignalProcess(systemInfo_, refSignal_);

    // init temp data
    tofOutput_ = 0.0;
    doaOutput_ = 0.0;
    agcPower_  = 0.0;
}

void ThreadDSP::creatThread_dspProcess() {
    if (enableThread_dspProcess_) {
        thread_dspProcess_ = std::thread(&ThreadDSP::dspProcess, this);
        std::cout << termColor("green") << "Thread DSP Process is created" << termColor("nocolor") << "\n";
    } else {
        std::cerr << termColor("red") << "Thread DSP Process is not created" << termColor("nocolor") << "\n";
        std::exit(EXIT_FAILURE);
    }
}

void ThreadDSP::joinThread_dspProcess() {
    if (thread_dspProcess_.joinable()) {
        thread_dspProcess_.join();
        std::cout << termColor("green") << "Thread DSP Process is joined" << termColor("nocolor") << "\n";
    }
}

void ThreadDSP::closeThread_dspProcess() {
    enableThread_dspProcess_ = false;
    joinThread_dspProcess();
    if (signalProcess_ != nullptr) {
        delete signalProcess_;
        signalProcess_ = nullptr;
    }
}

void ThreadDSP::dspProcess() {
    signalProcess_->loadRefSignal(refSignal_);

    while (enableThread_dspProcess_) {
        // get the signal from the queue
        signalInput_ = signalQueue_.wait_and_pop();
        // update the signal
        signalProcess_->updateInputSignal(signalInput_);
        // process the signal: TOF
        tofOutput_ = signalProcess_->calculateTOF();
        // process the signal: DOA
        doaOutput_ = signalProcess_->calculateDOA();
        // update the ACG
        agcPower_ = signalProcess_->updateACG();

        std::cout << "\n TOF: " << tofOutput_ << "\n DOA: " << doaOutput_ << "\n AGC: " << agcPower_ << std::endl;

        // save the result
        positionResult_.tof = tofOutput_;
        positionResult_.doa = doaOutput_;
        signalProcess_->getTOFResult(tofResult_);
        signalProcess_->getBeamPattern(beamPattern_);
        signalProcess_->getCorrelationResult(correlationResult_);
        signalProcess_->getSignalSideAmpSpec(signalSideAmpSpec_);

        // reset the process flag
        signalProcess_->resetFlag();

        // save the result to the output queue
        if (posResQueue_ != nullptr) {
            posResQueue_->push(positionResult_);
        }
        if (acgQueue_ != nullptr) {
            acgQueue_->push(agcPower_);
        }
        if (signalTOFQueue_ != nullptr) {
            signalTOFQueue_->push(tofResult_);
        }
        if (signalCorrelationQueue_ != nullptr) {
            signalCorrelationQueue_->push(correlationResult_);
        }
        if (signalSideAmpSpecQueue_ != nullptr) {
            signalSideAmpSpecQueue_->push(signalSideAmpSpec_);
        }
        if (beamPatternQueue_ != nullptr) {
            // test for relative beam pattern
            // beamPattern_.array() /= beamPattern_.maxCoeff();
            beamPatternQueue_->push(beamPattern_);
        }
    }
}

void ThreadDSP::setPosResQueue(sfq::Safe_Queue<PositionResult> *posResQueue) {
    posResQueue_ = posResQueue;
}

void ThreadDSP::setAGCQueue(sfq::Safe_Queue<double> *acgQueue) {
    acgQueue_ = acgQueue;
}

void ThreadDSP::setSignalTOFQueue(sfq::Safe_Queue<std::vector<double>> *signalTOFQueue) {
    signalTOFQueue_ = signalTOFQueue;
}

void ThreadDSP::setSignalCorrelationQueue(sfq::Safe_Queue<ChannelSignalVector> *signalCorrelationQueue) {
    signalCorrelationQueue_ = signalCorrelationQueue;
}

void ThreadDSP::setSignalSideAmpSpecQueue(sfq::Safe_Queue<ChannelSignalVector> *signalSideAmpSpecQueue) {
    signalSideAmpSpecQueue_ = signalSideAmpSpecQueue;
}

void ThreadDSP::setBeamPatternQueue(sfq::Safe_Queue<Eigen::MatrixXd> *beamPatternQueue) {
    beamPatternQueue_ = beamPatternQueue;
}
