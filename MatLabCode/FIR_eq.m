function [IR IR_cut] = FIR_eq(Wl,Wh,Gp,filter_l,win_l)
% FIR_eq design a FIR multiple bandpass filter
% inputs:
%	Wl		= Array of low cut-of frequencies (on a normalized frequency scale)
%	Wh		= Array of high cut-of frequencies (on a normalized frequency scale)
%		=> Wl and Wh have to be the same size and build in growing order.
%
%	Gp		= Array of gains for each bands in dB (positive or negative)
%	filter_l= Length of the filter (filter order + 1)
%	win_l	= relative length for windowing
%
% outputs:
%	IR		= filter's impulse response
%	IR_cut	= windowed impulse response

if length(Wl)~=length(Wh)||length(Wl)~=length(Gp)||length(Wh)~=length(Gp)
	error('Wl, Wh and Gp must be the same size!');
else
	N_bands = length(Wl);
end

spec = ones(1,filter_l);
for ii=1:N_bands
	% indices in the spectral domain where the corresponding cut-of frequencies
	idx_cutLow = ceil(Wl(ii)*filter_l/2);
	idx_cutHigh	= ceil(Wh(ii)*filter_l/2);
	if idx_cutLow == 0
		idx_cutLow = 1;
		spec(1)=db2mag(Gp(1));
	end
	% frequency response of the filter
	spec(idx_cutLow+1:idx_cutHigh+1) = db2mag(Gp(ii)); % positive frequencies
	spec(filter_l-idx_cutHigh+1 : filter_l-idx_cutLow+1)= db2mag(Gp(ii)); % neg freq
end

% IR with ifft()
IR = fftshift(ifft(spec));
if max(imag(IR)>0.000001)
	disp('IR is complex !!');
	return
end

if win_l~=1
	% windowing with a Hann funciton with an effective length of win_l %
	IR_l  = length(IR);
	IR_cut = zeros(1,IR_l);

	IR_cut(:) = IR;
	cut_low  = ceil(IR_l/2 - (win_l*IR_l)/2);
	cut_high = ceil(IR_l/2 + (win_l*IR_l)/2);
	IR_cut(1:cut_low)		= 0;
	IR_cut(cut_high:end)	= 0;

	win = hann(ceil(win_l*IR_l));
	if cut_low==0
		cut_low=cut_low+1;
	end
	if length(IR_cut(cut_low:cut_high))~=length(win)
		error('Increase filter order');
	end
	IR_cut(cut_low:cut_high) = win'.*IR_cut(cut_low:cut_high);
else
	IR_cut = IR;
end

end

