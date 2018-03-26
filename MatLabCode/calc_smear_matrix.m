function [A_s] = calc_smear_matrix(fs, N, b)
%CALC_SMEAR_MATRIX(fs, b, N) Returns the smearing matrix As
%   As = An^(-1)*Aw is the smearing matrix with a broadening factor b >= 1
%	for a signal of length N. It's normalized by the normal auditory filter An.

if mod(N,2)
	N = N+1;
end

% calculate normal and impaired auditory filter
A_n = calc_audit_filt(fs, N);	
A_w = calc_audit_filt(fs, N, b);

% calculate smearing matrix and output power spectrum
A_s			= inv(A_n)*A_w;
end

