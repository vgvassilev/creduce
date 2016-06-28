## -*- mode: Perl -*-
##
## Copyright (c) 2012, 2013, 2015 The University of Utah
## All rights reserved.
##
## This file is distributed under the University of Illinois Open Source
## License.  See the file COPYING for details.

###############################################################################

package pass_include_includes;

use strict;
use warnings;

use File::Copy;
use File::Spec;
use creduce_utils;

my $MAX_INC_SIZE = 1000;

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

    open INF, "<$cfile" or die;
    my $tmpfile = File::Temp::tmpnam();
    open OUTF, ">$tmpfile" or die;
    my $includes = 0;
    my $matched;
    while (my $line = <INF>) {
        if ($line =~ /^\s*#\s*include\s*(\"|\<)(.*?)(\"|\>)/) {
	    my $incfile = $2;
	    print "found include file '$incfile' (included by $cfile) \n" if $DEBUG;
            $includes++;
            if ($includes == $index) {
                if (!-r $incfile) {
                    # Repect the include paths if present:
                    my @creduce_inc_paths = split(/:/, $ENV{CREDUCE_INCLUDE_PATH});
                    foreach my $incdir (@creduce_inc_paths) {
                        $incdir = File::Spec->rel2abs($incdir);

                        next if !-d $incdir;

                        my @found;
                        my $base_name = File::Basename::basename($incfile);

                        File::Find::find(sub { push @found,
                                               $File::Find::name if (/$base_name/i)
                                         },
                                         $incdir);
                        print "INCFILE2 $incfile \n" if $DEBUG;
                        print "FOUND: @found \n" if $DEBUG;
                        if (@found) {
                            $incfile = $found[0];
                            last;
                        }
                    }
                }
		if ((-r $incfile) &&
		    ((-s $incfile) < $MAX_INC_SIZE) &&
		    (open INC, "<$incfile")) {
		    print "  including it\n" if $DEBUG;
		    $matched = 1;
		    while (my $l = <INC>) {
			print OUTF $l;
		    }
		    close INC;
		    next;
		}
            }
        }
        print OUTF $line;
    }

    close INF;
    close OUTF;
    if ($matched) {
        File::Copy::move($tmpfile, $cfile);
    } else {
        unlink $tmpfile;
    }
    return $matched;
}

sub transform ($$$) {
    (my $cfile, my $arg, my $state) = @_;
    my $index = ${$state};

    my $success;
    $success = do_transform($cfile, $index);
    return ($success ? $OK : $STOP, \$index);
}

1;
