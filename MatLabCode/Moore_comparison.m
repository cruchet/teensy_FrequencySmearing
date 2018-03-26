%% Moore_comparison.m
%
% This script loads an audio file and apply frequency smearing 
% to its spectrum. Its goal is ot reproduce as close as possible the
% procedure described in the article [1]  to compare algorithms' results.
%
% Author:	Vassili Cruchet, vassili.cruchet@gmail.com
% 
% Technical University of Denmark (DTU)
%
% Date:		Spring 2018
%

%% load audio file
% In the article a synthetic vowel /ae/ is used. This audio sample has
% a similar spectrum. Four different frames are plotted as in FIG.2 and 
% FIG.4 of the article.

clc

filename = ['sounds' filesep 'now.wav'];
[signal Fs] = audioread(filename);
fs = 16e3;									% wanted sampling frequency
[P,Q] = rat(fs/Fs);
signal = resample(signal,P,Q);	clear Fs;	% resample audio signal

l_win = 128;								% frame length
overlap = 0.5;								% amount of overlap
l_sig	= length(signal);
b = 3;										% broadening factor

% uncomment to low pass filter the signal at 7kHz
% 	fc=7000;
% 	wl = [0 fc/(fs/2)];		wh = [fc/(fs/2) 1];		Gp = [0 -100];
% 	[IR IR_win] = FIR_eq(wl, wh, Gp, 101, 0.7);
% 	signal = filter(IR_win,1,signal);

%% process with frames of length l_win
% Each frame is zero-padded at both ends with 64 zeros to increase
% frequency resolution.

close all
clc

% calculate smearing matrix
A_s = calc_smear_matrix(fs, 2*l_win, b);

signal_smear = zeros(1,l_sig);
n_win = floor(l_sig/(l_win*overlap));

for k=0:n_win-1/overlap
	% window
	win = sqrt(hamming(l_win,'periodic'));
	% cut frame and window it
	idx_l = k*l_win*overlap+1;
	idx_h = (k+1/overlap)*l_win*overlap;
	sig   = signal(idx_l:idx_h);
	
	% zero pad on each side with 64 zeros
	sig = vertcat(zeros(l_win/2,1), sig.*win, zeros(l_win/2,1));
	
	% calculate spectrum and separate power and phase
	spec = fft(sig);
	spec_pow	= abs(spec);
	spec_phase	= angle(spec);
	
	% process smearing
	Y_pow = smearing(spec_pow, A_s);
	% multiply with original phase
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
	
 	signal_smear(idx_l:idx_h) = signal_smear(idx_l:idx_h) + y';

%	
% plot frame spectrum and time domain for a few frames
	num=32;
	if k>=num && k<=num+3
		[S_y fvec]=make_spectrum(y,fs);
		fig_tit = ['short-term spectra comparison, b=' num2str(b)];
		H=figure(1);
		set(gcf,'Units','normalized','Position',[0.1 0.1 0.65 0.7]);
		subplot(4,3,1+3*(k-num))
			plot(fvec/1000,fftshift(db(spec_pow)),'b');	% clean spectrum
			xlim([0 fs/2000]); ylim([-50 20]);
			ylabel('mag [dB]');
			if k==num+3
				xlabel('frequency [kHz]');
				IDX_END = idx_h;
			elseif k==num
				title('input spec');
				IDX_START = idx_l;
			end
		subplot(4,3,2+3*(k-num))
			plot(fvec/1000,fftshift(db(Y_pow)),'b');	% smeared spectrum
			xlim([0 fs/2000]); ylim([-50 20]);
			if k==num+3
				xlabel('frequency [kHz]');
			elseif k==num
				title('smeared spec');
			end
			legend(['frame #' num2str(k)],'Location','northeast')
		subplot(4,3,3+3*(k-num))
			plot(fvec/1000,db(S_y)+50,'b');		% IFFT spec	
				% an offset of -50dB due to windowing is compensated.
			xlim([0 fs/2000]); ylim([-50 20]);
			if k==num+3
				xlabel('frequency [kHz]');
			elseif k==num
				title('output spec');
			end	
		set(H,'Name',fig_tit);
		
		% plot time domain frames
		H=figure(2);	fig_tit = 'windowed time signals';
		set(H,'Name',fig_tit);
		
		tVec = [0:1/fs:length(y)/fs - 1/fs];
		% in
		subplot(1,2,1); grid on;
		plot((tVec+(k-num)*length(y)/(4*fs))*1000, sig+(4-(k-num))); hold on;
		% out
		subplot(1,2,2); grid on;
		plot((tVec+(k-num)*length(y)/(4*fs))*1000, y+(4-(k-num))); hold on;
		
	elseif k==num + 4
		length(y)
		IDX_END-IDX_START
		figure(2)
		tVec = [0:1/fs:1.75*length(y)/fs - 1/fs]*1000; 
		% input signal
		subplot(1,2,1)
		plot(tVec, signal(IDX_START:IDX_END),'k');
		set(gca,'ytick',[]);	set(gca,'yticklabel',[]);
		xlabel('Time (ms)');		ylabel('Amplitude');	ylim([-1 4.5]);
		title('Input');
		% output signal
		subplot(1,2,2)
		plot(tVec, signal_smear(IDX_START:IDX_END),'k');
		set(gca,'ytick',[]);	set(gca,'yticklabel',[]);
		xlabel('Time (ms)');		ylabel('Amplitude');	ylim([-1 4.5]);
		title('Output');
	end	
end

%% plot 
% Plot the final results
close all
% l_sig=fs*T
% time domain
figure('Position',[50 150 500 400])
plot(signal_smear,'r-','LineWidth',1);		grid on; hold on;
plot(signal,'b--','LineWidth',1);
legend(['smeared (b=' num2str(b) ')'], 'normal');
xlabel('#samples');
ylabel('amplitude');
% xlim([1 1+10/f]);
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

xlim([0 fs/2]);				 ylim([-150 0])
xlabel('frequency (Hz)');	 ylabel('magnitude (dB)');
legend(['smeared (b=' num2str(b) ')'], 'normal');

%% Reference
% Thomas Baer, and Brian J.C. Moore, _Effects of spectral smearing on the 
% intelligibility of sentences in noise_, The Journal of the Acoustical 
% Society of America 94, 1229 (1993);

