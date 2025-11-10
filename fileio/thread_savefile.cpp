/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-07-04 16:40:11
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-09-21 15:47:51
 * @FilePath: /RaspiUSBL/fileio/thread_savefile.cpp
 * @Description: see thread_savefile.h
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */

#include "thread_savefile.h"

ThreadSaveFile::ThreadSaveFile(SystemInfo *systeminfo) {
    systemInfo_ = systeminfo;

    init();
}

ThreadSaveFile::~ThreadSaveFile() {
    closeThread_saveDAQAIData();
}

void ThreadSaveFile::init() {
    daqaiFileSaver_       = new FileSaver();
    posResFileSaver_      = new FileSaver();
    beamPatternFileSaver_ = new FileSaver();
    correlationFileSaver_ = new FileSaver();
    tofResFileSaver_      = new FileSaver();
    sideAmpSpecFileSaver_ = new FileSaver();

    // thread flag
    enableThread_saveDAQAIData_   = false;
    enableThread_savePosRes_      = false;
    enableThread_saveBeamPattern_ = false;
    enableThread_saveCorrelation_ = false;
    enableThread_saveTOFRes_      = false;
    enableThread_saveSideAmpSpec_ = false;

    // queue flag
    isLoadDAQAIQueue_       = false;
    isLoadPosResQueue_      = false;
    isLoadTOFResQueue_      = false;
    isLoadBeamPatternQueue_ = false;
    isLoadSideAmpSpecQueue_ = false;
}

void ThreadSaveFile::setDAQAIFile(const string filename, int filetype) {
    // daqaiFileSaver_->open(filename);
    if (daqaiFileSaver_->open(filename, filetype)) {
        std::cout << termColor("green") << "DAQ AI Data Saver file successfully opened" << termColor("nocolor") << "\n";
    } else {
        std::cerr << termColor("red") << "DAQ AI Data Saver file failed to open" << termColor("nocolor") << "\n";
    }
    enableThread_saveDAQAIData_ = true;
}

void ThreadSaveFile::setPosResFile(const string filename, int filetype) {
    // posResFileSaver_->open(filename);
    if (posResFileSaver_->open(filename, filetype)) {
        std::cout << termColor("green") << "Position Result Data Saver file successfully opened" << termColor("nocolor")
                  << "\n";
    } else {
        std::cerr << termColor("red") << "Position Result Data Saver file failed to open" << termColor("nocolor")
                  << "\n";
    }
    enableThread_savePosRes_ = true;
}

void ThreadSaveFile::setCorrelationFile(const string filename, int filetype) {
    // correlationFileSaver_->open(filename);
    if (correlationFileSaver_->open(filename, filetype)) {
        std::cout << termColor("green") << "Correlation Data Saver file successfully opened" << termColor("nocolor")
                  << "\n";
    } else {
        std::cerr << termColor("red") << "Correlation Data Saver file failed to open" << termColor("nocolor") << "\n";
    }
    enableThread_saveCorrelation_ = true;
}

void ThreadSaveFile::setTOFResFile(const string filename, int filetype) {
    // tofResFileSaver_->open(filename);
    if (tofResFileSaver_->open(filename, filetype)) {
        std::cout << termColor("green") << "TOF Result Data Saver file successfully opened" << termColor("nocolor")
                  << "\n";
    } else {
        std::cerr << termColor("red") << "TOF Result Data Saver file failed to open" << termColor("nocolor") << "\n";
    }
    enableThread_saveTOFRes_ = true;
}

void ThreadSaveFile::setBeamPatternFile(const string filename, int filetype) {
    // beamPatternFileSaver_->open(filename);
    if (beamPatternFileSaver_->open(filename, filetype)) {
        std::cout << termColor("green") << "Beam Pattern Data Saver file successfully opened" << termColor("nocolor")
                  << "\n";
    } else {
        std::cerr << termColor("red") << "Beam Pattern Data Saver file failed to open" << termColor("nocolor") << "\n";
    }
    enableThread_saveBeamPattern_ = true;
}

void ThreadSaveFile::setSideAmpSpecFile(const string filename, int filetype) {
    // sideAmpSpecFileSaver_->open(filename);
    if (sideAmpSpecFileSaver_->open(filename, filetype)) {
        std::cout << termColor("green") << "Side Amp Spec Data Saver file successfully opened" << termColor("nocolor")
                  << "\n";
    } else {
        std::cerr << termColor("red") << "Side Amp Spec Data Saver file failed to open" << termColor("nocolor") << "\n";
    }
    enableThread_saveSideAmpSpec_ = true;
}

void ThreadSaveFile::configAutoSetFile() {

    if (systemInfo_->savedFileInfo.isSavePosRes) {
        setPosResFile(systemInfo_->savedFileInfo.PosResFilePath, FileSaver::TEXT);
    }
    if (systemInfo_->savedFileInfo.isSaveCorrelation) {
        setCorrelationFile(systemInfo_->savedFileInfo.CorrelationFilePath, FileSaver::TEXT);
    }
    if (systemInfo_->savedFileInfo.isSaveBeamPattern) {
        setBeamPatternFile(systemInfo_->savedFileInfo.BeamPatternFilePath, FileSaver::TEXT);
    }
    if (systemInfo_->savedFileInfo.isSaveTOFRes) {
        setTOFResFile(systemInfo_->savedFileInfo.TOFResFilePath, FileSaver::TEXT);
    }
    if (systemInfo_->savedFileInfo.isSaveSideAmpSpec) {
        setSideAmpSpecFile(systemInfo_->savedFileInfo.SideAmpSpecFilePath, FileSaver::TEXT);
    }
}

void ThreadSaveFile::setDAQAIDataQueue(sfq::Safe_Queue<ChannelSignalVector> *dataque) {
    daqaiDataQue_     = dataque;
    isLoadDAQAIQueue_ = true;
}

void ThreadSaveFile::setPosResQueue(sfq::Safe_Queue<PositionResult> *dataque) {
    posResQue_         = dataque;
    isLoadPosResQueue_ = true;
}

void ThreadSaveFile::setCorrelationQueue(sfq::Safe_Queue<ChannelSignalVector> *dataque) {
    correlationQue_         = dataque;
    isLoadCorrelationQueue_ = true;
}

void ThreadSaveFile::setTOFResQueue(sfq::Safe_Queue<std::vector<double>> *dataque) {
    tofResQue_         = dataque;
    isLoadTOFResQueue_ = true;
}

void ThreadSaveFile::setBeamPatternQueue(sfq::Safe_Queue<Eigen::MatrixXd> *dataque) {
    beamPatternQue_         = dataque;
    isLoadBeamPatternQueue_ = true;
}

void ThreadSaveFile::setSideAmpSpecQueue(sfq::Safe_Queue<ChannelSignalVector> *dataque) {
    sideAmpSpecQue_         = dataque;
    isLoadSideAmpSpecQueue_ = true;
}

// void ThreadSaveFile::creatThread_saveDAQAIData(sfq::Safe_Queue<std::vector<double>> *dataque) {
//     if (daqaiFileSaver_->isOpen()) {
//         daqaiDataQue_         = dataque;
//         thread_saveDAQAIData_ = std::thread(&ThreadSaveFile::saveDAQAIData, this);
// #ifdef _SAVEFILE_DEBUG_
//         std::cout << termColor("green") << "Thread DAQ AI Data Saver is created" << termColor("nocolor") << "\n";
// #endif
//     } else {
//         std::cerr << termColor("red") << "DAQ AI Data Saver file is not open" << termColor("nocolor") << "\n";
//     }
// }

void ThreadSaveFile::creatThread_saveDAQAIData(sfq::Safe_Queue<ChannelSignalVector> *dataque) {
    if (daqaiFileSaver_->isOpen()) {
        if (isLoadDAQAIQueue_) {
            std::cerr << termColor("red") << "DAQ AI Data Saver file is already loaded" << termColor("nocolor") << "\n";
            std::cerr << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
            exit(EXIT_FAILURE);
        }
        daqaiDataQue_         = dataque;
        thread_saveDAQAIData_ = std::thread(&ThreadSaveFile::saveDAQAIData, this);
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread DAQ AI Data Saver is created" << termColor("nocolor") << "\n";
#endif
    } else {
        std::cerr << termColor("red") << "DAQ AI Data Saver file is not open" << termColor("nocolor") << "\n";
    }
}

void ThreadSaveFile::creatThread_saveProcessResult() {
    if (!(isLoadBeamPatternQueue_ || isLoadPosResQueue_ || isLoadSideAmpSpecQueue_ || isLoadTOFResQueue_)) {
        std::cout << "isLoadBeamPatternQueue_:" << isLoadBeamPatternQueue_ << std::endl;
        std::cout << termColor("red") << "No Queue is loaded" << termColor("nocolor") << "\n";
        std::cout << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
        exit(EXIT_FAILURE);
    }
    // check the file is open
    if (isLoadBeamPatternQueue_ && systemInfo_->savedFileInfo.isSaveBeamPattern) {
        if (!beamPatternFileSaver_->isOpen()) {
            std::cerr << termColor("red") << "Beam Pattern Saver file is not open" << termColor("nocolor") << "\n";
            std::cerr << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
            exit(EXIT_FAILURE);
        }
    }
    if (isLoadPosResQueue_ && systemInfo_->savedFileInfo.isSavePosRes) {
        if (!posResFileSaver_->isOpen()) {
            std::cerr << termColor("red") << "Position Result Saver file is not open" << termColor("nocolor") << "\n";
            std::cerr << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
            exit(EXIT_FAILURE);
        }
    }
    if (isLoadTOFResQueue_ && systemInfo_->savedFileInfo.isSaveTOFRes) {
        if (!tofResFileSaver_->isOpen()) {
            std::cerr << termColor("red") << "TOF Result Saver file is not open" << termColor("nocolor") << "\n";
            std::cerr << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
            exit(EXIT_FAILURE);
        }
    }
    if (isLoadSideAmpSpecQueue_ && systemInfo_->savedFileInfo.isSaveSideAmpSpec) {
        if (!sideAmpSpecFileSaver_->isOpen()) {
            std::cerr << termColor("red") << "Side Amp Spec Saver file is not open" << termColor("nocolor") << "\n";
            std::cerr << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
            exit(EXIT_FAILURE);
        }
    }
    enableThread_saveProcessResult_ = true;
    thread_saveProcessResult_       = std::thread(&ThreadSaveFile::saveProcessResult, this);
#ifdef _SAVEFILE_DEBUG_
    std::cout << termColor("green") << "Thread Process Result Saver is created" << termColor("nocolor") << "\n";
#endif
}

void ThreadSaveFile::creatThread_savePosRes(sfq::Safe_Queue<PositionResult> *dataque) {
    if (posResFileSaver_->isOpen()) {
        if (isLoadPosResQueue_) {
            std::cerr << termColor("red") << "Position Result Saver file is already loaded" << termColor("nocolor")
                      << "\n";
            std::cerr << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
            exit(EXIT_FAILURE);
        }
        posResQue_         = dataque;
        thread_savePosRes_ = std::thread(&ThreadSaveFile::savePosRes, this);
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread Position Result Saver is created" << termColor("nocolor") << "\n";
#endif
    } else {
        std::cerr << termColor("red") << "Position Result Saver file is not open" << termColor("nocolor") << "\n";
    }
}

void ThreadSaveFile::creatThread_saveCorrelation(sfq::Safe_Queue<ChannelSignalVector> *dataque) {
    if (correlationFileSaver_->isOpen()) {
        if (isLoadCorrelationQueue_) {
            std::cerr << termColor("red") << "Correlation Saver file is already loaded" << termColor("nocolor") << "\n";
            std::cerr << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
            exit(EXIT_FAILURE);
        }
        correlationQue_         = dataque;
        thread_saveCorrelation_ = std::thread(&ThreadSaveFile::saveCorrelation, this);
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread Correlation Saver is created" << termColor("nocolor") << "\n";
#endif
    }
}

void ThreadSaveFile::creatThread_saveTOFRes(sfq::Safe_Queue<std::vector<double>> *dataque) {
    if (tofResFileSaver_->isOpen()) {
        if (isLoadTOFResQueue_) {
            std::cerr << termColor("red") << "TOF Result Saver file is already loaded" << termColor("nocolor") << "\n";
            std::cerr << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
            exit(EXIT_FAILURE);
        }
        tofResQue_         = dataque;
        thread_saveTOFRes_ = std::thread(&ThreadSaveFile::saveTOFRes, this);
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread TOF Result Saver is created" << termColor("nocolor") << "\n";
#endif
    } else {
        std::cerr << termColor("red") << "TOF Result Saver file is not open" << termColor("nocolor") << "\n";
    }
}

void ThreadSaveFile::creatThread_saveBeamPattern(sfq::Safe_Queue<Eigen::MatrixXd> *dataque) {
    if (beamPatternFileSaver_->isOpen()) {
        if (isLoadBeamPatternQueue_) {
            std::cerr << termColor("red") << "Beam Pattern Saver file is already loaded" << termColor("nocolor")
                      << "\n";
            std::cerr << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
            exit(EXIT_FAILURE);
        }
        beamPatternQue_         = dataque;
        thread_saveBeamPattern_ = std::thread(&ThreadSaveFile::saveBeamPattern, this);
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread Beam Pattern Saver is created" << termColor("nocolor") << "\n";
#endif
    } else {
        std::cerr << termColor("red") << "Beam Pattern Saver file is not open" << termColor("nocolor") << "\n";
    }
}

void ThreadSaveFile::creatThread_saveSideAmpSpec(sfq::Safe_Queue<ChannelSignalVector> *dataque) {
    if (sideAmpSpecFileSaver_->isOpen()) {
        if (isLoadSideAmpSpecQueue_) {
            std::cerr << termColor("red") << "Side Amp Spec Saver file is already loaded" << termColor("nocolor")
                      << "\n";
            std::cerr << termColor("red") << "Thread Creat Terminate" << termColor("nocolor") << "\n";
            exit(EXIT_FAILURE);
        }
        sideAmpSpecQue_         = dataque;
        thread_saveSideAmpSpec_ = std::thread(&ThreadSaveFile::saveSideAmpSpec, this);
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread Side Amp Spec Saver is created" << termColor("nocolor") << "\n";
#endif
    } else {
        std::cerr << termColor("red") << "Side Amp Spec Saver file is not open" << termColor("nocolor") << "\n";
    }
}

void ThreadSaveFile::joinThread_saveDAQAIData() {
    if (thread_saveDAQAIData_.joinable()) {
        thread_saveDAQAIData_.join();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread DAQ AI Data Saver is joined" << termColor("nocolor") << "\n";
#endif
    }
}

void ThreadSaveFile::joinThread_saveProcessResult() {
    if (thread_saveProcessResult_.joinable()) {
        thread_saveProcessResult_.join();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread Process Result Saver is joined" << termColor("nocolor") << "\n";
#endif
    }
}

void ThreadSaveFile::joinThread_savePosRes() {
    if (thread_savePosRes_.joinable()) {
        thread_savePosRes_.join();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread Position Result Saver is joined" << termColor("nocolor") << "\n";
#endif
    }
}

void ThreadSaveFile::joinThread_saveCorrelation() {
    if (thread_saveCorrelation_.joinable()) {
        thread_saveCorrelation_.join();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread Correlation Saver is joined" << termColor("nocolor") << "\n";
#endif
    }
}

void ThreadSaveFile::joinThread_saveTOFRes() {
    if (thread_saveTOFRes_.joinable()) {
        thread_saveTOFRes_.join();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread TOF Result Saver is joined" << termColor("nocolor") << "\n";
#endif
    }
}

void ThreadSaveFile::joinThread_saveBeamPattern() {
    if (thread_saveBeamPattern_.joinable()) {
        thread_saveBeamPattern_.join();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread Beam Pattern Saver is joined" << termColor("nocolor") << "\n";
#endif
    }
}

void ThreadSaveFile::joinThread_saveSideAmpSpec() {
    if (thread_saveSideAmpSpec_.joinable()) {
        thread_saveSideAmpSpec_.join();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Thread Side Amp Spec Saver is joined" << termColor("nocolor") << "\n";
#endif
    }
}

void ThreadSaveFile::closeThread_saveDAQAIData() {
    enableThread_saveDAQAIData_ = false;
    joinThread_saveDAQAIData();
    if (daqaiFileSaver_->isOpen()) {
        daqaiFileSaver_->close();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "DAQ AI Data Saver file is closed" << termColor("nocolor") << "\n";
#endif
    }
    if (thread_saveDAQAIData_.joinable()) {
        thread_saveDAQAIData_.join();
    }
}

void ThreadSaveFile::closeThread_saveProcessResult() {
    enableThread_saveProcessResult_ = false;
    joinThread_saveProcessResult();
    if (posResFileSaver_->isOpen()) {
        posResFileSaver_->close();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Position Result Saver file is closed" << termColor("nocolor") << "\n";
#endif
    }
    if (tofResFileSaver_->isOpen()) {
        tofResFileSaver_->close();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "TOF Result Saver file is closed" << termColor("nocolor") << "\n";
#endif
    }
    if (beamPatternFileSaver_->isOpen()) {
        beamPatternFileSaver_->close();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Beam Pattern Saver file is closed" << termColor("nocolor") << "\n";
#endif
    }
    if (sideAmpSpecFileSaver_->isOpen()) {
        sideAmpSpecFileSaver_->close();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Side Amp Spec Saver file is closed" << termColor("nocolor") << "\n";
#endif
    }
    if (thread_saveProcessResult_.joinable()) {
        thread_saveProcessResult_.join();
    }
}

void ThreadSaveFile::closeThread_savePosRes() {
    enableThread_savePosRes_ = false;
    joinThread_savePosRes();
    if (posResFileSaver_->isOpen()) {
        posResFileSaver_->close();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Position Result Saver file is closed" << termColor("nocolor") << "\n";
#endif
    }
    if (thread_savePosRes_.joinable()) {
        thread_savePosRes_.join();
    }
}

void ThreadSaveFile::closeThread_saveCorrelation() {
    enableThread_saveCorrelation_ = false;
    joinThread_saveCorrelation();
    if (correlationFileSaver_->isOpen()) {
        correlationFileSaver_->close();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Correlation Saver file is closed" << termColor("nocolor") << "\n";
#endif
    }
    if (thread_saveCorrelation_.joinable()) {
        thread_saveCorrelation_.join();
    }
}

void ThreadSaveFile::closeThread_saveTOFRes() {
    enableThread_saveTOFRes_ = false;
    joinThread_saveTOFRes();
    if (tofResFileSaver_->isOpen()) {
        tofResFileSaver_->close();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "TOF Result Saver file is closed" << termColor("nocolor") << "\n";
#endif
    }
    if (thread_saveTOFRes_.joinable()) {
        thread_saveTOFRes_.join();
    }
}

void ThreadSaveFile::closeThread_saveBeamPattern() {
    enableThread_saveBeamPattern_ = false;
    joinThread_saveBeamPattern();
    if (beamPatternFileSaver_->isOpen()) {
        beamPatternFileSaver_->close();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Beam Pattern Saver file is closed" << termColor("nocolor") << "\n";
#endif
    }
    if (thread_saveBeamPattern_.joinable()) {
        thread_saveBeamPattern_.join();
    }
}

void ThreadSaveFile::closeThread_saveSideAmpSpec() {
    enableThread_saveSideAmpSpec_ = false;
    joinThread_saveSideAmpSpec();
    if (sideAmpSpecFileSaver_->isOpen()) {
        sideAmpSpecFileSaver_->close();
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "Side Amp Spec Saver file is closed" << termColor("nocolor") << "\n";
#endif
    }
    if (thread_saveSideAmpSpec_.joinable()) {
        thread_saveSideAmpSpec_.join();
    }
}

void ThreadSaveFile::saveDAQAIData() {
    // check the queue and file saver
    if (!isLoadDAQAIQueue_ && !daqaiFileSaver_->isOpen()) {
        std::cerr << termColor("red") << "DAQ AI Data Queue is not loaded or DAQ AI Data Saver file is not open"
                  << termColor("nocolor") << "\n";
        std::cerr << termColor("red") << "Thread Terminate" << termColor("nocolor") << "\n";
        exit(EXIT_FAILURE);
    }
    while (enableThread_saveDAQAIData_) {
        // std::cout << daqaiDataQue_->size() << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        daqaiTempData_ = daqaiDataQue_->wait_and_pop();
        for (int i = 0; i < daqaiTempData_.channelNum; ++i) {
            daqaiFileSaver_->dump(daqaiTempData_.channels[i]);
        }
#ifdef _SAVEFILE_DEBUG_
        std::cout << termColor("green") << "DAQ AI Data Saver is saving data" << termColor("nocolor") << "\n";
#endif
    }
}

void ThreadSaveFile::saveProcessResult() {
    while (enableThread_saveProcessResult_) {
        if (isLoadPosResQueue_) {
            if (posResFileSaver_->isOpen()) {
                posRes_ = posResQue_->wait_and_pop();
                std::vector<double> posResData;
                posResData.push_back(posRes_.time);
                posResData.push_back(posRes_.position.x());
                posResData.push_back(posRes_.position.y());
                posResData.push_back(posRes_.position.z());
                posResData.push_back(posRes_.tof);
                posResData.push_back(posRes_.doa);
                posResFileSaver_->dump(posResData);
            } else {
                if (!systemInfo_->savedFileInfo.isSavePosRes) {
                    posResQue_->try_pop(posRes_);
                } else {
                    std::cerr << termColor("red") << "Position Result Saver file is not open" << termColor("nocolor")
                              << "\n";
                }
            }
        }
        if (isLoadCorrelationQueue_) {
            if (correlationFileSaver_->isOpen()) {
                correlationRes_ = correlationQue_->wait_and_pop();
                for (int i = 0; i < correlationRes_.channelNum; ++i) {
                    correlationFileSaver_->dump(correlationRes_.channels[i]);
                }
            } else {
                if (!systemInfo_->savedFileInfo.isSaveCorrelation) {
                    correlationQue_->try_pop(correlationRes_);
                } else {
                    std::cerr << termColor("red") << "Correlation Saver file is not open" << termColor("nocolor")
                              << "\n";
                }
            }
        }
        if (isLoadTOFResQueue_) {
            if (tofResFileSaver_->isOpen()) {
                tofRes_ = tofResQue_->wait_and_pop();
                tofResFileSaver_->dump(tofRes_);
            } else {
                if (!systemInfo_->savedFileInfo.isSaveTOFRes) {
                    tofResQue_->try_pop(tofRes_);
                } else {
                    std::cerr << termColor("red") << "TOF Result Saver file is not open" << termColor("nocolor")
                              << "\n";
                }
            }
        }
        if (isLoadBeamPatternQueue_) {
            if (beamPatternFileSaver_->isOpen()) {
                beamPattern_ = beamPatternQue_->wait_and_pop();
                for (int i = 0; i < beamPattern_.rows(); ++i) {
                    std::vector<double> row(beamPattern_.cols());
                    for (int j = 0; j < beamPattern_.cols(); ++j) {
                        row[j] = beamPattern_(i, j);
                    }
                    beamPatternFileSaver_->dump(row);
                }
            } else {
                if (!systemInfo_->savedFileInfo.isSaveBeamPattern) {
                    beamPatternQue_->try_pop(beamPattern_);
                } else {
                    std::cerr << termColor("red") << "Beam Pattern Saver file is not open" << termColor("nocolor")
                              << "\n";
                }
            }
        }
        if (isLoadSideAmpSpecQueue_) {
            if (sideAmpSpecFileSaver_->isOpen()) {
                sideAmpSpec_ = sideAmpSpecQue_->wait_and_pop();
                for (int i = 0; i < sideAmpSpec_.channelNum; ++i) {
                    sideAmpSpecFileSaver_->dump(sideAmpSpec_.channels[i]);
                }
            } else {
                if (!systemInfo_->savedFileInfo.isSaveSideAmpSpec) {
                    sideAmpSpecQue_->try_pop(sideAmpSpec_);
                } else {
                    std::cerr << termColor("red") << "Side Amp Spec Saver file is not open" << termColor("nocolor")
                              << "\n";
                }
            }
        }
    }
}

void ThreadSaveFile::savePosRes() {
    // check the queue and file saver
    if (!isLoadPosResQueue_ && !posResFileSaver_->isOpen()) {
        std::cerr << termColor("red") << "Position Result Queue is not loaded or Position Result Saver file is not open"
                  << termColor("nocolor") << "\n";
        std::cerr << termColor("red") << "Thread Terminate" << termColor("nocolor") << "\n";
        exit(EXIT_FAILURE);
    }
    while (enableThread_savePosRes_) {
        posRes_ = posResQue_->wait_and_pop();
        std::vector<double> posResData;
        posResData.push_back(posRes_.time);
        posResData.push_back(posRes_.position.x());
        posResData.push_back(posRes_.position.y());
        posResData.push_back(posRes_.position.z());
        posResData.push_back(posRes_.tof);
        posResData.push_back(posRes_.doa);
        posResFileSaver_->dump(posResData);
    }
}

void ThreadSaveFile::saveCorrelation() {
    // check the queue and file saver
    if (!isLoadCorrelationQueue_ && !correlationFileSaver_->isOpen()) {
        std::cerr << termColor("red") << "Correlation Queue is not loaded or Correlation Saver file is not open"
                  << termColor("nocolor") << "\n";
        std::cerr << termColor("red") << "Thread Terminate" << termColor("nocolor") << "\n";
        exit(EXIT_FAILURE);
    }
    while (enableThread_saveCorrelation_) {
        correlationRes_ = correlationQue_->wait_and_pop();
        for (int i = 0; i < correlationRes_.channelNum; ++i) {
            correlationFileSaver_->dump(correlationRes_.channels[i]);
        }
    }
}

void ThreadSaveFile::saveTOFRes() {
    while (enableThread_saveTOFRes_) {
        tofRes_ = tofResQue_->wait_and_pop();
        tofResFileSaver_->dump(tofRes_);
    }
}

void ThreadSaveFile::saveBeamPattern() {
    // check the queue and file saver
    if (!isLoadBeamPatternQueue_ && !beamPatternFileSaver_->isOpen()) {
        std::cerr << termColor("red") << "Beam Pattern Queue is not loaded or Beam Pattern Saver file is not open"
                  << termColor("nocolor") << "\n";
        std::cerr << termColor("red") << "Thread Terminate" << termColor("nocolor") << "\n";
        exit(EXIT_FAILURE);
    }
    while (enableThread_saveBeamPattern_) {
        // std::cout << daqaiDataQue_->size() << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        beamPattern_ = beamPatternQue_->wait_and_pop();
        for (int i = 0; i < beamPattern_.rows(); ++i) {
            std::vector<double> row(beamPattern_.cols());
            for (int j = 0; j < beamPattern_.cols(); ++j) {
                row[j] = beamPattern_(i, j);
                // std::cout << beamPattern_(i, j) << std::endl;
            }
            // normalize the row
            // double maxvalue = *std::max_element(row.begin(), row.end());
            // for(auto &value: row) {
            //     value = value / maxvalue;
            // }
            // beamPatternFileSaver_->dump(row);
            beamPatternFileSaver_->dump(row);
        }
    }
}

void ThreadSaveFile::saveSideAmpSpec() {
    // check the queue and file saver
    if (!isLoadSideAmpSpecQueue_ && !sideAmpSpecFileSaver_->isOpen()) {
        std::cerr << termColor("red") << "Side Amp Spec Queue is not loaded or Side Amp Spec Saver file is not open"
                  << termColor("nocolor") << "\n";
        std::cerr << termColor("red") << "Thread Terminate" << termColor("nocolor") << "\n";
        exit(EXIT_FAILURE);
    }
    while (enableThread_saveSideAmpSpec_) {
        sideAmpSpec_ = sideAmpSpecQue_->wait_and_pop();
        for (int i = 0; i < sideAmpSpec_.channelNum; ++i) {
            sideAmpSpecFileSaver_->dump(sideAmpSpec_.channels[i]);
        }
    }
}
