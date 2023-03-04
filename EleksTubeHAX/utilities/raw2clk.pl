#!/usr/bin/env perl
    # read ".raw" files and convert to (a) clk rgb565, and (b) bmp, and
    # optionally, (c) bmp
    # ALL raw files in one "run" must be same width and height
    # ffmpeg's conversion to png is inaccurate, so use bmp for archives
    # William F. Dudley Jr. 2023 03 03
    # thanks to http://www.madox.net/blog/2011/06/06/converting-tofrom-rgb565-in-ubuntu-using-ffmpeg/ for the ffmpeg magic

    my @infiles = ();
    my $debug = 0;
    my $make_png = 0;
    my $width = 0;
    my $height = 0;
    if(!$ARGV[0]) {
	printf("Usage: raw2clk.pl [-debug] file1 file2 . . .\n");
	exit(0);
    }
    for(my $i = 0 ; $i <= $#ARGV ; $i++) {
	if($ARGV[$i] eq "-d") {
	    $debug++;
	}
	if($ARGV[$i] eq "-p") {
	    $make_png++;
	}
	elsif($ARGV[$i] eq "-w") {
	    $width = $ARGV[++$i];
	    $debug && printf("width = %d\n", $width);
	    next;
	}
	elsif($ARGV[$i] eq "-h") {
	    $height = $ARGV[++$i];
	    $debug && printf("height = %d\n", $height);
	    next;
	}
	elsif($ARGV[$i] =~ /\.raw$/) {
	    push(@infiles, $ARGV[$i]);
	    $debug && printf("adding %s to \@infiles\n", $ARGV[$i]);
	}
    }
    my $rawfile;
    $debug && printf("number of arguments = %d\n", $#ARGV);
    $debug && printf("\@infiles = %s\n", join(",", @infiles));
    if(($infiles[0] eq '') || !$width || !$height) {
	printf("Usage: raw2clk.pl [-debug] [-png-too] -w width -h height file1 file2 . . . \n");
	exit(0);
    }
    $debug && printf("width = %d, height = %d\n", $width, $height);
    while($rawfile = pop(@infiles)) {
	my $bmpfile = $rawfile;
	my $clkfile = $rawfile;
	my $pngfile = $rawfile;
	$bmpfile =~ s/\.raw$/.bmp/;
	$clkfile =~ s/\.raw$/.clk/;
	$pngfile =~ s/\.raw$/.png/;
	my $rawsize = -s $rawfile;
	printf("rawfile = %s, size = %d, width = %d, height = %d, clkfile = %s\n", $rawfile, $rawsize, $width, $height, $clkfile);
	open my $ifh, '<', $rawfile or die "can't open $rawfile";
	binmode $ifh;
	open my $ofh, '>', $clkfile or die "can't open $clkfile";
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
	if($make_png) {
	    system("ffmpeg -vcodec rawvideo -f rawvideo -pix_fmt rgb565 -s ${width}x${height} -i $rawfile -f image2 -vcodec png $pngfile");
	}
	system("ffmpeg -vcodec rawvideo -f rawvideo -pix_fmt rgb565 -s ${width}x${height} -i $rawfile -f image2 $bmpfile");
    }
