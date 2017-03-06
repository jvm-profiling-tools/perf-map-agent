#!/usr/bin/perl

use warnings;
no warnings 'portable';
use strict;

my $num_args = $#ARGV + 1;
if ($num_args != 1) {
    print "\nUsage: $0 perf-*.map\n";
    exit;
}

my $map_file = $ARGV[0];
my @map_entries;

open(my $fh, '<:encoding(UTF-8)', $map_file) or die "Could not open file '$map_file' $!";

# Create map of perf-*.map file entries.
while (my $row = <$fh>) {
  chomp $row;
  my @parts = split / /, $row;
  my $address = hex($parts[0]);
  my $size = hex($parts[1]);
  my $method = $parts[2];
  my @entry = (); 

  push @entry, $address;
  push @entry, $size;
  push @entry, $method;
  push @map_entries, [@entry];
}

# Process STDIN and replace unkown methods with these provided by the perf-*.map file.
while (my $row = <STDIN>) {
    chomp $row;
    if ($row =~ "^              0x(.+)") {
        my $addr = hex($1);
        my $size = -1;
        foreach my $e (@map_entries) {
            my $entry_addr = $e->[0];
            my $entry_size = $e->[1];

            # First check if we had a match before that had a smaller entry_size. If so its a better match and we should just continue.
            if (($size == -1 || $entry_size < $size) && $addr >= $entry_addr && $addr <= ($entry_addr + $entry_size)) {
                $row = "              $e->[2]";
                $size = $entry_size;
            } 
        } 
    }
    print "$row\n"; 
}
