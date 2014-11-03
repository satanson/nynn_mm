#!/usr/bin/perl -w
use strict;
use Getopt::Long;
use Carp qw(carp croak cluck confess);

my $actid="push";
my $sgsdir="/home/bsp/programer/nynn/graph/";
my $vtxno=int(rand(1024));
GetOptions(
	"actid=s"=>\$actid,
	"sgdir=s"=>\$sgsdir,
	"vtxno=i"=>\$vtxno,
) or croak "Something wrong with GetOptions";

my ($vbegin,$vend)=($vtxno,$vtxno+1);
print "./sg_write $actid $sgsdir $vbegin $vend poet.txt"."\n";
system "./sg_write $actid $sgsdir $vbegin $vend poet.txt";
