## -*- mode: Perl -*-
##
## Copyright (c) 2012, 2013, 2015, 2016 The University of Utah
## All rights reserved.
##
## This file is distributed under the University of Illinois Open Source
## License.  See the file COPYING for details.

###############################################################################

package pass_empty_files;

use strict;
use warnings;

use File::Copy;
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

sub do_transform($$) {
    my ($cfile, $index) = @_;

    my $old_size = -s $cfile;
    print "Emptying file: $cfile at index: $index \n" if $DEBUG;
    open INF, ">$cfile" or die "$cfile: $!";
    close INF;
    # Return success only if the size is smaller and the pass is run for the
    # first time on this file.
    return (-s $cfile) < $old_size && $index == 1;
}

# This pass just empties the contents of the files.
sub transform ($$$) {
    (my $cfile, my $arg, my $state) = @_;
    my $index = ${$state};

    my $success;
    $success = do_transform($cfile, $index);
    return ($success ? $OK : $STOP, \$index);
}

1;
