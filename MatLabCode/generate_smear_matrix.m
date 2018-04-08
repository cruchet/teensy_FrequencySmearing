% genereate smear matrix for use in C code
% -> copy-paste terminal output to fill in a two-dimmensional array:
%		float A_s[N/2][N/2] = {  {A[0,0],   ... , A[0,N/2]  } ,  
%								 {			...			    } ,   
%						         {A[N/2,0], ... , A[N/2,N/2]}   };
%

clear all
close all
clc

fs = 16e3;
N  = 1024;
b  = 4;

A_s = calc_smear_matrix(fs,N,b);
%plot(db(A_s(length(A_s)/2,:))); hold on;
count=0;
for i=1:length(A_s)
	for j=1:length(A_s)
		if(A_s(i,j)<1e-6)
			A_s(i,j)=0;
			count=count+1;
		end
	end
end
spy(A_s)
disp(['filled: ' num2str(((N/2)^2-count)/(N/2)^2*100) '%']);
%%
fileID = fopen(['As_b' num2str(b) '_fs' num2str(fs/1000) 'k_L' num2str(N) '.txt'],'w')
fprintf(fileID,'{');
for i=1:length(A_s)
	fprintf(fileID,'{');
	for j=1:length(A_s)
		if j==length(A_s)
			fprintf(fileID,'%f', A_s(i,j));
		else
			fprintf(fileID,'%f, ', A_s(i,j));
		end
	end
	if(i==length(A_s))
		fprintf(fileID,'}');
	else
		fprintf(fileID,'},\n');
	end
end
fprintf(fileID,'};\n');

fclose('all');