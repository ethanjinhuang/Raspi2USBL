function [tof, doa] = processData(cfg, sampleSignal, isDisplay)
% ProcessData Processing Multi-Channel Signal from Raspi2USBL System
%
% Input Parameters：
%   cfg          - Config Parameter
%   sampleSignal - Multi Channel Signal Data（channelNum x N）
%   isDisplay    - Display Figure Status
% Output Parameters:
%   TOF          - Time of Flight
%   DOA          - Direction of Arrival
% ------------------------ Author Info ------------------------ 
% Author:   Jin Huang
% Email:    jin.huang@zju.edu.cn
% ------------------------ Config Demo ------------------------ 
% cfg.channelNum = 6;
% cfg.sampleRate = 40000;
% cfg.refSignalFileName = "SIG.txt";
% cfg.refSignalTime = 0.02;
% cfg.refSignalStartFreq = 10e3;
% cfg.refSignalEndFreq = 12e3;
% cfg.intervalDOA = 0.1;
% cfg.arrayDiameter = 0.13;
% cfg.soundSpeed = 1500;
% ------------------------------------------------------------- 


% Load Reference Signal
refSignal = importdata(cfg.refSignalFileName);

% Time Vector
[~, samplePerChannel] = size(sampleSignal);
fs = cfg.sampleRate;
t = (0:samplePerChannel-1)/fs;

% Matched Filter
corrSs = conv2(sampleSignal, fliplr(refSignal), 'valid');
[~, idx] = max(corrSs, [], 2);
corrIdx = min(idx);

tof = corrIdx / fs;
distance = tof * cfg.soundSpeed;

% DOA Estimation
doaLen = round(cfg.refSignalTime * fs);
k1 = round(cfg.refSignalStartFreq * doaLen / fs) + 1;
k2 = round(cfg.refSignalEndFreq * doaLen / fs) + 1;

% Array Geomatric
theta = -179.9:cfg.intervalDOA:180;
theta_rad = deg2rad(theta);
x = 0.5 * cfg.arrayDiameter * cos(2*pi*(0:cfg.channelNum-1)/cfg.channelNum);
y = 0.5 * cfg.arrayDiameter * sin(2*pi*(0:cfg.channelNum-1)/cfg.channelNum);

% FFT
fftData = zeros(cfg.channelNum, doaLen);
for j = 1:cfg.channelNum
    segment = sampleSignal(j, corrIdx:corrIdx+doaLen-1);
    fftResult = fft(segment);
    fftResult = fftResult / doaLen;
    fftResult(2:end-1) = fftResult(2:end-1) * 2;
    fftData(j,:) = fftResult;
end

f_k = (0:doaLen-1) * fs / doaLen;

% Beam Pattern
B = zeros(k2-k1+1, length(theta));
for fk = k1:k2
    d = x.' * cos(theta_rad) + y.' * sin(theta_rad);
    exp_term = exp(1j * 2 * pi * f_k(fk) * d / cfg.soundSpeed);
    Bn = fftData(:, fk)' * exp_term / cfg.channelNum;
    B(fk - k1 + 1, :) = abs(Bn).^2;
end

bp = sum(B, 1);
[~, doaidx] = max(bp);
doa = theta(doaidx);

% Display Result
if isDisplay
    figure('Position', [100, 100, 1400, 800]);
    tlo = tiledlayout(6, 3, 'TileSpacing', 'Compact', 'Padding', 'Compact');

    % Plot Multi-channel Signal
    for i = 1:cfg.channelNum
        nexttile(tlo, (i-1)*3+1);
        plot(t, sampleSignal(i,:));
        xlim([t(1), t(end)]);
        title(['Channel ', num2str(i)]);
        xlabel('Time (s)');
        ylabel('Amplitude');
        grid on;
    end

    % Plot Signal-side Amplitude Spectrum
    nexttile(tlo, 2,[3,1]);
    plot(f_k(1:doaLen/2)/1e3, abs(fftData(1,1:doaLen/2)));
    title('Single-side Amplitude Spectrum');
    xlabel('Frequency (kHz)');
    ylabel('Amplitude');
    grid on;

    % Plot Matched Filter
    nexttile(tlo, 11,[3,1]);
    plot(t, sampleSignal(1,:));
    hold on;
    plot((corrIdx:corrIdx+doaLen-1)/fs, sampleSignal(1,corrIdx:corrIdx+doaLen-1));
    title('Matched Filter Output');
    xlabel('Time (s)');
    ylabel('Amplitude');
    legend('Original', 'Matched Segment');
    grid on;

    % Plot DOA Polar Plot
    nexttile(tlo, 3,[3,1]);
    polarplot(theta_rad, bp/max(bp), 'LineWidth', 2);
    title(['DOA: ', num2str(doa), '°']);
    set(gca, 'ThetaDir', 'clockwise', 'ThetaZeroLocation', 'top');
    grid on;

    % Plot Position in Relative Coordinate
    nexttile(tlo, 12, [3,1]);
    x_pos = distance * sind(doa);  % Forward
    y_pos = distance * cosd(doa);  % Rightward
    scatter(x_pos, y_pos, 100, 'filled');
    title("Body Frame Tracking Plot");
    grid on;
    axis equal;

    xlim([-1, 1]*max(abs([x_pos,y_pos])*1.2));
    ylim([-1, 1]*max(abs([x_pos,y_pos]))*1.2);

    hold on;
    plot([min(xlim), max(xlim)], [0,0], 'k--'); % y axis
    plot([0,0], [min(ylim), max(ylim)], 'k--'); % x axis
    hold off;

    xlabel('Right (m)');
    ylabel('Forward (m)');


    % Output Info
    disp(['Distance: ', num2str(distance), ' m']);
    disp(['DOA (deg): ', num2str(doa), ' deg']);
end
end
