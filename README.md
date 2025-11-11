<!--
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-08-15 17:02:28
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-11 11:38:49
 * @FilePath: /Raspi2USBL/README.md
 * @Description: 
 * 
 * Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved. 
-->

![Raspi$^2$USBL](etc/projectName.png)
# Raspi<sup>2</sup>USBL
## Raspberry Pi-based passive-inverted Ultar-Short Baseline positioning system


Raspi<sup>2</sup>USBL is an open-source, low-cost, modular underwater acoustic positioning system designed for marine robotics research.
It is based on the Raspberry Pi platform and provides a flexible, reproducible solution for underwater localization and navigation.

# 01 Introduction

Raspi<sup>2</sup>USBL integrates both hardware and software components to enable real-time acoustic signal acquisition, digital signal processing, and adaptive gain control.
The software is developed in C++, built using CMake, and interfaces with Measurement Computing DAQ/DAC devices via the MCC Universal Library for Linux (uldaq).
To simplify cross-compilation for Raspberry Pi, a Docker environment is provided.

Key Features:

- Raspberry Pi–based architecture for real-time underwater acoustic positioning, including signal acquisition, DSP, and adaptive gain control, etc.
- A unified software framework is developed in C++ using CMake for easy building and deployment, both the receiver and beacon side can be driven by the same software framework without compiling twice.
- A YAML-based configuration system for easy customization, allowing users to modify system parameters, such as sampling rate, duration, reference signal characteristics, and processing algorithms, without changing the source code.


### Related References
- Huang, Jin, et al. "Precise Time Delay Measurement and Compensation for Tightly Coupled Underwater SINS/piUSBL Navigation." arXiv preprint arXiv:2510.23286 (2025).
- Wang, Yingqiang, et al. "Passive inverted ultra-short baseline positioning for a disc-shaped autonomous underwater vehicle: Design and field experiments." IEEE Robotics and Automation Letters 7.3 (2022): 6942-6949.
- Wang, Yingqiang, et al. "Design and experimental results of passive iUSBL for small AUV navigation." Ocean Engineering 248 (2022): 110812.
- Rypkema, Nicholas Rahardiyan. Underwater & out of sight: Towards ubiquity in underwater robotics. Diss. Massachusetts Institute of Technology, 2019.
- Rypkema, Nicholas R., Henrik Schmidt, and Erin M. Fischell. "Synchronous-clock range-angle relative acoustic navigation: A unified approach to multi-AUV localization, command, control, and coordination." Field Robotics 2 (2022): 774-806.

### If you use this software or datasets for your academic research, please give acknowledgment as follows and cite our related references.

#### Acknowledgment

> The authors would like to acknowledge Mr. Jin Huang and Dr. Yingqiang Wang of Prof. Chen’s group at Zhejiang University for providing the open-source Raspi$^2$USBL software and datasets used in this study.

#### References

```bibtex
@misc{huangRaspi$^2$USBLOpensourceRaspberry2025,
  title = {Raspi\$\textasciicircum 2\$USBL: An Open-Source Raspberry Pi-Based Passive Inverted Ultra-Short Baseline Positioning System for Underwater Robotics},
  author = {Huang, Jin and Wang, Yingqiang and Chen, Ying},
  year = 2025,
  number = {arXiv:2511.06998},
  eprint = {2511.06998},
  primaryclass = {cs},
  publisher = {arXiv},
  doi = {10.48550/arXiv.2511.06998},
  urldate = {2025-11-11},
  archiveprefix = {arXiv}
}
```




# 02 How to use

## Config basic environment

Install CMake

```cmd
sudo apt-get install cmake
```


## How to config uldaq

MCC Universal Library for Linux (uldaq) is a library to access and control supported Measurement Computing DAQ devices over Linux and macOS platforms. 
The github source code can be download in [LINK](https://github.com/mccdaq/uldaq).

The config process can be shown below: (Specify for Ubuntu in RasPi 5)

### Prerequisites:

```cmd
sudo apt-get install gcc g++ make
sudo apt-get install libusb-1.0-0-dev
```
### Build Instructions

1. Download the latest version of uldaq:

```cmd
wget -N https://github.com/mccdaq/uldaq/releases/download/v1.2.1/libuldaq-1.2.1.tar.bz2
```

2. Extract the tar file:

```cmd
tar -xvjf libuldaq-1.2.1.tar.bz2
```

3. Run the following commands to build and install the library:

```cmd
cd libuldaq-1.2.1
./configure && make
sudo make install
```

## fftw

There are two methods to install fftw, one is using cmake, and the other is using the system installation. In this project, we default to use the cmake method, which means that there is no need to install fftw in the system. But if you want to use the system installation, you can follow the steps below.

Download `http://www.fftw.org/download.html`

unzip the file and enter the directory

```cmd
tar -xvjf fftw-3.3.10.tar.gz
cd fftw-3.3.10
```
**CMAKE method (recommend)**
```cmd
wget ftp://ftp.fftw.org/pub/fftw/fftw-3.3.10.tar.gz
tar -xvf fftw-3.3.10.tar.gz
rm -rf fftw-3.3.10.tar.gz
```

**System Installation (Unrecommended)**
FFTW comes with a configure program in the GNU style. Installation can be as simple as:

```cmd
./configure
make
sudo make install
sudo ldconfig
```


## AGC (Adaptive Gain Control)

In this project, the adaptive gain control (AGC) is implemented by a 10-bit DAC module to control the gain of the amplifier.
The DAC module is connected to the LNA (Low Noise Amplifier) to adjust the gain, where is calculated by the software based on the received signal strength.
The DAC module is TLC5615, which is controlled by CH340 USB to serial module or TTL.
The precision of the DAC module is 10-bit, which means the output voltage can be controlled from 0 to 5V with a resolution of 5/1024=4.88mV.
The serial configuration is 19200 baud rate, 8 data bits, 1 stop bit, and no parity bit.
The sending and receiving data is using HEX format.

The command format is as follows:

```cmd
5A 01 03 00 A5
```

`03 00` mean that set the DAC output as 3V, the `5A A5` is the head and tail of the command, and `01` is the DAC channel.
The value of the DAC output voltage is between 0 and 5V.

If the DAC module receives the command, it will return the same command to the sender. And if the voltage value exceeds the range (5V), the DAC module will return `FF FF FF FF FF` to the sender.

# 03 Datasets

In order to facilitate the testing and validation of the Raspi<sup>2</sup>USBL system, we provide some sample datasets that can be used for development and experimentation.

Sample datasets are provided for testing and validation under `data/demodata/`, including anechoic tank and lake experiments.
Configuration files are located in `.config/config_pi0.yaml` (receiver) and `.config/config_pi1.yaml` (beacon).


The details of the datasets are as followed.

## Anechoic Tank Experiment Data

The data was collected in an anechoic tank. 
The hydrophone array and the beacon were both deployed in the same depth, and then the array's direction was controlled by the overhead traveling crane.
The direction of the array was changed from 0 degrees to 360 degrees with a step of 10 degrees.
The MATLAB script for processing the anechoic tank data is provided in the `main.m`.

## Lake Experiment Data
The data was collected in a lake environment.
The beacon was deployed at a fixed position, and the hydrophone array was mounted on an experimental setup that can move around the lake.
More experimental details can be found in our related references.
The MATLAB script for processing the lake experiment data is provided in the `main.m`.

# 04 How to use docker for cross-compilation


## create a docker image for cross-compilation

Build the docker image

```bash
docker build -t raspi2usbl-cross-compile . 
```

Run the docker container

```bash
docker run -dit \
  --name raspi2usbl-cross-compile \
  -p 2404:22 \
  -v $(pwd):/home/dev/app \
  raspi2usbl-cross-compile
```

Open the container shell

```bash
docker exec -it raspi2usbl-cross-compile bash
```

Exit the container shell

```bash
exit
```


## Close the container

```bash
docker stop raspi2usbl-cross-compile
```

Force to close the container

```bash
docker kill raspi2usbl-cross-compile
```

## Remove the container

```bash
docker rm raspi2usbl-cross-compile
```

## Reopen the container

```bash
docker start raspi2usbl-cross-compile
docker exec -it raspi2usbl-cross-compile
```




