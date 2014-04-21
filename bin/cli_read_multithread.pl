#!/usr/bin/perl -w
use strict;
use Getopt::Long;
my $ip="localhost";
my $port_min="60000";
my $port_max="60008";
my $vtx_beg="0";
my $vtx_end="1024";
my $thdsz="16";
my $loop="1024";

GetOptions(
	"ip=s"=>\$ip,
	"port_min=s"=>\$port_min,
	"port_max=s"=>\$port_max,
	"vtx_beg=s"=>\$vtx_beg,
	"vtx_end=s"=>\$vtx_end,
	"thdsz=s"=>\$thdsz,
	"loop=s"=>\$loop,
) or die "failed to parse options";
`./cli_read_multithread $ip $port_min $port_max $vtx_beg $vtx_end $thdsz $loop`;
system "./util1 output.*|./quadtuples";
system "rm -fr output.*";
