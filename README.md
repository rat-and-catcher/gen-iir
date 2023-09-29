## It's work!

NOTE::the top (narrow TW@elliptic-filters) produce different results
in Windows MinGW64 and MinGW32 bits. All of the things seems related
to different FP implementation (FPU-80 bit on 32-bit and SSE2 on 64-bit
implementations). To have "exactly" double coeffs. please reimplement
this under high prec arythmetics.

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

	gen-iir -h

* If you run the program without arguments it should calculate
some default filter.

* While gen-iir code is public domain, DSPL parts has some license
conditions, see src/dspl/LICENSE
