#!/bin/bash
provHost=$1
provPort=$2
lnvtxnoBeg=$3
lnvtxnoEnd=$4
file=$4
for lnvtx in `eval "echo {${lnvtxnoBeg}..${lnvtxnoEnd}}"`;do
	beg=$((2**$lnvtx))
	end=$((2**($lnvtx+1)))
	echo "---------------[$beg,$end)---------------"
	bin/prov_write push $provHost $provPort $beg $end $file 	
done
