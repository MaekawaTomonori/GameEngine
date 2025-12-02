#!/usr/bin/perl
use strict;
use warnings;

# XML-style to Doxygen comment converter - Version 4
# Handles multi-line summaries and proper param/return placement

my $in_summary = 0;
my @buffer;  # Buffer to collect comment lines before output

while (<>) {
    my $line = $_;

    # Opening <summary> tag - start collecting
    if ($line =~ /^(\s*)\/\/\/ <summary>\s*$/) {
        my $indent = $1;
        $in_summary = 1;
        @buffer = ();
        push @buffer, {indent => $indent, type => 'start'};
        next;
    }

    # Closing </summary> tag
    if ($line =~ /^(\s*)\/\/\/ <\/summary>\s*$/) {
        $in_summary = 0;
        next;  # Don't close yet - wait for param/returns
    }

    # Content lines inside summary
    if ($in_summary && $line =~ /^(\s*)\/\/\/ (.+)$/) {
        push @buffer, {indent => $1, type => 'content', text => $2};
        next;
    }

    # <param> tag (single line)
    if ($line =~ /^(\s*)\/\/\/ <param name="([^"]+)">([^<]*)<\/param>\s*$/) {
        my ($indent, $param_name, $description) = ($1, $2, $3);
        my $desc = $description ? " $description" : "";
        push @buffer, {indent => $indent, type => 'param', name => $param_name, desc => $desc};
        next;
    }

    # <returns> tag (single line)
    if ($line =~ /^(\s*)\/\/\/ <returns>([^<]*)<\/returns>\s*$/) {
        my ($indent, $description) = ($1, $2);
        my $desc = $description ? " $description" : "";
        push @buffer, {indent => $indent, type => 'return', desc => $desc};
        next;
    }

    # End of comment block - flush buffer
    if (@buffer && $line !~ /^(\s*)\/\/\//) {
        # Output buffered comment
        my $indent = $buffer[0]->{indent};

        # Start with /** @brief and first content line
        print "$indent/** \@brief";
        my $first_content = 1;

        foreach my $item (@buffer) {
            if ($item->{type} eq 'content') {
                if ($first_content) {
                    print " $item->{text}\n";
                    $first_content = 0;
                } else {
                    print "$indent ** $item->{text}\n";
                }
            }
        }

        # Then param/return lines
        foreach my $item (@buffer) {
            if ($item->{type} eq 'param') {
                print "$indent ** \@param $item->{name}$item->{desc}\n";
            } elsif ($item->{type} eq 'return') {
                print "$indent ** \@return$item->{desc}\n";
            }
        }

        # Close with **/
        print "$indent **/\n";

        @buffer = ();
    }

    # Default: print line as-is
    print $line;
}

# Flush any remaining buffer at end of file
if (@buffer) {
    my $indent = $buffer[0]->{indent};
    print "$indent/** \@brief";
    my $first_content = 1;

    foreach my $item (@buffer) {
        if ($item->{type} eq 'content') {
            if ($first_content) {
                print " $item->{text}\n";
                $first_content = 0;
            } else {
                print "$indent ** $item->{text}\n";
            }
        }
    }

    foreach my $item (@buffer) {
        if ($item->{type} eq 'param') {
            print "$indent ** \@param $item->{name}$item->{desc}\n";
        } elsif ($item->{type} eq 'return') {
            print "$indent ** \@return$item->{desc}\n";
        }
    }

    print "$indent **/\n";
}
