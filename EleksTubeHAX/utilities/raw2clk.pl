#!/usr/bin/perl
    my $debug = 0;
    my $width = 135;
    my $height = 240;
    $debug && printf("number of arguments = %d\n", $#ARGV);
    for(my $i = 0 ; $i <= $#ARGV ; $i++) {
	if($ARGV[$i] eq "-d") {
	    $debug++;
	}
	if($ARGV[$i] eq "-w") {
	    $width = $ARGV[++$i];
	    $debug && printf("width = %d\n", $width);
	    next;
	}
	if($ARGV[$i] eq "-h") {
	    $height = $ARGV[++$i];
	    $debug && printf("height = %d\n", $height);
	    next;
	}
	if(!$infile) {
	    $infile = $ARGV[$i];
	    next;
	}
	elsif(!$outfile) {
	    $outfile = $ARGV[$i];
	    next;
	}
    }
    if(!$infile || !$outfile) {
	printf("Usage: raw2clk.pl infile outfile\n");
	exit(0);
    }
    $debug && printf("width = %d, height = %d\n", $width, $height);
    my $insize = -s $infile;
    printf("infile = %s, size = %d, width = %d, height = %d, outfile = %s\n", $infile, $insize, $width, $height, $outfile);
    open my $ifh, '<', $infile or die "can't open $infile";
    binmode $ifh;
    open my $ofh, '>', $outfile or die "can't open $outfile";
    binmode $ofh;
    print $ofh "C";
    print $ofh "K";
    printf $ofh ("%c%c", $width, 0);
    printf $ofh ("%c%c", $height, 0);
    my $buffer = '';
    my $row_no = 0;
    while ($row_no < 240) {
        my $success = read $ifh, $buffer, $width * 2, $row_no * ($width * 2);
	# printf("read %d bytes\n", length($buffer));
	die $! if not defined $success;
	last if not $success;
	$row_no++;
    }
    print $ofh $buffer;

    close($ifh);
    close($ofh);
