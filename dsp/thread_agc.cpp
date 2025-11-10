/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-09-16 23:19:09
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2025-04-17 20:54:46
 * @FilePath: /RaspiUSBL/dsp/thread_agc.cpp
 * @Description:
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */
#include "thread_agc.h"
// #include <cctype>

ThreadAGC::ThreadAGC(SystemInfo &systeminfo, sfq::Safe_Queue<double> *agcDataque)
    : systemInfo_(systeminfo)
    , agcDataque_(agcDataque) {

    init();

    if (serial_ != nullptr) {
        isSerialPortSet_ = true;
    }
    if (agcDataque_ != nullptr) {
        isAGCQueueSet_ = true;
    }
}

void ThreadAGC::init() {
    if (systemInfo_.agcInfo.isEnableAGC) {
        enableThread_agcProcess_ = true;
        // set agc parameter
        initGainValue_ = systemInfo_.agcInfo.initGainValue;
        gainValue_     = initGainValue_;
        gainMin_       = systemInfo_.agcInfo.minGainValue;
        gainMax_       = systemInfo_.agcInfo.maxGainValue;
        gainStep_      = systemInfo_.agcInfo.gainStep;

        // open serial port
        serial_ = new SerialDriver();
        serial_->open(systemInfo_.agcInfo.serialPortName, systemInfo_.agcInfo.serialPortBaudrate);
    }
}

bool ThreadAGC::setSerialPort(SerialDriver *serialPort) {
    serial_          = serialPort;
    isSerialPortSet_ = true;
    return true;
}

bool ThreadAGC::setAGCQueue(sfq::Safe_Queue<double> *agcDataque) {
    agcDataque_    = agcDataque;
    isAGCQueueSet_ = true;
    return true;
}

std::string ThreadAGC::creatDACCommand(double voltageValue) {
    std::string command;
    if (checkVoltageValue(voltageValue)) {
        int voltageInt = static_cast<int>(voltageValue);
        int voltageDec = std::round((voltageValue - voltageInt) * 100);
        if (voltageDec >= 100) { // avoid the decimal part is 100
            voltageDec = 99;
        }
        std::ostringstream ss;
        ss << std::setw(2) << std::setfill('0') << voltageInt;
        ss << std::setw(2) << std::setfill('0') << voltageDec;
        command = "5A01" + ss.str() + "A5";
    } else {
        command = "5A010000A5";
        std::cerr << termColor("red") << "The voltage value is out of range, reset voltage to 0.0 V."
                  << termColor("nocolor") << "\n";
    }
    return command;
}

bool ThreadAGC::sendDACCommand(double voltageValue) {
    if (isSerialPortSet_ && serial_->isOpen()) {
        // clear the serial buffer
        serial_->flush();
        // send the command to DAC
        std::string command = creatDACCommand(voltageValue);
        serial_->writeHexString(command);
        // check the command is sent successfully
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // wait for 10 ms to receive the response
        std::string response;
        serial_->readHexBytes(response, command.length());          // read response
        transform(response.begin(), response.end(), response.begin(), ::toupper);
        if (response == command) {
#ifdef _THREAD_AGC_DEBUG_
            std::cout << termColor("green") << "The command is sent successfully: " << command << termColor("nocolor")
                      << "\n";
#endif
            return true;
        } else {
            std::cerr << termColor("red") << "The command is not sent successfully: Response Error."
                      << termColor("nocolor") << "\n";
            return false;
        }
    }
    return false;
}

bool ThreadAGC::checkVoltageValue(double voltageValue) {
    if (voltageValue < gainMin_ || voltageValue > gainMax_) {
        // std::cerr << termColor("red") << "The voltage value is out of range, reset voltage to 0.0 V."
        //           << termColor("nocolor") << "\n";
        return false;
    }
    return true;
}

void ThreadAGC::processAGC() {
    if (enableThread_agcProcess_ && isSerialPortSet_ && isAGCQueueSet_) {
        // send the init gain value to DAC
        sendDACCommand(initGainValue_);

        while (enableThread_agcProcess_) {
            // get the agc value from the queue
            gainValue_ = agcDataque_->wait_and_pop();
#ifdef _THREAD_AGC_DEBUG_
            std::cout << termColor("blue") << "Received Gain Voltage: " << gainValue_ << termColor("nocolor")
                      << std::endl;
#endif // _THREAD_AGC_DEBUG_
       // check the gain value
            if (checkVoltageValue(gainValue_)) {
                // send the command to DAC
                sendDACCommand(gainValue_);
            } else {
                // reset the gain value
                gainValue_ = initGainValue_;
                sendDACCommand(gainValue_);
#ifdef _THREAD_AGC_DEBUG_
                std::cerr << termColor("red")
                          << "The voltage value is out of range, reset voltage to init voltage:" << initGainValue_
                          << " V." << termColor("nocolor") << "\n";
#endif
            }
        }
    }
}

void ThreadAGC::creatThread_agcProcess() {
    if (systemInfo_.agcInfo.isEnableAGC) {
        if (enableThread_agcProcess_) {
            thread_agcProcess_ = std::thread(&ThreadAGC::processAGC, this);
            std::cout << termColor("green") << "Thread AGC Process is created" << termColor("nocolor") << "\n";
        } else {
            std::cerr << termColor("red") << "Thread AGC Process is not created" << termColor("nocolor") << "\n";
            std::exit(EXIT_FAILURE);
        }
    } else {
        std::cout << termColor("yellow") << "Thread AGC Process is disable in config." << termColor("nocolor") << "\n";
    }
}

void ThreadAGC::joinThread_agcProcess() {
    if (thread_agcProcess_.joinable()) {
        thread_agcProcess_.join();
        std::cout << termColor("green") << "Thread AGC Process is joined" << termColor("nocolor") << "\n";
    }
}

void ThreadAGC::closeThread_agcProcess() {
    enableThread_agcProcess_ = false;
    joinThread_agcProcess();
    if (serial_ != nullptr) {
        serial_->close();
        delete serial_;
        serial_ = nullptr;
    }
}
