/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-08 10:29:33
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:28:25
 * @FilePath: /Raspi2USBL/daq/signalGenerator.cpp
 * @Description: see signalGenerator.h
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#include "signalGenerator.h"
#include "../tool/ColorParse.h"
#include <iostream>
#include <math.h>
#include <stdexcept>

SignalGenerator::SignalGenerator(double samplerate) {
    samplerate_      = samplerate;
    maxSignalLength_ = samplerate * 1;
    signalLength_    = 0;
}

SignalGenerator::SignalGenerator(double samplerate, double duration) {
    samplerate_      = samplerate;
    duration_        = duration;
    maxSignalLength_ = samplerate * duration;
    signalLength_    = 0;
}

void SignalGenerator::init() {
    signalVec_.clear();

#ifdef _SIGNALGENERATOR_DEBUG_
    std::cout << termColor("green") << "SignalGenerator is initialized" << termColor("nocolor") << "\n";
#endif
}

void SignalGenerator::addSignal(SIGNAL_PARTIAL signalPartialInfo) {
    signalPartials_.push_back(signalPartialInfo);

#ifdef _SIGNALGENERATOR_DEBUG_
    std::cout << termColor("green") << "Signal added: " << termColor("yellow")
              << signalTypeToString(signalPartialInfo.type) << termColor("nocolor") << "\n";
#endif
}

void SignalGenerator::addSignal(std::vector<SIGNAL_PARTIAL> signalPartialInfos) {
    signalPartials_.insert(signalPartials_.end(), signalPartialInfos.begin(), signalPartialInfos.end());

#ifdef _SIGNALGENERATOR_DEBUG_
    std::cout << termColor("green") << "Signal added: " << termColor("yellow");
    for (auto signalPartial : signalPartialInfos) {
        std::cout << signalTypeToString(signalPartial.type) << " ";
    }

    std::cout << termColor("nocolor") << "\n";
#endif
}

double *SignalGenerator::generateSignal() {
    double *signal          = nullptr;

    int    partialSignalNum = signalPartials_.size();
    // check data length
    double cumulativeSignalNum = 0;
    for (int i = 0; i < partialSignalNum; i++) {
        cumulativeSignalNum += signalPartials_[i].sampleRate * signalPartials_[i].duration;
    }
    if (cumulativeSignalNum > maxSignalLength_) {
        throw std::invalid_argument("SignalGenerator::generateSignal: signal length is not enough");
    }

    // generate partial signal
    signalVec_.clear();
    std::vector<double> signalVecTemp;
    for (int i = 0; i < partialSignalNum; i++) {
        signalVecTemp = generatePartialSignal(signalPartials_[i]);
        signalVec_.insert(signalVec_.end(), signalVecTemp.begin(), signalVecTemp.end());
        signalVecTemp.clear();
    }

    signalLength_ = signalVec_.size();

    // copy the signal vector to the signal array
    signal = (double *) malloc(signalLength_ * sizeof(double));
    std::copy(signalVec_.begin(), signalVec_.end(), signal);
    // // add zero to the end of the signal
    // std::fill_n(signal + signalVec_.size(), signalLength_ - signalVec_.size(), 0);

    return signal;
}

void SignalGenerator::saveSignalToFile(std::string filename, int filetype) {
    filesaver_.open(filename, filetype);
    filesaver_.dump(signalVec_);
    filesaver_.close();
}

std::vector<double> SignalGenerator::generatePartialSignal(SIGNAL_PARTIAL signalPartialInfo) {
    std::vector<double> signalVecTemp;
    double              sampleNum = signalPartialInfo.sampleRate * signalPartialInfo.duration;
    double              dt        = 1.0 / signalPartialInfo.sampleRate;
    double              phase     = signalPartialInfo.phase;
    double              tempdata  = 0;

    signalVecTemp.clear();
    signalVecTemp.reserve(sampleNum);

    switch (signalPartialInfo.type) {
        case SIG_SIN: {
            for (int i = 0; i < sampleNum; i++) {
                tempdata = (double) (sin(phase) * signalPartialInfo.amplitude / 2);
                signalVecTemp.push_back(tempdata);
                phase += 2 * M_PI * signalPartialInfo.frequency0 * dt;
                if (phase > 2 * M_PI) {
                    phase -= 2 * M_PI;
                }
            }
            break;
        }

        case SIG_COS: {
            for (int i = 0; i < sampleNum; i++) {
                tempdata = (double) (cos(phase) * signalPartialInfo.amplitude / 2);
                signalVecTemp.push_back(tempdata);
                phase += 2 * M_PI * signalPartialInfo.frequency0 * dt;
                if (phase > 2 * M_PI) {
                    phase -= 2 * M_PI;
                }
            }
            break;
        }

        case SIG_CHRIP: {
            double k             = (signalPartialInfo.frequency1 - signalPartialInfo.frequency0) / (sampleNum - 1);
            double initialPhase  = 2 * M_PI * signalPartialInfo.frequency0 * dt; // Calculate initial phase offset
            phase               += initialPhase;                                 // From a non-zero phase

            for (int i = 0; i < sampleNum; i++) {
                double currentFrequency = signalPartialInfo.frequency0 + k * i;
                tempdata                = (double) (sin(phase) * signalPartialInfo.amplitude / 2);
                signalVecTemp.push_back(tempdata);
                phase += 2 * M_PI * currentFrequency * dt;
                if (phase > 2 * M_PI) {
                    phase -= 2 * M_PI;
                }
            }
            break;
        }
        case SIG_ZERO: {
            signalVecTemp.resize(sampleNum, 0);
            break;
        }
        case SIG_RISING: {
            signalVecTemp.resize(sampleNum, signalPartialInfo.amplitude / 2);
            break;
        }
        case SIG_FALLING: {
            signalVecTemp.resize(sampleNum, -signalPartialInfo.amplitude / 2);
            break;
        }
        default: {
            throw std::invalid_argument("SignalGenerator::generatePartialSignal: unknown signal type");
            break;
        }
    }
    return signalVecTemp;
}