function [A] = calc_audit_filt(fs, N, b)
%CALC_AUDIT_FILT Calculate the auditory filter
%   Returns the matrix A of the auditory filter as 
%			A(n,i)=(1+p*g)*exp(-p*g).
%	The size of A is (N/2 * N/2), where N is the frame's length.
%	b is the broadening factor (>=1 to broaden the spectrum)
%

if ~exist('b','var')
 % no broadening
  b=1;
end

fMax	= fs/2;									% maximal freq to be processed
delta_f = fs/N;									% frequency resolution
fVec	= [delta_f:delta_f:fMax];
A		= zeros(N/2);

for n = 1:N/2
	% calculate ERB and p for the center frequency (symteric broadening)
	ERB = 24.7*(0.000437*fVec(n) + 1);			% Equivalent rectangular bandwidth
												%	for normal hearing
	p	= 4*fVec(n)/ERB/b;						% sharpness of the filter
	
	% calculate row
	for i=1:N/2
		g = abs(fVec(i)-fVec(n)) / fVec(n);		% normalized frequency
		
		A(n,i) = (1+p*g)*exp(-p*g) / ERB;		% auditory filter is normalized
												% by the normal ERB
	end
end
	
end

