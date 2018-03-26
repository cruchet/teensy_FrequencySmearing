clear all
close all
clc

% This script loads an audio file and apply frequency smearing 
% to its spectrum.
% Each frame is zero-padded at both ends so that its length is doubled.

%% load audio file
clc
saveFile = 1;
% filename = ['sounds' filesep 'sine1k2k4k.wav'];
% [signal fs] = audioread(filename);

fs = 16000;
T = 5;
f = 1000;
t = [0:1/fs:T-1/fs];
signal = (sin(2*pi*f*t))' +  0.75*(sin(2*pi*2*f*t))' ...
 	+  0.5*(sin(2*pi*4*f*t))';
% signal = signal + 0.05*randn(length(signal),1);
l_win = 1024;
overlap = 1/2;
l_sig	= length(signal);
b = 3;			% broadening factor

% truncate signal to a multilple  of frame length
N_frame = floor(l_sig/l_win);
signal = signal(1:N_frame*l_win);
l_sig = length(signal);
T = l_sig/fs;
t = 0:1/fs:T-1/fs;

%% process with frames of length l_win
close all

% calculate smearing matrix
tic
A_s = calc_smear_matrix(fs, 2*l_win, b);
t_matrix = toc

signal_smear = zeros(1,l_sig);
n_win = (l_sig/(l_win*overlap));

tic
for k=0:n_win-1/overlap
	win = sqrt(hann(l_win,'periodic'));
	% cut frame and window it
	idx_l = k*l_win*overlap+1;
	idx_h = (k+1/overlap)*l_win*overlap;
	sig   = signal(idx_l:idx_h);
	
	% zero pad on each side with l_win/2 zeros -> increase freq. res.
	sig = vertcat(zeros(l_win/2,1), sig.*win, zeros(l_win/2,1));
	
	% calculate spectrum and separate power and phase
	spec = fft(sig);
	
	spec_pow	= abs(spec);
	spec_phase	= angle(spec);
	
	% process smearing
	Y_pow = smearing(spec_pow, A_s);
	Y = Y_pow.* exp(i*spec_phase);

	% back to time domain and concatenate
	y = ifft(Y);
% 	y = ifft(spec);

	if imag(y)>1e-6
		disp('y(t) is complex!')
	else
		y=real(y);
	end
	% window the signal process overlap-and-add
	y(l_win/2+1:3*l_win/2) = y(l_win/2+1:3*l_win/2).*win;
	if k==0							% first frame
		y=y(l_win/2+1:end);
		idx_l=1;
		idx_h=idx_h + l_win/2;
	elseif k == n_win-1/overlap
		y=y(1:l_win/2 + l_win);		% last frame
		idx_l=idx_l - l_win/2;
	else							% else
		idx_l=idx_l - l_win/2;
		idx_h=idx_h + l_win/2;
	end
	
	plot([idx_l:1:idx_h],y); hold on; grid on;
 	signal_smear(idx_l:idx_h) = signal_smear(idx_l:idx_h) + y';
% [S fvec]=make_spectrum(signal_smear,fs);
% semilogx(fvec, db(S)); hold on;
end
t_process = toc

% % build first and last half of window
% 	A_s = calc_smear_matrix(fs, l_win/2, b);
% 	sig = signal(1:l_win/2);
% 	spec = fft(sig .*win(l_win/2+1:end));
% 	spec_pow	= abs(spec);
% 	spec_phase	= angle(spec);
% 
% 	% process smearing
% 	Y_pow = smearing(spec_pow, A_s);
% 	Y = Y_pow.* exp(i*spec_phase);
% 
% 	% back to time domain and concatenate
% 	y = ifft(Y);
% 	if imag(y)>1e-6
% 		disp('y(t) is complex!')
% 	else
% 		y=real(y);
% 	end
% % 	plot([1:l_win/2],y.*win(l_win/2+1:end),':'); hold on; grid on;
% 	signal_smear(1:l_win/2) = signal_smear(1:l_win/2) + ...
% 		y'.*win(l_win/2+1:end)';
% % end
% 	sig = signal(end-l_win/2+1:end);
% 	spec = fft(sig .*win(1:l_win/2));
% 	spec_pow	= abs(spec);
% 	spec_phase	= angle(spec);
% 
% 	% process smearing
% 	Y_pow = smearing(spec_pow, A_s);
% 	Y = Y_pow.* exp(i*spec_phase);
% 
% 	% back to time domain and concatenate
% 	y = ifft(Y);
% 	if imag(y)>1e-6
% 		disp('y(t) is complex!')
% 	else
% 		y=real(y);
% 	end
% 	plot([l_sig-l_win/2+1:l_sig],y.*win(1:l_win/2),':'); hold on; grid on;
% 	signal_smear(end-l_win/2+1:end) = signal_smear(end-l_win/2+1:end) +  ...
% 		y'.*win(1:l_win/2)';
	
% plot(signal_smear,'k--','LineWidth',1)
% scalling
% signal_smear=signal_smear/max(abs(signal_smear));

%% plot and listen
close all
% l_sig=fs*T
% time domain
figure('Position',[50 150 500 400])
plot(signal_smear,'r-','LineWidth',1);		grid on; hold on;
plot(signal,'b--','LineWidth',1);
legend(['smeared (b=' num2str(b) ')'], 'normal');
xlabel('#samples');
ylabel('amplitude');
if saveFile
	print(['outputs' filesep 'block_proc_zPad_time_sine_1k2k4k.pdf'], '-dpdf');
end

% xlim([1 1+10/f]);
% spectrum
[spec fVec]			= make_spectrum(signal(1:l_sig),fs);
[spec_smear fVec]	= make_spectrum(signal_smear(1:l_sig)',fs);

figure('Position',[550 150 500 400])
% semilogx(fVec,db(spec_smear),'b'); hold on; grid on;
% semilogx(fVec,db(spec),'r','LineWidth',0.5);		

plot(fVec, db(spec_smear),'r-'); hold on; grid on;
plot(fVec, db(spec),'b-'); 

xlim([0 fs/2])
% ylim([-200 0])
xlabel('frequency (Hz)');
ylabel('magnitude (dB)');
legend(['smeared (b=' num2str(b) ')'], 'normal');

if saveFile
	print(['outputs' filesep 'block_proc_zPad_spec_sine_1k2k4k.pdf'], '-dpdf');
	audiowrite(['outputs' filesep 'block_proc_zPad_sine_1k2k4k.wav'], ...
		signal_smear/max(abs(signal_smear)), fs);
end	
