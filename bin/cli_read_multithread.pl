#!/usr/bin/perl -w
use strict;
use Getopt::Long;
my $ip="192.168.255.114";
my $port="61000";
my $thdsz="16";

GetOptions(
	"ip=s"=>\$ip,
	"port=s"=>\$port,
	"thdsz=s"=>\$thdsz,
) or die "failed to parse options";
print qq{./cli_read_multithread $ip $port $thdsz} . "\n";
`./cli_read_multithread $ip $port $thdsz`;
system "./util1 output.*|./quadtuples";
system "rm -fr output.*";
