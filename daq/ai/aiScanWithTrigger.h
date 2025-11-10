/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-03 11:28:47
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:32:36
 * @FilePath: /Raspi2USBL/daq/ai/aiScanWithTrigger.h
 * @Description: analog input sampling with external trigger
 *                2024-08-18 using ChannelSignalVector replace std::vector<double>
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _AISCANWITHTRIGGER_H_
#define _AISCANWITHTRIGGER_H_

#include "../../general/typedef.h"
#include "../daqTypeDefine.h"
#include "../utility.h"
#include <stdexcept>
#include <thread>
#include <uldaq.h>

// void eventRecoverySampling(DaqDeviceHandle daqDeviceHandle, DaqEventType eventType, unsigned long long eventData,
// void* userData);

class AIScanWithTrigger {
public:
    /***
     * @description:
     * @param {AIScanInfo} *scanInfo                                    scan information
     * @param {Safe_Queue<ChannelSignalVector>*} dataQueue              data queue
     * @param {sfq::Safe_Queue<ChannelSignalVector>*} dataSaveQueue     data queue to save
     * @return {*}
     */
    AIScanWithTrigger(AIScanInfo *scanInfo, sfq::Safe_Queue<ChannelSignalVector> *dataQueue,
                      sfq::Safe_Queue<ChannelSignalVector> *dataSaveQueue);

    /***
     * @description:
     * @param {AIScanInfo} *scanInfo
     * @param {Safe_Queue<ChannelSignalVector>} *dataQueue
     * @param {Safe_Queue<ChannelSignalVector>} *dataSaveQueue
     * @param {Safe_Queue<ChannelSignalVector>} *dataSendQueue
     * @return {*}
     */
    AIScanWithTrigger(AIScanInfo *scanInfo, sfq::Safe_Queue<ChannelSignalVector> *dataQueue,
                      sfq::Safe_Queue<ChannelSignalVector> *dataSaveQueue,
                      sfq::Safe_Queue<ChannelSignalVector> *dataSendQueue);

    /***
     * @description: explicit destructor
     * @return {*}
     */
    ~AIScanWithTrigger();

    void init();

    /***
     * @description: create DAQ connection
     * @return {*}
     */
    bool createDaqConnection();

    /***
     * @description: start data acquisition
     * @return {*}
     */
    void dataAcquisition();

    /***
     * @description: an event function that is called when the specified event occurs
     * @param {DaqDeviceHandle} daqDeviceHandle     DAQ device handle
     * @param {DaqEventType} eventType              event type
     * @param {unsigned long long} eventData        event data (e.g. how many samples have to trigger the event)
     * @param {void*} userData                      user data
     * @return {*}
     */
    static void eventRecoverySampling(DaqDeviceHandle daqDeviceHandle, DaqEventType eventType,
                                      unsigned long long eventData, void *userData);

    /***
     * @description: print device information
     * @return {*}
     */
    void printDeviceInfo();

    /***
     * @description: save data to scan event parameters
     * @param {AIScanEventParameters} &scanEventParams    scan event parameters
     * @return {*}
     */
    void saveToScanEventParameters(AIScanEventParameters &scanEventParams);

    // /***
    //  * @description: save data to queue
    //  * @param {double*} buffer                  data buffer
    //  * @param {int} bufferSize                  data buffer size
    //  * @param {sfq::Safe_Queue<std::vector<double>>*} dataQueue         data queue
    //  * @param {sfq::Safe_Queue<std::vector<double>>*} dataSaveQueue     data queue to save
    //  * @return {*}
    //  */
    // static void saveDataToQueue(double *buffer, int bufferSize, sfq::Safe_Queue<std::vector<double>> *dataQueue,
    //                             sfq::Safe_Queue<std::vector<double>> *dataSaveQueue);

    /***
     * @description: save data to queue
     * @param {double} *buffer          data buffer
     * @param {int} bufferSize          data buffer size
     * @param {Safe_Queue<ChannelSignalVector>} *dataQueue              data queue
     * @param {Safe_Queue<ChannelSignalVector>} *dataSaveQueue          data queue to save
     * @return {*}
     */
    static void saveDataToQueue(double *buffer, int channelCount, int bufferSize,
                                sfq::Safe_Queue<ChannelSignalVector> *dataQueue,
                                sfq::Safe_Queue<ChannelSignalVector> *dataSaveQueue);

    /***
     * @description:
     * @param {double} *buffer
     * @param {int} channelCount
     * @param {int} bufferSize
     * @param {Safe_Queue<ChannelSignalVector>} *dataQueue
     * @param {Safe_Queue<ChannelSignalVector>} *dataSaveQueue
     * @param {Safe_Queue<ChannelSignalVector>} *dataSendQueue
     * @return {*}
     */
    static void saveDataToQueue(double *buffer, int channelCount, int bufferSize,
                                sfq::Safe_Queue<ChannelSignalVector> *dataQueue,
                                sfq::Safe_Queue<ChannelSignalVector> *dataSaveQueue,
                                sfq::Safe_Queue<ChannelSignalVector> *dataSendQueue);

    /***
     * @description: check buffer is full or not
     * @param {double*} buffer          data buffer
     * @param {int} bufferSize           data buffer size
     * @return {*}
     */
    static bool checkBufferFull(double *buffer, int bufferSize);

private:
    AIScanEventParameters scanEventParams_;
    AIScanInfo           *scanInfo_;

    // data
    int                                   chanCount_;     // ? [R] Channel number (used)
    int                                   bufferSize_;    // ? [R] Data buffer size
    double                               *buffer_ = NULL; // ? [R] Data buffer
    std::vector<double>                   dataBufferVec_; // ? [R] Data buffer vector
    // sfq::Safe_Queue<std::vector<double>> *dataQueue_;     // ? [R] Data queue
    // sfq::Safe_Queue<std::vector<double>> *dataSaveQueue_; // ? [R] Data save queue
    sfq::Safe_Queue<ChannelSignalVector> *dataQueue_;               // ? [R] Data queue
    sfq::Safe_Queue<ChannelSignalVector> *dataSaveQueue_ = nullptr; // ? [R] Data save queue
    sfq::Safe_Queue<ChannelSignalVector> *dataSendQueue_ = nullptr; // ? [R] Data send queue

    // system parameters for data acquisition
    int                 descriptorIndex_;
    // DAQ Decive define (including name etc.)
    DaqDeviceDescriptor devDescriptors_[MAX_DEV_COUNT];
    // bitmask defining the physical connection  interface used to communicate with DAQ decive
    DaqDeviceInterface  interfaceType_;
    // The DAQ device handle
    DaqDeviceHandle     daqDeviceHandle_;

    // The DAQ devices num
    unsigned int numDevs_;              // ? [R] The DAQ devices num
    AiInputMode  inputMode_;            // ? [R] A/D channel input modes
    Range        range_;                // ? [R] value for the infoValue argument
    int          lowChan_;              // * [S] first Channel
    int          highChan_;             // * [S] last Channel
    int          samplesPerChannel_;    // * [S] Channel cumulate Sample num
    double       rate_;                 // * [S] Channel Sample num per sec
    AInScanFlag  flags_;                // * [S] Data is returned with scaling and calibration factors applied control
    ScanOption   scanOptions_;          // * [S] Scan Option
    DaqEventType eventTypes_;           // * [S] Event Type
    int          availableSampleCount_; // * [S] Event Trigger sample num
    int          duration_;             // * [S] scan duration
    double       interval_;             // * [S] scan interval

    // status flag
    int         hasAI_;            // ? [R] Status of Support analog input
    int         hasPacer_;         // ? [R] Status of Support hardware paced analog input
    int         numberOfChannels_; // ? [R] Number of Channels
    TriggerType triggerType_;      // ? [R] Trigger type
    int         index_;            // ? [R] Current sample index
    char        inputModeStr_[MAX_STR_LENGTH];
    char        rangeStr_[MAX_STR_LENGTH];
    char        triggerTypeStr_[MAX_STR_LENGTH];
    char        scanOptionsStr_[MAX_SCAN_OPTIONS_LENGTH];
    UlError     err_;

    // tempdata
    int                         i = 0;
    int __attribute__((unused)) ret;
    char                        c;
};

#endif // _AISCANWITHTRIGGER_H_
