#!/usr/bin/perl -w
use strict;
use Getopt::Long;
use SelectSaver;
use Carp qw(carp croak confess cluck);
use IPC::Open3;

my $sgsdir="/home/bsp/programer/nynn/graph/";
my $sgnum=8;
GetOptions(
	"sgsdir=s"=>\$sgsdir,
	"sgnum=s"=>\$sgnum,
) or croak "Something wrong with GetOptions";
my @pids;
foreach my $i (0..$sgnum) {
	my $pid=fork;
	if ($pid==0){
		my $sgkey=$i*2**23;
		exec "./sg_format $sgsdir $sgkey";
		exit 0;
	}elsif ($pid>0){
		push @pids,$pid;
	}else {
		croak "Failed to fork a new process!";
	}
}

foreach (@pids){waitpid $_,0;}
