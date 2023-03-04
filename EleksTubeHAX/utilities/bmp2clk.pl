#!/usr/bin/env perl
    # read .bmp files and convert to (a) clk rgb565, and (b) raw, and
    # optionally, (c) png
    # ALL raw files in one "run" must be same width and height
    # ffmpeg's conversion to png is inaccurate, so use bmp for archives
    # William F. Dudley Jr. 2023 03 03
    # thanks to http://www.madox.net/blog/2011/06/06/converting-tofrom-rgb565-in-ubuntu-using-ffmpeg/ for the ffmpeg magic

    my @infiles = ();
    my $debug = 0;
    my $make_png = 0;
    for(my $i = 0 ; $i <= $#ARGV ; $i++) {
	if($ARGV[$i] eq "-d") {
	    $debug++;
	}
	if($ARGV[$i] eq "-p") {
	    $make_png++;
	}
	elsif($ARGV[$i] =~ /\.bmp$/) {
	    push(@infiles, $ARGV[$i]);
	    $debug && printf("adding %s to \@infiles\n", $ARGV[$i]);
	}
    }
    my $bmpfile;
    $debug && printf("number of arguments = %d\n", $#ARGV);
    $debug && printf("\@infiles = %s\n", join(",", @infiles));
    if($infiles[0] eq '') {
	printf("Usage: bmp2clk.pl [-debug] [-png-too] file1.bmp file2.bmp . . . \n");
	exit(0);
    }
    while($bmpfile = pop(@infiles)) {
	my $pngfile = $bmpfile;
	my $rawfile = $bmpfile;
	my $clkfile = $bmpfile;
	$clkfile =~ s/\.bmp$/.clk/;
	$pngfile =~ s/\.bmp$/.png/;
	$rawfile =~ s/\.bmp$/.raw/;
	my $size;
	chomp($size = `file $bmpfile`);
	my $width = $size;
	my $height = $size;
	if($size =~ /, ([1-9][0-9]*) x ([1-9][0-9]*) x ([1-9][0-9]*),/) {
	    $debug && printf("match /, ([1-9][0-9]*) x ([1-9][0-9]*) x ([1-9][0-9]*),/, \$1 = %s, \$2 = %s\n", $1, $2);
	    $width = $1;
	    $height = $2;
	}
	printf("bmpfile = %s, width = %d, height = %d, ", $bmpfile, $width, $height);
	# exit;
	system("ffmpeg -loglevel quiet -i $bmpfile -vcodec rawvideo -f rawvideo -pix_fmt rgb565 $rawfile");
	$rawsize = -s $rawfile;
	if($make_png) {
	    system("ffmpeg -loglevel quiet -i $bmpfile $pngfile");
	}
	printf("rawfile = %s, raw file size = %d, clkfile = %s\n", $rawfile, $rawsize, $clkfile);
	open my $ifh, '<', $rawfile or die "can't open $rawfile for read";
	binmode $ifh;
	open my $ofh, '>', $clkfile or die "can't open $clkfile for write";
	binmode $ofh;
	print $ofh "C";
	print $ofh "K";
	printf $ofh ("%c%c", $width, 0);
	printf $ofh ("%c%c", $height, 0);
	my $buffer = '';
	my $row_no = 0;
	while ($row_no < $height) {
	    my $success = read $ifh, $buffer, $width * 2, $row_no * ($width * 2);
	    # printf("read %d bytes\n", length($buffer));
	    die $! if not defined $success;
	    last if not $success;
	    $row_no++;
	}
	print $ofh $buffer;

	close($ifh);
	close($ofh);
    }
