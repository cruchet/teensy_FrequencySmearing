function [sa,ija] = compress_matrix(A, N, thresh, nmax)
%COMPRESS_MATRIX(A, N, thresh, nmax) Converts a square matrix A[1..n][1..n] into row-indexed sparse storage mode.
% function taken from _Numerical Recipes In C_, p.79
%	Converts a square matrix A[1..n][1..n] into row-indexed sparse storage mode. Only elements
%	of a with magnitude >=thresh are retained. Output is in two linear arrays with dimension
%	nmax (an input parameter): sa[1..] contains array values, indexed by ija[1..]. The
%	number of elements filled of sa and ija on output are both ija[ija[1]-1]-1 (see text).
%
% 

for j=1:N
	sa(j) = A(j,j);			% store diagonal element
end
ija(1) = N+1;
k=N+1;						% idx of the last element

for i=1:N					% loop over rows
	for j=1:N				% loop over columns
		if ((abs(A(i,j)) >= thresh) && (i~=j))
			k=k+1;
			if (k > nmax)
				error('nmax too small or thersh too high');
			end
			sa(k) = A(i,j);	% store off-diagonal elements 
			ija(k) = j-1;	% and their columns
		end
	end
	ija(i+1) = k;			% as each row is completed, store index to next
end		
end

