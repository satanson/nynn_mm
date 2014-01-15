#!/bin/bash
basedir=$1
lnvtxnoBeg=$2
lnvtxnoEnd=$3
file=$4
for lnvtx in `eval "echo {${lnvtxnoBeg}..${lnvtxnoEnd}}"`;do
	beg=$((2**$lnvtx))
	end=$((2**($lnvtx+1)))
	echo "---------------[$beg,$end)---------------"
	bin/sg_read pop $basedir $beg $end
done
