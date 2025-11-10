/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-09-08 unknown
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:40:28
 * @FilePath: /Raspi2USBL/dataio/thread_dataio.cpp
 * @Description:
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */
#include "thread_dataio.h"

ThreadDataIO::ThreadDataIO(SystemInfo &systemInfo)
    : systemInfo_(systemInfo) {
    init();
}

void ThreadDataIO::init() {
}

void ThreadDataIO::setPosResQueue(sfq::Safe_Queue<PositionResult> *posResQueue) {
    posResQueue_ = posResQueue;
    // create the sender
    posResSender_ = new PosResSender(systemInfo_);
}

void ThreadDataIO::openPosResSerial(const std::string &serial_port_name, const std::string &baudrate) {
    posResSerial_.open(serial_port_name, baudrate);
    posResSerial_.flush();

#ifdef _THREAD_DATAIO_DEBUG_
    std::cout << termColor("green") << "Serial Port: Position Result Sender is opened" << termColor("nocolor") << "\n";
#endif
}

void ThreadDataIO::closePosResSerial() {
    posResSerial_.close();

#ifdef _THREAD_DATAIO_DEBUG_
    std::cout << termColor("red") << "Serial Port: Position Result Sender is closed" << termColor("nocolor") << "\n";
#endif
}

void ThreadDataIO::sendPosResult() {
    if (posResQueue_ == nullptr) {
        std::cerr << termColor("red") << "Position Result Queue is not set" << termColor("nocolor") << "\n";
        exit(EXIT_FAILURE);
    }
    // get the data from the queue
    while (true) {
        posResData_ = posResQueue_->wait_and_pop();
        // send the data
        tempStr_.clear();
        tempStr_ = posResSender_->data2OutputString(posResData_);
        sendString(&posResSerial_, tempStr_);

#ifdef _THREAD_DATAIO_DEBUG_
        std::cout << termColor("green") << "Position Result is sent: " << termColor("nocolor");
        std::cout << termColor("yellow") << tempStr_ << termColor("nocolor");
#endif // _THREAD_DATAIO_DEBUG_
    }
}

// THREAD FUNCTION
void ThreadDataIO::creatThreadSendPosResult(sfq::Safe_Queue<PositionResult> *posResQueue) {
    if (posResQueue == nullptr) {
        std::cerr << termColor("red") << "Position Result Queue is not set" << termColor("nocolor") << "\n";
        exit(EXIT_FAILURE);
    }
    if (!posResSerial_.isOpen()) {
        std::cerr << termColor("red") << "Serial Port: Position Result is not open" << termColor("nocolor") << "\n";
        exit(EXIT_FAILURE);
    }
    posResSerial_.flush();
    posResQueue_         = posResQueue;
    threadSendPosResult_ = std::thread(&ThreadDataIO::sendPosResult, this);
}

void ThreadDataIO::joinThreadSendPosResult() {
    if (threadSendPosResult_.joinable()) {
        threadSendPosResult_.join();
    }
}

void ThreadDataIO::closeThreadSendPosResult() {
    if (threadSendPosResult_.joinable()) {
        threadSendPosResult_.join();
    }
    if (posResSender_ != nullptr) {
        delete posResSender_;
        posResSender_ = nullptr;
    }
}

void ThreadDataIO::sendString(SerialDriver *serial, const std::string &str) {
    for (char ch : str) {
        if (!serial->isOpen()) {
            // process error
            std::cerr << "Serial port is not open" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (!serial->writeByte(static_cast<uint8_t>(ch))) {
            // process error
            std::cerr << "Failed to send character: " << ch << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}