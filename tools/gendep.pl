#!/usr/bin/env perl

sub scan_proj {
	print(uc($_[0]) . "_OBJS =");

	opendir(my $dh, "external/$_[0]/src");
	my @files = readdir($dh);
	foreach my $fn (@files){
		if(($fn eq ".") || ($fn eq "..")){
			next;
		}

		$fn =~ s/\.c$/.o/;
		print(" external/$_[0]/src/$fn");
	}

	closedir($dh);

	print("\n");
	print("CFLAGS += -Iexternal/$_[0]/include\n");
	print("OBJS += external/$_[0].a\n");
	print("external/$_[0].a: \$(" . uc($_[0]) . "_OBJS)\n");
	print("\t\$(AR) rcs \$@ \$(" . uc($_[0]) . "_OBJS)\n");
	print("\n");
}

scan_proj("libpng");
scan_proj("libtiff");
scan_proj("libjpeg");
scan_proj("libz");
