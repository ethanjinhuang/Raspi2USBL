
#include "signalProcess.h"

SignalProcess::SignalProcess(SystemInfo &systemInfo, ChannelSignalVector &refSignal)
    : SignalBase(systemInfo)
    , systemInfo_(systemInfo) {
    refSignal_ = refSignal;
    init();
}

void SignalProcess::init() {
    tofProcess_          = new TOF(systemInfo_, refSignal_);
    doaProcess_          = new DOA(systemInfo_, refSignal_);

    maxPower_            = systemInfo_.agcInfo.maxPower;
    minPower_            = systemInfo_.agcInfo.minPower;
    receiveGain_         = systemInfo_.agcInfo.initGainValue;
    agcStep_             = systemInfo_.agcInfo.gainStep;

    processSignalLength_ = systemInfo_.signalProcessInfo.processDuration * systemInfo_.aiScanInfo.rate;
    beamPattern_.resize(systemInfo_.arrayInfo.arrayNum, (int) processSignalLength_);
    tofRes_.resize(systemInfo_.arrayInfo.arrayNum);

    tofOutput_ = 0.0;
    doaOutput_ = 0.0;
}

void SignalProcess::loadRefSignal(const ChannelSignalVector &refSignal) {
    refSignal_       = refSignal;
    isLoadRefSignal_ = true;
}

void SignalProcess::updateInputSignal(const ChannelSignalVector &inputSignal) {
    if (isUpdateInputSignal_) {
        std::cerr << termColor("red") << "SignalProcess::updateInputSignal: updated signal is not processed"
                  << termColor("nocolor") << std::endl;
        std::exit(EXIT_FAILURE);
    }
    signalInput_         = inputSignal;
    isUpdateInputSignal_ = true;
    // reset process status
    isTOFCalculated_ = false;
    isDOACalculated_ = false;
}

double SignalProcess::calculateTOF() {
    if (!isLoadRefSignal_) {
        std::cerr << termColor("red") << "SignalProcess::calculateTOF: reference signal is not loaded"
                  << termColor("nocolor") << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (!isUpdateInputSignal_) {
        std::cerr << termColor("red") << "SignalProcess::calculateTOF: input signal is not updated"
                  << termColor("nocolor") << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // process
    tofProcess_->calculateTOF(signalInput_, tofRes_);
    // save the correlation result
    correlationResult_ = tofProcess_->getCorrelationResult();
    // set the process status
    isTOFCalculated_ = true;

    // return the optimal TOF
    return tofOutput_ = calOptimalTOF(tofRes_);
}

double SignalProcess::calculateDOA() {
    if (!isTOFCalculated_) {
        std::cerr << termColor("red") << "SignalProcess::calculateDOA: TOF is not calculated" << termColor("nocolor")
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }
    // find the minimum TOF
    double minTOF     = *std::min_element(tofRes_.begin(), tofRes_.end());
    int    startIndex = (int) (minTOF * systemInfo_.signalInfo.sampleRate);
    // set doa parameters
    doaProcess_->setParam(startIndex, systemInfo_.signalProcessInfo.processDuration,
                          systemInfo_.signalProcessInfo.startFrequency, systemInfo_.signalProcessInfo.endFrequency,
                          systemInfo_.signalProcessInfo.doaStep);
    // process
    doaProcess_->calculateDOA_CBF(signalInput_, doaOutput_);
    // save the beam pattern
    doaProcess_->getBeamPattern(beamPattern_);
    // set the process status
    isDOACalculated_ = true;

    return doaOutput_;
}

double SignalProcess::calOptimalTOF(const std::vector<double> &tofres) {
    // using the minimum TOF as the optimal TOF
    return *std::min_element(tofres.begin(), tofres.end());
}

double SignalProcess::updateACG() {
    if (!isTOFCalculated_) {
        std::cerr << termColor("red") << "SignalProcess::updateACG: TOF is not calculated" << termColor("nocolor")
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // initialize the ACG
    agcPower_ = -std::numeric_limits<double>::infinity();
    // find the global maximum
    for (const auto &channel : correlationResult_.channels) {
        // find the maximum in each channel
        auto maxInChannel = *std::max_element(channel.begin(), channel.end());
        // update the ACG power
        if (maxInChannel > agcPower_) {
            agcPower_ = maxInChannel;
        }
    }

    // update the receive gain
    // 0->0v, 254->3.3v
    if (agcPower_ > maxPower_) {
        receiveGain_ -= agcStep_;
        // if (receiveGain_ < 11) {
        //     receiveGain_ = 10;
        // }
    } else if (agcPower_ < minPower_) {
        receiveGain_ += agcStep_;
        // if (receiveGain_ > 249) {
        //     receiveGain_ = 250;
        // }
    }

    // define the range of the receive gain
    if (receiveGain_ <= 0) {
        receiveGain_ = 0;
    } else if (receiveGain_ >= systemInfo_.agcInfo.maxGainValue) {
        receiveGain_ = systemInfo_.agcInfo.maxGainValue;
    }
    // update the ACG status
    isACGUpdated_ = true;

    return receiveGain_;
}

void SignalProcess::resetFlag() {
    isUpdateInputSignal_ = false;
    isTOFCalculated_     = false;
    isDOACalculated_     = false;
    isACGUpdated_        = false;
}

void SignalProcess::getBeamPattern(Eigen::MatrixXd &beamPattern) {
    if (!isDOACalculated_) {
        std::cerr << termColor("red") << "SignalProcess::getBeamPattern: DOA is not calculated" << termColor("nocolor")
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }
    beamPattern = beamPattern_;
}

void SignalProcess::getTOFResult(std::vector<double> &tofRes) {
    if (!isTOFCalculated_) {
        std::cerr << termColor("red") << "SignalProcess::getTOFResult: TOF is not calculated" << termColor("nocolor")
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }
    tofRes = tofRes_;
}

void SignalProcess::getCorrelationResult(ChannelSignalVector &correlationResult) {
    if (!isTOFCalculated_) {
        std::cerr << termColor("red") << "SignalProcess::getCorrelationResult: TOF is not calculated"
                  << termColor("nocolor") << std::endl;
        std::exit(EXIT_FAILURE);
    }
    correlationResult = correlationResult_;
}

void SignalProcess::getSignalSideAmpSpec(ChannelSignalVector &signalSideAmpSpec) {
    if (!isDOACalculated_) {
        std::cerr << termColor("red") << "SignalProcess::getSignalSideAmpSpec: DOA is not calculated"
                  << termColor("nocolor") << std::endl;
        std::exit(EXIT_FAILURE);
    }
    Eigen::MatrixXd tempMatrix;
    doaProcess_->getSignalSideAmpSpec(tempMatrix);
    signalSideAmpSpec.resize(tempMatrix.rows(), tempMatrix.cols());
    for (int i = 0; i < tempMatrix.rows(); ++i) {
        for (int j = 0; j < tempMatrix.cols(); ++j) {
            signalSideAmpSpec.channels[i][j] = tempMatrix(i, j);
        }
    }
}
