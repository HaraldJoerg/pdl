#ifndef __PDLCORE_H
#define __PDLCORE_H

/*
# version 2 is for versions after PDL 2.1.1
# version 4 has pdl_hard_copy included in the Core structure.
# version 6 is introduced after 2.4.2, due to the experimental
#   per-ndarray bad values code (the BADVAL_PER_PDL option)
# version 7 introduced for some changes to function prototypes
#   for pthreading (i.e. multi-threading) capabilities
# version 8 for beginning support for >2GiB ndarrays
# version 9 for STRLEN/Size_t/Off_t for mmap delete magic
# version 10 for 64bit index support (PDL index datatype)
# version 11 for core cleanup (proto-PDL-3)
# version 12 for PDL_Anyval union data type (full 64bit support)
# version 13 for complex data types (native complex support)
# version 14: NaN support for native complex, per-PDL badval on
# version 15: threadid, ndims become PDL_Indx
# version 16: zap tmp, flushcache, NaN_*, qsort_ for complex, add_deletedata_magic
# version 17: restore add_deletedata_magic
# version 18: zap twod, complex sorters
*/

#define PDL_CORE_VERSION 18

#include "EXTERN.h"   /* std perl include */
#include "perl.h"     /* std perl include */
#include "XSUB.h"  /* for the win32 perlCAPI crap */
#include "ppport.h"  /* include this AFTER XSUB.h */

#if defined(CONTEXT) && defined(__osf__)
#undef CONTEXT
#endif

#ifdef PDL_IN_CORE
#define PDL_CORE_(func) pdl_##func
#else
#define PDL_CORE_(func) PDL->func
#endif

#include "pdl.h"
#include "pdlperl.h"
#include "pdlthread.h"
/* the next one causes trouble in c++ compiles - exclude for now */
#ifndef __cplusplus
#include "pdlmagic.h"
#endif

#define PDL_TMP  0        /* Flags */
#define PDL_PERM 1

#define BIGGESTOF(a,b) ( a->nvals>b->nvals ? a->nvals : b->nvals )
#define SVavref(x) (SvROK(x) && SvTYPE(SvRV(x))==SVt_PVAV)

/*  Use our own barf and our own warn.
 *  We defer barf (and warn) handling until after multi-threaded (i.e pthreading)
 *  processing is finished.
 *  This is needed because segfaults happen when perl's croak is called
 *  during one of the spawned pthreads for PDL processing.
 */
#define barf PDL_CORE_(pdl_barf)
#undef warn
#define warn PDL_CORE_(pdl_warn)

#define PDL_XS_SCALAR(type, ctype, val) \
  SV *b_SV = NULL; \
  pdl  *b; \
  int nreturn = 1; \
  int ndims = 0; \
  PDL_Indx *pdims; \
  pdims = (PDL_Indx *) pdl_smalloc( ndims * sizeof(*pdims) ); \
  b = pdl_create(PDL_PERM); \
  pdl_setdims(b, pdims, ndims); \
  b->datatype = type; \
  pdl_allocdata(b); \
  ((ctype *)b->data)[0] = val; \
  b_SV = sv_newmortal(); \
  pdl_SetSV_PDL(b_SV, b); \
  if (nreturn > 0) EXTEND(SP, nreturn); \
  ST(0) = b_SV; \
  XSRETURN(nreturn);

typedef int Logical;

/*************** Function prototypes *********************/

/* pdlcore.c */

size_t  pdl_howbig (int datatype);           /* Size of data type (bytes) */
pdl*    pdl_SvPDLV ( SV* sv );               /* Map SV* to pdl struct */
void	pdl_SetSV_PDL( SV *sv, pdl *it );     /* Outputting a pdl from.. */
PDL_Indx *    pdl_packdims ( SV *sv, PDL_Indx *ndims ); /* Pack dims[] into SV aref */
void*   pdl_smalloc ( STRLEN nbytes );           /* malloc memory - auto free()*/

void pdl_makescratchhash(pdl *ret, PDL_Anyval data);
PDL_Indx pdl_safe_indterm(PDL_Indx dsz, PDL_Indx at, char *file, int lineno);
/* doubled for binary-compat with redundantly-named Core member */
void pdl_pdl_barf(const char* pat,...); /* General croaking utility */
void pdl_pdl_warn(const char* pat,...); /* General warn utility */
PDL_Indx av_ndcheck(AV* av, AV* dims, int level, int *datalevel);
pdl* pdl_from_array(AV* av, AV* dims, int type, pdl* p);

PDL_Indx  pdl_get_offset(PDL_Indx* pos, PDL_Indx* dims, PDL_Indx *incs, PDL_Indx offset, PDL_Indx ndims);      /* Offset of pixel x,y,z... */
PDL_Indx  pdl_validate_section( PDL_Indx* sec, PDL_Indx* dims,           /* Check section */
                           PDL_Indx ndims );
void pdl_row_plusplus ( PDL_Indx* pos, PDL_Indx* dims,              /* Move down one row */
                        PDL_Indx ndims );
PDL_Anyval pdl_at0( pdl* x );
PDL_Anyval pdl_at( void* x, int datatype, PDL_Indx* pos, PDL_Indx* dims, /* Value at x,y,z,... */
             PDL_Indx *incs, PDL_Indx offset, PDL_Indx ndims);
void  pdl_set( void* x, int datatype, PDL_Indx* pos, PDL_Indx* dims, /* Set value at x,y,z... */
                PDL_Indx *incs, PDL_Indx offs, PDL_Indx ndims, PDL_Anyval value);

#define X(symbol, ctype, ppsym, shortctype, defbval) \
PDL_Indx pdl_setav_ ## ppsym(ctype * pdata, AV* av, \
	PDL_Indx* pdims, PDL_Long ndims, int level, ctype undefval, pdl *p);
    PDL_GENERICLIST(X)
#undef X

/* pdlapi.c */

void pdl_vaffinechanged(pdl *it, int what);
void pdl_trans_mallocfreeproc(struct pdl_trans *tr);
void pdl_make_trans_mutual(pdl_trans *trans);
void pdl_destroytransform_nonmutual(pdl_trans *trans,int ensure);

void pdl_vafftrans_free(pdl *it);
void pdl_vafftrans_remove(pdl * it);
void pdl_make_physvaffine(pdl *it);
void pdl_vafftrans_alloc(pdl *it);

pdl *pdl_null();
pdl *pdl_get_convertedpdl(pdl *pdl,int type);

void pdl_destroytransform(pdl_trans *trans,int ensure);

pdl *pdl_hard_copy(pdl *src);

pdl* pdl_pdlnew();
pdl* pdl_tmp();
pdl* pdl_create(int type);
void pdl_destroy(pdl *it);
void pdl_setdims(pdl* it, PDL_Indx* dims, PDL_Indx ndims);
void pdl_reallocdims(pdl *it, PDL_Indx ndims); /* reallocate dims and incs */
void pdl_reallocthreadids(pdl *it, PDL_Indx ndims); /* reallocate threadids */
void pdl_resize_defaultincs ( pdl *it );     /* Make incs out of dims */
void pdl_unpackarray(HV* hash, char *key, PDL_Indx *dims, PDL_Indx ndims);
void pdl_dump(pdl *it);
void pdl_allocdata(pdl *it);

PDL_Indx *pdl_get_threadoffsp(pdl_thread *thread); /* For pthreading */
void pdl_thread_copy(pdl_thread *from,pdl_thread *to);
void pdl_clearthreadstruct(pdl_thread *it);
void pdl_initthreadstruct(int nobl,pdl **pdls,PDL_Indx *realdims,PDL_Indx *creating, PDL_Indx npdls,
	pdl_errorinfo *info,pdl_thread *thread,char *flags, int noPthreadFlag );
int pdl_startthreadloop(pdl_thread *thread,void (*func)(pdl_trans *),pdl_trans *);
int pdl_iterthreadloop(pdl_thread *thread, PDL_Indx which);
void pdl_freethreadloop(pdl_thread *thread);
void pdl_thread_create_parameter(pdl_thread *thread,PDL_Indx j,PDL_Indx *dims,
				 int temp);
void pdl_croak_param(pdl_errorinfo *info, int paramIndex, char *pat, ...);

void pdl_setdims_careful(pdl *pdl);
void pdl_put_offs(pdl *pdl,PDL_Indx offs, PDL_Anyval val);
PDL_Anyval pdl_get_offs(pdl *pdl,PDL_Indx offs);
PDL_Anyval pdl_get(pdl *pdl,PDL_Indx *inds);
void pdl_set_trans(pdl *it, pdl *parent, pdl_transvtable *vtable);

void pdl_make_physical(pdl *it);
void pdl_make_physdims(pdl *it);

void pdl_children_changesoon(pdl *it, int what);
void pdl_changed(pdl *it, int what, int recursing);
void pdl_separatefromparent(pdl *it);

void pdl_trans_changesoon(pdl_trans *trans,int what);
void pdl_trans_changed(pdl_trans *trans,int what);

void pdl_set_trans_childtrans(pdl *it, pdl_trans *trans, PDL_Indx nth);
void pdl_set_trans_parenttrans(pdl *it, pdl_trans *trans, PDL_Indx nth);

void pdl_set_datatype(pdl *a, int datatype);
pdl *pdl_sever(pdl *a);

void pdl_propagate_badflag (pdl *it, int newval);
void pdl_propagate_badvalue (pdl *it);
PDL_Anyval pdl_get_pdl_badvalue(pdl *it);
PDL_Anyval pdl_get_badvalue(int datatype);

/* pdlconv.c */

void pdl_writebackdata_vaffine(pdl *it);
void pdl_readdata_vaffine(pdl *it);

void   pdl_swap(pdl** a, pdl** b);             /* Swap two pdl ptrs */
void   pdl_converttype( pdl** a, int targtype, /* Change type of a pdl */
                        Logical changePerl );
void   pdl_grow  ( pdl* a, PDL_Indx newsize);   /* Change pdl 'Data' size */

/* Structure to hold pointers core PDL routines so as to be used by many modules */

#if defined(PDL_clean_namespace) || defined(PDL_OLD_API)
#error PDL_clean_namespace and PDL_OLD_API defines have been removed. Use PDL->pdlnew() instead of PDL->new().
#endif

struct Core {
    I32    Version;
    pdl*   (*SvPDLV)      ( SV*  );
    void   (*SetSV_PDL)   ( SV *sv, pdl *it );
    pdl*   (*pdlnew)      ( );
    pdl*   (*create)      (int type);
    void   (*destroy)     (pdl *it);
    pdl*   (*null)        ();
    pdl*   (*hard_copy)   ( pdl* );
    void   (*converttype) ( pdl**, int, Logical );
    void*  (*smalloc)      ( STRLEN );
    size_t (*howbig)      ( int );
    PDL_Indx*   (*packdims)    ( SV* sv, PDL_Indx *ndims ); /* Pack dims[] into SV aref */
    void   (*setdims)     ( pdl* it, PDL_Indx* dims, PDL_Indx ndims );
    void   (*grow)        ( pdl* a, PDL_Indx newsize); /* Change pdl 'Data' size */
    PDL_Anyval (*at0)     ( pdl* x );
    void (*reallocdims) ( pdl *it,PDL_Indx ndims );  /* reallocate dims and incs */
    void (*reallocthreadids) ( pdl *it,PDL_Indx ndims );
    void (*resize_defaultincs) ( pdl *it );     /* Make incs out of dims */

void (*thread_copy)(pdl_thread *from,pdl_thread *to);
void (*clearthreadstruct)(pdl_thread *it);
void (*initthreadstruct)(int nobl,pdl **pdls,PDL_Indx *realdims,PDL_Indx *creating,PDL_Indx npdls,
	pdl_errorinfo *info,pdl_thread *thread,char *flags, int noPthreadFlag );
int (*startthreadloop)(pdl_thread *thread,void (*func)(pdl_trans *),pdl_trans *);
PDL_Indx *(*get_threadoffsp)(pdl_thread *thread); /* For pthreading */
int (*iterthreadloop)(pdl_thread *thread, PDL_Indx which);
void (*freethreadloop)(pdl_thread *thread);
void (*thread_create_parameter)(pdl_thread *thread,PDL_Indx j,PDL_Indx *dims,
				int temp);
void (*add_deletedata_magic) (pdl *it,void (*func)(pdl *, Size_t param), Size_t param); /* Automagic destructor */

/* XXX NOT YET IMPLEMENTED */
void (*setdims_careful)(pdl *pdl);
void (*put_offs)(pdl *pdl,PDL_Indx offs, PDL_Anyval val);
PDL_Anyval (*get_offs)(pdl *pdl,PDL_Indx offs);
PDL_Anyval (*get)(pdl *pdl,PDL_Indx *inds);
void (*set_trans_childtrans)(pdl *it, pdl_trans *trans,PDL_Indx nth);
void (*set_trans_parenttrans)(pdl *it, pdl_trans *trans,PDL_Indx nth);
pdl *(*make_now)(pdl *it);

pdl *(*get_convertedpdl)(pdl *pdl,int type);

void (*make_trans_mutual)(pdl_trans *trans);

/* Affine trans. THESE ARE SET IN ONE OF THE OTHER Basic MODULES
   and not in Core.xs ! */
void (*readdata_affine)(pdl_trans *tr);
void (*writebackdata_affine)(pdl_trans *tr);
void (*affine_new)(pdl *par,pdl *child,PDL_Indx offs,SV *dims,SV *incs);

/* Converttype. Similar */
void (*converttypei_new)(pdl *par,pdl *child,int type);

void (*trans_mallocfreeproc)(struct pdl_trans *tr);

void (*make_physical)(pdl *it);
void (*make_physdims)(pdl *it);
void (*pdl_barf) (const char* pat,...);
void (*pdl_warn) (const char* pat,...);

void (*make_physvaffine)(pdl *it);
void (*allocdata) (pdl *it);
PDL_Indx (*safe_indterm)(PDL_Indx dsz, PDL_Indx at, char *file, int lineno);

#define X(symbol, ctype, ppsym, shortctype, defbval) \
void (*qsort_ ## ppsym) (ctype *xx, PDL_Indx a, PDL_Indx b ); \
void (*qsort_ind_ ## ppsym) (ctype *xx, PDL_Indx *ix, PDL_Indx a, PDL_Indx b );
PDL_GENERICLIST(X)
#undef X

  badvals bvals;  /* store the default bad values */
  void (*propagate_badflag) (pdl *it, int newval);
  void (*propagate_badvalue) (pdl *it);
  void (*children_changesoon)(pdl *it, int what);
  void (*changed)(pdl *it, int what, int recursing);
  void (*vaffinechanged)(pdl *it, int what);
  PDL_Anyval (*get_pdl_badvalue)(pdl *it);
  void (*set_datatype)(pdl *a, int datatype);
  pdl *(*sever)(pdl *a);
};

typedef struct Core Core;

/* __PDLCORE_H */
#endif