#!/bin/bash
prodHost=$1
prodPort=$2
lnvtxnoBeg=$3
lnvtxnoEnd=$4
file=$5
for lnvtx in `eval "echo {${lnvtxnoBeg}..${lnvtxnoEnd}}"`;do
	beg=$((2**$lnvtx))
	end=$((2**($lnvtx+1)))
	echo "---------------[$beg,$end)---------------"
	bin/prod_perform push $prodHost $prodPort $beg $end $file
done
