#!/usr/bin/perl -w
use strict;
use Getopt::Long;
use Carp qw(carp croak cluck confess);

my $actid="pop";
my $sgsdir="/home/bsp/programer/nynn/graph/";
my $vtxno=int(rand(1024));
GetOptions(
	"actid=s"=>\$actid,
	"sgdir=s"=>\$sgsdir,
	"vtxno=i"=>\$vtxno,
) or croak "Something wrong with GetOptions";

print "./sg_readvtx $actid $sgsdir $vtxno"."\n";
system "./sg_readvtx $actid $sgsdir $vtxno";
