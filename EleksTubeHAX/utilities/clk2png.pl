#!/usr/bin/perl
    # read ".clk" files and convert to (a) raw rgb565, and (b) png, and (c) bmp
    # ffmpeg's conversion to png is inaccurate, so use bmp
    # William F. Dudley Jr. 2023 03 03
    # thanks to http://www.madox.net/blog/2011/06/06/converting-tofrom-rgb565-in-ubuntu-using-ffmpeg/ for the ffmpeg magic

    my @infiles = ();
    my $debug = 0;
    my $keep_raw = 0;
    my $make_png = 0;
    if(!$ARGV[0]) {
	printf("Usage: raw2clk.pl [-debug] [-keep-raw] file1 file2 . . .\n");
	exit(0);
    }
    for(my $i = 0 ; $i <= $#ARGV ; $i++) {
	if($ARGV[$i] eq "-d") {
	    $debug++;
	}
	elsif($ARGV[$i] eq "-p") {
	    $make_png++;
	}
	elsif($ARGV[$i] eq "-k") {
	    $keep_raw++;
	}
	elsif($ARGV[$i] =~ /\.clk$/) {
	    push(@infiles, $ARGV[$i]);
	}
    }
    my $infile;
    while($infile = pop(@infiles)) {
	my $rawfile = $infile;
	my $pngfile = $infile;
	my $bmpfile = $infile;
	$bmpfile =~ s/\.clk$/.bmp/;
	$pngfile =~ s/\.clk$/.png/;
	$rawfile =~ s/\.clk$/.raw/;
	my $insize = -s $infile;
	printf("infile = %s, size = %d\n", $infile, $insize);
	printf("rawfile = %s\n", $rawfile);
	open my $ifh, '<', $infile or die "can't open $infile";
	binmode $ifh;
	open my $ofh, '>', $rawfile or die "can't open $rawfile";
	binmode $ofh;
	my $buffer = '';
	my $success = read $ifh, $buffer, 6, 0;
	die $! if not defined $success;
	printf("magic = %s\n", substr($buffer, 0, 2));
	($width, $height) = unpack('v2', substr($buffer, 2, 4));
	printf("width = %d, height = %d\n", $width, $height);
	my $row_no = 0;
	while ($row_no < $height) {
	    my $success = read $ifh, $buffer, $width * 2, 6 + ($row_no * ($width * 2));
	    # printf("read %d bytes\n", length($buffer));
	    die $! if not defined $success;
	    last if not $success;
	    $row_no++;
	}
	print $ofh substr($buffer, 6);;
	close($ifh);
	close($ofh);
	if($make_png) {
	    system("ffmpeg -vcodec rawvideo -f rawvideo -pix_fmt rgb565 -s ${width}x${height} -i $rawfile -f image2 -vcodec png $pngfile");
	}
	system("ffmpeg -vcodec rawvideo -f rawvideo -pix_fmt rgb565 -s ${width}x${height} -i $rawfile -f image2 $bmpfile");
	$keep_raw || unlink($rawfile);
    }
