/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-04 17:29:43
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:57:10
 * @FilePath: /Raspi2USBL/general/typedef.h
 * @Description: define typedef for general use
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include <Eigen/Dense>
#include <complex>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <vector>

typedef struct ChannelSignalVector {
    bool                             isInit       = false;
    int                              channelNum   = 0;
    int                              signalLength = 0;
    std::vector<std::vector<double>> channels;

    // default constructor
    ChannelSignalVector() = default;

    // custom constructor
    ChannelSignalVector(int cn, int sl)
        : channelNum(cn)
        , signalLength(sl) {
        channels.resize(channelNum);
        for (int i = 0; i < channelNum; ++i) {
            channels[i].resize(signalLength);
            for (int j = 0; j < signalLength; ++j) {
                channels[i][j] = 0.0; // init with 0
            }
        }
        isInit = true;
    }
    ChannelSignalVector(std::vector<double> signal)
        : channelNum(1)
        , signalLength(signal.size()) {
        channels.resize(channelNum);
        channels[0] = signal;
        isInit      = true;
    }

    // resize function
    void resize(int cn, int sl) {
        channelNum   = cn;
        signalLength = sl;
        channels.resize(channelNum);
        for (int i = 0; i < channelNum; ++i) {
            channels[i].resize(signalLength);
            std::fill(channels[i].begin(), channels[i].end(), 0.0);
        }
        isInit = true;
    }

    // copy constructor
    ChannelSignalVector(const ChannelSignalVector &other)
        : isInit(other.isInit)
        , channelNum(other.channelNum)
        , signalLength(other.signalLength)
        , channels(other.channels) {
    }

    // move constructor
    ChannelSignalVector(ChannelSignalVector &&other) noexcept
        : isInit(std::move(other.isInit))
        , channelNum(std::move(other.channelNum))
        , signalLength(std::move(other.signalLength))
        , channels(std::move(other.channels)) {
    }

    // copy assignment operator
    ChannelSignalVector &operator=(const ChannelSignalVector &other) {
        if (this == &other) {
            return *this;
        }
        isInit       = other.isInit;
        channelNum   = other.channelNum;
        signalLength = other.signalLength;
        channels     = other.channels;
        return *this;
    }

    // move assignment operator
    ChannelSignalVector &operator=(ChannelSignalVector &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        isInit       = std::move(other.isInit);
        channelNum   = std::move(other.channelNum);
        signalLength = std::move(other.signalLength);
        channels     = std::move(other.channels);
        return *this;
    }

    // destructor
    ~ChannelSignalVector() = default;
} ChannelSignalVector;

typedef struct ChannelSignal {
    bool     isInit       = false;
    int      channelNum   = 0;
    int      signalLength = 0;
    double **channels     = nullptr;

    ChannelSignal(int cn, int sl)
        : channelNum(cn)
        , signalLength(sl) {
        channels = allocateArray(channelNum, signalLength);
        if (!channels) {
            throw std::runtime_error("Failed to allocate memory for ChannelSignal.");
        }
        isInit = true;
    }

    ~ChannelSignal() {
        freeArray(channels, channelNum);
    }

    // resize function
    void resize(int cn, int sl) {
        if (channels) {
            freeArray(channels, channelNum); // Free existing memory
        }
        channelNum   = cn;
        signalLength = sl;
        channels     = allocateArray(channelNum, signalLength);
        if (!channels) {
            throw std::runtime_error("Failed to allocate memory for ChannelSignal.");
        }
        isInit = true;
    }

    // forbidden copy constructor and assignment operator
    ChannelSignal(const ChannelSignal &)            = delete;
    ChannelSignal &operator=(const ChannelSignal &) = delete;

    // define move constructor and move assignment operator
    ChannelSignal(ChannelSignal &&other) noexcept
        : isInit(other.isInit)
        , channelNum(other.channelNum)
        , signalLength(other.signalLength)
        , channels(other.channels) {
        other.channels     = nullptr;
        other.channelNum   = 0;
        other.signalLength = 0;
        other.isInit       = false;
    }

    ChannelSignal &operator=(ChannelSignal &&other) noexcept {
        if (this != &other) {
            freeArray(channels, channelNum); // free current resource

            isInit             = other.isInit;
            channelNum         = other.channelNum;
            signalLength       = other.signalLength;
            channels           = other.channels;

            other.channels     = nullptr; // avoid double free
            other.channelNum   = 0;
            other.signalLength = 0;
            other.isInit       = false;
        }
        return *this;
    }

    ChannelSignal &operator*(double scalar) {
        for (int i = 0; i < channelNum; ++i) {
            for (int j = 0; j < signalLength; ++j) {
                channels[i][j] *= scalar;
            }
        }
        return *this;
    }

    ChannelSignal &operator/(double scalar) {
        if (scalar == 0) {
            throw std::runtime_error("Division by zero.");
        }
        for (int i = 0; i < channelNum; ++i) {
            for (int j = 0; j < signalLength; ++j) {
                channels[i][j] /= scalar;
            }
        }
        return *this;
    }

private:
    double **allocateArray(int rows, int cols) {
        double **array = (double **) malloc(rows * sizeof(double *));
        if (!array) {
            return nullptr;
        }
        for (int i = 0; i < rows; ++i) {
            array[i] = (double *) malloc(cols * sizeof(double));
            if (!array[i]) {
                // if allocation fails, free all previous allocations
                for (int j = 0; j < i; ++j) {
                    free(array[j]);
                }
                free(array);
                return nullptr;
            }
            for (int j = 0; j < cols; ++j) {
                array[i][j] = 0.0;
            }
        }
        return array;
    }

    void freeArray(double **array, int rows) {
        for (int i = 0; i < rows; ++i) {
            free(array[i]);
        }
        free(array);
    }

} ChannelSignal;

typedef struct ChannelSignalComplex {
    bool                   isInit       = false;
    int                    channelNum   = 0;
    int                    signalLength = 0;
    std::complex<double> **channels     = nullptr;

    ChannelSignalComplex(int cn, int sl)
        : channelNum(cn)
        , signalLength(sl) {
        channels = allocateComplexArray(channelNum, signalLength);
        if (!channels) {
            throw std::runtime_error("Failed to allocate memory for ChannelSignalComplex.");
        }
        isInit = true;
    }

    ~ChannelSignalComplex() {
        freeComplexArray(channels, channelNum);
    }

    // resize function
    void resize(int cn, int sl) {
        if (channels) {
            freeComplexArray(channels, channelNum); // Free existing memory
        }
        channelNum   = cn;
        signalLength = sl;
        channels     = allocateComplexArray(channelNum, signalLength);
        if (!channels) {
            throw std::runtime_error("Failed to allocate memory for ChannelSignalComplex.");
        }
        isInit = true;
    }

    // forbidden copy constructor and assignment operator
    ChannelSignalComplex(const ChannelSignalComplex &)            = delete;
    ChannelSignalComplex &operator=(const ChannelSignalComplex &) = delete;

    // move constructor
    ChannelSignalComplex(ChannelSignalComplex &&other) noexcept
        : isInit(other.isInit)
        , channelNum(other.channelNum)
        , signalLength(other.signalLength)
        , channels(other.channels) {
        other.channels     = nullptr;
        other.channelNum   = 0;
        other.signalLength = 0;
        other.isInit       = false;
    }

    // move assignment operator
    ChannelSignalComplex &operator=(ChannelSignalComplex &&other) noexcept {
        if (this != &other) {
            freeComplexArray(channels, channelNum); // free current resource

            isInit             = other.isInit;
            channelNum         = other.channelNum;
            signalLength       = other.signalLength;
            channels           = other.channels;

            other.channels     = nullptr;
            other.channelNum   = 0;
            other.signalLength = 0;
            other.isInit       = false;
        }
        return *this;
    }

    ChannelSignalComplex &operator*(double scalar) {
        for (int i = 0; i < channelNum; ++i) {
            for (int j = 0; j < signalLength; ++j) {
                channels[i][j] *= scalar;
            }
        }
        return *this;
    }

    ChannelSignalComplex &operator/(double scalar) {
        if (scalar == 0) {
            throw std::runtime_error("Division by zero.");
        }
        for (int i = 0; i < channelNum; ++i) {
            for (int j = 0; j < signalLength; ++j) {
                channels[i][j] /= scalar;
            }
        }
        return *this;
    }

private:
    std::complex<double> **allocateComplexArray(int rows, int cols) {
        std::complex<double> **array = (std::complex<double> **) malloc(rows * sizeof(std::complex<double> *));
        if (!array) {
            return nullptr;
        }
        for (int i = 0; i < rows; ++i) {
            array[i] = (std::complex<double> *) malloc(cols * sizeof(std::complex<double>));
            if (!array[i]) {
                // if allocation fails, free all previous allocations
                for (int j = 0; j < i; ++j) {
                    free(array[j]);
                }
                free(array);
                return nullptr;
            }
            for (int j = 0; j < cols; ++j) {
                array[i][j] = std::complex<float>(0.0, 0.0);
            }
        }
        return array;
    }

    void freeComplexArray(std::complex<double> **array, int rows) {
        if (array) {
            for (int i = 0; i < rows; ++i) {
                free(array[i]);
            }
            free(array);
        }
    }

} ChannelSignalComplex;

struct ChannelSignalEigenD {
    bool            isInit       = false;
    int             channelNum   = 0;
    int             signalLength = 0;
    Eigen::MatrixXd channels;

    // default constructor
    ChannelSignalEigenD() = default;

    // custom constructor
    ChannelSignalEigenD(int cn, int sl)
        : channelNum(cn)
        , signalLength(sl)
        , channels(cn, sl) {
        channels.setZero();
        isInit = true;
    }

    // resize function
    void resize(int cn, int sl) {
        channelNum   = cn;
        signalLength = sl;
        channels.resize(channelNum, signalLength);
        channels.setZero(); // Initialize with zeros
        isInit = true;
    }

    // function to convert Eigen::MatrixXd to double**
    double **toDoubleArray() const {
        double **array = (double **) malloc(channelNum * sizeof(double *));
        if (!array) {
            throw std::runtime_error("Failed to allocate memory for double array.");
        }
        for (int i = 0; i < channelNum; ++i) {
            array[i] = (double *) malloc(signalLength * sizeof(double));
            if (!array[i]) {
                // 分配失败，释放之前的内存
                // Allocation failed, free previous memory
                for (int j = 0; j < i; ++j) {
                    free(array[j]);
                }
                free(array);
                throw std::runtime_error("Failed to allocate memory for double array.");
            }
            for (int j = 0; j < signalLength; ++j) {
                array[i][j] = channels(i, j);
            }
        }
        return array;
    }

    // function to convert double** to Eigen::MatrixXd
    void fromDoubleArray(double **array) {
        if (!array) {
            throw std::invalid_argument("Input array is null.");
        }
        channels.resize(channelNum, signalLength);
        for (int i = 0; i < channelNum; ++i) {
            for (int j = 0; j < signalLength; ++j) {
                channels(i, j) = array[i][j];
            }
        }
    }
};

// manage the signal with std::complex<double> type
struct ChannelSignalEigenC {
    bool             isInit       = false;
    int              channelNum   = 0;
    int              signalLength = 0;
    Eigen::MatrixXcd channels;

    // default constructor
    ChannelSignalEigenC() = default;

    // custom constructor
    ChannelSignalEigenC(int cn, int sl)
        : channelNum(cn)
        , signalLength(sl)
        , channels(cn, sl) {
        channels.setZero();
        isInit = true;
    }

    // resize function
    void resize(int cn, int sl) {
        channelNum   = cn;
        signalLength = sl;
        channels.resize(channelNum, signalLength);
        channels.setZero(); // Initialize with zeros
        isInit = true;
    }

    // function to convert Eigen::MatrixXcd to std::complex<double>**
    std::complex<double> **toComplexArray() const {
        std::complex<double> **array = (std::complex<double> **) malloc(channelNum * sizeof(std::complex<double> *));
        if (!array) {
            throw std::runtime_error("Failed to allocate memory for complex array.");
        }
        for (int i = 0; i < channelNum; ++i) {
            array[i] = (std::complex<double> *) malloc(signalLength * sizeof(std::complex<double>));
            if (!array[i]) {
                // 分配失败，释放之前的内存
                // free all previous allocations
                for (int j = 0; j < i; ++j) {
                    free(array[j]);
                }
                free(array);
                throw std::runtime_error("Failed to allocate memory for complex array.");
            }
            for (int j = 0; j < signalLength; ++j) {
                array[i][j] = channels(i, j);
            }
        }
        return array;
    }

    // function to convert std::complex<double>** to Eigen::MatrixXcd
    void fromComplexArray(std::complex<double> **array) {
        if (!array) {
            throw std::invalid_argument("Input array is null.");
        }
        channels.resize(channelNum, signalLength);
        for (int i = 0; i < channelNum; ++i) {
            for (int j = 0; j < signalLength; ++j) {
                channels(i, j) = array[i][j];
            }
        }
    }
};

typedef struct PositionResult {
    double          time;
    Eigen::Vector3d position;
    double          doa;
    double          tof;
} positionResult;

// function to free double** array
inline void freeDoubleArray(double **array, int rows) {
    if (!array)
        return;
    for (int i = 0; i < rows; ++i) {
        free(array[i]);
    }
    free(array);
}

// function to free std::complex<double>** array
inline void freeComplexArray(std::complex<double> **array, int rows) {
    if (!array)
        return;
    for (int i = 0; i < rows; ++i) {
        free(array[i]);
    }
    free(array);
}

#endif // _TYPEDEF_H_
