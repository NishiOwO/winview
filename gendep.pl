#!/usr/bin/env perl

sub scan_proj {
	print(uc($_[0]) . "_OBJS =");

	opendir(my $dh, "$_[0]/src");
	my @files = readdir($dh);
	foreach my $fn (@files){
		if(($fn eq ".") || ($fn eq "..") || ($fn eq "tif_unix.c")){
			next;
		}

		$fn =~ s/\.c$/.o/;
		print(" $_[0]/src/$fn");
	}

	closedir($dh);

	print("\n");
	print("CFLAGS += -I$_[0]/include\n");
	print("$_[0].a: \$(" . uc($_[0]) . "_OBJS)\n");
	print("\t\$(AR) rcs \$@ \$(" . uc($_[0]) . "_OBJS)\n");
	print("\n");
}

scan_proj("libz");
scan_proj("libpng");
scan_proj("libjpeg");
scan_proj("libtiff");
