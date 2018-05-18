#!/usr/bin/env perl

use warnings;
use strict;

while (my $line = <>) {
    chomp $line;
    $line =~ /^([^:]+):.*?(\d+) x (\d+).*$/ or die "failed to match line: $line\n";
    my ($filename, $width, $height) = ($1, $2, $3);

    if ($line =~ /progress-(\d{4}-\d{2}-\d{2})/) {
        print "<h4>$1</h4>\n"
    }


    print qq{<img src="blank.gif" data-echo="$filename" width="$width" height="$height">\n};
}
