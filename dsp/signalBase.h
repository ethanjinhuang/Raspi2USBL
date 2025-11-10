/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-08-19 15:05:00
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-09-05 16:43:12
 * @FilePath: /RaspiUSBL/dsp/signalBase.h
 * @Description: Signal base class defination
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */

#ifndef _SIGNALBASE_H_
#define _SIGNALBASE_H_

#include "../config/defineconfig.h"
#include "../core/systeminfo.h"
#include "../general/typedef.h"
#include "../tool/ColorParse.h"
#include "../tool/SafeQueue.hpp"
#include <fftw3.h>

// general FFT function
void perform_fft(const std::complex<double> *input, std::complex<double> *output, int signalLength,
                 bool inverse = false);

class SignalBase {
public:
    SignalBase(SystemInfo &systeminfo);
    ~SignalBase() = default;

    // data type conversion
    void csv2cs(const ChannelSignalVector &csv, ChannelSignal &cs);
    void cs2csv(const ChannelSignal &cs, ChannelSignalVector &csv);
    void cs2csc(const ChannelSignal &cs, ChannelSignalComplex &csc);
    void cs2csed(const ChannelSignal &cs, ChannelSignalEigenD &csed);
    void csed2cs(const ChannelSignalEigenD &csed, ChannelSignal &cs);
    void csv2csed(const ChannelSignalVector &csv, ChannelSignalEigenD &csed);
    void csed2csv(const ChannelSignalEigenD &csed, ChannelSignalVector &csv);
    void cs2csec(const ChannelSignal &cs, ChannelSignalEigenC &csec);
    void csec2cs(const ChannelSignalEigenC &csec, ChannelSignal &cs);
    void csv2csec(const ChannelSignalVector &csv, ChannelSignalEigenC &csec);
    void csec2csv(const ChannelSignalEigenC &csec, ChannelSignalVector &csv);
    // FFT

    /***
     * @description: FFT for each channel of ChannelSignal
     * @param {ChannelSignal} &cs                   The input signal in ChannelSignal format
     * @param {ChannelSignalComplex} &csfft The     FFT result
     * @return {*}
     */
    void csfft(const ChannelSignal &cs, ChannelSignalComplex &csfft);

    /***
     * @description: FFT for each channel of ChannelSignal
     * @param {ChannelSignalVector} &csv            The input signal in ChannelSignalVector format
     * @param {ChannelSignalComplex} &csfft The     FFT result
     * @return {*}
     */
    void csvfft(const ChannelSignalVector &csv, ChannelSignalComplex &csfft);

    void csedfft(const ChannelSignalEigenD &csed, ChannelSignalEigenC &csecfft);
    void csecfft(const ChannelSignalEigenC &csec, ChannelSignalEigenC &csecfft);

    /***
     * @description: Convolution function w using FFT
     * returns the full convolution length
     * SEE Matlab -CONV(A,B,'full')
     * Length of the output signal is MAX([LENGTH(A)+LENGTH(B)-1,LENGTH(A),LENGTH(B)])
     * @param {ChannelSignalVector} &signal1    The sampling signal for each channel (can be multi channels)
     * @param {ChannelSignalVector} &signal2    The reference signal (single channel, signal2.channelNum must be 1)
     * @param {ChannelSignalVector} &output     The convolution result
     * @return {*}
     */
    void csvconv(const ChannelSignalVector &signal1, const ChannelSignalVector &signal2, ChannelSignalVector &output);

    /***
     * @description: Convolution function without the zero-padded edges using FFT
     * returns only those parts of the convolution that are computed without the zero-padded edges.
     * SEE Matlab -CONV(A,B,'valid')
     * Length of the output signal is MAX(LENGTH(A)-MAX(0,LENGTH(B)-1),0)
     * @param {ChannelSignalVector} &signal1    The sampling signal for each channel (can be multi channels)
     * @param {ChannelSignalVector} &signal2    The reference signal (single channel, signal2.channelNum must be 1)
     * @param {ChannelSignalVector} &output     The convolution result
     * @return {*}
     */
    void csvconv_valid(const ChannelSignalVector &signal1, const ChannelSignalVector &signal2,
                       ChannelSignalVector &output);

    void csedconv(const ChannelSignalEigenD &signal1, const ChannelSignalEigenD &signal2, ChannelSignalEigenD &output);
    void csedconv_valid(const ChannelSignalEigenD &signal1, const ChannelSignalEigenD &signal2,
                        ChannelSignalEigenD &output);

    // data trim
    void dataTrim(ChannelSignal &cs, int start, int end);
    void dataTrim(ChannelSignalComplex &csv, int start, int end);
    void dataTrim(ChannelSignalVector &csv, int start, int end);
    void dataTrim(ChannelSignalEigenD &csed, int start, int end);
    void dataTrim(ChannelSignalEigenC &csec, int start, int end);

    // signal power
    std::vector<double> signalPower(const ChannelSignalVector &csv);

protected:
    SystemInfo systemInfo_;
    int        channelNum_;
    int        sampleRate_;
    int        sampleNum_;

    // std::vector<double>  signalGenerated_;
    // ChannelSignal        channelSignal_;
    // ChannelSignalVector  channelSignalVec_;
    // ChannelSignalComplex channelSignalComplex_;
};

// general FFT function
inline void perform_fft(const std::complex<double> *input, std::complex<double> *output, int signalLength,
                        bool inverse) {
    // Create FFT plan
    fftw_plan p = fftw_plan_dft_1d(
        signalLength, reinterpret_cast<fftw_complex *>(const_cast<std::complex<double> *>(input)),
        reinterpret_cast<fftw_complex *>(output), inverse ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE);
    // reinterpret_cast<fftw_complex *>(output), inverse ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_MEASURE);
    if (!p) {
        throw std::runtime_error("[Error] Failed to create FFT plan.");
    }

    // Execute FFT or IFFT
    fftw_execute(p);

    // If it's an inverse FFT, normalize the output
    if (inverse) {
        for (int i = 0; i < signalLength; ++i) {
            output[i] /= signalLength;
        }
    }

    // std::cout << "FFT result: " << std::endl;
    // for (int i = 0; i < signalLength; ++i) {
    //     std::cout << output[i] << std::endl;
    // }
    // Destroy FFT plan
    fftw_destroy_plan(p);
}

inline SignalBase::SignalBase(SystemInfo &systeminfo) {
    systemInfo_ = systeminfo;
    channelNum_ = systemInfo_.aiScanInfo.highChan - systemInfo_.aiScanInfo.lowChan + 1;
    sampleRate_ = systemInfo_.aiScanInfo.rate;
    sampleNum_  = systemInfo_.aiScanInfo.samplesPerChannel;
}

inline void SignalBase::csv2cs(const ChannelSignalVector &csv, ChannelSignal &cs) {
    if (!csv.isInit) {
        throw std::runtime_error("[Error] ChannelSignalVector is not initialized.");
    }
    if (cs.channelNum != csv.channelNum || cs.signalLength != csv.signalLength) {
        throw std::invalid_argument("[Error] Mismatched dimensions between csv and cs.");
    }
    for (int i = 0; i < csv.channelNum; ++i) {
        for (int j = 0; j < csv.signalLength; ++j) {
            cs.channels[i][j] = csv.channels[i][j];
        }
    }
}

inline void SignalBase::cs2csv(const ChannelSignal &cs, ChannelSignalVector &csv) {
    if (!cs.isInit) {
        throw std::runtime_error("[Error] ChannelSignal is not initialized.");
    }
    if (csv.channelNum != cs.channelNum || csv.signalLength != cs.signalLength) {
        throw std::invalid_argument("[Error] Mismatched dimensions between cs and csv.");
    }
    for (int i = 0; i < cs.channelNum; ++i) {
        for (int j = 0; j < cs.signalLength; ++j) {
            csv.channels[i][j] = cs.channels[i][j];
        }
    }
}
inline void SignalBase::cs2csc(const ChannelSignal &cs, ChannelSignalComplex &csc) {
    if (!cs.isInit) {
        throw std::runtime_error("[Error] ChannelSignal is not initialized.");
    }
    if (csc.channelNum != cs.channelNum || csc.signalLength != cs.signalLength) {
        throw std::invalid_argument("[Error] Mismatched dimensions between cs and csc.");
    }
    for (int i = 0; i < cs.channelNum; ++i) {
        for (int j = 0; j < cs.signalLength; ++j) {
            csc.channels[i][j] = std::complex<float>(cs.channels[i][j], 0.0f);
        }
    }
}

inline void SignalBase::cs2csed(const ChannelSignal &cs, ChannelSignalEigenD &csed) {
    if (!cs.isInit) {
        throw std::runtime_error("[Error] ChannelSignal is not initialized.");
    }
    csed.channelNum   = cs.channelNum;
    csed.signalLength = cs.signalLength;
    csed.channels.resize(cs.channelNum, cs.signalLength);
    for (int i = 0; i < cs.channelNum; ++i) {
        for (int j = 0; j < cs.signalLength; ++j) {
            csed.channels(i, j) = cs.channels[i][j];
        }
    }
    csed.isInit = true;
}

inline void SignalBase::csed2cs(const ChannelSignalEigenD &csed, ChannelSignal &cs) {
    if (!csed.isInit) {
        throw std::runtime_error("[Error] ChannelSignalEigenD is not initialized.");
    }
    cs.channelNum   = csed.channelNum;
    cs.signalLength = csed.signalLength;
    for (int i = 0; i < cs.channelNum; ++i) {
        for (int j = 0; j < cs.signalLength; ++j) {
            cs.channels[i][j] = csed.channels(i, j);
        }
    }
    cs.isInit = true;
}

inline void SignalBase::csv2csed(const ChannelSignalVector &csv, ChannelSignalEigenD &csed) {
    if (!csv.isInit) {
        throw std::runtime_error("[Error] ChannelSignalVector is not initialized.");
    }
    csed.channelNum   = csv.channelNum;
    csed.signalLength = csv.signalLength;
    csed.channels.resize(csv.channelNum, csv.signalLength);
    for (int i = 0; i < csv.channelNum; ++i) {
        for (int j = 0; j < csv.signalLength; ++j) {
            csed.channels(i, j) = csv.channels[i][j];
        }
    }
    csed.isInit = true;
}

inline void SignalBase::csed2csv(const ChannelSignalEigenD &csed, ChannelSignalVector &csv) {
    if (!csed.isInit) {
        throw std::runtime_error("[Error] ChannelSignalEigenD is not initialized.");
    }
    csv.channelNum   = csed.channelNum;
    csv.signalLength = csed.signalLength;
    csv.channels.resize(csed.channelNum);
    for (int i = 0; i < csed.channelNum; ++i) {
        csv.channels[i].resize(csed.signalLength);
        for (int j = 0; j < csed.signalLength; ++j) {
            csv.channels[i][j] = csed.channels(i, j);
        }
    }
    csv.isInit = true;
}

inline void SignalBase::cs2csec(const ChannelSignal &cs, ChannelSignalEigenC &csec) {
    if (!cs.isInit) {
        throw std::runtime_error("[Error] ChannelSignal is not initialized.");
    }
    csec.channelNum   = cs.channelNum;
    csec.signalLength = cs.signalLength;
    csec.channels.resize(cs.channelNum, cs.signalLength);
    for (int i = 0; i < cs.channelNum; ++i) {
        for (int j = 0; j < cs.signalLength; ++j) {
            csec.channels(i, j) = std::complex<double>(cs.channels[i][j], 0.0);
        }
    }
    csec.isInit = true;
}

inline void SignalBase::csec2cs(const ChannelSignalEigenC &csec, ChannelSignal &cs) {
    if (!csec.isInit) {
        throw std::runtime_error("[Error] ChannelSignalEigenC is not initialized.");
    }
    cs.channelNum   = csec.channelNum;
    cs.signalLength = csec.signalLength;
    for (int i = 0; i < cs.channelNum; ++i) {
        for (int j = 0; j < cs.signalLength; ++j) {
            cs.channels[i][j] = csec.channels(i, j).real();
        }
    }
    cs.isInit = true;
}

inline void SignalBase::csv2csec(const ChannelSignalVector &csv, ChannelSignalEigenC &csec) {
    if (!csv.isInit) {
        throw std::runtime_error("[Error] ChannelSignalVector is not initialized.");
    }
    csec.channelNum   = csv.channelNum;
    csec.signalLength = csv.signalLength;
    csec.channels.resize(csv.channelNum, csv.signalLength);
    for (int i = 0; i < csv.channelNum; ++i) {
        for (int j = 0; j < csv.signalLength; ++j) {
            csec.channels(i, j) = std::complex<double>(csv.channels[i][j], 0.0);
        }
    }
    csec.isInit = true;
}

inline void SignalBase::csec2csv(const ChannelSignalEigenC &csec, ChannelSignalVector &csv) {
    if (!csec.isInit) {
        throw std::runtime_error("[Error] ChannelSignalEigenC is not initialized.");
    }
    csv.channelNum   = csec.channelNum;
    csv.signalLength = csec.signalLength;
    csv.channels.resize(csec.channelNum);
    for (int i = 0; i < csec.channelNum; ++i) {
        csv.channels[i].resize(csec.signalLength);
        for (int j = 0; j < csec.signalLength; ++j) {
            csv.channels[i][j] = csec.channels(i, j).real();
        }
    }
    csv.isInit = true;
}

inline void SignalBase::csfft(const ChannelSignal &cs, ChannelSignalComplex &cscfft_out) {
    if (!cs.isInit) {
        throw std::runtime_error("[Error] ChannelSignal is not initialized.");
    }
    if (cscfft_out.channelNum != cs.channelNum || cscfft_out.signalLength != cs.signalLength) {
        throw std::invalid_argument("[Error] Mismatched dimensions between cs and csfft.");
    }

    // Convert cs to csc
    ChannelSignalComplex cscInput(cs.channelNum, cs.signalLength);
    cs2csc(cs, cscInput);

    // Allocate memory for output
    std::complex<double> *output = (std::complex<double> *) fftw_malloc(cs.signalLength * sizeof(std::complex<double>));
    if (!output) {
        throw std::runtime_error("[Error] Failed to allocate memory for FFT output.");
    }

    // Execute FFT for each channel
    try {
        for (int i = 0; i < cs.channelNum; ++i) {
            perform_fft(cscInput.channels[i], output, cs.signalLength, false);

            // Check and free existing memory
            if (cscfft_out.channels[i]) {
                fftw_free(cscfft_out.channels[i]); // Free existing memory before reallocating
            }

            // Allocate memory for cscfft_out
            cscfft_out.channels[i] =
                (std::complex<double> *) fftw_malloc(cs.signalLength * sizeof(std::complex<double>));
            if (!cscfft_out.channels[i]) {
                fftw_free(output);
                throw std::runtime_error("[Error] Failed to allocate memory for FFT result.");
            }

            // Copy output to csfft
            std::copy(output, output + cs.signalLength, cscfft_out.channels[i]);
        }
    } catch (const std::exception &e) {
        fftw_free(output);
        std::cerr << e.what() << '\n';
    }

    // Free memory
    fftw_free(output);
}

inline void SignalBase::csvfft(const ChannelSignalVector &csv, ChannelSignalComplex &csfft_out) {
    if (!csv.isInit) {
        throw std::runtime_error("[Error] ChannelSignalVector is not initialized.");
    }
    if (csfft_out.channelNum != csv.channelNum || csfft_out.signalLength != csv.signalLength) {
        throw std::invalid_argument("[Error] Mismatched dimensions between csv and csfft.");
    }

    // convert csv to csc
    ChannelSignal cs(csv.channelNum, csv.signalLength);
    csv2cs(csv, cs);

    // using csfft
    csfft(cs, csfft_out);
}

inline void SignalBase::csedfft(const ChannelSignalEigenD &csed, ChannelSignalEigenC &csecfft_out) {
    if (!csed.isInit) {
        throw std::runtime_error("[Error] ChannelSignalEigenD is not initialized.");
    }
    if (csecfft_out.channelNum != csed.channelNum || csecfft_out.signalLength != csed.signalLength) {
        throw std::invalid_argument("[Error] Mismatched dimensions between csed and csecfft_out.");
    }

    csecfft_out.channelNum   = csed.channelNum;
    csecfft_out.signalLength = csed.signalLength;
    csecfft_out.channels.resize(csed.channelNum, csed.signalLength);

    std::complex<double> *output =
        (std::complex<double> *) fftw_malloc(csed.signalLength * sizeof(std::complex<double>));
    std::complex<double> *input =
        (std::complex<double> *) fftw_malloc(csed.signalLength * sizeof(std::complex<double>));
    for (int i = 0; i < csed.channelNum; ++i) {

        for (int j = 0; j < csed.signalLength; ++j) {
            input[j] = std::complex<double>(csed.channels(i, j), 0.0);
        }
        // std::cout << "Input to FFT for channel " << i << ": " << input[2] << std::endl;
        perform_fft(input, output, csed.signalLength, false);

        // save the result to csecfft_out
        for (int j = 0; j < csed.signalLength; ++j) {
            csecfft_out.channels(i, j) = output[j];
        }
    }
    csecfft_out.isInit = true;
    fftw_free(input);
    fftw_free(output);
}

inline void SignalBase::csecfft(const ChannelSignalEigenC &csec, ChannelSignalEigenC &csecfft_out) {
    if (!csec.isInit) {
        throw std::runtime_error("[Error] ChannelSignalEigenC is not initialized.");
    }
    if (csecfft_out.channelNum != csec.channelNum || csecfft_out.signalLength != csec.signalLength) {
        throw std::invalid_argument("[Error] Mismatched dimensions between csec and csecfft_out.");
    }

    for (int i = 0; i < csec.channelNum; ++i) {
        perform_fft(csec.channels.row(i).data(), csecfft_out.channels.row(i).data(), csec.signalLength, false);
    }

    csecfft_out.isInit = true;
}

inline void SignalBase::csvconv(const ChannelSignalVector &signal1, const ChannelSignalVector &signal2,
                                ChannelSignalVector &output) {
    if (!signal1.isInit || !signal2.isInit) {
        throw std::runtime_error("ChannelSignalVector is not initialized.");
    }

    if (signal2.channelNum != 1) {
        throw std::runtime_error("[Error] The channel number of signal2 must be 1.");
    }

    int outputLength    = signal1.signalLength + signal2.signalLength - 1;

    output.channelNum   = signal1.channelNum;
    output.signalLength = outputLength;
    output.channels.resize(output.channelNum);
    for (int i = 0; i < output.channelNum; ++i) {
        output.channels[i].resize(outputLength, 0.0);
    }

    int N           = outputLength;

    fftw_complex *X = fftw_alloc_complex(N);
    fftw_complex *H = fftw_alloc_complex(N);
    fftw_complex *Y = fftw_alloc_complex(N);

    for (int ch = 0; ch < signal1.channelNum; ++ch) {
        // std::fill(X, X + N, fftw_complex{0.0, 0.0});
        // std::fill(H, H + N, fftw_complex{0.0, 0.0});

        for (int i = 0; i < N; ++i) {
            X[i][0] = 0.0;
            X[i][1] = 0.0;
            H[i][0] = 0.0;
            H[i][1] = 0.0;
        }
        for (int i = 0; i < signal1.signalLength; ++i) {
            X[i][0] = signal1.channels[ch][i];
        }

        for (int i = 0; i < signal2.signalLength; ++i) {
            H[i][0] = signal2.channels[0][i];
        }

        perform_fft(reinterpret_cast<std::complex<double> *>(X), reinterpret_cast<std::complex<double> *>(X), N, false);
        perform_fft(reinterpret_cast<std::complex<double> *>(H), reinterpret_cast<std::complex<double> *>(H), N, false);

        for (int i = 0; i < N; ++i) {
            Y[i][0] = X[i][0] * H[i][0] - X[i][1] * H[i][1];
            Y[i][1] = X[i][0] * H[i][1] + X[i][1] * H[i][0];
        }

        perform_fft(reinterpret_cast<std::complex<double> *>(Y), reinterpret_cast<std::complex<double> *>(Y), N, true);

        for (int i = 0; i < outputLength; ++i) {
            output.channels[ch][i] = Y[i][0];
        }
    }

    fftw_free(X);
    fftw_free(H);
    fftw_free(Y);
}

inline void SignalBase::csvconv_valid(const ChannelSignalVector &signal1, const ChannelSignalVector &signal2,
                                      ChannelSignalVector &output) {
    if (!signal1.isInit || !signal2.isInit) {
        throw std::runtime_error("ChannelSignalVector is not initialized.");
    }

    if (signal2.channelNum != 1) {
        throw std::runtime_error("[Error] The channel number of signal2 must be 1.");
    }
    if (signal1.signalLength < signal2.signalLength || signal2.signalLength == 0) {
        throw std::invalid_argument("[Error] Invalid signal length.");
    }

    ChannelSignalVector fullConvResult(signal1.channelNum, signal1.signalLength + signal2.signalLength);
    csvconv(signal1, signal2, fullConvResult);

    int outputLength    = signal1.signalLength - signal2.signalLength + 1;

    output.channelNum   = signal1.channelNum;
    output.signalLength = outputLength;
    output.channels.resize(output.channelNum);
    for (int i = 0; i < output.channelNum; ++i) {
        output.channels[i].resize(outputLength, 0.0);
    }

    for (int ch = 0; ch < signal1.channelNum; ++ch) {
        std::copy(fullConvResult.channels[ch].begin() + (signal2.signalLength - 1),
                  fullConvResult.channels[ch].begin() + (signal2.signalLength - 1 + outputLength),
                  output.channels[ch].begin());
    }
}

inline void SignalBase::csedconv(const ChannelSignalEigenD &signal1, const ChannelSignalEigenD &signal2,
                                 ChannelSignalEigenD &output) {
    if (!signal1.isInit || !signal2.isInit) {
        throw std::runtime_error("[Error] ChannelSignalEigenD is not initialized.");
    }

    if (signal2.channelNum != 1) {
        throw std::runtime_error("[Error] The channel number of signal2 must be 1.");
    }

    int outputLength    = signal1.signalLength + signal2.signalLength - 1;

    output.channelNum   = signal1.channelNum;
    output.signalLength = outputLength;
    output.channels.resize(output.channelNum, outputLength);
    output.channels.setZero();

    int N           = outputLength;

    fftw_complex *X = fftw_alloc_complex(N);
    fftw_complex *H = fftw_alloc_complex(N);
    fftw_complex *Y = fftw_alloc_complex(N);

    for (int ch = 0; ch < signal1.channelNum; ++ch) {
        for (int i = 0; i < N; ++i) {
            X[i][0] = 0.0;
            X[i][1] = 0.0;
            H[i][0] = 0.0;
            H[i][1] = 0.0;
        }
        for (int i = 0; i < signal1.signalLength; ++i) {
            X[i][0] = signal1.channels(ch, i);
        }

        for (int i = 0; i < signal2.signalLength; ++i) {
            H[i][0] = signal2.channels(0, i);
        }

        perform_fft(reinterpret_cast<std::complex<double> *>(X), reinterpret_cast<std::complex<double> *>(X), N, false);
        perform_fft(reinterpret_cast<std::complex<double> *>(H), reinterpret_cast<std::complex<double> *>(H), N, false);

        for (int i = 0; i < N; ++i) {
            Y[i][0] = X[i][0] * H[i][0] - X[i][1] * H[i][1];
            Y[i][1] = X[i][0] * H[i][1] + X[i][1] * H[i][0];
        }

        perform_fft(reinterpret_cast<std::complex<double> *>(Y), reinterpret_cast<std::complex<double> *>(Y), N, true);

        for (int i = 0; i < outputLength; ++i) {
            output.channels(ch, i) = Y[i][0];
        }
    }

    fftw_free(X);
    fftw_free(H);
    fftw_free(Y);

    output.isInit = true;
}

inline void SignalBase::csedconv_valid(const ChannelSignalEigenD &signal1, const ChannelSignalEigenD &signal2,
                                       ChannelSignalEigenD &output) {
    if (!signal1.isInit || !signal2.isInit) {
        throw std::runtime_error("[Error] ChannelSignalEigenD is not initialized.");
    }

    if (signal2.channelNum != 1) {
        throw std::runtime_error("[Error] The channel number of signal2 must be 1.");
    }
    if (signal1.signalLength < signal2.signalLength || signal2.signalLength == 0) {
        throw std::invalid_argument("[Error] Invalid signal length.");
    }

    ChannelSignalEigenD fullConvResult;
    csedconv(signal1, signal2, fullConvResult);

    int outputLength    = signal1.signalLength - signal2.signalLength + 1;

    output.channelNum   = signal1.channelNum;
    output.signalLength = outputLength;
    output.channels.resize(output.channelNum, outputLength);
    output.channels.setZero();

    for (int ch = 0; ch < signal1.channelNum; ++ch) {
        output.channels.row(ch) = fullConvResult.channels.row(ch).segment(signal2.signalLength - 1, outputLength);
    }

    output.isInit = true;
}

inline void SignalBase::dataTrim(ChannelSignal &cs, int start, int end) {
    if (!cs.isInit) {
        throw std::runtime_error("[Error] ChannelSignal is not initialized.");
    }
    if (start < 0 || end >= cs.signalLength || start > end) {
        throw std::invalid_argument("[Error] Invalid start or end index.");
    }

    int newLength = end - start + 1;
    for (int i = 0; i < cs.channelNum; ++i) {
        // Allocate new trimmed channel data
        double *trimmedChannel = (double *) malloc(newLength * sizeof(double));
        if (!trimmedChannel) {
            throw std::runtime_error("[Error] Failed to allocate memory for trimmed data.");
        }
        // Use std::copy to copy data from start to end
        std::copy(cs.channels[i] + start, cs.channels[i] + end + 1, trimmedChannel);
        // Free old channel data and assign new trimmed data
        free(cs.channels[i]);
        cs.channels[i] = trimmedChannel;
    }
    cs.signalLength = newLength;
}

inline void SignalBase::dataTrim(ChannelSignalComplex &csc, int start, int end) {
    if (!csc.isInit) {
        throw std::runtime_error("[Error] ChannelSignalComplex is not initialized.");
    }
    if (start < 0 || end >= csc.signalLength || start > end) {
        throw std::invalid_argument("[Error] Invalid start or end index.");
    }

    int newLength = end - start + 1;
    for (int i = 0; i < csc.channelNum; ++i) {
        // Allocate new trimmed channel data
        std::complex<double> *trimmedChannel =
            (std::complex<double> *) malloc(newLength * sizeof(std::complex<double>));
        if (!trimmedChannel) {
            throw std::runtime_error("[Error] Failed to allocate memory for trimmed data.");
        }
        // Use std::copy to copy data from start to end
        std::copy(csc.channels[i] + start, csc.channels[i] + end + 1, trimmedChannel);
        // Free old channel data and assign new trimmed data
        free(csc.channels[i]);
        csc.channels[i] = trimmedChannel;
    }
    csc.signalLength = newLength;
}

inline void SignalBase::dataTrim(ChannelSignalVector &csv, int start, int end) {
    if (!csv.isInit) {
        throw std::runtime_error("[Error] ChannelSignalVector is not initialized.");
    }
    if (start < 0 || end >= csv.signalLength || start > end) {
        throw std::invalid_argument("Invalid start or end index.");
    }

    int newLength = end - start + 1;
    for (int i = 0; i < csv.channelNum; ++i) {
        // Trim vector data from start to end using std::copy
        std::vector<double> trimmedChannel(newLength);
        std::copy(csv.channels[i].begin() + start, csv.channels[i].begin() + end + 1, trimmedChannel.begin());
        csv.channels[i].assign(csv.channels[i].begin() + start, csv.channels[i].begin() + end + 1);
    }
    csv.signalLength = newLength;
}

inline void SignalBase::dataTrim(ChannelSignalEigenD &csed, int start, int end) {
    if (!csed.isInit) {
        throw std::runtime_error("[Error] ChannelSignalEigenD is not initialized.");
    }
    if (start < 0 || end >= csed.signalLength || start > end) {
        throw std::invalid_argument("[Error] Invalid start or end index.");
    }

    int newLength     = end - start + 1;
    csed.channels     = csed.channels.block(0, start, csed.channelNum, newLength);
    csed.signalLength = newLength;
}

inline void SignalBase::dataTrim(ChannelSignalEigenC &csec, int start, int end) {
    if (!csec.isInit) {
        throw std::runtime_error("[Error] ChannelSignalEigenC is not initialized.");
    }
    if (start < 0 || end >= csec.signalLength || start > end) {
        throw std::invalid_argument("[Error] Invalid start or end index.");
    }

    int newLength     = end - start + 1;
    csec.channels     = csec.channels.block(0, start, csec.channelNum, newLength);
    csec.signalLength = newLength;
}

inline std::vector<double> SignalBase::signalPower(const ChannelSignalVector &csv) {
    if (!csv.isInit) {
        throw std::runtime_error("[Error] ChannelSignalVector is not initialized.");
    }

    std::vector<double> power(csv.channelNum, 0.0);
    for (int i = 0; i < csv.channelNum; ++i) {
        for (int j = 0; j < csv.signalLength; ++j) {
            power[i] += csv.channels[i][j] * csv.channels[i][j];
        }
        power[i] /= csv.signalLength;
    }

    return power;
}

#endif // _SIGNALBASE_H_