/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-08-14 17:04:58
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:23:45
 * @FilePath: /Raspi2USBL/config/yamlconfig.cpp
 * @Description: See yamlconfig.h
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#include "yamlconfig.h"
#include "../tool/ColorParse.h"

YamlConfig::YamlConfig(std::string loadConfigPath) {
    try {
        yamlConfigNode_ = YAML::LoadFile(loadConfigPath);
    } catch (YAML::Exception &e) {
        std::cerr << termColor("red")
                  << "Failed to read configuration file. Please check the path and format of the configuration file!"
                  << termColor("nocolor") << std::endl;
        std::cerr << "YamlConfig::YamlConfig: " << e.what() << std::endl;
        return;
    }
    isOpenYaml_ = true;
    init();
}

bool YamlConfig::open(std::string loadConfigPath) {
    try {
        yamlConfigNode_ = YAML::LoadFile(loadConfigPath);
    } catch (YAML::Exception &e) {
        std::cerr << termColor("red")
                  << "Failed to read configuration file. Please check the path and format of the configuration file!"
                  << termColor("nocolor") << std::endl;
        std::cerr << "YamlConfig::open: " << e.what() << std::endl;
        return false;
    }
    isOpenYaml_ = true;
    init();
    return true;
}

void YamlConfig::init() {
    // saving time
    auto              now       = std::chrono::system_clock::now();
    auto              in_time_t = std::chrono::system_clock::to_time_t(now);
    // Format time using stringstream and <iomanip> library
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    timeStr = ss.str();
}

bool YamlConfig::loadConfig(SystemInfo &systemInfo) {
    // temp variables
    __attribute__((unused)) std::string strTemp1, strTemp2, strTemp3, strTemp4;
    __attribute__((unused)) bool        boolTemp1, boolTemp2, boolTemp3, boolTemp4, boolTemp5, boolTemp6, boolTemp7;
    __attribute__((unused)) int         intTemp1, intTemp2, intTemp3, intTemp4, intTemp5, intTemp6;
    __attribute__((unused)) double      doubleTemp1, doubleTemp2, doubleTemp3, doubleTemp4, doubleTemp5, doubleTemp6;
    __attribute__((unused)) std::vector<double> doubleVecTemp1;

    // load work mode
    try {
        strTemp1 = yamlConfigNode_["System"]["workMode"].as<std::string>();
    } catch (YAML::Exception &e) {
        std::cerr << termColor("red")
                  << "Failed to read work mode. Please check the path and format of the configuration file!"
                  << termColor("nocolor") << std::endl;
        std::cerr << termColor("red") << "YamlConfig::loadConfig: " << e.what() << std::endl;
        return false;
    }

    systemInfo.workMode = str2WorkMode(strTemp1);

    // load file info
    try {
        // load yaml
        boolTemp1 = yamlConfigNode_["File"]["enableGenerateSignalSave"].as<bool>();
        boolTemp2 = yamlConfigNode_["File"]["enableReceiveSignalSvae"].as<bool>();
        boolTemp3 = yamlConfigNode_["File"]["enablePositionResultSave"].as<bool>();
        boolTemp4 = yamlConfigNode_["File"]["enableCorrelationResultSave"].as<bool>();
        boolTemp5 = yamlConfigNode_["File"]["enableTOFResultSave"].as<bool>();
        boolTemp6 = yamlConfigNode_["File"]["enableBeamPatternResultSave"].as<bool>();
        boolTemp7 = yamlConfigNode_["File"]["enableSideAmpSpecResultSave"].as<bool>();
        // save to systemInfo
        systemInfo.savedFileInfo.isSaveGeneratedSignal = boolTemp1;
        systemInfo.savedFileInfo.isSaveAnalogInput     = boolTemp2;
        systemInfo.savedFileInfo.isSavePosRes          = boolTemp3;
        systemInfo.savedFileInfo.isSaveCorrelation     = boolTemp4;
        systemInfo.savedFileInfo.isSaveTOFRes          = boolTemp5;
        systemInfo.savedFileInfo.isSaveBeamPattern     = boolTemp6;
        systemInfo.savedFileInfo.isSaveSideAmpSpec     = boolTemp7;

    } catch (YAML::Exception &e) {
        std::cerr << termColor("red") << "Failed to read file enable parameter. Please check the file enable parameter"
                  << termColor("nocolor") << std::endl;
        std::cerr << "YamlConfig::File: " << e.what() << std::endl;
        return false;
    }

    // load file save path
    try {
        if (systemInfo.savedFileInfo.isSaveGeneratedSignal) {
            strTemp1 = yamlConfigNode_["File"]["generateSignalFileSavePath"].as<std::string>();
            strTemp1 = replaceKeyStr(strTemp1);
            systemInfo.savedFileInfo.GeneratedSignalFilePath = strTemp1;
        }
        if (systemInfo.savedFileInfo.isSaveAnalogInput) {
            strTemp1 = yamlConfigNode_["File"]["receiveSignalFileSavePath"].as<std::string>();
            strTemp1 = replaceKeyStr(strTemp1);
            systemInfo.savedFileInfo.AnalogInputFilePath = strTemp1;
        }
        if (systemInfo.savedFileInfo.isSavePosRes) {
            strTemp1 = yamlConfigNode_["File"]["positionResultFileSavePath"].as<std::string>();
            strTemp1 = replaceKeyStr(strTemp1);
            systemInfo.savedFileInfo.PosResFilePath = strTemp1;
        }
        if (systemInfo.savedFileInfo.isSaveCorrelation) {
            strTemp1 = yamlConfigNode_["File"]["correlationFileSavePath"].as<std::string>();
            strTemp1 = replaceKeyStr(strTemp1);
            systemInfo.savedFileInfo.CorrelationFilePath = strTemp1;
        }
        if (systemInfo.savedFileInfo.isSaveTOFRes) {
            strTemp1 = yamlConfigNode_["File"]["tofResultFileSavePath"].as<std::string>();
            strTemp1 = replaceKeyStr(strTemp1);
            systemInfo.savedFileInfo.TOFResFilePath = strTemp1;
        }
        if (systemInfo.savedFileInfo.isSaveBeamPattern) {
            strTemp1 = yamlConfigNode_["File"]["beamPatternResultFileSavePath"].as<std::string>();
            strTemp1 = replaceKeyStr(strTemp1);
            systemInfo.savedFileInfo.BeamPatternFilePath = strTemp1;
        }
        if (systemInfo.savedFileInfo.isSaveSideAmpSpec) {
            strTemp1 = yamlConfigNode_["File"]["sideAmpSpecResultFileSavePath"].as<std::string>();
            strTemp1 = replaceKeyStr(strTemp1);
            systemInfo.savedFileInfo.SideAmpSpecFilePath = strTemp1;
        }
    } catch (YAML::Exception &e) {
        std::cerr << termColor("red") << "Failed to read file save path. Please check the file save path"
                  << termColor("nocolor") << std::endl;
        std::cerr << "YamlConfig::File: " << e.what() << std::endl;
        return false;
    }

    // load signal info
    try {
        intTemp1 = yamlConfigNode_["Signal"]["signalInfo"].size();
        if ((intTemp1) % 6 != 0) {
            std::cerr << termColor("red") << "The signalInfo size is incorrect. Please check the signal info"
                      << std::endl;
            return false;
        }
        systemInfo.signalInfo.partialSignalNum = (intTemp1 + 1) / 6;
        doubleTemp1                            = yamlConfigNode_["Signal"]["sampleRate"].as<double>();
        systemInfo.signalInfo.sampleRate       = doubleTemp1;
        for (int i = 0; i < systemInfo.signalInfo.partialSignalNum; i++) {
            // load yaml
            strTemp1    = yamlConfigNode_["Signal"]["signalInfo"][i * 6].as<std::string>();
            doubleTemp2 = yamlConfigNode_["Signal"]["signalInfo"][i * 6 + 1].as<double>();
            doubleTemp3 = yamlConfigNode_["Signal"]["signalInfo"][i * 6 + 2].as<double>();
            doubleTemp4 = yamlConfigNode_["Signal"]["signalInfo"][i * 6 + 3].as<double>();
            doubleTemp5 = yamlConfigNode_["Signal"]["signalInfo"][i * 6 + 4].as<double>();
            doubleTemp6 = yamlConfigNode_["Signal"]["signalInfo"][i * 6 + 5].as<double>();
            // save to systemInfo
            systemInfo.signalInfo.signalPartial.push_back(createPartialSignal(str2SignalType(strTemp1), doubleTemp1,
                                                                              doubleTemp2, doubleTemp3, doubleTemp4,
                                                                              doubleTemp5, doubleTemp6));
        }

    } catch (YAML::Exception &e) {
        std::cerr << termColor("red") << "Failed to read signal info. Please check the signal info"
                  << termColor("nocolor") << std::endl;
        std::cerr << "YamlConfig::Signal: " << e.what() << std::endl;
        return false;
    }

    // load dataio info
    try {
        strTemp1                                 = yamlConfigNode_["DataIO"]["outputSerialName"].as<std::string>();
        strTemp2                                 = yamlConfigNode_["DataIO"]["outputSerialBaudrate"].as<std::string>();
        systemInfo.dataIOInfo.outputPortName     = strTemp1;
        systemInfo.dataIOInfo.outputPortBaudrate = strTemp2;
    } catch (YAML::Exception &e) {
        std::cerr << termColor("red") << "Failed to read dataio info. Please check the dataio info"
                  << termColor("nocolor") << std::endl;
        std::cerr << "YamlConfig::DataIO: " << e.what() << std::endl;
        return false;
    }

    // load tcp info
    try {
        // strTemp1 = yamlConfigNode_["TCP"]["serverIP"].as<std::string>();
        intTemp1                          = yamlConfigNode_["TCP"]["serverPort"].as<int>();
        intTemp2                          = yamlConfigNode_["TCP"]["connectTimeout"].as<int>();
        intTemp3                          = yamlConfigNode_["TCP"]["sendTimeout"].as<int>();
        systemInfo.tcpInfo.serverPort     = intTemp1;
        systemInfo.tcpInfo.connectTimeout = intTemp2;
        systemInfo.tcpInfo.sendTimeout    = intTemp3;
    } catch (YAML::Exception &e) {
        std::cerr << termColor("red") << "Failed to read tcp info. Please check the tcp info" << termColor("nocolor")
                  << std::endl;
        std::cerr << "YamlConfig::TCP: " << e.what() << std::endl;
        return false;
    }

    // load transmit / receive info
    switch (systemInfo.workMode) {
        case WorkMode::MODE_TRANSMIT: {
            try {
                // load yaml
                intTemp1    = yamlConfigNode_["Transmit"]["lowChannel"].as<int>();
                intTemp2    = yamlConfigNode_["Transmit"]["highChannel"].as<int>();
                doubleTemp1 = yamlConfigNode_["Transmit"]["sampleRate"].as<double>();
                intTemp3    = yamlConfigNode_["Transmit"]["duration"].as<int>();
                // save to systemInfo
                systemInfo.aoScanInfo.lowChan  = intTemp1;
                systemInfo.aoScanInfo.highChan = intTemp2;
                systemInfo.aoScanInfo.rate     = doubleTemp1;
                systemInfo.aoScanInfo.duration = intTemp3;

            } catch (YAML::Exception &e) {
                std::cerr << termColor("red") << "Failed to read transmit info. Please check the transmit info"
                          << termColor("nocolor") << std::endl;
                std::cerr << "YamlConfig::Transmit: " << e.what() << std::endl;
                return false;
            }
            break;
        }
        case WorkMode::MODE_RECEIVE: {
            try {
                // load yaml
                intTemp1    = yamlConfigNode_["Receive"]["lowChannel"].as<int>();
                intTemp2    = yamlConfigNode_["Receive"]["highChannel"].as<int>();
                intTemp3    = yamlConfigNode_["Receive"]["samplesPerChannel"].as<int>();
                doubleTemp1 = yamlConfigNode_["Receive"]["sampleRate"].as<double>();
                intTemp4    = yamlConfigNode_["Receive"]["duration"].as<int>();
                doubleTemp2 = yamlConfigNode_["Receive"]["interval"].as<double>();

                // save to systemInfo
                systemInfo.aiScanInfo.lowChan           = intTemp1;
                systemInfo.aiScanInfo.highChan          = intTemp2;
                systemInfo.aiScanInfo.samplesPerChannel = intTemp3;
                systemInfo.aiScanInfo.rate              = doubleTemp1;
                systemInfo.aiScanInfo.duration          = intTemp4;
                systemInfo.aiScanInfo.interval          = doubleTemp2;

            } catch (YAML::Exception &e) {
                std::cerr << termColor("red") << "Failed to read receive info. Please check the receive info"
                          << termColor("nocolor") << std::endl;
                std::cerr << "YamlConfig::Receive: " << e.what() << std::endl;
                return false;
            }
            // load Array Info
            try {
                // load yaml
                intTemp1    = yamlConfigNode_["Array"]["elementNumber"].as<int>();
                doubleTemp1 = yamlConfigNode_["Array"]["diameter"].as<double>();
                doubleTemp2 = yamlConfigNode_["Array"]["initReceiveGain"].as<double>();
                // doubleTemp3 = yamlConfigNode_["Array"]["maxPower"].as<double>(); // move to AGC
                // doubleTemp4 = yamlConfigNode_["Array"]["minPower"].as<double>();
                // save to systemInfo
                systemInfo.arrayInfo.arrayNum        = intTemp1;
                systemInfo.arrayInfo.arrayDiameter   = doubleTemp1;
                systemInfo.arrayInfo.initReceiveGain = doubleTemp2;
                // systemInfo.arrayInfo.maxPower        = doubleTemp3;
                // systemInfo.arrayInfo.minPower        = doubleTemp4;
            } catch (YAML::Exception &e) {
                std::cerr << termColor("red") << "Failed to read array info. Please check the array info"
                          << termColor("nocolor") << std::endl;
                std::cerr << "YamlConfig::Array: " << e.what() << std::endl;
                return false;
            }

            // load Signal Processing Info
            try {
                // load yaml
                doubleTemp1 = yamlConfigNode_["SignalProcess"]["soundSpeed"].as<double>();
                doubleTemp2 = yamlConfigNode_["SignalProcess"]["processDuration"].as<double>();
                doubleTemp3 = yamlConfigNode_["SignalProcess"]["processStartFrequency"].as<double>();
                doubleTemp4 = yamlConfigNode_["SignalProcess"]["processEndFrequency"].as<double>();
                doubleTemp5 = yamlConfigNode_["SignalProcess"]["doaStep"].as<double>();
                doubleTemp6 = yamlConfigNode_["SignalProcess"]["referenceFrequency"].as<double>();
                // save to systemInfo
                systemInfo.signalProcessInfo.soundSpeed               = doubleTemp1;
                systemInfo.signalProcessInfo.processDuration          = doubleTemp2;
                systemInfo.signalProcessInfo.startFrequency           = doubleTemp3;
                systemInfo.signalProcessInfo.endFrequency             = doubleTemp4;
                systemInfo.signalProcessInfo.doaStep                  = doubleTemp5;
                systemInfo.signalProcessInfo.referenceSignalFrequency = doubleTemp6;
            } catch (YAML::Exception &e) {
                std::cerr << termColor("red")
                          << "Failed to read signal process info. Please check the signal process info"
                          << termColor("nocolor") << std::endl;
                std::cerr << "YamlConfig::SignalProcess: " << e.what() << std::endl;
                return false;
            }
            // load AGC Info
            try {
                // load yaml
                boolTemp1   = yamlConfigNode_["AGC"]["enableAGC"].as<bool>();
                strTemp1    = yamlConfigNode_["AGC"]["serialPortName"].as<std::string>();
                strTemp2    = yamlConfigNode_["AGC"]["serialPortBaudrate"].as<std::string>();
                doubleTemp1 = yamlConfigNode_["AGC"]["initGain"].as<double>();
                doubleTemp2 = yamlConfigNode_["AGC"]["minPower"].as<double>();
                doubleTemp3 = yamlConfigNode_["AGC"]["maxPower"].as<double>();
                doubleTemp4 = yamlConfigNode_["AGC"]["minGain"].as<double>();
                doubleTemp5 = yamlConfigNode_["AGC"]["maxGain"].as<double>();

                doubleTemp6 = yamlConfigNode_["AGC"]["agcStep"].as<double>();
                // save to systemInfo
                systemInfo.agcInfo.isEnableAGC        = boolTemp1;
                systemInfo.agcInfo.serialPortName     = strTemp1;
                systemInfo.agcInfo.serialPortBaudrate = strTemp2;
                systemInfo.agcInfo.initGainValue      = doubleTemp1;
                systemInfo.agcInfo.minPower           = doubleTemp2;
                systemInfo.agcInfo.maxPower           = doubleTemp3;
                systemInfo.agcInfo.minGainValue       = doubleTemp4;
                systemInfo.agcInfo.maxGainValue       = doubleTemp5;
                systemInfo.agcInfo.gainStep           = doubleTemp6;
            } catch (YAML::Exception &e) {
                std::cerr << termColor("red") << "Failed to read AGC info. Please check the AGC info"
                          << termColor("nocolor") << std::endl;
                std::cerr << "YamlConfig::AGC: " << e.what() << std::endl;
                return false;
            }

            break;
        }
        default:
            std::cerr << termColor("red") << "Unknown work mode: " << systemInfo.workMode << termColor("nocolor")
                      << std::endl;
            break;
    }
    return true;
}

std::string YamlConfig::replaceKeyStr(const std::string str) {
    std::string filename = str;
    std::string strKey   = "${TIME}";
    // Check if ${TIME} exists
    size_t      start_pos = filename.find(strKey);
    if (start_pos != std::string::npos) {
        // Replace ${TIME} with actual time string
        filename.replace(start_pos, strKey.length(), timeStr);
    }
    // std::cout << filename << std::endl;
    return filename;
}
