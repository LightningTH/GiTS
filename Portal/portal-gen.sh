#!/bin/bash
rm images/*
rm portal-gits.avi
rm portal-gits.ogv

python portal-gen-pngs.py
#do two conversions as a straight conversion results in a rainbow effect on the characters
#due to some improper decode/encode of png to ogv
avconv -f image2 -framerate 30 -i images/frame%03d.png -vcodec copy portal-gits.avi
avconv -i portal-gits.avi -r 30 -q 10 -force_fps portal-gits.ogv

rm images/*
rm portal-gits.avi
