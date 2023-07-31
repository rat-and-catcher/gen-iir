#!/bin/sh
# build Hilbert transformer filters for in_cwave.dll plugin

gen-iir -j -o:chexui

gen-iir -w -o:chexui -t:lpf -a:ellip -p:1.5 -s:100 -n:15 -l:0.495
gen-iir -w -o:chexui -t:lpf -a:ellip -p:1.8 -s:100 -n:19 -l:0.499
gen-iir -w -o:chexui -t:lpf -a:ellip -p:2.0 -s:90  -n:18 -l:0.499

gen-iir -w -o:chexui -t:lpf -a:ellip -p:2.0 -s:96  -n:19 -l:0.4982
gen-iir -w -o:chexui -t:lpf -a:ellip -p:2.0 -s:100 -n:20 -l:0.4982
gen-iir -w -o:chexui -t:lpf -a:ellip -p:2.0 -s:100 -n:20 -l:0.4985
