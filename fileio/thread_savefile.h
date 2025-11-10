/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-07-04 16:21:07
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-09-03 11:24:29
 * @FilePath: /RaspiUSBL/fileio/thread_savefile.h
 * @Description: save daq analog input data to file
 * @             2024-08-18 use ChannelSignalVector replace std::vector<double>
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */
#ifndef _THREAD_SAVEFILE_H_
#define _THREAD_SAVEFILE_H_

#include <thread>

#include "../config/defineconfig.h"
#include "../core/systeminfo.h"
#include "../general/typedef.h"
#include "../tool/ColorParse.h"
#include "../tool/SafeQueue.hpp"
#include "filesaver.h"

class ThreadSaveFile {
public:
    ThreadSaveFile(SystemInfo *systeminfo);
    ~ThreadSaveFile();

    void init();

    // thread config
    void setDAQAIFile(const string filename, int filetype = FileSaver::TEXT);
    void setPosResFile(const string filename, int filetype = FileSaver::TEXT);
    void setCorrelationFile(const string filename, int filetype = FileSaver::TEXT);
    void setTOFResFile(const string filename, int filetype = FileSaver::TEXT);
    void setBeamPatternFile(const string filename, int filetype = FileSaver::TEXT);
    void setSideAmpSpecFile(const string filename, int filetype = FileSaver::TEXT);
    void configAutoSetFile();

    // set data queue
    void setDAQAIDataQueue(sfq::Safe_Queue<ChannelSignalVector> *dataque);
    void setPosResQueue(sfq::Safe_Queue<PositionResult> *dataque);
    void setCorrelationQueue(sfq::Safe_Queue<ChannelSignalVector> *dataque);
    void setTOFResQueue(sfq::Safe_Queue<std::vector<double>> *dataque);
    void setBeamPatternQueue(sfq::Safe_Queue<Eigen::MatrixXd> *dataque);
    void setSideAmpSpecQueue(sfq::Safe_Queue<ChannelSignalVector> *dataque);

    // thread function
    // void creatThread_saveDAQAIData(sfq::Safe_Queue<std::vector<double>> *dataque);
    void creatThread_saveDAQAIData(sfq::Safe_Queue<ChannelSignalVector> *dataque);
    void creatThread_saveProcessResult();
    void creatThread_savePosRes(sfq::Safe_Queue<PositionResult> *dataque);
    void creatThread_saveCorrelation(sfq::Safe_Queue<ChannelSignalVector> *dataque);
    void creatThread_saveTOFRes(sfq::Safe_Queue<std::vector<double>> *dataque);
    void creatThread_saveBeamPattern(sfq::Safe_Queue<Eigen::MatrixXd> *dataque);
    void creatThread_saveSideAmpSpec(sfq::Safe_Queue<ChannelSignalVector> *dataque);

    // join thread
    void joinThread_saveDAQAIData();
    void joinThread_saveProcessResult();
    void joinThread_savePosRes();
    void joinThread_saveCorrelation();
    void joinThread_saveTOFRes();
    void joinThread_saveBeamPattern();
    void joinThread_saveSideAmpSpec();

    // close thread
    void closeThread_saveDAQAIData();
    void closeThread_saveProcessResult();
    void closeThread_savePosRes();
    void closeThread_saveCorrelation();
    void closeThread_saveTOFRes();
    void closeThread_saveBeamPattern();
    void closeThread_saveSideAmpSpec();

    // save data to file
    void saveDAQAIData();
    void savePosRes();
    void saveProcessResult();
    void saveCorrelation();
    void saveTOFRes();
    void saveBeamPattern();
    void saveSideAmpSpec();

private:
    SystemInfo *systemInfo_;
    FileSaver  *daqaiFileSaver_;
    FileSaver  *posResFileSaver_;
    FileSaver  *correlationFileSaver_;
    FileSaver  *tofResFileSaver_;
    FileSaver  *beamPatternFileSaver_;
    FileSaver  *sideAmpSpecFileSaver_;

    bool isLoadDAQAIQueue_;
    bool isLoadPosResQueue_;
    bool isLoadCorrelationQueue_;
    bool isLoadTOFResQueue_;
    bool isLoadBeamPatternQueue_;
    bool isLoadSideAmpSpecQueue_;

    bool enableThread_saveDAQAIData_;
    bool enableThread_saveProcessResult_;
    bool enableThread_savePosRes_;
    bool enableThread_saveCorrelation_;
    bool enableThread_saveTOFRes_;
    bool enableThread_saveBeamPattern_;
    bool enableThread_saveSideAmpSpec_;

    // std::vector<double>                   daqaiTempData_;
    // sfq::Safe_Queue<std::vector<double>> *daqaiDataQue_;
    ChannelSignalVector daqaiTempData_;
    PositionResult      posRes_;
    ChannelSignalVector correlationRes_;
    std::vector<double> tofRes_;
    Eigen::MatrixXd     beamPattern_;
    ChannelSignalVector sideAmpSpec_;

    sfq::Safe_Queue<ChannelSignalVector> *daqaiDataQue_;
    sfq::Safe_Queue<PositionResult>      *posResQue_;
    sfq::Safe_Queue<ChannelSignalVector> *correlationQue_;
    sfq::Safe_Queue<std::vector<double>> *tofResQue_;
    sfq::Safe_Queue<Eigen::MatrixXd>     *beamPatternQue_;
    sfq::Safe_Queue<ChannelSignalVector> *sideAmpSpecQue_;

    std::thread thread_saveDAQAIData_;
    std::thread thread_saveProcessResult_; // including posRes, tofRes, beamPattern, sideAmpSpec
    std::thread thread_savePosRes_;
    std::thread thread_saveCorrelation_;
    std::thread thread_saveTOFRes_;
    std::thread thread_saveBeamPattern_;
    std::thread thread_saveSideAmpSpec_;
};

#endif // _THREAD_SAVEFILE_H_