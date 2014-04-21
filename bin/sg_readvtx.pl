#!/usr/bin/perl -w
use strict;
use Getopt::Long;
use Carp qw(carp croak cluck confess);

my $actid="pop";
my $basedir="/root/graph/";
my $vtxno=int(rand(1024));
GetOptions(
	"actid=s"=>\$actid,
	"basedir=s"=>\$basedir,
	"vtxno=i"=>\$vtxno,
) or croak "Something wrong with GetOptions";

print "./sg_readvtx $actid $basedir $vtxno"."\n";
system "./sg_readvtx $actid $basedir $vtxno";
