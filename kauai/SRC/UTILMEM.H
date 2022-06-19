/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Memory handling

***************************************************************************/
#ifndef UTILMEM_H
#define UTILMEM_H

// used for asserts and limiting memory
const byte kbGarbage = 0xA3;    // new blocks are filled with this
const long kcbMax = 0x08000000; // 128 Megabytes
const short kswMagicMem = (short)0xA253;
const long klwMagicMem = (long)0xA253A253;

/***************************************************************************
    When an allocation fails, vpfnlib is called to free some memory (if it's
    not nil).
***************************************************************************/
typedef long (*PFNLIB)(long cb, long mpr);
extern PFNLIB vpfnlib;
extern bool _fInAlloc;

/****************************************
    OS memory handles and management
****************************************/
#ifdef MAC
typedef Handle HN;
// version of SetHandleSize that returns an error code
inline short ErrSetHandleSize(HN hn, Size cb)
{
    SetHandleSize(hn, cb);
    return MemError();
}

// address stipper
class ADST
{
  private:
    long _lwMaskAddress;

  public:
    ADST(void);

    void *PvStrip(void *pv)
    {
        return (void *)((long)pv & _lwMaskAddress);
    }
};
extern ADST vadst;

#elif defined(WIN)
typedef HGLOBAL HN;
#endif

/****************************************
    Moveable/resizeable memory management
****************************************/
typedef void *HQ;
#define hNil 0
#define hqNil ((HQ)0)

// memory request priority
enum
{
    // lower priority
    mprDebug,
    mprForSpeed,
    mprNormal,
    mprCritical,
    // higher priority
};

// memory allocation options
enum
{
    fmemNil = 0,
    fmemClear = 1,
};

void FreePhq(HQ *phq);
long CbOfHq(HQ hq);
bool FCopyHq(HQ hqSrc, HQ *phqDst, long mpr);
bool FResizePhq(HQ *phq, long cb, ulong grfmem, long mpr);
void *PvLockHq(HQ hq);
void UnlockHq(HQ hq);

#ifdef DEBUG

// debug memory allocator globals
// enter vmutxMem before modifying these...
struct DMAGL
{
    long cv;    // number of allocations
    long cvTot; // total number of allocations over all time
    long cvRun; // running max of cv
    long cb;    // total size of allocations
    long cbRun; // running max of cb

    long cactDo;   // number of times to succeed before failing
    long cactFail; // number of times to fail

    bool FFail(void);
    void Allocate(long cbT);
    void Resize(long dcb);
    void Free(long cbT);
};

// debug memory globals
struct DMGLOB
{
    DMAGL dmaglBase; // for NewObj
    DMAGL dmaglHq;   // for HQs
    DMAGL dmaglPv;   // for FAllocPv, etc
};
extern DMGLOB vdmglob;

extern long vcactSuspendCheckPointers;
#define SuspendCheckPointers() vcactSuspendCheckPointers++;
#define ResumeCheckPointers() vcactSuspendCheckPointers--;

bool FAllocHqDebug(HQ *phq, long cb, ulong grfmem, long mpr, schar *pszsFile, long lwLine);
#define FAllocHq(phq, cb, grfmem, mpr) FAllocHqDebug(phq, cb, grfmem, mpr, __szsFile, __LINE__)
void *QvFromHq(HQ hq);

void AssertHq(HQ hq);
void MarkHq(HQ hq);
#ifdef MAC
void _AssertUnmarkedHqs(void);
void _UnmarkAllHqs(void);
#endif // MAC

#else //! DEBUG

#define FAllocHqDebug(phq, cb, grfmem, mpr, pszsFile, luLine) FAllocHq(phq, cb, grfmem, mpr)
bool FAllocHq(HQ *phq, long cb, ulong grfmem, long mpr);
#ifdef MAC
inline void *QvFromHq(HQ hq)
{
    return vadst.PvStrip(*(void **)hq);
}
#elif defined(WIN)
inline void *QvFromHq(HQ hq)
{
    return (void *)hq;
}
#endif // WIN

#define AssertHq(hq)
#define MarkHq(hq)

#endif //! DEBUG

/****************************************
    Fixed (non-moveable) memory.
****************************************/
#ifdef DEBUG

// allocation routine
bool FAllocPvDebug(void **ppv, long cb, ulong grfmem, long mpr, schar *pszsFile, long lwLine, DMAGL *pdmagl);
#define FAllocPv(ppv, cb, grfmem, mpr) FAllocPvDebug(ppv, cb, grfmem, mpr, __szsFile, __LINE__, &vdmglob.dmaglPv)

// resizing routine - WIN only
#ifdef WIN
bool _FResizePpvDebug(void **ppv, long cbNew, long cbOld, ulong grfmem, long mpr, DMAGL *pdmagl);
#endif // WIN

// freeing routine
void FreePpvDebug(void **ppv, DMAGL *pdmagl);
#define FreePpv(ppv) FreePpvDebug(ppv, &vdmglob.dmaglPv)

void AssertPvAlloced(void *pv, long cb);
void AssertUnmarkedMem(void);
void UnmarkAllMem(void);
void MarkPv(void *pv);

#else //! DEBUG

#define SuspendCheckPointers()
#define ResumeCheckPointers()

// allocation routine
#define FAllocPvDebug(ppv, cb, grfmem, mpr, pszsFile, luLine, pdmagl) FAllocPv(ppv, cb, grfmem, mpr)
bool FAllocPv(void **ppv, long cb, ulong grfmem, long mpr);

// resizing routine - WIN only
#ifdef WIN
#define _FResizePpvDebug(ppv, cbNew, cbOld, grfmem, mpr, pdmagl) _FResizePpv(ppv, cbNew, cbOld, grfmem, mpr)
bool _FResizePpv(void **ppv, long cbNew, long cbOld, ulong grfmem, long mpr);
#endif // WIN

// freeing routine
#define FreePpvDebug(ppv, pdmagl) FreePpv(ppv)
void FreePpv(void **ppv);

#define AssertPvAlloced(pv, cb)
#define AssertUnmarkedMem()
#define UnmarkAllMem()
#define MarkPv(pv)
#endif //! DEBUG

/****************************************
    Memory trashing
****************************************/
#ifdef DEBUG

#define TrashVar(pfoo)                                                                                                 \
    if (pvNil != (pfoo))                                                                                               \
        FillPb(pfoo, size(*(pfoo)), kbGarbage);                                                                        \
    else                                                                                                               \
        (void)0
#define TrashVarIf(f, pfoo)                                                                                            \
    if ((f) && pvNil != (pfoo))                                                                                        \
        FillPb(pfoo, size(*(pfoo)), kbGarbage);                                                                        \
    else                                                                                                               \
        (void)0
#define TrashPvCb(pv, cb)                                                                                              \
    if (pvNil != (pv))                                                                                                 \
        FillPb(pv, cb, kbGarbage);                                                                                     \
    else                                                                                                               \
        (void)0
#define TrashPvCbIf(f, pv, cb)                                                                                         \
    if ((f) && pvNil != (pv))                                                                                          \
        FillPb(pv, cb, kbGarbage);                                                                                     \
    else                                                                                                               \
        (void)0

#else //! DEBUG

#define TrashVar(pfoo)
#define TrashVarIf(f, pfoo)
#define TrashPvCb(pv, cb)
#define TrashPvCbIf(f, pv, cb)

#endif //! DEBUG

/****************************************
    Pointer arithmetic
****************************************/
inline void *PvAddBv(void *pv, long bv)
{
    return (byte *)pv + bv;
}
inline void *PvSubBv(void *pv, long bv)
{
    return (byte *)pv - bv;
}
inline long BvSubPvs(void *pv1, void *pv2)
{
    return (byte *)pv1 - (byte *)pv2;
}

/****************************************
    Mutex (critical section) object
****************************************/
typedef class MUTX *PMUTX;
class MUTX
{
  protected:
#ifdef WIN
    CRITICAL_SECTION _crit;
#endif // WIN

  public:
    MUTX(void)
    {
        Win(InitializeCriticalSection(&_crit);)
    }
    ~MUTX(void)
    {
        Win(DeleteCriticalSection(&_crit);)
    }

    void Enter(void)
    {
        Win(EnterCriticalSection(&_crit);)
    }
    void Leave(void)
    {
        Win(LeaveCriticalSection(&_crit);)
    }
};

extern MUTX vmutxMem;

/****************************************
    Current thread id
****************************************/
inline long LwThreadCur(void)
{
    return MacWin(0, GetCurrentThreadId());
}

/***************************************************************************
    Pointer to blob of data. A convenience class to combine a pointer with
    a size in bytes. This class does not own any data, therefore the data
    will not be deleted.
***************************************************************************/
class BlobPtr
{
public:
    BlobPtr() = default;
    BlobPtr(void* ptr, size_t size) : _ptr(ptr), _size(size) { }

    template<typename T>
    static BlobPtr Make(T& data, size_t count = 1)
    {
        return BlobPtr(&data, sizeof(T) * count);
    }

    template<typename T>
    T* Cast(size_t count = 1) const
    {
        AssertH(_size >= sizeof(T) * count);
        return reinterpret_cast<T*>(_ptr);
    }

    explicit operator bool() const
    {
        return _ptr != pvNil;
    }

    void* Ptr() const
    {
        return _ptr;
    }

    size_t Size() const
    {
        return _size;
    }

private:
    void* _ptr = pvNil;
    size_t _size = 0;
};

/****************************************
    GLPtr provides a template-typed wrapper around a PGL.
****************************************/
// XXX: In order to escape from #include hell, typedef FP is duplicated here.
// The original comes from FILE.H.
typedef long FP;
template<typename T>
class GLPtr
{
public:
    GLPtr() = default;

    PGL operator->() const
    {
        return _pgl;
    }

    bool operator==(const GLPtr& rhs) const
    {
        return _pgl == rhs._pgl;
    }

    explicit operator bool() const
    {
        return _pgl != pvNil;
    }

    static GLPtr Wrap(PGL pgl)
    {
        return GLPtr(pgl);
    }

    static GLPtr New(long cvInit = 0)
    {
        return GLPtr(GL::PglNew(sizeof(T), cvInit));
    }

    static GLPtr Read(PBLCK pblck, short* pbo = pvNil, short* posk = pvNil)
    {
        return GLPtr(GL::PglRead(pblck, pbo, posk));
    }

    static GLPtr Read(class FIL* pfil, FP fp, long cb, short* pbo = pvNil, short* posk = pvNil)
    {
        // FIXME: assert cb valid?
        return GLPtr(GL::PglRead(pfil, fp, cb, pbo, posk));
    }

    GLPtr Dup() const
    {
        return GLPtr(_pgl->PglDup());
    }

    void Release()
    {
        ReleasePpo(&_pgl);
    }

    void Get(long iv, T* pv) const
    {
        _pgl->Get(iv, BlobPtr::Make(*pv));
    }

    void Put(long iv, T* pv) const
    {
        _pgl->Put(iv, BlobPtr::Make(*pv));
    }

    bool FAdd(T* pv, long* piv = pvNil) const
    {
        return _pgl->FAdd(BlobPtr::Make(*pv), piv);
    }

    bool FInsert(long iv, T* pv = pvNil, long cv = 1) const
    {
        return _pgl->FInsert(iv, BlobPtr::Make(*pv), cv);
    }

    bool FPush(T* pv) const
    {
        return _pgl->FPush(BlobPtr::Make(*pv));
    }

    bool FPop(T* pv) const
    {
        return _pgl->FPop(BlobPtr::Make(*pv));
    }

    bool FEnqueue(T* pv) const
    {
        return _pgl->FEnqueue(BlobPtr::Make(*pv));
    }

    bool FDequeue(T* pv) const
    {
        return _pgl->FDequeue(BlobPtr::Make(*pv));
    }

private:
    explicit GLPtr(PGL pgl) : _pgl(pgl)
    {
        // FIXME: should this be _pgl->CbEntry() == sizeof(T)?
        AssertH(_pgl == pvNil || _pgl->CbEntry() >= sizeof(T));
    }

    PGL _pgl = pvNil;
};

/****************************************
    GGPtr provides a template-typed wrapper around a PGG.
****************************************/
template<typename TFixed>
class GGPtr
{
public:
    GGPtr() = default;

    PGG operator->() const
    {
        return _pgg;
    }

    explicit operator bool() const
    {
        return _pgg != pvNil;
    }

    static GGPtr New(long cvInit = 0, long cbInit = 0)
    {
        return GGPtr(GG::PggNew(sizeof(TFixed), cvInit, cbInit));
    }

    static GGPtr Read(PBLCK pblck, short* pbo = pvNil, short* posk = pvNil)
    {
        return GGPtr(GG::PggRead(pblck, pbo, posk));
    }

    static GGPtr Read(class FIL* pfil, FP fp, long cb, short* pbo = pvNil, short* posk = pvNil)
    {
        // TODO: validate cb?
        return GGPtr(GG::PggRead(pfil, fp, cb, pbo, posk));
    }

    GGPtr Dup() const
    {
        return GGPtr(_pgg->PggDup());
    }

    void Release()
    {
        ReleasePpo(&_pgg);
    }

    void GetFixed(long iv, TFixed* pv) const
    {
        _pgg->GetFixed(iv, BlobPtr::Make(*pv));
    }

    void PutFixed(long iv, TFixed* pv) const
    {
        _pgg->PutFixed(iv, BlobPtr::Make(*pv));
    }

    template<typename U>
    void Get(long iv, U* pv) const
    {
        _pgg->Get(iv, BlobPtr::Make(*pv));
    }

    template<typename U>
    void Put(long iv, U* pv) const
    {
        _pgg->Put(iv, BlobPtr::Make(*pv));
    }

    bool FCopyEntries(GGPtr pggSrc, long ivSrc, long ivDst, long cv) const
    {
        return _pgg->FCopyEntries(pggSrc._pgg, ivSrc, ivDst, cv);
    }

private:
    explicit GGPtr(PGG pgg) : _pgg(pgg)
    {
        // FIXME: should this be _pgg->CbFixed() == sizeof(TFixed)?
        AssertH(_pgg == pvNil || _pgg->CbFixed() >= sizeof(TFixed));
    }

    PGG _pgg = pvNil;
};


/****************************************
    AGPtr provides a template-typed wrapper around a PAG.
****************************************/
template<typename TFixed>
class AGPtr
{
public:
    AGPtr() = default;

    PAG operator->() const
    {
        return _pag;
    }

    explicit operator bool() const
    {
        return _pag != pvNil;
    }

    static AGPtr New(long cvInit = 0, long cbInit = 0)
    {
        return AGPtr(AG::PagNew(sizeof(TFixed), cvInit, cbInit));
    }

    static AGPtr Read(PBLCK pblck, short* pbo = pvNil, short* posk = pvNil)
    {
        return AGPtr(AG::PagRead(pblck, pbo, posk));
    }

    void Release()
    {
        ReleasePpo(&_pag);
    }

    void GetFixed(long iv, TFixed* pv) const
    {
        _pag->GetFixed(iv, BlobPtr::Make(*pv));
    }

    void PutFixed(long iv, TFixed* pv) const
    {
        _pag->PutFixed(iv, BlobPtr::Make(*pv));
    }

private:
    explicit AGPtr(PAG pag) : _pag(pag)
    {
        // FIXME: should this be _pag->CbFixed() == sizeof(TFixed)?
        AssertH(_pag == pvNil || _pag->CbFixed() >= sizeof(TFixed));
    }

    PAG _pag = pvNil;
};

/****************************************
    GSTPtr provides a template-typed wrapper around a GST.
    TExtra is the type of the extra data associated with each string.
****************************************/
template<typename TExtra>
class GSTPtr
{
public:
    GSTPtr() = default;

    PGST operator->() const
    {
        return _pgst;
    }

    explicit operator bool() const
    {
        return _pgst != pvNil;
    }

    static GSTPtr Wrap(PGST pgst)
    {
        return GSTPtr(pgst);
    }

    static GSTPtr New(long cstnInit = 0, long cchInit = 0)
    {
        return GSTPtr(GST::PgstNew(sizeof(TExtra), cstnInit, cchInit));
    }

    static GSTPtr Read(PBLCK pblck, short* pbo = pvNil, short* posk = pvNil)
    {
        return GSTPtr(GST::PgstRead(pblck, pbo, posk));
    }

    PGST Unwrap() const
    {
        return _pgst;
    }

    void Release()
    {
        ReleasePpo(&_pgst);
    }

    void GetExtra(long istn, TExtra* pv) const
    {
        _pgst->GetExtra(istn, BlobPtr::Make(*pv));
    }

    void PutExtra(long istn, TExtra* pv) const
    {
        _pgst->PutExtra(istn, BlobPtr::Make(*pv));
    }

private:
    explicit GSTPtr(PGST pgst) : _pgst(pgst)
    {
        // FIXME: should this be _pgst->CbExtra() == sizeof(TExtra)?
        AssertH(_pgst == pvNil || _pgst->CbExtra() >= sizeof(TExtra));
    }

    PGST _pgst = pvNil;
};

#endif //! UTILMEM_H
