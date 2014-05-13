#!/usr/bin/perl -w
use strict;
use Getopt::Long;
use SelectSaver;
use Carp qw(carp croak confess cluck);
use IPC::Open3;

my $actid="pop";
my $sgsdir="/root/graph";
my $vbegin=0;
my $vend=1024;
my $loop=16;
my $procsz=16;

GetOptions(
	"actid=s"=>\$actid,
	"sgsdir=s"=>\$sgsdir,
	"vbegin=i"=>\$vbegin,
	"vend=i"=>\$vend,
	"loop=i"=>\$loop,
	"procsz=i"=>\$procsz,
) or croak "Something wrong with GetOptions";
my @pids=();
my $cmd="./sg_read";
my $args="$actid $sgsdir $vbegin $vend $loop";
foreach (1 .. $procsz) {
	print "./sg_read $actid $sgsdir $vbegin $vend $loop"."\n";
	#open my $out,">","output.$_" or croak "Can't create file 'output.$_' for writing!";
	#my $saver=SelectSaver->new($out);
	#my $pid=open3(*CHLD_IN,*CHLD_OUT,*CHLD_OUT,$cmd_args,">output.$_","&");
	my $pid=fork;
	if ($pid==0){
		close STDOUT;
		open STDOUT,">","output.$_";
		exec $cmd,$actid,$sgsdir,$vbegin,$vend,$loop;
		exit 0;
	}elsif ($pid>0){
		push @pids,$pid;
	}else {
		croak "Failed to fork a new process!";
	}
}

foreach (@pids){waitpid $_,0;}
print "DONE\n";
system "./util1 output.*|./quadtuples";
system "rm -fr output.*";
