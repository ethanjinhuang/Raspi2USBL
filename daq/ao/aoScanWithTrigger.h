/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-07 21:05:29
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:33:58
 * @FilePath: /Raspi2USBL/daq/ao/aoScanWithTrigger.h
 * @Description: analog output scan with trigger
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _AOSCANWITHTRIGGER_H_
#define _AOSCANWITHTRIGGER_H_

#include "../daqTypeDefine.h"
#include "../signalGenerator.h"
#include "../utility.h"
#include <stdexcept>
#include <thread>
#include <uldaq.h>

class AOScanWithTrigger {
public:
    AOScanWithTrigger(AOScanInfo *scanInfo, double *data);
    ~AOScanWithTrigger();

    void init();
    /***
     * @description: create DAQ connection
     * @return {*}
     */
    bool createDaqConnection();

    /***
     * @description: print device information
     * @return {*}
     */
    void printDeviceInfo();

    /***
     * @description: analog output
     * @return {*}
     */
    void analogOutput();

private:
    AOScanInfo *scanInfo_;
    double     *data_;

    // system parameters for data acquisition
    int                 descriptorIndex_;
    // DAQ Decive define (including name etc.)
    DaqDeviceDescriptor devDescriptors_[MAX_DEV_COUNT];
    // bitmask defining the physical connection  interface used to communicate with DAQ decive
    DaqDeviceInterface  interfaceType_;
    // The DAQ device handle
    DaqDeviceHandle     daqDeviceHandle_;
    // The DAQ devices num
    Range               range_;          // ? [R] value for the infoValue argument
    unsigned int        numDevs_;        // ? [R] The DAQ devices num
    int                 lowChan_;        // * [S] first Channel
    int                 highChan_;       // * [S] last Channel
    double              sampleRate_;     // * [S] Channel Sample num per sec
    int                 samplesPerChan_; // * [S] Channel cumulate Sample num
    ScanOption          scanOptions_;    // * [S] Scan Option
    AOutScanFlag        flags_;          // * [S] Data is returned with scaling and calibration factors applied control
    int                 duration_;       // * [S] scan duration

    // status flag
    int         hasAO_;       // ? [R] Status of Support analog input
    int         hasPacer_;    // ? [R] Status of Support hardware paced analog input
    int         index_;       // ? [R] Current ouput index
    TriggerType triggerType_; // ? [R] Trigger type
    char        triggerTypeStr_[MAX_STR_LENGTH];
    char        rangeStr_[MAX_STR_LENGTH];
    char        scanOptionsStr_[MAX_SCAN_OPTIONS_LENGTH];
    UlError     err_;

    // tempdata
    int                         i = 0;
    int __attribute__((unused)) ret;
    char                        c;
};

#endif // _AOSCANWITHTRIGGER_H_