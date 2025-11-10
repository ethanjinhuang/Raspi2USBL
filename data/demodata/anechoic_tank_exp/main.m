%% Process Anechoic Tank Experiment Data
% date:     01/11/2025
% author:   Jin Huang
% email:    jin.huang@zju.edu.cn
% annotiation:
% In the experiment, the zero direction of the Raspi2USBL array
% was not perfectly aligned with that of the crane system, 
% resulting in a certain angular deviation.
clear; close all; clc;

%% Experiemnt Config
leverLength_Tx = 0.67;
leverLength_Rx = 0.69;
craneDepth_Tx = 3;
craneDepth_Rx = 3;
crane_d = 3.76;
soundspeed = 1465.8;    % in 15 celcius

%% Load POS data
rawdata = importdata("rawdata.mat");

%% Config Parameter
cfg.channelNum = 6;
cfg.sampleRate = 40000;
cfg.refSignalFileName = "REFSIGNAL.txt";
cfg.refSignalTime = 0.02;
cfg.refSignalStartFreq = 10e3;
cfg.refSignalEndFreq = 12e3;
cfg.intervalDOA = 0.1;
cfg.arrayDiameter = 0.12;
cfg.soundSpeed = 1465.8;

%% Process

expectedAngle = 0:10:360;
datalength = length(expectedAngle);

doa = zeros(datalength,1);
tof = doa;

for ii = 1:datalength
    [tof(ii),doa(ii)] = processData(cfg, rawdata{ii},false);
end

%% display

plot(doa);
