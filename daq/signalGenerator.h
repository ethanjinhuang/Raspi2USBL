/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-07 21:40:21
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:29:11
 * @FilePath: /Raspi2USBL/daq/signalGenerator.h
 * @Description: Signal generator
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _SIGNALGENERATOR_H_
#define _SIGNALGENERATOR_H_

#include <stdio.h>
#include <string>
#include <vector>

#include "../config/defineconfig.h"
#include "../fileio/filesaver.h"

enum SIGNAL_TYPE { SIG_SIN, SIG_COS, SIG_RISING, SIG_FALLING, SIG_CHRIP, SIG_ZERO, SIG_UNKNOWN };

std::string signalTypeToString(SIGNAL_TYPE type);

typedef struct SIGNAL_PARTIAL {
    SIGNAL_TYPE type;       // signal type
    double      sampleRate; // sample rate
    double      frequency0; // start frequency (using for chirp signal)
    double      frequency1; // end frequency (using for chirp signal)
    double      amplitude;  // amplitude
    double      phase;      // phase
    double      duration;   // duration
} SIGNAL_PARTIAL;

/***
 * @description: create partial signal
 * @param {SIGNAL_TYPE} type    signal type
 * @param {double} sampleRate   sample rate
 * @param {double} frequency0   start frequency (using for chirp signal)
 * @param {double} frequency1   end frequency (using for chirp signal)
 * @param {double} amplitude    amplitude (e.g. +-5 volt, the amplitude is 10)
 * @param {double} phase        phase
 * @param {double} duration     signal duration (sec)
 * @return {*}
 */
SIGNAL_PARTIAL createPartialSignal(SIGNAL_TYPE type, double sampleRate, double frequency0, double frequency1,
                                   double amplitude, double phase, double duration);

class SignalGenerator {
public: // public functions
    /***
     * @description: Init Signal Generator (default duration is 1 sec)
     * @param samplerate    DAQ device sample rate
     * @return
     */
    SignalGenerator(double samplerate);
    /***
     * @description: Init Signal Generator
     * @param {double} samplerate   DAQ device sample rate
     * @param {double} duration     analog output signal duration
     * @return {*}
     */
    SignalGenerator(double samplerate, double duration);
    ~SignalGenerator() = default;

    void init();

    /***
     * @description: add a partial signal config to the signal generator
     * @param {SIGNAL_PARTIAL} signalPartialInfo    partial signal config
     * @return {*}
     */
    void addSignal(SIGNAL_PARTIAL signalPartialInfo);

    /***
     * @description: add a series of partial signal config to the signal generator
     * @param {vector<SIGNAL_PARTIAL>} signalPartialInfos   partial signal config
     * @return {*}
     */
    void addSignal(std::vector<SIGNAL_PARTIAL> signalPartialInfos);

    /***
     * @description: generate analog output signal
     * @return {*}  signal array
     */
    double *generateSignal();

    /***
     * @description: save signal to file
     * @param {string} filename   file name
     * @param {int} filetype    file type
     * @return {*}
     */
    void saveSignalToFile(std::string filename, int filetype = FileSaver::TEXT);

    /***
     * @description: get signal length (all partial signal length sum up)
     * @return {*}  signal length
     */
    int getSignalLength() {
        return signalLength_;
    }

private: // private functions
    /***
     * @description: generate partial signal through the partial signal config
     * @param {SIGNAL_PARTIAL} signalPartialInfo    partial signal config
     * @return {*}  partial signal array
     */
    std::vector<double> generatePartialSignal(SIGNAL_PARTIAL signalPartialInfo);

private: // private parameters
    std::vector<double>         signalVec_;
    std::vector<SIGNAL_PARTIAL> signalPartials_;

    FileSaver filesaver_;

    double samplerate_;
    double duration_;
    double maxSignalLength_;
    double signalLength_;
    double frequency0_;
    double frequency1_;
    double amplitude_;
    double phase_;
};

inline std::string signalTypeToString(SIGNAL_TYPE type) {
    switch (type) {
        case SIG_SIN:
            return "SIG_SIN";
            break;
        case SIG_COS:
            return "SIG_COS";
            break;
        case SIG_CHRIP:
            return "SIG_CHRIP";
            break;
        case SIG_ZERO:
            return "SIG_ZERO";
            break;
        case SIG_RISING:
            return "SIG_RISING";
            break;
        case SIG_FALLING:
            return "SIG_FALLING";
            break;
        default:
            return "SIG_UNKNOWN";
            break;
    }
}

inline SIGNAL_PARTIAL createPartialSignal(SIGNAL_TYPE type, double sampleRate, double frequency0, double frequency1,
                                          double amplitude, double phase, double duration) {
    SIGNAL_PARTIAL signalPartial;
    signalPartial.type       = type;
    signalPartial.sampleRate = sampleRate;
    signalPartial.frequency0 = frequency0;
    signalPartial.frequency1 = frequency1;
    signalPartial.amplitude  = amplitude;
    signalPartial.phase      = phase;
    signalPartial.duration   = duration;
    return signalPartial;
}

#endif // _SIGNALGENERATOR_H_