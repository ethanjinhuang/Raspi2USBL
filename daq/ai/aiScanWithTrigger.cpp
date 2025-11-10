/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-03 11:30:29
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:32:13
 * @FilePath: /Raspi2USBL/daq/ai/aiScanWithTrigger.cpp
 * @Description: see aiScanWithTrigger.h
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#include "aiScanWithTrigger.h"
#include "../../config/defineconfig.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <thread>

AIScanWithTrigger::AIScanWithTrigger(AIScanInfo *scanInfo, sfq::Safe_Queue<ChannelSignalVector> *dataQueue,
                                     sfq::Safe_Queue<ChannelSignalVector> *dataSaveQueue) {
    scanInfo_      = scanInfo;
    dataQueue_     = dataQueue;
    dataSaveQueue_ = dataSaveQueue;

    init();
}

AIScanWithTrigger::AIScanWithTrigger(AIScanInfo *scanInfo, sfq::Safe_Queue<ChannelSignalVector> *dataQueue,
                                     sfq::Safe_Queue<ChannelSignalVector> *dataSaveQueue,
                                     sfq::Safe_Queue<ChannelSignalVector> *dataSendQueue) {
    scanInfo_      = scanInfo;
    dataQueue_     = dataQueue;
    dataSaveQueue_ = dataSaveQueue;
    dataSendQueue_ = dataSendQueue;

    init();
}

AIScanWithTrigger::~AIScanWithTrigger() {
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
    if (buffer_) {
        free(buffer_);
    }
}

void AIScanWithTrigger::init() {
    // init parameter
    chanCount_       = 0; // which is calculated in AIScanWithTrigger::createDaqConnection()
    descriptorIndex_ = 0;
    interfaceType_   = ANY_IFC;
    daqDeviceHandle_ = 0;
    numDevs_         = MAX_DEV_COUNT;
    err_             = ERR_NO_ERROR;

    // sync scanInfo
    lowChan_           = scanInfo_->lowChan;
    highChan_          = scanInfo_->highChan;
    samplesPerChannel_ = scanInfo_->samplesPerChannel;
    rate_              = scanInfo_->rate;
    flags_             = scanInfo_->flags;
    scanOptions_       = scanInfo_->scanOption;
    eventTypes_        = scanInfo_->eventTypes;

    // availableSampleCount_ = scanInfo_->availableSampleCount;
    // using event sample count to save data
    availableSampleCount_ = samplesPerChannel_;

    duration_             = scanInfo_->duration;
    interval_             = scanInfo_->interval;
}

bool AIScanWithTrigger::createDaqConnection() {
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

    // verify the specified DAQ device supports analog input
    err_ = getDevInfoHasAi(daqDeviceHandle_, &hasAI_);
    if (!hasAI_) {
        // printf("\nThe specified DAQ device does not support analog input\n");
        throw std::runtime_error("\nError: The specified DAQ device does not support analog input\n");
        return false;
    }

    // verify the specified device supports hardware pacing for analog input
    err_ = getAiInfoHasPacer(daqDeviceHandle_, &hasPacer_);
    if (!hasPacer_) {
        // printf("\nThe specified DAQ device does not support hardware paced analog input\n");
        throw std::runtime_error("\nError: The specified DAQ device does not support hardware paced analog input\n");
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

    // get the first supported analog input mode
    err_ = getAiInfoFirstSupportedInputMode(daqDeviceHandle_, &numberOfChannels_, &inputMode_, inputModeStr_);

    if (highChan_ >= numberOfChannels_) {
        highChan_ = numberOfChannels_ - 1;
    }
    chanCount_ = highChan_ - lowChan_ + 1;

    // allocate a buffer to receive the data
    // define buffer size
    bufferSize_ = chanCount_ * samplesPerChannel_;
    // allocate memory for the data buffer
    buffer_ = (double *) malloc(bufferSize_ * sizeof(double));
    if (buffer_ == NULL) {
        throw std::runtime_error("\nOut of memory, unable to create scan buffer\n");
    }
    // fill the buffer with NaN
    std::fill_n(buffer_, bufferSize_, NAN);
    // reserve memory for the data buffer vector
    dataBufferVec_.reserve(bufferSize_);

    // get the first supported analog input range
    err_ = getAiInfoFirstSupportedRange(daqDeviceHandle_, inputMode_, &range_, rangeStr_);

    // get the first supported trigger type (this returns a digital trigger type)
    getAiInfoFirstTriggerType(daqDeviceHandle_, &triggerType_, triggerTypeStr_);
    ConvertScanOptionsToString(scanOptions_, scanOptionsStr_);

    // enable the event
    err_ =
        ulEnableEvent(daqDeviceHandle_, eventTypes_, availableSampleCount_, eventRecoverySampling, &scanEventParams_);

    // get the first supported analog input range
    err_ = getAiInfoFirstSupportedRange(daqDeviceHandle_, inputMode_, &range_, rangeStr_);
    ConvertScanOptionsToString(scanOptions_, scanOptionsStr_);

    return true;
}

void AIScanWithTrigger::dataAcquisition() {
    // create DAQ connection
    createDaqConnection();

    // print device info
    printDeviceInfo();

    // save scan event parameters
    saveToScanEventParameters(scanEventParams_);

    // create a new scan
    err_ = ulAInScan(daqDeviceHandle_, lowChan_, highChan_, inputMode_, range_, samplesPerChannel_, &rate_,
                     scanOptions_, flags_, buffer_);

    // set scan event parameters
    scanEventParams_.rate = rate_;

    if (err_ == ERR_NO_ERROR) {
        std::this_thread::sleep_for(std::chrono::seconds(duration_));
        if (err_ != ERR_NO_ERROR) {
            printf("ulAInScanWait error = %d\n", err_);
        }

        err_ = ulAInScanStop(daqDeviceHandle_);
    }
    // disable events : Disables one or more event conditions, and disconnects their user-defined handlers.
    ulDisableEvent(daqDeviceHandle_, eventTypes_);

    // disconnect from the DAQ device
    ulDisconnectDaqDevice(daqDeviceHandle_);
}

void AIScanWithTrigger::printDeviceInfo() {
    try {
        printf("\n%s ready\n", devDescriptors_[descriptorIndex_].devString);
        printf("    Function demonstrated: ulEnableEvent()\n");
        printf("    Channels: %d - %d\n", lowChan_, highChan_);
        printf("    Input mode: %s\n", inputModeStr_);
        printf("    Range: %s\n", rangeStr_);
        printf("    Samples per channel: %d\n", samplesPerChannel_);
        printf("    Rate: %f\n", rate_);
        printf("    Scan options: %s\n", scanOptionsStr_);
        // printf("\nHit ENTER to continue\n");
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void AIScanWithTrigger::saveToScanEventParameters(AIScanEventParameters &scanEventParams) {
    try {
        scanEventParams.buffer        = buffer_;
        scanEventParams.bufferSize    = bufferSize_;
        scanEventParams.dataQueue     = dataQueue_;
        scanEventParams.dataSaveQueue = dataSaveQueue_;
        scanEventParams.dataSendQueue = dataSendQueue_;
        scanEventParams.lowChan       = lowChan_;
        scanEventParams.highChan      = highChan_;
        // scanEventParams.inputMode = inputMode_;
        // scanEventParams.range = range_;
        // scanEventParams.samplesPerChan = samplesPerChannel_;
        scanEventParams.rate     = rate_;
        scanEventParams.interval = interval_;
        // scanEventParams.options = scanOptions_;
        // scanEventParams.flags = flags_;
#ifdef _DAQAI_MCC1608FSPLUS_
        scanEventParams.daqDeviceHandle   = daqDeviceHandle_;
        scanEventParams.aiInputMode       = inputMode_;
        scanEventParams.range             = range_;
        scanEventParams.samplesPerChannel = samplesPerChannel_;
        scanEventParams.scanOptions       = scanOptions_;
        scanEventParams.flag              = flags_;
#endif // _DAQAI_MCC1608FSPLUS_
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void AIScanWithTrigger::saveDataToQueue(double *buffer, int channelCount, int bufferSize,
                                        sfq::Safe_Queue<ChannelSignalVector> *dataQueue,
                                        sfq::Safe_Queue<ChannelSignalVector> *dataSaveQueue) {
    std::vector<double> dataBufferVec;
    dataBufferVec.reserve(bufferSize);
    // check if the buffer is full
    if (checkBufferFull(buffer, bufferSize)) {
        // init the ChannelSignalVector
        ChannelSignalVector csvTemp(channelCount, bufferSize / channelCount);
        // copy the data to the data buffer vector
        dataBufferVec.assign(buffer, buffer + bufferSize);
        // std::cout << "dataBufferVec: " << dataBufferVec[1] << std::endl;
        // convert the data buffer vector to the channel signal vector
        for (int i = 0; i < channelCount; ++i) {
            for (int j = 0; j < bufferSize / channelCount; ++j) {
                csvTemp.channels[i][j] = dataBufferVec[i + j * channelCount];
            }
        }

        // push the data buffer vector to the data queue
        // dataQueue->push(dataBufferVec);
        // dataSaveQueue->push(dataBufferVec);
        dataQueue->push(csvTemp);
        dataSaveQueue->push(csvTemp);
        // clear the data
        std::fill_n(buffer, bufferSize, NAN);
        dataBufferVec.clear();
    } else {
        throw std::runtime_error("\nData buffer is not full\n");
    }
}

void AIScanWithTrigger::saveDataToQueue(double *buffer, int channelCount, int bufferSize,
                                        sfq::Safe_Queue<ChannelSignalVector> *dataQueue,
                                        sfq::Safe_Queue<ChannelSignalVector> *dataSaveQueue,
                                        sfq::Safe_Queue<ChannelSignalVector> *dataSendQueue) {
    std::vector<double> dataBufferVec;
    dataBufferVec.reserve(bufferSize);
    // check if the buffer is full
    if (checkBufferFull(buffer, bufferSize)) {
        // init the ChannelSignalVector
        ChannelSignalVector csvTemp(channelCount, bufferSize / channelCount);
        // copy the data to the data buffer vector
        dataBufferVec.assign(buffer, buffer + bufferSize);
        // std::cout << "dataBufferVec: " << dataBufferVec[1] << std::endl;
        // convert the data buffer vector to the channel signal vector
        for (int i = 0; i < channelCount; ++i) {
            for (int j = 0; j < bufferSize / channelCount; ++j) {
                csvTemp.channels[i][j] = dataBufferVec[i + j * channelCount];
            }
        }

        // push the data buffer vector to the data queue
        // dataQueue->push(dataBufferVec);
        // dataSaveQueue->push(dataBufferVec);
        dataQueue->push(csvTemp);
        dataSaveQueue->push(csvTemp);
        dataSendQueue->push(csvTemp);
        // clear the data
        std::fill_n(buffer, bufferSize, NAN);
        dataBufferVec.clear();
    } else {
        throw std::runtime_error("\nData buffer is not full\n");
    }
}

bool AIScanWithTrigger::checkBufferFull(double *buffer, int bufferSize) {
    for (int i = 0; i < bufferSize; ++i) {
        if (std::isnan(buffer[i])) {
            return false;
        }
    }
    return true;
}

void AIScanWithTrigger::eventRecoverySampling(DaqDeviceHandle daqDeviceHandle, DaqEventType eventType,
                                              unsigned long long eventData, void *userData) {
    char __attribute__((unused)) eventTypeStr[MAX_STR_LENGTH];
    UlError                      err = ERR_NO_ERROR;
    DaqDeviceDescriptor          activeDevDescriptor;
    int                          i;

    AIScanEventParameters *scanEventParameters = (AIScanEventParameters *) userData;
    int                    chanCount           = scanEventParameters->highChan - scanEventParameters->lowChan + 1;
    int                    newlineCount        = chanCount + 7;

    // reset the cursor to the top of the display and show the termination message
    // resetCursor();

    // get device description
    ulGetDaqDeviceDescriptor(daqDeviceHandle, &activeDevDescriptor);

#ifdef _DAQAI_DEBUG_
    // print event type
    ConvertEventTypesToString(eventType, eventTypeStr);
    printf("eventType: %s \n", eventTypeStr);
#endif

    // process event
    switch (eventType) {
        case DE_ON_DATA_AVAILABLE: { // data available, print data

#ifdef _DAQAI_DEBUG_
            std::cout << "Queue Size" << scanEventParameters->dataQueue->size() << std::endl;
#endif

#ifdef _DAQAI_DEBUG_
            long long scanCount    = eventData;
            long long totalSamples = scanCount * chanCount;
            long long index        = 0;

            printf("eventData: %lld \n\n", eventData);
            // calcuate index
            index = (totalSamples - chanCount) % scanEventParameters->bufferSize;

            // print index
            printf("currentIndex = %lld \n\n", index);

            // print data
            for (i = 0; i < chanCount; i++) {
                printf("chan %d = %+-10.6f\n", i + scanEventParameters->lowChan,
                       scanEventParameters->buffer[index + i]);
            }
#endif
            // using event sample count to save data and clear the buffer with NAN
            // We should define the availableSampleCount equal to the samplesPerChannel, to save the data
            int channelCount = scanEventParameters->highChan - scanEventParameters->lowChan + 1;
            if (scanEventParameters->dataSendQueue == nullptr) {
                saveDataToQueue(scanEventParameters->buffer, channelCount, scanEventParameters->bufferSize,
                                scanEventParameters->dataQueue, scanEventParameters->dataSaveQueue);
            } else {
                saveDataToQueue(scanEventParameters->buffer, channelCount, scanEventParameters->bufferSize,
                                scanEventParameters->dataQueue, scanEventParameters->dataSaveQueue,
                                scanEventParameters->dataSendQueue);
            }

            // std::cout << "Queue Size" << scanEventParameters->dataQueue->size() << std::endl;
            // std::cout << "Data: " << scanEventParameters->buffer[0] << std::endl;

            // sleep for a while
            // auto now      = std::chrono::system_clock::now();
            // auto duration = now.time_since_epoch();
            // auto seconds      = std::chrono::duration_cast<std::chrono::seconds>(duration);
            // auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) -
            //                     std::chrono::duration_cast<std::chrono::milliseconds>(seconds);

            // double fractional_seconds = seconds.count() % 60 + milliseconds.count() / 1000.0;
            // std::cout << std::fixed << std::setprecision(3) << fractional_seconds << "s: ";
            // std::cout << scanEventParameters->dataQueue->size() << std::endl;
            // std::this_thread::sleep_for(std::chrono::milliseconds((int) (scanEventParameters->interval * 1000.0)));

            break;
        }
        case DE_ON_INPUT_SCAN_ERROR: { // error
            for (i = 0; i < newlineCount; i++) {
                putchar('\n');
            }
            err = (UlError) eventData;
            char errMsg[ERR_MSG_LEN];
            ulGetErrMsg(err, errMsg);
            printf("Error Code: %d \n", err);
            printf("Error Message: %s \n", errMsg);
            break;
        }
        case DE_ON_END_OF_INPUT_SCAN: { // Define

            // [Attention] When using SO_RETRIGGER | SO_CONTINUOUS, the event will be triggered when the scan is stopped
            // It can be simply understood as the scan process will not stop, so the event will not be triggered whtn a
            // loop is completed saveDataToQueue(scanEventParameters->buffer, scanEventParameters->bufferSize,
            // scanEventParameters->dataQueue, scanEventParameters->dataSaveQueue);

#ifdef _DAQAI_DEBUG_
            // std::cout << "Queue Size" << scanEventParameters->dataQueue->size() << std::endl;
#endif
#ifdef _DAQAI_MCC1608FSPLUS_
            // start new scan
            // release the data buffer
            // if (scanEventParameters->buffer) {
            //     free(scanEventParameters->buffer);
            // }

            // check and release the previously allocated buffer to prevent memory leaks
            if (scanEventParameters->buffer != nullptr) {
                free(scanEventParameters->buffer);
                scanEventParameters->buffer = nullptr;
            }

            // reallocate the buffer to ensure no memory leaks
            scanEventParameters->buffer = (double *) malloc(scanEventParameters->bufferSize * sizeof(double));
            if (scanEventParameters->buffer == nullptr) {
                printf("Memory allocation failed\n");
                break;
            }

            err = ulAInScan(daqDeviceHandle, scanEventParameters->lowChan, scanEventParameters->highChan,
                            scanEventParameters->aiInputMode, scanEventParameters->range,
                            scanEventParameters->samplesPerChannel, &scanEventParameters->rate,
                            scanEventParameters->scanOptions, scanEventParameters->flag, scanEventParameters->buffer);
#endif // _DAQAI_MCC1608FSPLUS_
       // start new scan
       // err = ulAInScan(daqDeviceHandle, scanEventParameters->lowChan,
       // scanEventParameters->highChan, scanEventParameters->inputMode,
       // scanEventParameters->range, scanEventParameters->samplesPerChan,
       //  &scanEventParameters->rate, scanEventParameters->options,
       //  scanEventParameters->flags, scanEventParameters->buffer);

            break;
        }
        default:
            break;
    }
}