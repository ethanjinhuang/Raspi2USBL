/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-08-19 23:09:27
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-09-05 17:05:11
 * @FilePath: /RaspiUSBL/dsp/doa.cpp
 * @Description: See doa.h
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */
#include "doa.h"
#include "../general/convert.h"

DOA::DOA(SystemInfo &systesminfo, ChannelSignalVector &refSignal)
    : SignalBase(systesminfo)
    , refSignal_(refSignal) {
    init();
}

DOA::DOA(SystemInfo &systeminfo, ChannelSignalEigenD &refSignal)
    : SignalBase(systeminfo)
    , refSignalEigenD_(refSignal) {
    init();
}

void DOA::init() {
    isSetParam_ = false;
}

void DOA::setParam(int startDir, double selectSigDuration, double freStart, double freEnd, double doaStep) {
    selectSigDuration_ = selectSigDuration;
    startDir_          = startDir;
    doaFreStart_       = freStart;
    doaFreEnd_         = freEnd;
    doaStep_           = doaStep;
    isSetParam_        = true;
}

void DOA::calculateDOA_CBF(ChannelSignalVector &signal, double &doa) {
    // check if the doa parameters are set
    if (!isSetParam_) {
        throw std::runtime_error("DOA::calculateDOA_CBF: DOA parameters are not set");
    }
    // check if the signal is initialized
    if (!signal.isInit) {
        throw std::runtime_error("DOA::calculateDOA_CBF: signal is not initialized");
    }

    // calculate parameters for beamforming
    int doaSignalLength = static_cast<int>(selectSigDuration_ * systemInfo_.signalInfo.sampleRate);
    int dirFFTStart     = static_cast<int>((doaFreStart_ * doaSignalLength) / systemInfo_.signalInfo.sampleRate);
    int dirFFTEnd       = static_cast<int>((doaFreEnd_ * doaSignalLength) / systemInfo_.signalInfo.sampleRate);

    // trim the signal
    ChannelSignalVector signal_trim = signal;
    dataTrim(signal_trim, startDir_, startDir_ + doaSignalLength - 1);

    // fft the signal
    ChannelSignalComplex signal_fft(systemInfo_.arrayInfo.arrayNum, signal_trim.signalLength);
    csvfft(signal_trim, signal_fft);
    Eigen::MatrixXcd signal_fft_eigen(signal_fft.channelNum, signal_fft.signalLength);
    signal_fft_eigen.setZero();
    for (int i = 0; i < signal_fft.channelNum; ++i) {
        for (int j = 0; j < signal_fft.signalLength; ++j) {
            signal_fft_eigen(i, j) = signal_fft.channels[i][j] / static_cast<double>(doaSignalLength);
            // std::cout << signal_fft_eigen(i, j) << std::endl;
            if (j > 0 && j < signal_fft.signalLength - 1) {
                signal_fft_eigen(i, j) *= 2.0;
            }
        }
    }

    // calculate the signal frequency using Eigen
    Eigen::VectorXd signal_freq = Eigen::VectorXd::LinSpaced(
        signal_fft.signalLength, 0,
        (systemInfo_.aiScanInfo.rate * (signal_fft.signalLength - 1)) / signal_fft.signalLength);

    signalSideAmpSpec_.resize(signal_fft.channelNum + 1, signal_fft.signalLength);
    signalSideAmpSpec_.setZero();
    signalSideAmpSpec_.block(0, 0, 1, signal_fft.signalLength)                     = signal_freq.transpose();
    signalSideAmpSpec_.block(1, 0, signal_fft.channelNum, signal_fft.signalLength) = signal_fft_eigen.array().abs();

    // half the matrix to acquire signal side amplitude spectrum
    int             halfSignalLength = signal_fft.signalLength / 2;
    Eigen::MatrixXd signalSideAmpSpec_half(signal_fft.channelNum + 1, halfSignalLength);
    signalSideAmpSpec_half = signalSideAmpSpec_.block(0, 0, signal_fft.channelNum + 1, halfSignalLength);
    signalSideAmpSpec_     = signalSideAmpSpec_half;

    // calculate array position
    Eigen::VectorXd ArrayXPos(systemInfo_.arrayInfo.arrayNum);
    Eigen::VectorXd ArrayYPos(systemInfo_.arrayInfo.arrayNum);
    for (int i = 0; i < systemInfo_.arrayInfo.arrayNum; ++i) {
        ArrayXPos(i) = systemInfo_.arrayInfo.arrayDiameter / 2.0 * cos(2 * M_PI * i / systemInfo_.arrayInfo.arrayNum);
        ArrayYPos(i) = systemInfo_.arrayInfo.arrayDiameter / 2.0 * sin(2 * M_PI * i / systemInfo_.arrayInfo.arrayNum);
    }

    Eigen::MatrixXd beamPattern =
        Eigen::MatrixXd::Zero(dirFFTEnd - dirFFTStart + 1, static_cast<int>(360.0 / doaStep_));
    for (int fk = dirFFTStart; fk <= dirFFTEnd; ++fk) {
        for (double theta = -180 + doaStep_; theta < 180; theta += doaStep_) {
            Eigen::VectorXd distance =
                ArrayXPos.array() * cos(theta * M_PI / 180.0) + ArrayYPos.array() * sin(theta * M_PI / 180.0);

            std::complex<double> exponent =
                std::complex<double>(0, 2 * M_PI * signal_freq[fk] / systemInfo_.signalProcessInfo.soundSpeed);

            // calculate the exponential part
            Eigen::ArrayXcd exp_part = (exponent * distance.array()).exp() / (double) systemInfo_.arrayInfo.arrayNum;

            // [Attention] Should be noticed that the conjugate of the signal_fft_eigen should be used!!!
            std::complex<double> bn = (signal_fft_eigen.col(fk).transpose().conjugate() * exp_part.matrix()).sum();

            // calculate the index of the beam pattern
            int n2 = static_cast<int>(round((theta + 180) / doaStep_)) - 1;

            // save the beam pattern with the frequency index
            beamPattern(fk - dirFFTStart, n2) = std::abs(bn * bn);
        }
    }

    // Sum over the frequency range
    Eigen::VectorXd bp = beamPattern.colwise().sum();
    bp.maxCoeff(&doaIndex_);
    doa          = -180.0 + (doaIndex_ + 1) * doaStep_;
    beamPattern_ = beamPattern;
}
