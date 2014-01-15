#!/bin/bash
agentHost=$1
agentPort=$2
lnvtxnoBeg=$3
lnvtxnoEnd=$4

for lnvtx in `eval "echo {${lnvtxnoBeg}..${lnvtxnoEnd}}"`;do
	beg=$((2**$lnvtx))
	end=$((2**($lnvtx+1)))
	echo "---------------[$beg,$end)---------------"
	bin/agent_perform pop $agentHost $agentPort $beg $end
done
