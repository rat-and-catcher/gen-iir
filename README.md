# gen-iir -- tiny IIR filter calculator

This small CLI program is some front-end to DSPL (www.dsplib.org)
IIR filter designer (by analogue prototype only).

The program can be build under Linux (gcc) or Windows (both gcc (MinGW)
and Microsoft Visual C).

## Some notes:

* The program can type filter coefficients in different forms,
including hexadecimal floats and integer dumps of doubles to
eliminate lost of significant digits.

* According DSPL rules under Windows libdspl.dll should be
placed or along gen-iir.exe or somewhere to folder in your %PATH%.
Under Linux the easiest way is to place both gen-iir executable and
libdspl.so into current folder. See www.dsplib.org or sources for
details.

For usage information, please run

```
	gen-iir -h
```

* If you run the program without arguments it should calculate
some default filter.

* While gen-iir code is public domain, DSPL parts has some license
conditions, see src/dspl/LICENSE

* Just in case we add usable source of dspl (https://github.com/Dsplib/libdspl-2.0)
downloaded from github.com 31-aug-2024 into release X01.01.00. Note
thar our fork of libdspl-2 was removed from github due to original repo
looks now alive.

NOTE::the top (narrow TW@elliptic-filters) produce different results
in Windows MinGW64 and MinGW32 bits. All of the things seems related
to different FP implementation (FPU-80 bit on 32-bit and SSE2 on 64-bit
implementations). To have "exactly" double coeffs. please reimplement
this under high prec arythmetics.

WARNING::while using the tool we found, that absolutely legal
filter designs with narrow (or wide) pass band (0.1- or 0.9+ or so
for LPF) whith not so high order (10-15+) bring unstable
implementation (even for Batterworth approximation). Filters near
half-band works just fine (how fine?). For example, the next designs
doesnt work:

```
gen-iir -x -n:15 -s:60 -l:0.05 -a:butter -t:lpf
gen-iir -x -n:15 -s:60 -l:0.1 -a:ellip -t:lpf
```

Filters calculators by other tools (demo programs or commercial
products; we dont take into a count Matlab) give soficient results
in the case, but them have other disadvantages.

It seems to us, that the problem related to polyroots() function
algorithm, but we unsure. We implement quick'n'dirty test
("-x" option) for the check. So, if you need robust IIR calculator,
you, probably should to continue your search..

