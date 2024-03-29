#!/usr/bin/env perl

use warnings;
use strict;

use File::Basename qw(basename dirname);
use File::Spec::Functions qw(catfile);
use File::Copy qw(move);

for my $filename (@ARGV) {

    open(my $fh, '<', $filename) or die "Failed to open input file '$filename': $!\n";

    my $basedir = dirname($filename);

    # This is a hack to let us run from project root
    # instead of having to run from within src or test dirs.
    $basedir =~ s/^(src|test)\///;

    $filename =~ /^(.+)\.[^\.]+$/ or die "File has no extension: $filename\n";
    my $filename_without_extension = basename($1);
    my $header_filename = "$filename_without_extension.h";

    my $tmp_file_name = "$filename-fix-includes-tmp";
    open (my $tmpfh, '>', $tmp_file_name);

    while (my $line = <$fh>) {
        chomp $line;

        # don't "fix" the include of the corresponding header for this source file
        if ($filename =~ /\.cpp$/ && $line eq "#include \"$header_filename\"") {
            print $tmpfh "$line\n";
        }
        elsif ($line =~ /^#include "([^"]+)"$/) {
            my $file = $1;
            my $fixed_file = catfile($basedir, $file);

            # Only fix the include if the referenced file actually exists
            # at the relative location.
            # Otherwise just convert to angle brackets,
            # it's probably a system include.
            if (-f catfile("src", $fixed_file)) {
                print $tmpfh "#include <$fixed_file>\n";
            }
            else {
                print $tmpfh "#include <$file>\n";
            }
        }
        else {
            print $tmpfh "$line\n";
        }
    }

    close($fh) or die "Failed to close input file: $!\n";
    close($tmpfh) or die "Failed to close temp file: $!\n";

    move($tmp_file_name, $filename) or die "Failed to move temp file back over the top of the file: $!\n";
}

__END__

=head1 NAME

fix-includes.pl - Normalise includes to be angle-bracketed

=head1 SYNOPSIS

    fix-includes.pl sourcefile.cpp

=head1 DESCRIPTION

This script converts includes that use quotes
into includes that use angle brackets using paths
relative to the current working directory.
