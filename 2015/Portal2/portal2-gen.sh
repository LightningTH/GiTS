#!/bin/bash
rm images/*
rm portal2-gits.avi
#rm portal2-gits.ogv
rm portal2-gits-final.avi

python portal2-gen-pngs.py
avconv -f image2 -framerate 30 -i images/frame%04d.png -vcodec copy portal2-gits.avi
#avconv -i portal2-gits.avi -r 30 -q 10 -force_fps portal2-gits.ogv
avconv -i Waste\ Your\ Time.flac -i portal2-gits.avi -r 30 -q 4 -force_fps -map 1:v -map 0:a:0 portal2-gits-final.avi

rm images/*
rm portal2-gits.avi
