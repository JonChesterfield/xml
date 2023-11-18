#!/bin/bash

if [[ -f "relaxng.rng" ]]
then
    echo "Using existing relaxng.rng"
else
    echo "Fetch relaxng.rng"
    wget https://relaxng.org/jclark/relaxng.rng
fi

if [[ -f "xslt.rng" ]]
then
    echo "Using existing xslt.rng"
else
    echo "Fetch xslt.rng"
    wget https://relaxng.org/jclark/xslt.rng
fi

# This would rebuild the etc/schema/xslt.rnc in emacs exactly:
if false ; then
trang xslt.rng wip-xslt.rnc
sed -i 's$ns1$xsl$g' wip-xslt.rnc

cat << EOF > emacs-xslt.rnc
# Copyright (C) 2001-2008 World Wide Web Consortium, (Massachusetts
# Institute of Technology, European Research Consortium for
# Informatics and Mathematics, Keio University).  All Rights Reserved.
# This work is distributed under the W3C(R) Software License in the
# hope that it will be useful, but WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.

EOF
cat wip-xslt.rnc >> emacs-xslt.rnc
fi

# The xslt.rng file doesn't round trip through trang as the
# first conversion introduces a namespace. It does converge,
# with that namespace introduced, subsequent round trips are equal

if false ; then
# This is noticably different to the one in emacs, would guess emacs'
# version is newer based on the structure
trang relaxng.rng relaxng.rnc
fi


# Sanity check, is the relaxng schema an instance of the relaxng schema?
xmllint --relaxng relaxng.rng relaxng.rng --noout

# Second sanity check, is the xslt schema an instance of the relaxng schema?
xmllint --relaxng relaxng.rng xslt.rng --noout

