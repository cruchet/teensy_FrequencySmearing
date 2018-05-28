%% test_algo outputs
%
% This script loads audio signals written in a text file created by the 
% sketch test_algo.ino. Then specrtum analyses is done.
%
% Author:	Vassili Cruchet, vassili.cruchet@gmail.com
% 
% Technical University of Denmark (DTU)
%
% Date:		Spring 2018
%

clc
clear all
close all

savefile = 1;
fontsize = 24;
output = importdata(['test_algo_outputs' filesep 'fft_ifft_comp.txt'], ' ');
xVec = output(1,:);
yVec = output(2,:);

fs = 16e3;
[xSpec fVec] = make_spectrum(xVec,fs);
[ySpec fVec] = make_spectrum(yVec,fs);

% plots %
subplot(2,1,1)
plot(xVec(:),'.-');
% xlim([3000,3040]);
subplot(2,1,2);
plot(yVec(:),'.-');
% xlim(	[3000,3040]);

figure('Units','normalized','Position',[0.1 0.1 0.7 0.8]);
plot(fVec, db(xSpec),'b'); hold on;
plot(fVec, db(ySpec),'r');
xlim([0 fs/2]);	ylim([-150 0]);
xlabel('Frequency [Hz]');	ylabel('Magnitude [dB]');
% title('embeded process of sine (b=6, N=256)');
l=legend('original','smeared (b = 6)');
set(l,'Fontsize',fontsize);
set(gca,'Fontsize',fontsize);
set(findobj('Type','line'),'Linewidth',1);

if savefile
	print(['test_algo_outputs' filesep 'figures' filesep 'offline_teensy_sines_comp_POSTER.eps'], '-depsc');
end