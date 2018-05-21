%% test_frequency_smearing.m
% 
% This script tests the frequency smearing algorithm for one single frame.
% To keep computation time low enough, the signal's length should not
% exceed ~10000 samples.
% The second part test computing time for different frame length.
%
% Author:	Vassili Cruchet, vassili.cruchet@gmail.com
% 
% Technical University of Denmark (DTU)
%
% Date:		Spring 2018
%

%% calculations
clc
close all
clear all

saveFile = 0;			% boolean to save plots (1=save)
signalName = 'sine_1k2k4k';

% create input time signal
fs = 16000;				% sampling frequency
T = 0.5;				% signal duration
f = 1000;				% signal fundammental frequency
t = [0:1/fs:T-1/fs];	% time vector
x = (sin(2*pi*f*t))' +  0.75*(sin(2*pi*2*f*t))' ...
	+  0.5*(sin(2*pi*4*f*t))';
l = length(x);
b=3;					% broadening factor (b>=1)

% compute spectrum
X=(fft(x));

% calculate smearing matrix and output power spectrum
A_s = calc_smear_matrix(fs, l, b);
Y = smearing(abs(X), A_s);

% compute output time-domain signal, using original phase
y = real(ifft(Y.*exp(i*unwrap(angle(X)))));

%% plots
clc
close all
saveFile = 0;
% time domain
figure('Units','normalized','Position', [0.1 0.4 0.35 0.35]);
plot(t,x/max(x),t,y/max(y));	hold on; grid on;
xlabel('Time [s]');		ylabel('Amplitude');
legend('normal',['smeared, b = ' num2str(b)]);
xlim([0.005 0.01]);		ylim([-1.1 1.1]);
% save plot
if saveFile
	print(['outputs' filesep 'smear_time_' signalName '.eps'], '-depsc');
end

% spectrum
[spec fVec]			= make_spectrum(x,fs);
[spec_smear fVec]	= make_spectrum(y,fs);

figure('Units','normalized','Position', [0.5 0.4 0.35 0.35]);
plot(fVec,db(spec),'Linewidth',1);		hold on; grid on;
plot(fVec,db(spec_smear),'Linewidth',1);
xlim([0 fs/2]);				ylim([-200 0]);
xlabel('Frequency [Hz]');	ylabel('Magnitude [dB]');
legend('normal',['smeared, b = ' num2str(b)]);

% save spectrum plot and normalized audio file
if saveFile
	print(['outputs' filesep 'smear_spec_' signalName '.eps'], '-depsc');
% 	audiowrite(['outputs' filesep 'smear_' signalName '.wav'], y/max(abs(y)), fs);
end	


%% computing complexity test
% Runs the algorithm for different signal length to study computation time.
% Execution can be long.

clc
clear all
fs = [50:20:10000];
T = 1;
f = 10;


for i=1:length(fs)
	fs_i = fs(i);
	t = [0:1/fs_i:T-1/fs_i];
	x = sin(2*pi*f*t)';
	l = length(x);
	X=(fft(x.*hann(l)));
	b=1;
	
	% calculate smearing matrix
	A_s = calc_smear_matrix(fs_i, l, b);
	tic
	Y = smearing(abs(X),A_s);
	T_elapsed(i)=toc;
end
%% plot computation time
close all
clc
t_av = mean(T_elapsed)
plot(fs,fs.^2/4.2e6,'--r','LineWidth',1);	 
hold on;
plot(fs,T_elapsed*1000); grid on; 
ylim([0 1])
xlabel('frame length (#samples)');
ylabel('smearing computing time (ms)');
legend('(#samples)^2');

