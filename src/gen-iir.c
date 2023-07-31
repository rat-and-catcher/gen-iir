/*
 * Create IIR filter via Digital Signal Processing Library Dspl-2.0
 * Please see http://dsplib.org for details.
 *
 * Author Rat And Catcher tech., 2023.
 * This trivial code is public domain.
 * It comes without any warranty.
 */

// nobody die from this
#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#if !defined(_CRT_NON_CONFORMING_SWPRINTFS)
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>

#include "dspl/dspl.h"

// the program version
const char gi_ver[] = "X01.00.00";

// the double parameter type
typedef struct s_double_p
{
 double val;                                // the value
 int is_set;                                // bool - true, if set by user
} t_double_p;

// the int parameter type
typedef struct s_int_p
{
 int val;                                   // the value
 int is_set;                                // bool - true, if set by user
} t_int_p;

// map C-string to the int value
typedef struct s_str2val
{
 const char *str;                           // the ID
 int val;                                   // associated value
 const char *ename;                         // external name or so
} t_str2val;

// filter types
const char ename_lpf[]    = "RP_IIR_LPF";   // for consumer code
const char ename_hpf[]    = "RP_IIR_HPF";
const char ename_bpf[]    = "RP_IIR_BPF";
const char ename_bsf[]    = "RP_IIR_BSF";

const t_str2val f_types[] =
{
 {  "LPF",          DSPL_FILTER_LPF,    ename_lpf   },  // [0] - default

 {  "HPF",          DSPL_FILTER_HPF,    ename_hpf   },  // [1]

 {  "BPF",          DSPL_FILTER_BPASS,  ename_bpf   },  // [2]
 {  "BPASS",        DSPL_FILTER_BPASS,  ename_bpf   },  // [3]

 {  "BSF",          DSPL_FILTER_BSTOP,  ename_bsf   },  // [4]
 {  "BSTOP",        DSPL_FILTER_BSTOP,  ename_bsf   },  // [5]
 {  "NOTCH",        DSPL_FILTER_BSTOP,  ename_bsf   },  // [6]

 {  NULL,           -1               ,  NULL        }
};

// filter approximation kinds
const char ename_butter[] = "RP_IIR_BUTTER";    // for consumer code
const char ename_cheby1[] = "RP_IIR_CHEBY1";
const char ename_cheby2[] = "RP_IIR_CHEBY2";
const char ename_ellip [] = "RP_IIR_ELLIP" ;

const t_str2val f_aprxs[] =
{
 {  "BUTTER",       DSPL_FILTER_BUTTER, ename_butter  },  // [0] - default
 {  "Batterworth",  DSPL_FILTER_BUTTER, ename_butter  },  // [1]

 {  "CHEBY1",       DSPL_FILTER_CHEBY1, ename_cheby1  },  // [2]
 {  "CHEB1",        DSPL_FILTER_CHEBY1, ename_cheby1  },  // [3]
 {  "Chebyshev1",   DSPL_FILTER_CHEBY1, ename_cheby1  },  // [4]

 {  "CHEBY2",       DSPL_FILTER_CHEBY2, ename_cheby2  },  // [5]
 {  "CHEB2",        DSPL_FILTER_CHEBY2, ename_cheby2  },  // [6]
 {  "Chebyshev2",   DSPL_FILTER_CHEBY2, ename_cheby2  },  // [7]

 {  "ELLIP",        DSPL_FILTER_ELLIP , ename_ellip   },  // [8]
 {  "Elliptic",     DSPL_FILTER_ELLIP , ename_ellip   },  // [9]

 {  NULL,           -1                , NULL          }
};

// output types
#define GEN_IIR_RAW             (0)         /* raw list of coefficients */
#define GEN_IIR_CDEC            (1)         /* C-code in conventional decimal form */
#define GEN_IIR_CHEXF           (2)         /* C-code in 0xh.hhhPee (%A) form */
#define GEN_IIR_CHEXUI          (3)         /* C-code with doubles as uint64_t dumps */

const t_str2val l_types[] =
{
 {  "RAW",          GEN_IIR_RAW       , NULL          },  // [0] - default
 {  "CDEC",         GEN_IIR_CDEC      , NULL          },  // [1]
 {  "CHEXF",        GEN_IIR_CHEXF     , NULL          },  // [2]
 {  "CHEXUI",       GEN_IIR_CHEXUI    , NULL          },  // [3]
 {  NULL,           -1                , NULL          }
};

// -- the default values --
// filter parameter values -- more or less from libdspl-2.0/examples/src/iir-test.c
#define IX_F_TYPES_DEFAULT      (0)     /* index in f_types - 0 always safe */
#define IX_F_APRXS_DEFAULT      (0)     /* index in f_aprxs - 0 always safe */
#define F_ORDER_DEFAULT         (6)     /* filter order, *2 for default BPF/BSF */
#define F_SUPPRESSION_DEFAULT   (60.0)  /* stop-band suppression, dB */
#define F_RIPPLE_DEFAULT        (2.5)   /* pass-band ripple, dB */
#define F_CUTOFF_DEFAULT        (0.3)   /* cutoff freq, norm. to 0..1 */
#define F_CUTOFF_RIGHT_DEFAULT  (0.6)   /* "right" cutoff freq, norm. to 0..1, BPF/BSF only */
#define IX_GEN_IIR_DEFAULT      (0)     /* index of def. output */

// parameters table -- fiil by init_parms(), recalc by parse_cmd_line() + norm_parms()
typedef struct s_parms
{
 t_int_p        show_ver;               // -v               :: show version, no processing
 t_int_p        show_help;              // -h               :: show help, no processing
 t_int_p        show_c_defs;            // -j               :: show C-defs for -o:C*, no processing
 t_double_p     do_dbl_bin;             // -c:<dbl>         :: double hex calculator, no processing
 t_int_p        ix_f_type;              // -t:LPF-or-so     :: index in f_types[]
 t_int_p        ix_f_aprx;              // -a:BUTTER-or-so  :: index in f_aprxs[]
 t_int_p        f_order;                // -n:<int>         :: filter order
 t_double_p     f_suppr;                // -s:<dbl>         :: suppression in stop band, dB
 t_double_p     f_ripple;               // -p:<dbl>         :: max ripple in pass band, dB
 t_double_p     f_cutoff;               // -l:<dbl>         :: [left] cutoff freq, (0..1)
 t_double_p     f_cutoff_right;         // -r:<dbl>         :: right cutoff freq, (0..1), band only
 t_int_p        ix_lis_type;            // -o:RAW-or-so     :: index of type of output in l_types[]
 t_int_p        lis_wide;               // -w               :: output all doubles representation
} t_parms;

// filter implementation (all that was calculated)
typedef struct s_iir_impl
{
 int size;                              // size of a and b == order + 1
 double tw;                             // norm. transition width
 double *a;                             // denominator
 double *b;                             // numerator
} t_iir_impl;

// misc. stuff
#if defined(_WIN32)
#define STRCASECMP      _stricmp
#define SNPRINTF        _snprintf
#define FMT_X64         "%I64X"
#else
#define STRCASECMP      strcasecmp
#define SNPRINTF        snprintf
#define FMT_X64         "%llX"
#endif

// type error message to stderr and exit
void error(const char *fmtmsg, ...)
{
 va_list args;
 va_start(args, fmtmsg);

 vfprintf(stderr, fmtmsg, args);
 exit(1);

 va_end(args);
}

// malloc() with clear memory, which always ok
void *cmalloc(size_t size)
{
 void *res = malloc(size);

 if(NULL == res)
  error("??Out of memory, need %u bytes\n", (unsigned)size);

 memset(res, 0, size);
 return res;
}

// create parameters table, the result should be free()
t_parms *init_parms(void)
{
 t_parms *parms = cmalloc(sizeof(t_parms));

#define INI_PARM(P_, V_)    do { parms -> P_ .val = (V_); parms -> P_ .is_set = 0; } while(0)

 INI_PARM(show_ver,         0                     );    // -v               :: show version, no processing
 INI_PARM(show_help,        0                     );    // -h               :: show help, no processing
 INI_PARM(show_c_defs,      0                     );    // -j               :: show C-defs for -o:C*, no processing
 INI_PARM(do_dbl_bin,       0.0                   );    // -c:<dbl>         :: double hex calculator, no processing
 INI_PARM(ix_f_type,        IX_F_TYPES_DEFAULT    );    // -t:LPF-or-so     :: index in f_types[]
 INI_PARM(ix_f_aprx,        IX_F_APRXS_DEFAULT    );    // -a:BUTTER-or-so  :: index in f_aprxs[]
 INI_PARM(f_order,          F_ORDER_DEFAULT       );    // -n:<int>         :: filter order
 INI_PARM(f_suppr,          F_SUPPRESSION_DEFAULT );    // -s:<dbl>         :: suppression in stop band, dB
 INI_PARM(f_ripple,         F_RIPPLE_DEFAULT      );    // -p:<dbl>         :: max ripple in pass band, dB
 INI_PARM(f_cutoff,         F_CUTOFF_DEFAULT      );    // -l:<dbl>         :: [left] cutoff freq, (0..1)
 INI_PARM(f_cutoff_right,   F_CUTOFF_RIGHT_DEFAULT);    // -r:<dbl>         :: right cutoff freq, (0..1), band only
 INI_PARM(ix_lis_type,      IX_GEN_IIR_DEFAULT    );    // -o:RAW-or-so     :: index of type of output in l_types[]
 INI_PARM(lis_wide,         0                     );    // -w               :: output all doubles representation

#undef  INI_PARM

 return parms;
}

// command line parser helpers
// -- multiply definitions warning
void warn_mult(const char *p)
{
 printf("!!Warning: multiply definitition of \'%s\'\n", p);
}

// -- parse key's parameter
double parse_parm(const char *p, const t_str2val *sv /* NULL for numerical value */)
{
 // double exactly any int value. p[1] is not a '\0', p[2] should be ':' or '='.
 // the parameter string should begin from p[3].
 if(p[2] != ':' && p[2] != '=')
  error("??Wrong value delimiter at \'%s\'\n", p);

 if(sv)
 {
  // string with case ignore
  int i;

  for(i = 0; sv[i].str; ++i)
  {
   if(0 == STRCASECMP(p + 3, sv[i].str))
    return (double)i;                           // it's safe
  }
  // not found
  fprintf(stderr
       , "??The key \'%s\' has a bad value.\n"
         "The only possible values are:\n"
       , p);
  for(i = 0; sv[i].str; ++i)
  {
   fprintf(stderr, "    -%c:%s\n", p[1], sv[i].str);
  }
  error("??Please specify correct value\n");
 }
 else
 {
  // real or integer number
  double res;

  errno = 0;                                    // wow!
  res = strtod(p + 3, NULL);

  if(errno)
   error("??The key \'%s\' has bad numerical value\n", p);

  return res;
 }

 return -1.0;                                   // unreachable
}

// command line parsing
void parse_cmd_line(int argc, char **argv, t_parms *parms)
{
 int i;

 for(i = 1; i < argc; ++i)
 {
  const char *ps = argv[i];

  if(NULL == ps)
   error("??Something goes wrong in program arguments parsing\n");
  if('-' == *ps)
  {
   switch(ps[1])
   {
#define UPD_PARM(C_, P_, V_)                                \
    case C_ :                                               \
     if(parms -> P_ .is_set) warn_mult(ps);                 \
     parms -> P_ .val    = (V_);  parms -> P_ .is_set = 1;  \
     break

    UPD_PARM('v', show_ver,           1                        ); // -v               :: show version, no processing
    UPD_PARM('h', show_help,          1                        ); // -h               :: show help, no processing
    UPD_PARM('j', show_c_defs,        1                        ); // -j               :: show C-defs for -o:C*, no processing
    UPD_PARM('c', do_dbl_bin,         parse_parm(ps, NULL     )); // -c:<dbl>         :: double hex calculator, no processing
    UPD_PARM('t', ix_f_type,    (int) parse_parm(ps, f_types  )); // -t LPF-or-so     :: index in f_types[]
    UPD_PARM('a', ix_f_aprx,    (int) parse_parm(ps, f_aprxs  )); // -a BUTTER-or-so  :: index in f_aprxs[]
    UPD_PARM('n', f_order,      (int) parse_parm(ps, NULL     )); // -n <int>         :: filter order
    UPD_PARM('s', f_suppr,            parse_parm(ps, NULL     )); // -s <dbl>         :: suppression in stop band, dB
    UPD_PARM('p', f_ripple,           parse_parm(ps, NULL     )); // -p <dbl>         :: max ripple in pass band, dB
    UPD_PARM('l', f_cutoff,           parse_parm(ps, NULL     )); // -l <dbl>         :: [left] cutoff freq, (0..1)
    UPD_PARM('r', f_cutoff_right,     parse_parm(ps, NULL     )); // -r <dbl>         :: right cutoff freq, (0..1), band only
    UPD_PARM('o', ix_lis_type,  (int) parse_parm(ps, l_types  )); // -o RAW-or-so     :: index of type of output in l_types[]
    UPD_PARM('w', lis_wide,           1                        ); // -w               :: output all doubles representation

#undef  UPD_PARM

    default:
     error("??Wrong key: \'%s'; use -h for help\n", ps);
     break;
   }
  }
  else
  {
   error("??Wrong parameter: \'%s\'; use -h for help\n", ps);
  }
 }
}

// check and "normalize" parameters
void norm_parms(t_parms *parms)
{
 int is_band_filter = ((DSPL_FILTER_BPASS == f_types[parms -> ix_f_type.val].val)
    ||  (DSPL_FILTER_BSTOP == f_types[parms -> ix_f_type.val].val));

 // "service" operations has priority and does not need filter's parameters check
 if(    parms -> show_ver   .is_set                   // if .is_set then .val == 1
    ||  parms -> show_help  .is_set
    ||  parms -> show_c_defs.is_set
    ||  parms -> do_dbl_bin .is_set
   )
 {
  return;
 }

 // "band" filters specials
 if(is_band_filter)
 {
  if(parms -> f_order.is_set)
  {
   // user supplied order should be even
   if(parms -> f_order.val & 1)
   {
    fprintf(stderr
        , "!!Warning: the order of band pass/stop filter should be even: %d -> %d\n"
        , parms -> f_order.val
        , parms -> f_order.val + 1);
    ++(parms -> f_order.val);
   }
  }
  else
  {
   // if config has default order && filter has "band" type -> double default order value
   parms -> f_order.val *= 2;                       // true default value
  }
 }
 else
 {
  parms -> f_cutoff_right.val = 0.0;                // LPF/HPF does not have right cutoff
 }

 // very roughly boundary checks. We keep some space to trigger internal DSPL limits
 if(parms -> f_order.val < 1 || parms -> f_order.val > 1000)
 {
  error("??Nonsence filter order specified: %d\n", parms -> f_order.val);
 }
 if(parms -> f_suppr.val <= 0.0 || parms -> f_suppr.val > 500.0)
 {
  error("??Filter supression %G should be in (0..400) dB\n", parms -> f_suppr.val);
 }
 if(parms -> f_ripple.val <= 0.0 || parms -> f_ripple.val > parms -> f_suppr.val)
 {
  error("??Ripple %G should be in in (0..<suppression> == %G) dB\n"
    , parms -> f_ripple.val
    , parms -> f_suppr.val);
 }
 if(parms -> f_cutoff.val <= 0.0 || parms -> f_cutoff.val >= 1.0)
 {
  error("?? [left] Cutoff freq should be in (0..1)\n", parms -> f_cutoff.val);
 }
 // "right" cutoff freq. has meaning only for band filter
 if(is_band_filter
    && ((parms -> f_cutoff_right.val <= parms -> f_cutoff.val)
        ||
        (parms -> f_cutoff_right.val >= 1.0)
       )
   )
 {
  error("??Riht cutoff freq should be in (0..1)\n", parms -> f_cutoff_right.val);
 }
}

// check our system to support "%A" printf format
int chk_a_printf(void)                  // !=0 -> %A supported
{
 int res = 0;
 char buf[64];
 static const char *afmt[] = { "%a", "%A", NULL };
 int i;

 for(i = 0; afmt[i]; ++i)
 {
  char *p;

  errno = 0;
  SNPRINTF(buf, sizeof(buf) - sizeof(int) /*hmm..*/, afmt[i], 3.14E42);
  if(errno != 0)                            // errno no set for unknown format!
   break;

  // silly
  for(p = buf; *p; ++p)
  {
   if('P' == *p || 'p' == *p || '.' == *p)
    ++res;                                  // += 2 for each test format;
  }                                         // shoul be 4 at the end
 }
 return (res == 4);                         // silly idiotic compare
}

// show program/dspl version
void print_version(void)
{
 printf("gen-iir %s -- DSPL-2 - based (see www.dsplib.org) IIR filters disigner.\n"
        "The front-end CLI program written by Rat-and-Catcher tech., 2023\n"
        "Internal DSPL info:\n"
        , gi_ver);

 fflush(stdout);
 dspl_info();
 fflush(stdout);
 printf("Plaese use \'gen-iir -h\' for help\n");
}

// show help
void print_help(void)
{
 printf("Usage:\n"
        "gen-iir -v     -- print version info;\n"
        "or\n"
        "gen-iir -h     -- print this text;\n"
        "or\n"
        "gen-iir -j [-o:<listing-type>]\n"
        "             -- print C-definitions for selected ouput mode\n"
        "                (see below);\n"
        "or\n"
        "gen-iir -c:<legal-floating-number>\n"
        "               -- to show the number in different forms\n"
        "or, to compute IIR-filter coefficients via DSPL library:\n"
        "gen-iir [-t:<filter-type>] [-a:<approx-type] [-n:<order>] \\\n"
        "   [-s:<suppression-dB>] [-p:<ripple-dB>] \\\n"
        "   [-l:<cutoff>] [-r:<right-cutoff>] \\\n"
        "   [-o:<listing-type>] [-w]\n"
        "Note, that all filter design and listing parameters are optional.\n\n");
 printf("<filter-type> can be:\n"
        "%s (default)\t\t-- Low Pass Filter,\n"
        "%s\t\t\t-- High Pass Filter,\n"
        "%s or %s\t\t-- Band Pass Filter,\n"
        "%s or %s or %s\t-- Band Stop Filter.\n\n"
        , f_types[0].str
        , f_types[1].str
        , f_types[2].str
        , f_types[3].str
        , f_types[4].str
        , f_types[5].str
        , f_types[6].str);
 printf("<approx-type> can be:\n"
        "%s or %s (default),\n"
        "%s or %s or %s,\n"
        "%s or %s or %s,\n"
        "%s or %s,\n"
        "for Batterworth, Chebyshev1/2 and Elliptic approximations.\n\n"
        , f_aprxs[0].str
        , f_aprxs[1].str
        , f_aprxs[2].str
        , f_aprxs[3].str
        , f_aprxs[4].str
        , f_aprxs[5].str
        , f_aprxs[6].str
        , f_aprxs[7].str
        , f_aprxs[8].str
        , f_aprxs[9].str);
 printf("<order> (default %d) -- filter order, integer >=1;\n"
        "   should be even for \"band\" types filters\n\n"
        , F_ORDER_DEFAULT);
 printf("<suppression-dB> (default %.1f dB)\n"
        " -- suppression in stop band -- dB, float >0.0\n\n"
        , F_SUPPRESSION_DEFAULT);
 printf("<ripple-dB> (default %.1f dB)\n"
        " -- maximum ripple level in pass band -- dB, float (0.0 .. <suppression-dB>)\n\n"
        , F_RIPPLE_DEFAULT);
 printf("<cutoff> (default %.2f)\n"
        " -- cutoff normalized frequency, float (0.0 .. 1.0);\n"
        "    (left cutoff for \"band\" types filters\n\n"
        , F_CUTOFF_DEFAULT);
 printf("<right-cutoff> (default %.2f)\n"
        " -- right cutoff normalized frequency, float (<cutoff> .. 1.0);\n"
        "    (specified only for \"band\" types filters)\n\n"
        , F_CUTOFF_RIGHT_DEFAULT);
 printf("<listing-type> can be:\n"
        "%s (default)\t-- simply coefficient list in conventional floats,\n"
        "%s\t\t-- C-source in conventional floats,\n"
        "%s\t\t-- C-source in hexadecimal floats (if supported),\n"
        "%s\t\t-- C-source in hexadecimal floats as uint64_t dumps.\n\n"
        , l_types[0].str
        , l_types[1].str
        , l_types[2].str
        , l_types[3].str);
 printf("-w (\"wide output\") turn on output of filter coefficients\n"
        "   to all available forms\n\n");
 printf("Please note, that all \"floats\" represented as double (64 bits) values.\n"
        "You can use \'=\' against \':\' in values specifications, but the\n"
        "blanks in '-<key>:<value>' are inadmissible. Note also, that <value> is\n"
        "always not case sensible, but <key> letters are sensible.\n"
        "For further details please visit www.dsplib.org\n");
}

// type a C-header (const(s) and type(s)) for filter design output
void print_c_header(const t_parms *parms)
{
 if(GEN_IIR_CHEXUI == parms -> ix_lis_type.val)
 {
  printf("#include <stdint.h>\n\n");
 }
 printf("// -- gen-iir (DSPL, www.dsplib.org) filter descriptor\n"
        "// filter types:\n"
        "#define RP_IIR_TYPE_SHIFT  (0)\n"
        "#define RP_IIR_TYPE_MASK   (0xFF << RP_IIR_TYPE_SHIFT)\n"
        "#define %s       (0 << RP_IIR_TYPE_SHIFT)\n"
        "#define %s       (1 << RP_IIR_TYPE_SHIFT)\n"
        "#define %s       (2 << RP_IIR_TYPE_SHIFT)\n"
        "#define %s       (3 << RP_IIR_TYPE_SHIFT)\n"
        "// approximation types:\n"
        "#define RP_IIR_APPROX_SHIFT  (8)\n"
        "#define RP_IIR_APPROX_MASK   (0xFF << RP_IIR_APPROX_SHIFT)\n"
        "#define %s       (0 << RP_IIR_APPROX_SHIFT)\n"
        "#define %s       (1 << RP_IIR_APPROX_SHIFT)\n"
        "#define %s       (2 << RP_IIR_APPROX_SHIFT)\n"
        "#define %s       (3 << RP_IIR_APPROX_SHIFT)\n"
        , ename_lpf
        , ename_hpf
        , ename_bpf
        , ename_bsf
        , ename_butter
        , ename_cheby1
        , ename_cheby2
        , ename_ellip
 );
 printf("// the filter (design + coefficients):\n"
        "typedef struct s_rp_iir_filter_descr\n"
        "{\n"
        "  unsigned flags;            // RP_IIR_LPF-or-so | RP_IIR_BUTTER-or-so\n"
        "  int      order;\n"
        "  double   suppression;\n"
        "  double   rillpe;\n"
        "  double   left_cutoff;\n"
        "  double   right_cutoff;\n"
        "  double   transition_width;\n"
 );
 printf(GEN_IIR_CHEXUI == parms -> ix_lis_type.val?
        "  uint64_t *vec_b;           // order + 1 values\n"
        "  uint64_t *vec_a;           // order + 1 values\n"
        :
        "  double   *vec_b;           // order + 1 values\n"
        "  double   *vec_a;           // order + 1 values\n"
 );
 printf("} RP_IIR_FILTER_DESCR;\n\n");
}

// double - unsigned hex converter (be careful with *buf)
char *dbl2hb(double val, char *buf, size_t buflen)
{
 union du
 {
  double d;
  uint64_t u;
 } d2u64;

 // some silly sanity check, we love to be silly
 if(    sizeof(union du) != sizeof(double)
    ||  sizeof(union du) != sizeof(uint64_t)
    ||  sizeof(double)   != sizeof(uint64_t))
 {
  error("\n??Something terrible with sizeofs / alignments:\n"
        "sizeof(double) = %u; sizeof(uint64_t) = %u;\n"
        "sizeof(double|uint64_t) = %u\n"
        , (unsigned)sizeof(double)
        , (unsigned)sizeof(uint64_t)
        , (unsigned)sizeof(union du));
 }

 d2u64.d = val;
 SNPRINTF(  buf
          , buflen - sizeof(int)
          , FMT_X64
          , ((unsigned long long) d2u64.u) & 0xFFFFFFFFFFFFFFFFULL);

 return buf;
}

// type double value in different forms
void print_hex_calc(double val, int is_afmt)
{
 char buf[64];

 if(is_afmt)
 {
  printf("dec: %25.17E; hex: %23.14A; bin: 0x%s\n"
    , val
    , val
    , dbl2hb(val, buf, sizeof(buf)));
 }
 else
 {
  printf("dec: %25.17E; bin: 0x%s\n"
    , val
    , dbl2hb(val, buf, sizeof(buf)));
 }
}

// print one vector of coefficients
void print_fcvector(const t_parms *parms, const double *vec, int size, int is_afmt)
{
 int i;
 char buf[64];
 const char err_int_msg[] = "??Wrong listing type -- internal error\n";

 for(i = 0; i < size; ++i)
 {
  if(is_afmt)
  {
   switch(l_types[parms -> ix_lis_type.val].val)
   {
    case GEN_IIR_RAW:
     if(parms -> lis_wide.val)
     {
      printf("#%2d: %25.17E; hex: %23.14A; bin: 0x%s\n"
        , i
        , vec[i]
        , vec[i]
        , dbl2hb(vec[i], buf, sizeof(buf)));
     }
     else
     {
      printf("#%2d: %25.17E\n", i, vec[i]);
     }
     break;

    case GEN_IIR_CDEC:
     if(parms -> lis_wide.val)
     {
      printf("    %25.17E%c  // [%2d] hex: %23.14A; bin: 0x%s\n"
        , vec[i]
        , i != size - 1? ',' : ' '
        , i
        , vec[i]
        , dbl2hb(vec[i], buf, sizeof(buf)));
     }
     else
     {
      printf("    %25.17E%c   // [%2d]\n", vec[i], i != size - 1? ',' : ' ', i);
     }
     break;

    case GEN_IIR_CHEXF:
     if(parms -> lis_wide.val)
     {
      printf("    %23.14A%c  // [%2d] dec: %25.17E; bin: 0x%s\n"
        , vec[i]
        , i != size - 1? ',' : ' '
        , i
        , vec[i]
        , dbl2hb(vec[i], buf, sizeof(buf)));
     }
     else
     {
      printf("    %23.14A%c   // [%2d] dec: %25.17E\n"
        , vec[i]
        , i != size - 1? ',' : ' '
        , i
        , vec[i]);
     }
     break;

    case GEN_IIR_CHEXUI:
     if(parms -> lis_wide.val)
     {
      printf("    0x%sU%c  // [%2d] dec: %25.17E; hex: %23.14A\n"
        , dbl2hb(vec[i], buf, sizeof(buf))
        , i != size - 1? ',' : ' '
        , i
        , vec[i]
        , vec[i]);
     }
     else
     {
      printf("    0x%sU%c   // [%2d] dec: %25.17E\n"
        , dbl2hb(vec[i], buf, sizeof(buf))
        , i != size - 1? ',' : ' '
        , i
        , vec[i]);
     }
     break;

    default:
     error(err_int_msg);
     break;
   }
  }
  else
  {
   // "%A" not working
   switch(l_types[parms -> ix_lis_type.val].val)
   {
    case GEN_IIR_RAW:
     if(parms -> lis_wide.val)
     {
      printf("#%2d: %25.17E; bin: 0x%s\n"
        , i
        , vec[i]
        , dbl2hb(vec[i], buf, sizeof(buf)));
     }
     else
     {
      printf("#%2d: %25.17E\n", i, vec[i]);
     }
     break;

    case GEN_IIR_CDEC:
    case GEN_IIR_CHEXF:
     if(parms -> lis_wide.val)
     {
      printf("    %25.17E%c  // [%2d] bin: 0x%s\n"
        , vec[i]
        , i != size - 1? ',' : ' '
        , i
        , dbl2hb(vec[i], buf, sizeof(buf)));
     }
     else
     {
      printf("    %25.17E%c   // [%2d]\n", vec[i], i != size - 1? ',' : ' ', i);
     }
     break;

    case GEN_IIR_CHEXUI:
     if(parms -> lis_wide.val)
     {
      printf("    0x%s%c  // [%2d] dec: %25.17E\n"
        , dbl2hb(vec[i], buf, sizeof(buf))
        , i != size - 1? ',' : ' '
        , i
        , vec[i]);
     }
     else
     {
      printf("    0x%s%c   // [%2d] dec: %25.17E\n"
        , dbl2hb(vec[i], buf, sizeof(buf))
        , i != size - 1? ',' : ' '
        , i
        , vec[i]);
     }
     break;

    default:
     error(err_int_msg);
     break;
   }
  }
 }
}

// print complete filter info
void print_filter(const t_parms *parms, const t_iir_impl *impl, int is_afmt)
{
 int is_band_filter = ((DSPL_FILTER_BPASS == f_types[parms -> ix_f_type.val].val)
    ||  (DSPL_FILTER_BSTOP == f_types[parms -> ix_f_type.val].val));
 const char *array_cast =
       GEN_IIR_CHEXUI == l_types[parms -> ix_lis_type.val].val?
       "(uint64_t [])"
       :
       "(double [])";

 // header
 if(GEN_IIR_RAW == parms -> ix_lis_type.val)
 {
  printf("** Filter defign:\n"
         "-- Type: %s; Approximation: %s;\n"
         "-- Filter order: %d (%d coefficients in numerator / denominator);\n"
         "-- Suppression in stop band %.4f dB;\n"
         "-- Max. ripple in pass band %.4f dB;\n"
         "-- %s frequency: %.8G ([0..1] norm.)\n"
    , f_types[parms -> ix_f_type.val].str
    , f_aprxs[parms -> ix_f_aprx.val].str
    , parms -> f_order  .val
    , impl  -> size
    , parms -> f_suppr  .val
    , parms -> f_ripple .val
    , is_band_filter? "Left cutoff" : "Cutoff"
    , parms -> f_cutoff .val
  );
  if(is_band_filter)
  {
   printf("-- Right cutoff frequency: %.8G ([0..1] norm.)\n"
    , parms -> f_cutoff_right.val);
  }
  printf("-- Transition width %.8G (??unsure-R&C) ([0..1] norm.)\n"
         "\n** Numerator coefficients (B[%d]):\n"
    , impl  -> tw
    , impl -> size);
 }
 else
 {
  printf("RP_IIR_FILTER_DESCR rp_iir_filter =   // TODO: CHANGE THE NAME!:\n"
         "{\n"
         "// -- the design parameters:\n"
         "  %s | %s,\n"
         "  %d,       // order\n"
         "  %.4f,     // suppression\n"
         "  %.4f,     // ripple\n"
         "  %.8G,     // left cutoff\n"
         "  %.8G,     // right cutoff\n"
         "  %.8G,     // (??unsure-R&C) transition width\n"
         "// Numerator coefficients (B[%d]):\n"
         "  %s                            // !! not MSVC friendly !!\n"
         "  {\n"
    , f_types[parms -> ix_f_type.val].ename
    , f_aprxs[parms -> ix_f_aprx.val].ename
    , parms -> f_order        .val
    , parms -> f_suppr        .val
    , parms -> f_ripple       .val
    , parms -> f_cutoff       .val
    , parms -> f_cutoff_right .val
    , impl  -> tw
    , impl  -> size
    , array_cast
  );
 }

 // numerator vector
 print_fcvector(parms, impl -> b, impl -> size, is_afmt);

 // num / den separator
 if(GEN_IIR_RAW == parms -> ix_lis_type.val)
 {
  printf("\n** Denominator coefficients (A[%d]):\n", impl -> size);
 }
 else
 {
  printf("  },\n"
         "// Denominator coefficients (A[%d]):\n"
         "  %s                            // !! not MSVC friendly !!\n"
         "  {\n"
    , impl  -> size
    , array_cast
  );
 }

 // denominator vector
 print_fcvector(parms, impl -> a, impl -> size, is_afmt);

 // footer
 if(GEN_IIR_RAW == parms -> ix_lis_type.val)
 {
  printf("\n** It\'s your filter.\n");
 }
 else
 {
  printf("  }\n};\n\n");
 }
}

// the main
int main(int argc, char **argv)
{
 void *hdspl    = dspl_load();
 t_parms *parms = init_parms();
 int is_afmt    = chk_a_printf();

 if(NULL == hdspl)
  error("Can't load libdspl DLL / shared object\n");

 parse_cmd_line(argc, argv, parms);
 norm_parms(parms);

 if(parms -> show_ver.val)
 {
  print_version();
 }
 else if(parms -> show_help.val)
 {
  print_help();
 }
 else if(parms -> show_c_defs.val)
 {
  print_c_header(parms);
 }
 else if(parms -> do_dbl_bin.is_set)
 {
  print_hex_calc(parms -> do_dbl_bin.val, is_afmt);
 }
 else
 {                                          // build the filter
  t_iir_impl impl;

  impl.size   = parms -> f_order.val + 1;
  impl.tw     = -1.0;
  impl.a      = cmalloc(impl.size * sizeof(double));
  impl.b      = cmalloc(impl.size * sizeof(double));

  int res =   iir
              ( parms -> f_ripple         .val
              , parms -> f_suppr          .val
              , parms -> f_order          .val
              , parms -> f_cutoff         .val
              , parms -> f_cutoff_right   .val
              ,      f_types[parms -> ix_f_type.val].val
                  |  f_aprxs[parms -> ix_f_aprx.val].val
              , impl.b
              , impl.a
              );
  if(res != RES_OK)
  {
   fprintf(stderr, "Oops... DSPL irr() return error. The code: 0x%08X\n", res);
  }
  else
  {
   int an_order = ((DSPL_FILTER_BPASS == f_types[parms -> ix_f_type.val].val)
                      ||  (DSPL_FILTER_BSTOP == f_types[parms -> ix_f_type.val].val))?
                    parms -> f_order.val / 2
                    :
                    parms -> f_order.val;

   // it seems return stop freauency for analog LPF with pass freq. 0.5?? rad/sec
   impl.tw = filter_ws1
              ( an_order
              , parms -> f_ripple .val
              , parms -> f_suppr  .val
              ,      f_types[parms -> ix_f_type.val].val
                  |  f_aprxs[parms -> ix_f_aprx.val].val
   );
   if(impl.tw < 0.0)
   {
    fprintf(stderr, "!!Warning: can\'t to compute transition width\n");
   }
   else
   {
    // this, probably, incorrect -- obtained during some meditation
    impl.tw = (2.0 * atan(impl.tw) / M_PI - 0.5);
   }
   print_filter(parms, &impl, is_afmt);
  }

  free(impl.b);
  impl.b = NULL;
  free(impl.a);
  impl.a = NULL;
 }

 // shutdown
 if(parms)
 {
  free(parms);
  parms = NULL;
 }
 if(hdspl)
 {
  dspl_free(hdspl);
  hdspl = NULL;
 }
 return 0;
}


/* the End..
*/

