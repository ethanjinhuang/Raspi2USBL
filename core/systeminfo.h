/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-04 18:54:18
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:21:42
 * @FilePath: /Raspi2USBL/core/systeminfo.h
 * @Description: Define the system information
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _SYSTEMINFO_H_
#define _SYSTEMINFO_H_

#include "../config/defineconfig.h"
#include "../daq/daqTypeDefine.h"
#include "../tool/ColorParse.h"
#include <iomanip>
#include <iostream>
#include <string>

enum WorkMode { MODE_TRANSMIT, MODE_RECEIVE, MODE_ERROR };
struct SystemInfo;

// function declaration
WorkMode    str2WorkMode(std::string str);
std::string workMode2Str(WorkMode workMode);
SIGNAL_TYPE str2SignalType(std::string str);
std::string signalType2Str(SIGNAL_TYPE signalType);
void        setDefualtDAQConfig(SystemInfo &systemInfo);
typedef struct ArrayInfo {
    int    arrayNum;
    double arrayDiameter;
    double initReceiveGain;
} ArrayInfo;

typedef struct SignalProcessInfo {
    double referenceSignalFrequency;
    double processDuration;
    double startFrequency;
    double endFrequency;
    double doaStep;
    double soundSpeed;
} SignalProcessInfo;

typedef struct AgcInfo {
    bool        isEnableAGC;
    std::string serialPortName;
    std::string serialPortBaudrate;
    double      initGainValue;
    double      maxPower;
    double      minPower;
    double      minGainValue;
    double      maxGainValue;
    double      gainStep;
} AgcInfo;

typedef struct SavedFileInfo {
    bool isSaveAnalogInput;
    bool isSaveGeneratedSignal;
    bool isSavePosRes;
    bool isSaveCorrelation;
    bool isSaveTOFRes;
    bool isSaveBeamPattern;
    bool isSaveSideAmpSpec;

    std::string AnalogInputFilePath;
    std::string GeneratedSignalFilePath;
    std::string PosResFilePath;
    std::string CorrelationFilePath;
    std::string TOFResFilePath;
    std::string BeamPatternFilePath;
    std::string SideAmpSpecFilePath;
} SavedFileInfo;

typedef struct DataIOInfo {
    std::string outputPortName;
    std::string outputPortBaudrate;
    std::string controlPortName;
    std::string controlPortBaudrate;
} DataIOInfo;

typedef struct TcpInfo {
    // std::string serverIP;
    int serverPort;
    int connectTimeout;
    int sendTimeout;
} TcpInfo;

typedef struct SystemInfo {
    WorkMode          workMode;
    SignalProcessInfo signalProcessInfo;
    AgcInfo           agcInfo;
    ArrayInfo         arrayInfo;
    DataIOInfo        dataIOInfo;
    TcpInfo           tcpInfo;
    SavedFileInfo     savedFileInfo;
    AIScanInfo        aiScanInfo;
    AOScanInfo        aoScanInfo;
    SignalInfo        signalInfo;
} SystemInfo;

inline void setDefualtDAQConfig(SystemInfo &systemInfo) {
    // AI Scan Info
    systemInfo.aiScanInfo.flags = AINSCAN_FF_DEFAULT;
    // AI Scan Option
#ifndef _DAQAI_MCC1608FSPLUS_
    systemInfo.aiScanInfo.scanOption = (ScanOption) (SO_DEFAULTIO | SO_RETRIGGER | SO_CONTINUOUS);
#else
    systemInfo.aiScanInfo.scanOption = (ScanOption) (SO_DEFAULTIO | SO_EXTTRIGGER);
#endif // _DAQAI_MCC1608FSPLUS_

    systemInfo.aiScanInfo.eventTypes =
        (DaqEventType) (DE_ON_DATA_AVAILABLE | DE_ON_INPUT_SCAN_ERROR | DE_ON_END_OF_INPUT_SCAN);

    // AO Scan Info
    systemInfo.aoScanInfo.flags      = AOUTSCAN_FF_DEFAULT;
    systemInfo.aoScanInfo.scanOption = (ScanOption) (SO_DEFAULTIO | SO_RETRIGGER | SO_CONTINUOUS);
}

inline void pinrtSystemConfig(SystemInfo &systemInfo) {
    std::cout << termColor("blue")
              << "--------------------------------Configuration Parameters Start--------------------------------"
              << termColor("nocolor") << std::endl;

    try {
        // print work mode
        std::string workModeStr = workMode2Str(systemInfo.workMode);
        std::cout << termColor("blue") << "WorkMode: " << termColor("yellow") << workModeStr << termColor("nocolor")
                  << std::endl;

        // print whether to save files
        std::cout << termColor("blue") << "Enable Transmit Signal Save: " << termColor("yellow")
                  << (systemInfo.savedFileInfo.isSaveGeneratedSignal ? "true" : "false") << termColor("nocolor")
                  << std::endl;
        std::cout << termColor("blue") << "Enable Receive Signal Save: " << termColor("yellow")
                  << (systemInfo.savedFileInfo.isSaveAnalogInput ? "true" : "false") << termColor("nocolor")
                  << std::endl;

        // print file path
        if (systemInfo.savedFileInfo.isSaveGeneratedSignal) {
            std::cout << termColor("blue") << "Generate Signal File Save Path: " << termColor("yellow")
                      << systemInfo.savedFileInfo.GeneratedSignalFilePath << termColor("nocolor") << std::endl;
        }
        if (systemInfo.savedFileInfo.isSaveAnalogInput) {
            std::cout << termColor("blue") << "Receive Signal File Save Path: " << termColor("yellow")
                      << systemInfo.savedFileInfo.AnalogInputFilePath << termColor("nocolor") << std::endl;
        }

        // print signal info
        std::cout << termColor("blue") << "Signal Sample Rate: " << termColor("yellow")
                  << systemInfo.signalInfo.sampleRate << termColor("nocolor") << std::endl;

        // print cuttent line
        std::cout << std::string(95, '-') << std::endl;
        // print signal partial info header
        std::cout << termColor("blue") << std::left << std::setw(15) << "Signal Type" << std::setw(20)
                  << "Start Frequency" << std::setw(20) << "End Frequency" << std::setw(15) << "Amplitude"
                  << std::setw(10) << "Phase" << std::setw(15) << "Duration" << termColor("nocolor") << std::endl;

        // print cuttent line
        std::cout << std::string(95, '-') << std::endl;

        // print signal partial info
        for (const auto &signal : systemInfo.signalInfo.signalPartial) {
            std::cout << termColor("yellow") << std::left << std::setw(15) << signalType2Str(signal.type)
                      << std::setw(20) << signal.frequency0 << std::setw(20) << signal.frequency1 << std::setw(15)
                      << signal.amplitude << std::setw(10) << signal.phase << std::setw(15) << signal.duration
                      << termColor("nocolor") << std::endl;
        }
        // Output separator line
        std::cout << std::string(95, '-') << std::endl;
        // Print configuration based on mode
        switch (systemInfo.workMode) {
            case WorkMode::MODE_TRANSMIT: {
                std::cout << termColor("blue") << "Transmit Low Channel: " << termColor("yellow")
                          << systemInfo.aoScanInfo.lowChan << termColor("nocolor") << std::endl;
                std::cout << termColor("blue") << "Transmit High Channel: " << termColor("yellow")
                          << systemInfo.aoScanInfo.highChan << termColor("nocolor") << std::endl;
                std::cout << termColor("blue") << "Transmit Sample Rate: " << termColor("yellow")
                          << systemInfo.aoScanInfo.rate << termColor("nocolor") << std::endl;
                std::cout << termColor("blue") << "Transmit Duration: " << termColor("yellow")
                          << systemInfo.aoScanInfo.duration << termColor("nocolor") << std::endl;
                break;
            }
            case WorkMode::MODE_RECEIVE: {
                std::cout << termColor("blue") << "Receive Low Channel: " << termColor("yellow")
                          << systemInfo.aiScanInfo.lowChan << termColor("nocolor") << std::endl;
                std::cout << termColor("blue") << "Receive High Channel: " << termColor("yellow")
                          << systemInfo.aiScanInfo.highChan << termColor("nocolor") << std::endl;
                std::cout << termColor("blue") << "Receive Samples Per Channel: " << termColor("yellow")
                          << systemInfo.aiScanInfo.samplesPerChannel << termColor("nocolor") << std::endl;
                std::cout << termColor("blue") << "Receive Sample Rate: " << termColor("yellow")
                          << systemInfo.aiScanInfo.rate << termColor("nocolor") << std::endl;
                std::cout << termColor("blue") << "Receive Duration: " << termColor("yellow")
                          << systemInfo.aiScanInfo.duration << termColor("nocolor") << std::endl;
                break;
            }
            default:
                std::cerr << termColor("red") << "Unknown work mode: " << systemInfo.workMode << termColor("nocolor")
                          << std::endl;
                break;
        }

        std::cout << termColor("blue")
                  << "---------------------------------Configuration Parameters End---------------------------------"
                  << termColor("nocolor") << std::endl;

    } catch (std::exception &e) {
        std::cerr << termColor("red") << "Failed to print configuration. Error: " << e.what() << std::endl;
    }
}

inline WorkMode str2WorkMode(std::string str) {
    if (str == "MODE_TRANSMIT") {
        return MODE_TRANSMIT;
    } else if (str == "MODE_RECEIVE") {
        return MODE_RECEIVE;
    } else {
        std::cerr << termColor("red") << "Error: Unknown work mode: " << str << termColor("nocolor") << std::endl;
        std::cout << "The standard work mode is " << termColor("yellow") << "MODE_TRANSMIT" << termColor("nocolor")
                  << " or " << termColor("yellow") << "MODE_RECEIVE" << termColor("nocolor") << std::endl;
        return MODE_ERROR;
    }
}

inline std::string workMode2Str(WorkMode workMode) {
    switch (workMode) {
        case MODE_TRANSMIT:
            return "MODE_TRANSMIT";
        case MODE_RECEIVE:
            return "MODE_RECEIVE";
        default:
            return "MODE_ERROR";
    }
}

inline SIGNAL_TYPE str2SignalType(std::string str) {
    if (str == "SIG_SIN") {
        return SIG_SIN;
    } else if (str == "SIG_COS") {
        return SIG_COS;
    } else if (str == "SIG_RISING") {
        return SIG_RISING;
    } else if (str == "SIG_FALLING") {
        return SIG_FALLING;
    } else if (str == "SIG_CHRIP") {
        return SIG_CHRIP;
    } else if (str == "SIG_ZERO") {
        return SIG_ZERO;
    } else {
        std::cerr << termColor("red") << "Error: Unknown signal type: " << str << termColor("nocolor") << std::endl;
        return SIG_UNKNOWN;
    }
}

inline std::string signalType2Str(SIGNAL_TYPE signalType) {
    switch (signalType) {
        case SIG_SIN:
            return "SIG_SIN";
        case SIG_COS:
            return "SIG_COS";
        case SIG_RISING:
            return "SIG_RISING";
        case SIG_FALLING:
            return "SIG_FALLING";
        case SIG_CHRIP:
            return "SIG_CHRIP";
        case SIG_ZERO:
            return "SIG_ZERO";
        default:
            return "SIG_UNKNOWN";
    }
}

#endif // _SYSTEMINFO_H_