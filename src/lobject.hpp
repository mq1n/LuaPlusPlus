#pragma once
/*
** $Id: lobject.h,v 2.117 2016/08/01 19:51:24 roberto Exp $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/

#include <cstdarg>
#include <cstdint>
#include <llimits.hpp>
#include <lua.hpp>


/*
** Extra tags for non-values
*/
#define LUA_TPROTO	LUA_NUMTAGS		/* function prototypes */
#define LUA_TDEADKEY	(LUA_NUMTAGS+1)		/* removed keys in tables */

/*
** number of all possible tags (including LUA_TNONE but excluding DEADKEY)
*/
#define LUA_TOTALTAGS	(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* value)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/


/*
** LUA_TFUNCTION variants:
** 0 - Lua function
** 1 - light C function
** 2 - regular C function (closure)
*/

/* Variant tags for functions */
#define LUA_TLCL	(LUA_TFUNCTION | (0 << 4))  /* Lua closure */
#define LUA_TLCF	(LUA_TFUNCTION | (1 << 4))  /* light C function */
#define LUA_TCCL	(LUA_TFUNCTION | (2 << 4))  /* C closure */


/* Variant tags for strings */
#define LUA_TSHRSTR	(LUA_TSTRING | (0 << 4))  /* short strings */
#define LUA_TLNGSTR	(LUA_TSTRING | (1 << 4))  /* long strings */


/* Variant tags for numbers */
#define LUA_TNUMFLT	(LUA_TNUMBER | (0 << 4))  /* float numbers */
#define LUA_TNUMINT	(LUA_TNUMBER | (1 << 4))  /* integer numbers */


/* Bit mark for collectable types */
#define BIT_ISCOLLECTABLE	(1 << 6)

/* mark a tag as collectable */
#define ctb(t)			((t) | BIT_ISCOLLECTABLE)


/*
** Common Header for all collectable objects (to be inherited by other objects)
*/
struct GCObject
{
  GCObject *next;
  uint8_t tt;
  uint8_t marked;
};

/*
** Tagged Values. This is the basic representation of values in Lua,
** an actual value plus a tag with its type.
*/

/*
** Union of all Lua values
*/
union Value
{
  GCObject *gc;    /* collectable objects */
  void *p;         /* light userdata */
  int b;           /* booleans */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_Number n;    /* float numbers */
};


#define TValuefields	Value value_; int tt_


struct TValue
{
  TValuefields;
};



/* macro defining a nil value */
#define NILCONSTANT	{NULL}, LUA_TNIL


#define val_(o)		((o)->value_)


/* raw type tag of a TValue */
#define rttype(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#define novariant(x)	((x) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
#define ttype(o)	(rttype(o) & 0x3F)

/* type tag of a TValue with no variants (bits 0-3) */
#define ttnov(o)	(novariant(rttype(o)))


/* Macros to test type */
#define checktag(o,t)		(rttype(o) == (t))
#define checktype(o,t)		(ttnov(o) == (t))
#define ttisnumber(o)		checktype((o), LUA_TNUMBER)
#define ttisfloat(o)		checktag((o), LUA_TNUMFLT)
#define ttisinteger(o)		checktag((o), LUA_TNUMINT)
#define ttisnil(o)		checktag((o), LUA_TNIL)
#define ttisboolean(o)		checktag((o), LUA_TBOOLEAN)
#define ttislightuserdata(o)	checktag((o), LUA_TLIGHTUSERDATA)
#define ttisstring(o)		checktype((o), LUA_TSTRING)
#define ttisshrstring(o)	checktag((o), ctb(LUA_TSHRSTR))
#define ttislngstring(o)	checktag((o), ctb(LUA_TLNGSTR))
#define ttistable(o)		checktag((o), ctb(LUA_TTABLE))
#define ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#define ttisclosure(o)		((rttype(o) & 0x1F) == LUA_TFUNCTION)
#define ttisCclosure(o)		checktag((o), ctb(LUA_TCCL))
#define ttisLclosure(o)		checktag((o), ctb(LUA_TLCL))
#define ttislcf(o)		checktag((o), LUA_TLCF)
#define ttisfulluserdata(o)	checktag((o), ctb(LUA_TUSERDATA))
#define ttisthread(o)		checktag((o), ctb(LUA_TTHREAD))
#define ttisdeadkey(o)		checktag((o), LUA_TDEADKEY)


/* Macros to access values */
#define ivalue(o)	check_exp(ttisinteger(o), val_(o).i)
#define fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#define nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_num(ivalue(o)) : fltvalue(o)))
#define gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)
#define pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#define tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))
#define uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))
#define clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#define clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))
#define fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#define hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))
#define bvalue(o)	check_exp(ttisboolean(o), val_(o).b)
#define thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))
/* a dead value may get the 'gc' field, but cannot access its contents */
#define deadvalue(o)	check_exp(ttisdeadkey(o), cast(void *, val_(o).gc))

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))


#define iscollectable(o)	(rttype(o) & BIT_ISCOLLECTABLE)


/* Macros for internal tests */
#define righttt(obj)		(ttype(obj) == gcvalue(obj)->tt)

#define checkliveness(L,obj) \
	lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == NULL || !isdead(L->globalState,gcvalue(obj)))))


/* Macros to set values */
#define settt_(o,t)	((o)->tt_=(t))

#define setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_TNUMFLT); }

#define chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#define setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_TNUMINT); }

#define chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

#define setnilvalue(obj) settt_(obj, LUA_TNIL)

#define setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_TLCF); }

#define setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_TLIGHTUSERDATA); }

#define setbvalue(obj,x) \
  { TValue *io=(obj); val_(io).b=(x); settt_(io, LUA_TBOOLEAN); }

#define setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

#define setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

#define setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TUSERDATA)); \
    checkliveness(L,io); }

#define setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TTHREAD)); \
    checkliveness(L,io); }

#define setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TLCL)); \
    checkliveness(L,io); }

#define setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TCCL)); \
    checkliveness(L,io); }

#define sethvalue(L,obj,x) \
  { TValue *io = (obj); Table *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TTABLE)); \
    checkliveness(L,io); }

#define setdeadvalue(obj)	settt_(obj, LUA_TDEADKEY)



#define setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); *io1 = *(obj2); \
	  (void)L; checkliveness(L,io1); }


/*
** different types of assignments, according to destination
*/

/* from stack to (same) stack */
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to new object */
#define setobj2n	setobj
#define setsvalue2n	setsvalue

/* to table (define it as an expression to be used in macros) */
#define setobj2t(L,o1,o2)  ((void)L, *(o1)=*(o2), checkliveness(L,(o1)))




/*
** {======================================================
** types and prototypes
** =======================================================
*/


using StkId = TValue*;  /* index to stack elements */




/*
** Header for string value; string bytes follow the end of this structure
** (aligned according to 'UTString'; see next).
*/
class TString : public GCObject
{
  friend class LGCFactory;
  friend struct TStringAlign;
  TString() = default;
  ~TString();
public:
  TString(const TString&) = delete;
  TString(TString&&) = delete;
  TString& operator=(const TString&) = delete;
  TString& operator=(TString&&) = delete;

  uint8_t extra;  /* reserved words for short strings; "has hash" for longs */
  uint8_t shrlen;  /* length for short strings */
  uint32_t hash;
  union
  {
    size_t lnglen;  /* length for long strings */
    TString* hnext;  /* linked list for hash table */
  } u;
};

struct TStringAlign
{
  TStringAlign() = delete;
  /*
  ** Ensures that address after this type is always fully aligned.
  */
  using UTString = union
  {
    L_Umaxalign dummy;
    TString tsv;
  };
};


/*
** Get the actual string (array of bytes) from a 'TString'.
** (Access to 'extra' ensures that value is really a 'TString'.)
*/
#define getstr(ts)  \
  check_exp(sizeof((ts)->extra), cast(char *, (ts)) + sizeof(TStringAlign::UTString))


/* get the actual string (array of bytes) from a Lua value */
#define svalue(o)       getstr(tsvalue(o))

/* get string length from 'TString *s' */
#define tsslen(s)	((s)->tt == LUA_TSHRSTR ? (s)->shrlen : (s)->u.lnglen)

/* get string length from 'TValue *o' */
#define vslen(o)	tsslen(tsvalue(o))


/*
** Header for userdata; memory area follows the end of this structure
** (aligned according to 'UUdata'; see next).
*/
class Udata : public GCObject
{
  friend class LGCFactory;
  friend struct UDataAlign;
  Udata() = default;
  ~Udata();
public:
  Udata(const Udata&) = delete;
  Udata(Udata&&) = delete;
  Udata& operator=(const Udata&) = delete;
  Udata& operator=(Udata&&) = delete;

  uint8_t ttuv_;  /* user value's tag */
  class Table* metatable;
  size_t len;  /* number of bytes */
  union Value user_;  /* user value */
};

struct UDataAlign
{
  UDataAlign() = delete;

  /*
  ** Ensures that address after this type is always fully aligned.
  */
  using UUdata = union
  {
    L_Umaxalign dummy;  /* ensures maximum alignment for 'local' udata */
    Udata uv;
  };
};


/*
**  Get the address of memory block inside 'Udata'.
** (Access to 'ttuv_' ensures that value is really a 'Udata'.)
*/
#define getudatamem(u)  \
  check_exp(sizeof((u)->ttuv_), (cast(char*, (u)) + sizeof(UDataAlign::UUdata)))

#define setuservalue(L,u,o) \
	{ const TValue *io=(o); Udata *iu = (u); \
	  iu->user_ = io->value_; iu->ttuv_ = rttype(io); \
	  checkliveness(L,io); }


#define getuservalue(L,u,o) \
	{ TValue *io=(o); const Udata *iu = (u); \
	  io->value_ = iu->user_; settt_(io, iu->ttuv_); \
	  checkliveness(L,io); }


/*
** Description of an upvalue for function prototypes
*/
struct Upvaldesc {
  TString *name;  /* upvalue name (for debug information) */
  uint8_t instack;  /* whether it is in stack (register) */
  uint8_t idx;  /* index of upvalue (in stack or in outer function's list) */
};


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
struct LocVar {
  TString *varname;
  int startpc;  /* first point where variable is active */
  int endpc;    /* first point where variable is dead */
};


/*
** Function Prototypes
*/
class Proto : public GCObject
{
  friend class LGCFactory;
  Proto() = default;
  ~Proto();
public:
  Proto(const Proto&) = delete;
  Proto(Proto&&) = delete;
  Proto& operator=(const Proto&) = delete;
  Proto& operator=(Proto&&) = delete;

  uint8_t numparams;  /* number of fixed parameters */
  uint8_t is_vararg;
  uint8_t maxstacksize;  /* number of registers needed by this function */
  int sizeupvalues;  /* size of 'upvalues' */
  int sizek;  /* size of 'k' */
  int sizecode;
  int sizelineinfo;
  int sizep;  /* size of 'p' */
  int sizelocvars;
  int linedefined;  /* debug information  */
  int lastlinedefined;  /* debug information  */
  TValue *k;  /* constants used by the function */
  Instruction *code;  /* opcodes */
  struct Proto **p;  /* functions defined inside the function */
  int *lineinfo;  /* map from opcodes to source lines (debug information) */
  LocVar *locvars;  /* information about local variables (debug information) */
  Upvaldesc *upvalues;  /* upvalue information */
  class LClosure *cache;  /* last-created closure with this prototype */
  TString  *source;  /* used for debug information */
  GCObject *gclist;
};



/*
** Lua Upvalues
*/
using UpVal = struct UpVal;


/*
** Closures
*/

class ClosureHeader : public GCObject
{
protected:
  ClosureHeader() = default;
  ~ClosureHeader() = default;
public:
  ClosureHeader(const ClosureHeader&) = delete;
  ClosureHeader(ClosureHeader&&) = delete;
  ClosureHeader& operator=(const ClosureHeader&) = delete;
  ClosureHeader& operator=(ClosureHeader&&) = delete;

	uint8_t nupvalues;
  GCObject* gclist;
};

class CClosure : public ClosureHeader
{
  friend class LGCFactory;
  CClosure() = default;
  ~CClosure();
public:
  CClosure(const CClosure&) = delete;
  CClosure(CClosure&&) = delete;
  CClosure& operator=(const CClosure&) = delete;
  CClosure& operator=(CClosure&&) = delete;

  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues */
};

class LClosure : public ClosureHeader
{
  friend class LGCFactory;
  LClosure() = default;
  ~LClosure();
public:
  LClosure(const LClosure&) = delete;
  LClosure(LClosure&&) = delete;
  LClosure& operator=(const LClosure&) = delete;
  LClosure& operator=(LClosure&&) = delete;

  Proto *p;
  UpVal *upvals[1];  /* list of upvalues */
};

[[maybe_unused]]
static size_t sizeCClosure(int32_t upvalues)
{
  return sizeof(CClosure) + (sizeof(TValue) * (upvalues - 1));
}

[[maybe_unused]]
static size_t sizeLClosure(int32_t upvalues)
{
  return sizeof(LClosure) + (sizeof(TValue*) * (upvalues - 1));
}

#define isLfunction(o)	ttisLclosure(o)

#define getproto(o)	(clLvalue(o)->p)


/*
** Tables
*/

union TKey
{
  struct
  {
    TValuefields;
    int next;  /* for chaining (offset for next node) */
  } nk;
  TValue tvk;
};


/* copy a value into a key without messing up field 'next' */
#define setnodekey(L,key,obj) \
	{ TKey *k_=(key); const TValue *io_=(obj); \
	  k_->nk.value_ = io_->value_; k_->nk.tt_ = io_->tt_; \
	  (void)L; checkliveness(L,io_); }


struct Node
{
  TValue i_val;
  TKey i_key;
};


class Table : public GCObject
{
  friend class LGCFactory;
  Table() = default;
  ~Table();
public:
  Table(const Table&) = delete;
  Table(Table&&) = delete;
  Table& operator=(const Table&) = delete;
  Table& operator=(Table&&) = delete;

  uint8_t flags;  /* 1<<p means tagmethod(p) is not present */
  uint8_t lsizenode;  /* log2 of size of 'node' array */
  uint32_t sizearray;  /* size of 'array' array */
  TValue* array;  /* array part */
  Node* node;
  Node* lastfree;  /* any free position is before this position */
  Table* metatable;
  GCObject* gclist;
};



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))


/*
** (address of) a fixed nil value
*/
#define luaO_nilobject		(&luaO_nilobject_)


LUAI_DDEC const TValue luaO_nilobject_;

/* size of buffer for 'luaO_utf8esc' function */
#define UTF8BUFFSZ	8

LUAI_FUNC int luaO_int2fb (uint32_t x);
LUAI_FUNC int luaO_fb2int (int x);
LUAI_FUNC int luaO_utf8esc (char *buff, unsigned long x);
LUAI_FUNC int luaO_ceillog2 (uint32_t x);
LUAI_FUNC void luaO_arith (lua_State *L, int op, const TValue *p1,
                           const TValue *p2, TValue *res);
LUAI_FUNC size_t luaO_str2num (const char *s, TValue *o);
LUAI_FUNC int luaO_hexavalue (int c);
LUAI_FUNC void luaO_tostring (lua_State *L, StkId obj);
LUAI_FUNC const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid (char *out, const char *source, size_t len);
