#!/usr/bin/perl
    # run this from all_fonts/font-name
    use File::Basename;
    my $fontsdir = "../../data";
    printf("enter font number to write: ");
    my $fnum, $bname;
    chomp($fnum = <>);
    chomp($bname = basename(`pwd`));
    printf("basename = %s\n", $bname);
    open(SLOT, ">../slot$fnum");
    printf SLOT ("%d %s\n", $fnum, $bname);
    close(SLOT);
    for($i = 0 ; $i < 10 ; $i++) {
	my $f1 = sprintf("%d.clk", $i);
	if(-e $f1) {
	    my $cmd = sprintf("cp -p %s %s/%d%s\n", $f1, $fontsdir, $fnum, $f1);
	    print $cmd;
	    system($cmd);
	}
    }
    print("installed fonts:\n");
    print `cat ../slot?`;
