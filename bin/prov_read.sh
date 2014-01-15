#!/bin/bash
provHost=$1
provPort=$2
lnvtxnoBeg=$3
lnvtxnoEnd=$4

for lnvtx in `eval "echo {${lnvtxnoBeg}..${lnvtxnoEnd}}"`;do
	beg=$((2**$lnvtx))
	end=$((2**($lnvtx+1)))
	echo "---------------[$beg,$end)---------------"
	bin/prov_read pop $provHost $provPort $beg $end
done
