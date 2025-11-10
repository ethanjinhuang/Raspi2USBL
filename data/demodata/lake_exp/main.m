%% Process Field Experiment Data
% date:     01/11/2025
% author:   Jin Huang
% email:    jin.huang@zju.edu.cn
% annotiation:
clear; close all; clc;
%% Import data

heading = importdata("heading.mat");
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
cfg.soundSpeed = 1500;

%% Process signal

datalength = length(rawdata);
doa = zeros(datalength,1);
tof = doa;

for ii = 1:1:datalength
    [tof(ii),doa(ii)] = processData(cfg, rawdata{ii},false);
end

%% relative coordinate

theta = wrapTo360(heading + doa);
dist = tof * cfg.soundSpeed;

polarplot(deg2rad(theta), dist);

