/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-07 21:06:11
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:33:21
 * @FilePath: /Raspi2USBL/daq/ao/aoScanWithTrigger.cpp
 * @Description: See aoScanWithTrigger.h
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#include "aoScanWithTrigger.h"
#include "../../config/defineconfig.h"
#include <cmath>

AOScanWithTrigger::AOScanWithTrigger(AOScanInfo *scanInfo, double *data) {
    scanInfo_ = scanInfo;
    data_     = data;

    init();
}

AOScanWithTrigger::~AOScanWithTrigger() {
    // release the DAQ device
    if (daqDeviceHandle_) {
        ulReleaseDaqDevice(daqDeviceHandle_);
    }

    // return error status
    if (err_ != ERR_NO_ERROR) {
        char errMsg[ERR_MSG_LEN];
        ulGetErrMsg(err_, errMsg);
        printf("Error Code: %d \n", err_);
        printf("Error Message: %s \n", errMsg);
    }

    // release the data buffer
    if (data_) {
        free(data_);
    }
}

void AOScanWithTrigger::init() {
    // init parameter
    descriptorIndex_ = 0;
    interfaceType_   = ANY_IFC;
    daqDeviceHandle_ = 0;
    numDevs_         = MAX_DEV_COUNT;
    err_             = ERR_NO_ERROR;

    // sync scanInfo
    lowChan_        = scanInfo_->lowChan;
    highChan_       = scanInfo_->highChan;
    samplesPerChan_ = scanInfo_->samplesPerChannel;
    sampleRate_     = scanInfo_->rate;
    flags_          = scanInfo_->flags;
    scanOptions_    = scanInfo_->scanOption;

    // system duration
    duration_ = scanInfo_->duration;
}

bool AOScanWithTrigger::createDaqConnection() {
    // get descriptors for all of the available DAQ devices
    err_ = ulGetDaqDeviceInventory(interfaceType_, devDescriptors_, &numDevs_);
    if (err_ != ERR_NO_ERROR) {
        throw std::runtime_error("\nError: ulGetDaqDeviceInventory() failed\n");
        return false;
    }

    // verify at least one DAQ device is detected
    if (numDevs_ == 0) {
        //  printf("No DAQ device is detected\n");
        throw std::runtime_error("\nError: No DAQ devices found\n");
        return false;
    }

    // print device info
    printf("Found %d DAQ device(s)\n", numDevs_);
    for (unsigned int i = 0; i < numDevs_; i++) {
        printf("  [%d] %s: (%s)\n", i, devDescriptors_[i].productName, devDescriptors_[i].uniqueId);
    }

    // use cmd to select DAQ devices (default id:0)
    if (numDevs_ > 1) {
        descriptorIndex_ = selectDAQDevice(numDevs_);
    }

    // get a handle to the DAQ device associated with the first descriptor
    daqDeviceHandle_ = ulCreateDaqDevice(devDescriptors_[descriptorIndex_]);
    if (daqDeviceHandle_ == 0) {
        // printf ("\nUnable to create a handle to the specified DAQ device\n");
        throw std::runtime_error("\nError: ulCreateDaqDevice() failed\n");
        return false;
    }

    // verify the specified DAQ device supports analog output
    err_ = getDevInfoHasAi(daqDeviceHandle_, &hasAO_);
    if (!hasAO_) {
        // printf("\nThe specified DAQ device does not support analog input\n");
        throw std::runtime_error("\nError: The specified DAQ device does not support analog output\n");
        return false;
    }

    // verify the specified device supports hardware pacing for analog input
    err_ = getAiInfoHasPacer(daqDeviceHandle_, &hasPacer_);
    if (!hasPacer_) {
        // printf("\nThe specified DAQ device does not support hardware paced analog input\n");
        throw std::runtime_error("\nError: The specified DAQ device does not support hardware paced analog output\n");
        return false;
    }

    // connect to device
    printf("\nConnecting to device %s - please wait ...\n", devDescriptors_[descriptorIndex_].devString);
    // establish a connection to the device
    err_ = ulConnectDaqDevice(daqDeviceHandle_);
    if (err_ != ERR_NO_ERROR) {
        // printf("\nUnable to connect to the DAQ device\n");
        throw std::runtime_error("\nError: ulConnectDaqDevice() failed\n");
        return false;
    }

    // get the first supported output range
    err_ = getAoInfoFirstSupportedRange(daqDeviceHandle_, &range_, rangeStr_);

    // get the first supported trigger type (this returns a digital trigger type)
    err_ = getAoInfoFirstTriggerType(daqDeviceHandle_, &triggerType_, triggerTypeStr_);
    ConvertScanOptionsToString(scanOptions_, scanOptionsStr_);

    return true;
}

void AOScanWithTrigger::printDeviceInfo() {
    try {
        printf("\n%s ready\n", devDescriptors_[descriptorIndex_].devString);
        printf("    Function demonstrated: ulEnableEvent()\n");
        printf("    Channels: %d - %d\n", lowChan_, highChan_);
        printf("    Range: %s\n", rangeStr_);
        printf("    Samples per channel: %d\n", samplesPerChan_);
        printf("    Rate: %f\n", sampleRate_);
        printf("    Scan options: %s\n", scanOptionsStr_);
        printf("    Trigger type: %s\n", triggerTypeStr_);
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void AOScanWithTrigger::analogOutput() {
    // create DAQ connection
    createDaqConnection();

    // print device info
    printDeviceInfo();

    // create new analog output scan
    err_ = ulAOutScan(daqDeviceHandle_, lowChan_, highChan_, range_, samplesPerChan_, &sampleRate_, scanOptions_,
                      flags_, data_);

    if (err_ == ERR_NO_ERROR) {
        std::this_thread::sleep_for(std::chrono::seconds(duration_));
        if (err_ != ERR_NO_ERROR) {
            printf("ulAOnScanWait error = %d\n", err_);
        }

        err_ = ulAInScanStop(daqDeviceHandle_);
    }
    // disconnect from the DAQ device
    ulDisconnectDaqDevice(daqDeviceHandle_);
}