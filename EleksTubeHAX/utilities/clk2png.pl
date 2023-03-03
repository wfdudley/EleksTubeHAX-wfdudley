#!/usr/bin/perl
    # read ".clk" files and convert to (a) raw rgb565, and (b) png
    # William F. Dudley Jr. 2023 03 02
    # thanks to http://www.madox.net/blog/2011/06/06/converting-tofrom-rgb565-in-ubuntu-using-ffmpeg/ for the ffmpeg magic

    if(!$ARGV[0]) {
	printf("Usage: raw2clk.pl infile\n");
	exit(0);
    }
    my $infile = $ARGV[0];
    my $outfile = $infile;
    $outfile =~ s/clk/raw/;
    my $pngfile = $infile;
    $pngfile =~ s/clk/png/;
    my $insize = -s $infile;
    printf("infile = %s, size = %d\n", $infile, $insize);
    printf("outfile = %s\n", $outfile);
    open my $ifh, '<', $infile or die "can't open $infile";
    binmode $ifh;
    open my $ofh, '>', $outfile or die "can't open $outfile";
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
    system("ffmpeg -vcodec rawvideo -f rawvideo -pix_fmt rgb565 -s ${width}x${height} -i $outfile -f image2 -vcodec png $pngfile");
