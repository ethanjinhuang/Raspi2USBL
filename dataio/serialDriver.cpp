/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-06-11 17:16:49
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:37:52
 * @FilePath: /Raspi2USBL/dataio/serialDriver.cpp
 * @Description: See serialDriver.h
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#include "serialDriver.h"

SerialDriver::SerialDriver(const std::string &serial_port_name, std::string &baudrate) {
    open(serial_port_name, baudrate);
}

void SerialDriver::open(const std::string &serial_port_name, const std::string &baudrate) {

    file_handle_ = ::open(serial_port_name.data(), O_RDWR | O_NOCTTY | O_NDELAY);
    fcntl(file_handle_, F_SETFL, 0); // Open in blocking mode
    // Check Serial Open Seccessful
    if (file_handle_ < 0) {
        throw std::runtime_error{std::string{strerror(errno)} + ": " + serial_port_name};
    }

    // Check Serial Port is TTY Device
    if (!isatty(file_handle_)) {
        throw std::runtime_error{"Serial port is not TTY device"};
    }

    // Get Current Serial Config
    if (tcgetattr(file_handle_, &config_) < 0) {
        throw std::runtime_error{"Could not retrieve current serial config"};
    }

    //
    // Input flags - Turn off input processing
    //
    // convert break to null byte, no CR to NL translation,
    // no NL to CR translation, don't mark parity errors or breaks
    // no input parity check, don't strip high bit off,
    // no XON/XOFF software flow control
    //
    config_.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);

    //
    // Output flags - Turn off output processing
    //
    // no CR to NL translation, no NL to CR-NL translation,
    // no NL to CR translation, no column 0 CR suppression,
    // no Ctrl-D suppression, no fill characters, no case mapping,
    // no local output processing
    //
    // config_.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
    //                     ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
    config_.c_oflag = 0;

    //
    // No line processing
    //
    // echo off, echo newline off, canonical mode off,
    // extended input processing off, signal chars off
    //
    config_.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    //
    // Turn off character processing
    //
    // clear current char size mask, no parity checking,
    // no output processing, force 8 bit input
    //
    config_.c_cflag &= ~(CSIZE | PARENB);
    config_.c_cflag |= CS8;

    //
    // Zero input byte is enough to return from read()
    // Inter-character timer off
    // I.e. no blocking: return immediately with what is available.
    //
    config_.c_cc[VMIN]  = 0;
    config_.c_cc[VTIME] = 0;
    //
    // Communication speed (simple version, using the predefined
    // constants)
    //

    bool baudrate_set = false;

    if (baudrate == "9600") {
        if (cfsetispeed(&config_, B9600) < 0 || cfsetospeed(&config_, B9600) < 0) {
            throw std::runtime_error{"Could not set desired baud rate"};
        }
        baudrate_set = true;
    } else if (baudrate == "19200") {
        if (cfsetispeed(&config_, B19200) < 0 || cfsetospeed(&config_, B19200) < 0) {
            throw std::runtime_error{"Could not set desired baud rate"};
        }
        baudrate_set = true;
    } else if (baudrate == "115200") {
        if (cfsetispeed(&config_, B115200) < 0 || cfsetospeed(&config_, B115200) < 0) {
            throw std::runtime_error{"Could not set desired baud rate"};
        }
        baudrate_set = true;
    } else if (baudrate == "377400") {
        if (cfsetispeed(&config_, 377400) < 0 || cfsetospeed(&config_, 377400) < 0) {
            throw std::runtime_error{"Could not set desired baud rate"};
        }
        baudrate_set = true;
    } else if (baudrate == "460800") {
        if (cfsetispeed(&config_, B460800) < 0 || cfsetospeed(&config_, B460800) < 0) {
            throw std::runtime_error{"Could not set desired baud rate"};
        }
        baudrate_set = true;
    } else if (baudrate == "921600") {
        if (cfsetispeed(&config_, B921600) < 0 || cfsetospeed(&config_, B921600) < 0) {
            throw std::runtime_error{"Could not set desired baud rate"};
        }
        baudrate_set = true;
    } else if (baudrate == "1843200") {
        if (cfsetispeed(&config_, 1843200) < 0 || cfsetospeed(&config_, 1843200) < 0) {
            throw std::runtime_error{"Could not set desired baud rate"};
        }
        baudrate_set = true;
    }
    // Check Baudrate Set Successful
    if (!baudrate_set) {
        throw std::runtime_error{"Desired baud rate is not supported"};
    }

    // Apply configuration
    if (tcsetattr(file_handle_, TCSAFLUSH, &config_) < 0) {
        throw std::runtime_error{"Could not apply serial port configuration"};
    }
}

bool SerialDriver::isOpen() const {
    return file_handle_ >= 0;
}

bool SerialDriver::readByte(uint8_t &byte) {
    int bytes_read = ::read(file_handle_, &byte, 1);
    return bytes_read == 1;
}

bool SerialDriver::readHexByte(std::string &hexString) {
    uint8_t byte;
    if (readByte(byte)) { // use the existing readByte function to read a byte
        std::stringstream ss;
        // convert to two-digit HEX format
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        hexString = ss.str(); // store the result in the passed string
        return true;
    }
    return false; // read failed
}

int SerialDriver::readBytes(uint8_t *buffer, const unsigned int packetsize) {
    int bytes_read = ::read(file_handle_, buffer, packetsize);
    if (bytes_read < 0) {
        throw std::runtime_error{"ReadPacket error: " + std::string{strerror(errno)}};
    }
    return bytes_read;
}

int SerialDriver::readHexBytes(std::string &hexString, const unsigned int packetsize) {
    uint8_t *buffer     = new uint8_t[packetsize];
    int      bytes_read = readBytes(buffer, packetsize); // use the existing readBytes function to read multiple bytes

    if (bytes_read > 0) {
        std::stringstream ss;
        // convert each byte to two-digit HEX format and append to the string
        for (int i = 0; i < bytes_read; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
        }
        hexString = ss.str(); // store the result in the passed string
    }

    delete[] buffer;   // free dynamically allocated memory
    return bytes_read; // return the number of bytes read
}

std::string SerialDriver::readString() {
    std::string data;
    char        buffer[256];
    ssize_t     bytes_read;
    bool        crReceived = false;

    while (!crReceived) {
        bytes_read = ::read(file_handle_, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            for (int i = 0; i < bytes_read; i++) {
                if (buffer[i] == '\r') {
                    crReceived = true;
                    break;
                } else {
                    data += buffer[i];
                }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    return data;
}

bool SerialDriver::writeByte(uint8_t byte) {
    const int maxRetries = 5;
    int       retries    = 0;
    while (retries < maxRetries) {
        int bytes_written = ::write(file_handle_, &byte, 1);
        if (bytes_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                // These errors might be temporary, so retry
                std::cerr << "WriteByte error: " << strerror(errno) << ". Retrying... (" << retries + 1 << "/"
                          << maxRetries << ")" << std::endl;
                retries++;
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait before retrying
            } else {
                // Other errors are likely permanent, so throw an exception
                throw std::runtime_error{"WriteByte error: " + std::string{strerror(errno)}};
            }
        } else {
            return bytes_written == 1; // Write successful
        }
    }
    // If we exhaust retries, throw an error
    throw std::runtime_error{"WriteByte error: Resource temporarily unavailable after multiple retries"};
}

bool SerialDriver::writeHexString(const std::string &hex_string) {
    // Check if the string length is even (since each two characters represent one byte)
    if (hex_string.length() % 2 != 0) {
        throw std::invalid_argument("Invalid HEX string length. Must be an even number of characters.");
        return false;
    }

    // Iterate over the HEX string, converting each two characters to one byte
    for (size_t i = 0; i < hex_string.length(); i += 2) {
        std::string byte_string = hex_string.substr(i, 2); // Get two characters

        // Convert the two-character hexadecimal string to a byte
        uint8_t byte = static_cast<uint8_t>(std::stoi(byte_string, nullptr, 16));

        // Call the existing writeByte function to send the byte
        if (!writeByte(byte)) {
            return false;
        }
    }
    return true;
}

bool SerialDriver::flush() {
    return tcflush(file_handle_, TCIOFLUSH) == 0;
}

void SerialDriver::close() {
    ::close(file_handle_);
}