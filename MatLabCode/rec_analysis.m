%% test_frequency_smearing.m
% 
% This script analyses recordings from the TEENSY.
%
% Author:	Vassili Cruchet, vassili.cruchet@gmail.com
% 
% Technical University of Denmark (DTU)
%
% Date:		Spring 2018
%

%% load audio recordings
close all
clear all
clc

% filename_clean = ['final_recordings' filesep 'Teensy_recordings_cocktail_clean.wav'];
% filename_smear = ['final_recordings' filesep 'Teensy_recordings_cocktail_smear.wav'];
filename_clean = ['final_recordings' filesep 'Teensy_recordings_sine_clean.wav'];
filename_smear = ['final_recordings' filesep 'Teensy_recordings_sine_smear.wav'];

[sig_clean Fs] = audioread(filename_clean);
%sig_clean = sig_clean(:,1);						% take mono
% % low-pass filter
% fc = 7000;	
% wl = [0 fc/(Fs/2)];		wh = [fc/(Fs/2) 1];		Gp = [0 -100];
% [IR IR_win] = FIR_eq(wl, wh, Gp, 101, 0.7);
% sig_clean = filter(IR_win,1,sig_clean);
% % resample
% fs = 16e3;									% wanted sampling frequency
% [P,Q] = rat(fs/Fs);
% sig_clean = resample(sig_clean,P,Q);	clear Fs;	% resample audio signal


[sig_smear Fs] = audioread(filename_smear);
% sig_smear = sig_smear(:,1);
% % low-pass filter
% sig_smear = filter(IR_win,1,sig_smear);
% % resample
% sig_smear = resample(sig_smear,P,Q);	clear Fs;	% resample audio signal

sig_clean = sig_clean(1:min(length(sig_smear),length(sig_clean)));
sig_smear = sig_smear(1:min(length(sig_smear),length(sig_clean)));

fs = Fs;
b = 6;		% smearing coefficient used
t = [0:1/fs:(length(sig_clean)-1)/fs];

%% plots
clc
close all
saveFile = 1;
fontsize = 14;
% time domain
figure('Units','normalized','Position', [0.1 0.1 0.35 0.35]);
plot(t(5000:5512),sig_clean(5000:5512)/max(sig_clean),t(5000:5512),sig_smear(5000:5512)/max(sig_smear));	hold on; grid on;
xlabel('Time [s]');		ylabel('Amplitude');
legend('normal',['smeared, b = ' num2str(b)]);
% save plot
% if saveFile
% 	print(['smear_time_' signalName '.eps'], '-depsc');
% end

% spectrum
l=length(sig_clean)/8;
idx=1;
[spec_clean fVec_C]	= make_spectrum(sig_clean(idx:idx+l-1).*hann(l),fs);
[spec_smear fVec_S]	= make_spectrum(sig_smear(idx:idx+l-1).*hann(1),fs);

figure('Units','normalized','Position', [0.1 0.1 0.75 0.8]);
plot(fVec_S,db(spec_smear),'r','Linewidth',1);		grid on; hold on;
plot(fVec_C,db(spec_clean),'b','Linewidth',1);		hold on; grid on;
set(gca,'Fontsize',fontsize-2);
xlabel('Frequency [Hz]','Fontsize',fontsize);	ylabel('Magnitude [dB]','Fontsize',fontsize);
xlim([0 16e3/2]);	ylim([-100 -15]);
l=legend(['smeared, b = ' num2str(b)],'normal','Location','best');
set(l,'Fontsize',fontsize);
% save spectrum plot and normalized audio file
if saveFile
	print(['final_recordings' filesep 'Teensy_recordings_sine.png'], '-dpng');
% 	audiowrite(['outputs' filesep 'smear_' signalName '.wav'], y/max(abs(y)), fs);
end	