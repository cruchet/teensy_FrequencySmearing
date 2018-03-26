
%% frequency_smearing.m
%
% This script loads an audio file and apply frequency smearing 
% to its spectrum. Input signal is processed frame by frame and 
% reconstructed unsing overlap-and-add method.
%
% Author:	Vassili Cruchet, vassili.cruchet@gmail.com
% 
% Technical University of Denmark (DTU)
%
% Date:		Spring 2018
%

clear all
close all
clc
%% load or create audio signal
clc
saveFile = 0;	% boolean to save files
signalName = 'sine1k2k4k';	% name to save

% uncomment to use a external audio file
	filename = ['sounds' filesep 'cocktailparty.wav'];
	[signal fs] = audioread(filename);
	% add some conversation shaped noise
	% [noise fsNoise] = audioread(['sounds' filesep '07_icra_7.wav'], [1 length(signal)]);
	%signal = signal/max(abs(signal)) + 0.5*noise(:,1)/max(abs(noise(:,1)));

% % uncomment to synthesize input signal
% 	fs = 16000;				% sampling frequency
% 	T = 5;					% signal duration
% 	f = 1000;				% signal fundammental frequency
% 	t = [0:1/fs:T-1/fs];	% time vector
% 	signal = (sin(2*pi*f*t))' +  0.75*(sin(2*pi*2*f*t))' ...
% 	 	+  0.5*(sin(2*pi*4*f*t))';
% 	% add some noise
% 	signal = signal + 0.05*randn(length(signal),1);

% block processing parameters
l_win = 2048;	% frame length (should be a power of 2, smaller than 10000
overlap = 0.5;	% percentage of overlap
l_sig	= length(signal);
b = 3;			% broadening factor

% truncate signal to a multilple of frame length multiplied by overlap
n_win = floor(l_sig/(l_win*overlap));		% number of frames 
signal = signal(1:n_win*l_win*overlap);
l_sig = length(signal);
T = l_sig/fs;								% new duration
t = 0:1/fs:T-1/fs;

%% process with frames of length l_win
close all

% calculate smearing matrix
A_s = calc_smear_matrix(fs, l_win, b);

signal_smear = zeros(1,l_sig);

% process frame by frame
for k=0:n_win-1/overlap
	% set window
	win = sqrt(hann(l_win,'periodic'));
	% cut frame and window it
	idx_l = k*l_win*overlap+1;
	idx_h = (k+1/overlap)*l_win*overlap;
	sig   = signal(idx_l:idx_h);
	
	% calculate spectrum and separate power and phase
	spec		= fft(sig);
	spec_pow	= abs(spec);
	spec_phase	= angle(spec);
	
	% process smearing
	Y_pow = smearing(spec_pow, A_s);
	Y = Y_pow.* exp(i*spec_phase);

	% back to time domain and concatenate
	y = ifft(Y);
	% suppress residual imaginary part
	if imag(y)>1e-6
		disp('y(t) is complex!')
	else
		y=real(y);
	end
	
	% window the signal process overlap-and-add
	y = y.*win;
 	signal_smear(idx_l:idx_h) = signal_smear(idx_l:idx_h) + y';
	
% uncomment to plot each framea and its spectrum for inspection
% 	plot([idx_l:1:idx_h],y); hold on; grid on;
%	[S fvec]=make_spectrum(signal_smear,fs);
%	semilogx(fvec, db(S)); hold on;
end	
% plot(signal_smear,'k--','LineWidth',1);	% total signal

%% plot results
close all
% time domain
figure('Position',[50 150 500 400])
plot(signal_smear,'r-','LineWidth',1);		grid on; hold on;
plot(signal,'b--','LineWidth',1);
legend(['smeared (b=' num2str(b) ')'], 'normal');
xlabel('#samples');
ylabel('amplitude');

if saveFile
	print(['outputs' filesep 'block_proc_time_' signalName '.pdf'], '-dpdf');
end	

% spectrum
[spec fVec]			= make_spectrum(signal(1:l_sig),fs);
[spec_smear fVec]	= make_spectrum(signal_smear(1:l_sig)',fs);

figure('Position',[550 150 500 400])
% uncomment to use logarithmic scale
%	semilogx(fVec,db(spec_smear),'r'); hold on; grid on;
%	semilogx(fVec,db(spec),'b','LineWidth',0.5);		
% uncomment to use linear scale
	plot(fVec, db(spec_smear),'r-'); hold on; grid on;
	plot(fVec, db(spec),'b-'); 
	
xlim([0 fs/2]);				ylim([-80 0]);
xlabel('Frequency [Hz]');	ylabel('Magnitude [dB]');
legend(['smeared (b=' num2str(b) ')'], 'normal');

if saveFile
	print(['outputs' filesep 'block_proc_spec' signaName '.pdf'], '-dpdf');
	audiowrite(['outputs' filesep 'block_proc_' signaName '.wav'], ...
		signal_smear/max(abs(signal_smear)), fs);
end	
