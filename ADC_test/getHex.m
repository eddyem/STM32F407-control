function [x, D] = getHex(fname)
% read values in hex and return dec
	F = fopen(fname);
	D = fscanf(F, "%x");
	fclose(F);
	Dmean = mean(D);
	Dmedian = median(D);
	Dstd = std(D);
	Dplus = max(D) - Dmean;
	Dminus = min(D) - Dmean;
	Ddig = 24 - ceil(log2(Dplus - Dminus));
	printf("Статистика: mean=%d, median=%d, std=%d, refErr=%g;  размах от среднего: +%d, %d; %d значащих разрядов\n", ...
		Dmean, Dmedian, Dstd, Dstd/Dmean, Dplus, Dminus, Ddig);
	S = size(D,1);
	x = [0 : S-1] / S;
endfunction
