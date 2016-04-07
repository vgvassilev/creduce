#!/bin/bash -x

### Author Vassil Vassilev ###

function sanity_check() {
    rm -fr ./mod_cache/*
    ./test.sh
    echo $?
}

# Find all folders to reduce.
DIRS=$(find includes -type d)
#DIRS="includes/Users"
# Reduce the modulemap ~/workspace/github/creduce/cmake_obj/creduce/creduce --tidy --no-default-passes --add-pass pass_balanced curly-only 1 test.sh TStopwatch.cxx includes/*
# perl -0777 -i -pe 's/(\{[^{}]+\})/\{\}/s' includes/usr/include/module.modulemap
# Modulemaps should be reduced first because their submodules retain headers and
# folders
MMAPS=$(find includes -name "module.*")
for i in $MMAPS ; do
    # Make sure the setup is correct.
    if [[ $(sanity_check) != 0 ]]; then
        echo "Broken test.\n"
        exit 1
    fi
    #-pie is your standard "replace in place" command-line sequence, and -0777
    #causes perl to slurp files whole.
    # FIXME: We should be able to tell perl which occurence it should remove,
    # otherwise like this is useless.
    perl -0777 -i.original -pe 's/(\{[^{}]+\})/\{\}/s' "$i"
    if [[ $(sanity_check) != 0 ]]; then
        echo "Revert $i"
        mv "$i.original" "$i"
    fi
    
done;

counter=0
for i in $DIRS ; do
    # Check we are not trying to remove a subfolder of already removed one.
    # Eg. /tmp/foo was removed and now we are trying to remove /tmp/foo/bar.
    if [ ! -d "$i" ]; then
        continue
    fi

    # Make sure the setup is correct.
    if [[ $(sanity_check) != 0 ]]; then
        echo "Broken test.\n"
        exit 1
    fi

    TMPDIR=`mktemp -d`
    # Move the folder elsewhere.
    mv "$i" "$TMPDIR/"
    echo "Moving $i to $TMPDIR"

    # If the move was wrong, move it back.
    if [[ $(sanity_check) != 0 ]]; then
        echo "Move back $i from $TMPDIR/`basename $i`"
        mv "$TMPDIR/`basename $i`" "$i"
    fi
    rm -fr "$TMPDIR"
done;

exit 0
