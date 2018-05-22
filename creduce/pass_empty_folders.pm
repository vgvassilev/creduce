## -*- mode: Perl -*-
##
## Copyright (c) 2012, 2013, 2015, 2016 The University of Utah
## All rights reserved.
##
## This file is distributed under the University of Illinois Open Source
## License.  See the file COPYING for details.

###############################################################################

package pass_empty_folders;

use strict;
use warnings;

use File::Basename;
use File::Spec;
use creduce_utils;

sub check_prereqs () {
    return 1;
}

sub new ($$) {
    my $index = 1;
    return \$index;
}

sub advance ($$$) {
    (my $cfile, my $arg, my $state) = @_;
    my $index = ${$state};
    $index++;
    return \$index;
}

sub advance_on_success ($$$) {
    (my $cfile, my $arg, my $state) = @_;
    return $state;
}

# Create a hash map used as a bit set.
my %visited_folders;

sub do_transform($$) {
    my ($cfile, $index) = @_;

    my $dir = dirname($cfile);
    my $rel_dir = File::Spec->abs2rel($dir, File::Spec->curdir());

    return 0 if $visited_folders{$rel_dir} && $visited_folders{$rel_dir} == 1;

    my $old_size = 0;
    print "Emptying all files in: $dir at index: $index \n" if $DEBUG;
    while (defined(my $file = glob "$dir/*")) {
        # Do nothing if this is a folder or an empty file.
        next if -d $file || -z $file;
        $old_size += -s $file;
        print "  Emptying $file \n";
        open INF, ">$file" or die "$file: $!";
        close INF;
    }
    $visited_folders{$rel_dir} = 1;
    my $bitset_size = scalar keys %visited_folders;
    print "Bitset size: $bitset_size \n" if $DEBUG;
    # Return success only if the size is smaller and the pass is run for the
    # first time on this file.
    return 1;
}

# This pass just empties all files in the contained folder.
sub transform ($$$) {
    (my $cfile, my $arg, my $state) = @_;
    my $index = ${$state};

    return $STOP if $index != 1;

    my $success;
    $success = do_transform($cfile, $index);
    return ($success ? $OK : $STOP, \$index);
}

1;
