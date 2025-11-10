/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-03 10:07:43
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:18:48
 * @FilePath: /Raspi2USBL/main.cpp
 * @Description: Main Function
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#include "config/yamlconfig.h"
#include "core/systeminfo.h"
#include "daq/ai/aiScanWithTrigger.h"
#include "daq/ao/aoScanWithTrigger.h"
#include "daq/signalGenerator.h"
#include "dataio/posResSender.h"
#include "dataio/tcpServer.h"
#include "dataio/thread_dataio.h"
#include "dataio/thread_tcpComm.h"
#include "dsp/doa.h"
#include "dsp/signalBase.h"
#include "dsp/thread_agc.h"
#include "dsp/thread_dsp.h"
#include "dsp/tof.h"
#include "fileio/thread_savefile.h"
#include "general/typedef.h"

#include <memory>
#include <stdio.h>
#define _MAIN_FUNCTION_

#ifdef _MAIN_FUNCTION_
int main(int argc, char *argv[]) {
    std::string yamlConfigPath;
    if (argc > 1) {
        yamlConfigPath = argv[1];
        std::cout << termColor("green") << "Load Config File: " << yamlConfigPath << termColor("nocolor") << std::endl;
    } else {
        yamlConfigPath = "../.config/config_pi.yaml";
        std::cout << termColor("yellow") << "No input config file path, load Default Config File: " << yamlConfigPath
                  << termColor("nocolor") << std::endl;
    }

    SystemInfo systemInfo;
    YamlConfig YamlConfig;

    ChannelSignalVector                  refSignal;
    sfq::Safe_Queue<ChannelSignalVector> dataQueue;
    sfq::Safe_Queue<ChannelSignalVector> dataSaveQueue;
    sfq::Safe_Queue<ChannelSignalVector> dataSendQueue;
    sfq::Safe_Queue<double>              agcQueue;

    // Set output queue
    sfq::Safe_Queue<PositionResult>      posResQueue;
    sfq::Safe_Queue<std::vector<double>> signalTOFQueue;
    sfq::Safe_Queue<ChannelSignalVector> signalCorrelationQueue;
    sfq::Safe_Queue<ChannelSignalVector> signalSideAmpSpecQueue;
    sfq::Safe_Queue<Eigen::MatrixXd>     beamPatternQueue;

    // Load Config from YAML
    YamlConfig.open(yamlConfigPath);
    YamlConfig.loadConfig(systemInfo);
    setDefualtDAQConfig(systemInfo);
    pinrtSystemConfig(systemInfo);

    // creat save file thread
    ThreadSaveFile threadSaveFile(&systemInfo);

    // Generate Signal
    double         *signal;
    SignalGenerator signalgernerator(systemInfo.signalInfo.sampleRate);
    // add partial signal
    for (const auto &signalPartial : systemInfo.signalInfo.signalPartial) {
        signalgernerator.addSignal(signalPartial);
    }
    // generate signal
    signal = signalgernerator.generateSignal();
    // save signal to file
    if (systemInfo.savedFileInfo.isSaveGeneratedSignal) {
        signalgernerator.saveSignalToFile(systemInfo.savedFileInfo.GeneratedSignalFilePath, FileSaver::TEXT);
    }

    // save generated signal info to systemInfo
    systemInfo.aoScanInfo.samplesPerChannel = signalgernerator.getSignalLength();

    // save generated signal to refSignal
    refSignal.resize(1, systemInfo.aoScanInfo.samplesPerChannel);
    for (int i = 0; i < systemInfo.aoScanInfo.samplesPerChannel; ++i) {
        refSignal.channels[0].push_back(signal[i]);
    }

    // Config DAQ Device
    switch (systemInfo.workMode) {
        case WorkMode::MODE_RECEIVE: { // Analog Input

            // Print Working Mode
            std::cout << termColor("reversegreen") << "Work Mode: Receive" << termColor("nocolor") << std::endl;

            // Start process thread
            // initialize dsp process thread
            ThreadDSP threadDSP(systemInfo, refSignal, dataQueue);
            // set output queue (process result)
            threadDSP.setPosResQueue(&posResQueue);
            threadDSP.setSignalTOFQueue(&signalTOFQueue);
            threadDSP.setAGCQueue(&agcQueue);
            threadDSP.setSignalCorrelationQueue(&signalCorrelationQueue);
            threadDSP.setSignalSideAmpSpecQueue(&signalSideAmpSpecQueue);
            threadDSP.setBeamPatternQueue(&beamPatternQueue);

            // Thread Save Process Result
            threadSaveFile.setBeamPatternQueue(&beamPatternQueue);
            threadSaveFile.setPosResQueue(&posResQueue);
            threadSaveFile.setCorrelationQueue(&signalCorrelationQueue);
            threadSaveFile.setTOFResQueue(&signalTOFQueue);
            threadSaveFile.setSideAmpSpecQueue(&signalSideAmpSpecQueue);

            threadSaveFile.configAutoSetFile();
            threadSaveFile.creatThread_saveProcessResult();

            // Thread AGC
            ThreadAGC threadAGC(systemInfo, &agcQueue);
            threadAGC.creatThread_agcProcess();

            // start dsp process thread
            threadDSP.creatThread_dspProcess();

            // TCP communication
            // int       port = 8080;
            tcpServer server(systemInfo.tcpInfo.serverPort);
            server.start();

            // create and start data sending thread
            ThreadTcpCommunication dataSender(systemInfo, server);
            dataSender.setDataQueue(&dataSendQueue);
            dataSender.startSending();

            AIScanInfo scanInfo;

            // config AI Scan Info
            scanInfo.flags             = systemInfo.aiScanInfo.flags;
            scanInfo.scanOption        = systemInfo.aiScanInfo.scanOption;
            scanInfo.eventTypes        = systemInfo.aiScanInfo.eventTypes;
            scanInfo.lowChan           = systemInfo.aiScanInfo.lowChan;
            scanInfo.highChan          = systemInfo.aiScanInfo.highChan;
            scanInfo.samplesPerChannel = systemInfo.aiScanInfo.samplesPerChannel;
            scanInfo.rate              = systemInfo.aiScanInfo.rate;
            scanInfo.duration          = systemInfo.aiScanInfo.duration;
            scanInfo.interval          = systemInfo.aiScanInfo.interval;

            // config file save thread
            ThreadSaveFile threadSaveFile(&systemInfo);
            if (systemInfo.savedFileInfo.isSaveAnalogInput) {
                threadSaveFile.setDAQAIFile(systemInfo.savedFileInfo.AnalogInputFilePath, FileSaver::BINARY);
                threadSaveFile.creatThread_saveDAQAIData(&dataSaveQueue);
            }

            // start scan
            // AIScanWithTrigger aiScanWithTrigger(&scanInfo, &dataQueue, &dataSaveQueue);
            AIScanWithTrigger aiScanWithTrigger(&scanInfo, &dataQueue, &dataSaveQueue, &dataSendQueue);
            aiScanWithTrigger.dataAcquisition();

            break;
        }

        case WorkMode::MODE_TRANSMIT: { // Analog Output

            // Print Working Mode
            std::cout << termColor("reverseblue") << "Work Mode: Transmit" << termColor("nocolor") << std::endl;

            AOScanInfo aoScanInfo;

            // config AO Scan Info
            aoScanInfo.flags             = systemInfo.aoScanInfo.flags;
            aoScanInfo.scanOption        = systemInfo.aoScanInfo.scanOption;
            aoScanInfo.lowChan           = systemInfo.aoScanInfo.lowChan;
            aoScanInfo.highChan          = systemInfo.aoScanInfo.highChan;
            aoScanInfo.samplesPerChannel = systemInfo.aoScanInfo.samplesPerChannel;
            aoScanInfo.rate              = systemInfo.aoScanInfo.rate;
            aoScanInfo.duration          = systemInfo.aoScanInfo.duration;

            // start transmit
            AOScanWithTrigger aoScanWithTrigger(&aoScanInfo, signal);
            aoScanWithTrigger.analogOutput();
            break;
        }

        default:
            std::cerr << termColor("red") << "Unknown work mode: " << systemInfo.workMode << termColor("nocolor")
                      << std::endl;
            break;
    }
    return 0;
}
#endif // _MAIN_FUNCTION_
