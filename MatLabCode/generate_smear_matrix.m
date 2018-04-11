%% genereate smear matrix for use in C code
% -> copy-paste terminal output to fill in a two-dimmensional array:
%		float A_s[N/2][N/2] = {  {A[0,0],   ... , A[0,N/2]  } ,  
%								 {			...			    } ,   
%						         {A[N/2,0], ... , A[N/2,N/2]}   };
%

clear all
close all
clc

fs = 16e3;
N  = 256;
b  = 6;
tol = 1e-3;

A_s = calc_smear_matrix(fs,N,b);

% mesh(A_s,'CDataMapping','scaled')
% colorbar
% %plot(db(A_s(length(A_s)/2,:))); hold on;
count=0;
for i=1:length(A_s)
	for j=1:length(A_s)
		if(A_s(i,j)<tol)
			A_s(i,j)=0;
			count=count+1;
		end
	end
end
spy(A_s)
disp(['filled: ' num2str(((N/2)^2-count)/(N/2)^2*100) '%']);

%% compressed matrix
clc
% % find width
% rowIdx = find(A_s(:,end)>tol,1);
% colIdx = find(A_s(rowIdx,:)>tol,1);
% width = N/2-colIdx
% width*N

[sa ija] = compress_matrix(A_s, N/2, tol, N^2);
disp(['usage ' num2str( (length(sa)+length(ija))/(N/2)^2*100) '% => ' num2str(length(sa)*8/1000) ' kbytes']);

% write to .txt file
fileID = fopen(['sa&ija_b' num2str(b) '_fs' num2str(fs/1000) 'k_L' num2str(N) '.txt'],'w')
fprintf(fileID,['unsigned int ija_b' num2str(b) '_fs' num2str(fs/1000) 'k_L' num2str(N) '[' num2str(length(ija)) '] = {']);
for i=1:length(ija)
	if i==length(ija)
		fprintf(fileID,'%u', ija(i));
	else
		fprintf(fileID,'%u, ', ija(i));
	end
end
fprintf(fileID,'};\n');
fprintf(fileID,['float sa_b' num2str(b) '_fs' num2str(fs/1000) 'k_L' num2str(N) '[' num2str(length(sa)) '] = {']);
for i=1:length(sa)
	if i==length(sa)
		fprintf(fileID,'%f', sa(i));
	else
		fprintf(fileID,'%f, ', sa(i));
	end
end
fprintf(fileID,'};\n');
fclose('all');
		
% %% uncompressed matrix
% 
% fileID = fopen(['As_b' num2str(b) '_fs' num2str(fs/1000) 'k_L' num2str(N) '.txt'],'w')
% fprintf(fileID,'{');
% for i=1:length(A_s)
% 	fprintf(fileID,'{');
% 	for j=1:length(A_s)
% 		if j==length(A_s)
% 			fprintf(fileID,'%f', A_s(i,j));
% 		else
% 			fprintf(fileID,'%f, ', A_s(i,j));
% 		end
% 	end
% 	if(i==length(A_s))
% 		fprintf(fileID,'}');
% 	else
% 		fprintf(fileID,'},\n');
% 	end
% end
% fprintf(fileID,'};\n');
% 
% fclose('all');
