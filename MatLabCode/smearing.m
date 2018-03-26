function [Y] = smearing(X, A_s)
%SMEARING 
%   SMEARING(X, A_s) Smears the power spectrum X (column vector) 
%   using the auditory filter As = An^(-1)*A_w calculated 
%	with CALC_SMEAR_MATRIX.

% parameters
N	 = length(X);
if mod(N,2)
	N = N+1;
end
Xpos = X(1:N/2);		% positive frequencies
Y	 = zeros(N,1);

% calculate smearing matrix and output power spectrum
Y(1:N/2)	= A_s * Xpos;

% reconstruct negative frequencies
Y(N/2+2:end) = flipud(Y(2:N/2));
end

