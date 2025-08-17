#!/usr/bin/env perl

my $cmdline = "wlib -fo -n -q -b";
my $mode = "";

foreach my $arg (@ARGV){
	if($arg eq "rcs"){
		$mode = "libname";
		next;
	}
	if($mode eq "libname"){
		$cmdline = $cmdline . " $arg";
	}else{
		$cmdline = $cmdline . " +$arg";
	}
	$mode = "";
}

exec("$cmdline");
