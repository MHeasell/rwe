#!/usr/bin/env perl

use warnings;
use strict;

use File::Basename qw(basename dirname);
use File::Spec::Functions qw(catfile);
use File::Copy qw(move);
use File::Find;

my $dir = 'src';

my %empty_files;

find(\&process_file, $dir);

for my $f (keys %empty_files) {
    unlink($f);
}

process_cmakelists('CMakeLists.txt', \%empty_files);

sub process_cmakelists {
    my ($cmakelists, $files_to_remove) = @_;

    my $tmp_name = "$cmakelists.remove-empty-cpp-files-tmp";

    open(my $fh, '<', $cmakelists) or die "Failed to open $cmakelists: $!";
    open(my $out, '>', $tmp_name) or die "Failed to open tmp file: $!";

    while (my $line = <$fh>) {
        chomp $line;
        die "blarg" unless $line =~ /^\s*(.*?)\s*$/;
        next if $files_to_remove->{$1};
        print $out "$line\n";
    }
    close($fh);
    close($out);

    rename($tmp_name, $cmakelists);

    return;
}

sub process_file {
    my $path = $_;
    return if ! -f $path;
    return if not $path =~ /^.*\.cpp$/;

    my $fullpath = $File::Find::name;

    my $state = 'before';

    open(my $fh, '<', $path) or die "Failed to open $path: $!";
    while (my $line = <$fh>) {
        chomp $line;
        if ($state eq 'before') {
            if ($line eq 'namespace rwe') {
                $state = 'open';
            }
        }
        elsif ($state eq 'open') {
            if ($line eq '{') {
                $state = 'in'
            }
            else {
                die "Expected '{' in $fullpath at line $.";
            }
        }
        elsif ($state eq 'in') {
            if ($line eq '}') {
                $empty_files{$fullpath} = 1;
            }
            return;
        }
    }

    return;
}

__END__

=head1 NAME

remove-empty-cpp-files.pl - Remove empty .cpp files

=head1 SYNOPSIS

    remove-empty-cpp-files.pl

=head1 DESCRIPTION

This script deletes empty cpp files and removes them
from CMakeLists.txt
These empty files can be created by the IDE
but then never actually used.
