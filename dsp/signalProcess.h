/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-08-19 14:35:00
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-09-18 20:16:40
 * @FilePath: /RaspiUSBL/dsp/signalProcess.h
 * @Description:
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */
#ifndef _SIGNALPROCESS_H_
#define _SIGNALPROCESS_H_

#include "../config/defineconfig.h"
#include "../core/systeminfo.h"
#include "../general/typedef.h"
#include "../tool/ColorParse.h"
#include "../tool/SafeQueue.hpp"
#include "doa.h"
#include "tof.h"

class SignalProcess : public SignalBase {
public:
    SignalProcess(SystemInfo &SystemInfo, ChannelSignalVector &refSignal);
    ~SignalProcess() = default;
    void init();

    void loadRefSignal(const ChannelSignalVector &refSignal);

    void updateInputSignal(const ChannelSignalVector &inputSignal);

    double calculateTOF();

    double calculateDOA();

    double calOptimalTOF(const std::vector<double> &tofres);

    double updateACG(); // using the TOF result to update the ACG

    void resetFlag();

    void getBeamPattern(Eigen::MatrixXd &beamPattern);

    void getTOFResult(std::vector<double> &tofRes);

    void getCorrelationResult(ChannelSignalVector &correlationResult);

    void getSignalSideAmpSpec(ChannelSignalVector &signalSideAmpSpec);


private:
    // process object
    TOF *tofProcess_;
    DOA *doaProcess_;

    // status flag
    bool isUpdateInputSignal_ = false;
    bool isLoadRefSignal_     = false;
    bool isTOFCalculated_     = false;
    bool isDOACalculated_     = false;
    bool isACGUpdated_        = false;

    // system parameter
    SystemInfo         &systemInfo_;
    PositionResult      positionResult_;
    ChannelSignalVector refSignal_;
    ChannelSignalVector signalInput_;
    ChannelSignalVector correlationResult_;
    Eigen::MatrixXd     beamPattern_;
    std::vector<double> tofRes_;
    double              tofOutput_;
    double              doaOutput_;

    // adaptive gain control
    double maxPower_;
    double receiveGain_;
    double agcPower_;
    double minPower_;
    double agcStep_;

    // temp parameter
    int processSignalLength_;
};

#endif // _SIGNALPROCESS_H_