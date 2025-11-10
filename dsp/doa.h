/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-08-19 15:04:51
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:42:40
 * @FilePath: /Raspi2USBL/dsp/doa.h
 * @Description: Degree of arrival calculation
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _DOA_H_
#define _DOA_H_

#include "signalBase.h"
#include <Eigen/Dense>

class DOA : public SignalBase {
public:
    DOA(SystemInfo &systesminfo, ChannelSignalVector &refSignal);
    DOA(SystemInfo &systeminfo, ChannelSignalEigenD &refSignal);
    ~DOA() = default;

    void init();

    /***
     * @description: Set the parameters for DOA calculation
     * @param {int} startDir                The start direction of the signal
     * @param {double} selectSigDuration    The length of the signal to be selected (unit: second)
     * @param {double} freStart             The start frequency of the signal (unit: Hz)
     * @param {double} freEnd               The end frequency of the signal (unit: Hz)
     * @param {double} doaStep              The step of the DOA calculation (unit: degree)
     * @return {*}
     */
    void setParam(int startDir, double selectSigDuration, double freStart, double freEnd, double doaStep);

    /***
     * @description: Calculate the DOA using the convensional beamforming method
     * @param {ChannelSignalVector} &signal
     * @param {double} &doa
     * @return {*}
     */
    void calculateDOA_CBF(ChannelSignalVector &signal, double &doa);

    /***
     * @description: Calculate the DOA using the convensional beamforming method
     * @param {ChannelSignalEigenD} &signal
     * @param {double} &doa
     * @return {*}
     */
    // void calculateDOA_CBF(ChannelSignalEigenD &signal, double &doa);

    void getSignalSideAmpSpec(Eigen::MatrixXd &signalSideAmpSpec) {
        if (signalSideAmpSpec.rows() != signalSideAmpSpec_.rows() ||
            signalSideAmpSpec.cols() != signalSideAmpSpec_.cols()) {
            signalSideAmpSpec.resize(signalSideAmpSpec_.rows(), signalSideAmpSpec_.cols());
        }
        signalSideAmpSpec = signalSideAmpSpec_;
    }

    void getBeamPattern(Eigen::MatrixXd &beamPattern) {
        if (beamPattern.rows() != beamPattern_.rows() || beamPattern.cols() != beamPattern_.cols()) {
            beamPattern.resize(beamPattern_.rows(), beamPattern_.cols());
        }
        beamPattern = beamPattern_;
    }

private:
    ChannelSignalVector refSignal_;
    ChannelSignalEigenD refSignalEigenD_;
    std::vector<double> signal_freq_;
    Eigen::MatrixXd     signalSideAmpSpec_;
    Eigen::MatrixXd     beamPattern_;
    bool                isSetParam_;
    int                 startDir_;
    int                 doaIndex_;
    double              selectSigDuration_;
    double              doaFreStart_;
    double              doaFreEnd_;
    double              doaStep_;
};

#endif // _DOA_H_