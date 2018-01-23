#!/usr/bin/env perl

use warnings;
use strict;

my %openers = map { $_ => 1 } (
    'set(SOURCE_FILES',
    'set(TEST_FILES',
);

my $state = 'look-for-opener';

my @items;

# skip until source files list
while (my $line = <>) {
    chomp $line;

    if ($state eq 'look-for-opener') {
        print "$line\n";
        if ($openers{$line}) {
            $state = 'process-files';
        }
    }
    elsif ($state eq 'process-files') {
        $line =~ s/^\s+//;
        my $close_paren_found = ($line =~ s/\)//);
        my @line_items = split(/\s+/, $line);
        push(@items, @line_items);

        if ($close_paren_found) {
            @items = sort @items;
            for my $item (@items) {
                print "    $item\n";
            }
            print "    )\n";
            @items = ();
            $state = 'look-for-opener';
        }
    }
    else {
        die "Invalid state '$state'";
    }
}

__END__

=head1 NAME

fix-cmakelists.pl - Reformat CMakeLists source entries

=head1 SYNOPSIS

    perl -i fix-cmakelists.pl CMakeLists.txt

=head1 DESCRIPTION

This script reformats source and test file entries in CMakeLists.txt
to recover the original formatting after an IDE adds files
to the lists during development.
