#include <ruby.h>
#include <dystopia.h>
#include <tcqdb.h>
#include <laputa.h>
#include <tcwdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

static VALUE mTD;
static VALUE eTD;
static VALUE cIDB;
static VALUE cQDB;
static VALUE cJDB;
static VALUE cWDB;
static VALUE eMisc;

#define NERRORS (TCENOREC+1)

VALUE errors[NERRORS];

static void tc_error(int ecode, const char *msg)
{
    if (ecode < NERRORS)
        rb_raise(errors[ecode], "%s", msg);
    rb_raise(eMisc, "%s", msg);
}

/* Core */

static VALUE idb_allocate(VALUE klass)
{
    TCIDB *idb = tcidbnew();
    return Data_Wrap_Struct(klass, NULL, tcidbdel, idb);
}

#define IDB_CHK(x) if (!(x)) tc_error(tcidbecode(idb), tcidberrmsg(tcidbecode(idb)))

static VALUE idb_tune(VALUE obj, VALUE ernum, VALUE etnum, VALUE iusiz, VALUE opts)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbtune(idb, NUM2LL(ernum), NUM2LL(etnum), NUM2LL(iusiz), NUM2INT(opts)));
    return obj;
}

static VALUE idb_setcache(VALUE obj, VALUE icsiz, VALUE lcnum)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbsetcache(idb, NUL2LL(icsiz), NUL2INT(lcnum)));
    return obj;
}

static VALUE idb_setfwmmax(VALUE obj, VALUE fwmmax)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbsetfwmmax(idb, NUM2ULONG(fwmmax)));
    return obj;
}

static VALUE idb_open(VALUE obj, VALUE path, VALUE omode)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    FilePathValue(path);
    IDB_CHK(tcidbopen(idb, RSTRING_PTR(path), NUM2INT(omode)));
    return obj;
}

static VALUE idb_close(VALUE obj)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbclose(idb));
    return obj;
}

static VALUE idb_put(VALUE obj, VALUE id, VALUE text)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbput(idb, NUM2LL(id), StringValueCStr(text)));
    return obj;
}

static VALUE idb_out(VALUE obj, VALUE id)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbout(idb, NUM2LL(id)));
    return obj;
}

static VALUE idb_get(VALUE obj, VALUE id)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbget(idb, NUM2LL(id)));
    return obj;
}

static VALUE idb_search(VALUE obj, VALUE word, VALUE smode)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    int np;
    uint64_t *idlist;
    idlist = tcidbsearch(idb, StringValueCStr(word), NUM2INT(smode), &np);
    if (idlist == NULL)
        tc_error(tcidbcode(idb), tcidberrmsg(tcidbecode(idb)));
    VALUE ret = rb_ary_new2(np);
    int i;
    for (i = 0; i < np; i++)
        rb_ary_push(ret, ULL2NUM(idlist[i]));
    free(idlist);
    return ret;
}

static VALUE idb_search2(VALUE obj, VALUE expr)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    int np;
    uint64_t *idlist;
    idlist = tcidbsearch2(idb, StringValueCStr(expr), &np);
    if (idlist == NULL)
        tc_error(tcidbcode(idb), tcidberrmsg(tcidbecode(idb)));
    VALUE ret = rb_ary_new2(np);
    int i;;
    for (i = 0; i < np; i++)
        rb_ary_push(ret, ULL2NUM(idlist[i]));
    free(idlist);
    return ret;
}

static VALUE idb_iterinit(VALUE obj)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbiterinit(idb));
    return obj;
}

static VALUE idb_iternext(VALUE obj)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbiternext(idb));
    return obj;
}

static VALUE idb_sync(VALUE obj)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbsync(idb));
    return obj;
}

static VALUE idb_optimize(VALUE obj)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidboptimize(idb));
    return obj;
}

static VALUE idb_vanish(VALUE obj)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    IDB_CHK(tcidbvanish(idb));
    return obj;
}

static VALUE idb_copy(VALUE obj, VALUE path)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    FilePathValue(path);
    IDB_CHK(tcidbcopy(idb, RSTRING_PTR(path)));
    return obj;
}

static VALUE idb_path(VALUE obj)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    const char *path = tcidbpath(idb);
    return path ? rb_tainted_str_new2(path) : Qnil;
}

static VALUE idb_rnum(VALUE obj)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    return ULL2NUM(tcidbrnum(idb));
}

static VALUE idb_fsiz(VALUE obj)
{
    TCIDB *idb;
    Data_Get_Struct(obj, TCIDB, idb);
    return ULL2NUM(tcidbfsiz(idb));
}

/* Q-gram */

static VALUE qdb_allocate(VALUE klass)
{
    TCQDB *qdb = tcqdbnew();
    return Data_Wrap_Struct(klass, NULL, tcqdbdel, qdb);
}

#define QDB_CHK(x) if (!(x)) tc_error(tcqdbecode(qdb), tcqdberrmsg(tcqdbecode(qdb)))

static VALUE qdb_tune(VALUE obj, VALUE etnum, VALUE opts)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    QDB_CHK(tcqdbtune(qdb, NUM2LL(etnum), NUM2INT(opts)));
    return obj;
}

static VALUE qdb_setcache(VALUE obj, VALUE icsiz, VALUE lcnum)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    QDB_CHK(tcqdbsetcache(qdb, NUM2LL(icsiz), NUM2LONG(lcnum)));
    return obj;
}

static VALUE qdb_setfwmmax(VALUE obj, VALUE fwmmax)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    QDB_CHK(tcqdbsetfwmmax(qdb, NUM2ULONG(fwmmax)));
    return obj;
}

static VALUE qdb_open(VALUE obj, VALUE path, VALUE omode)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    FilePathValue(path);
    QDB_CHK(tcqdbopen(qdb, RSTRING_PTR(path), NUM2INT(omode)));
    return obj;
}

static VALUE qdb_close(VALUE obj)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    QDB_CHK(tcqdbclose(qdb));
    return obj;
}

static VALUE qdb_put(VALUE obj, VALUE id, VALUE text)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    QDB_CHK(tcqdbput(qdb, NUM2LL(id), StringValueCStr(text)));
    return obj;
}

static VALUE qdb_out(VALUE obj, VALUE id, VALUE text)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    QDB_CHK(tcqdbout(qdb, NUM2LL(id), StringValueCStr(text)));
    return obj;
}

static VALUE qdb_search(VALUE obj, VALUE word, VALUE smode)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    int np;
    uint64_t *idlist;
    idlist = tcqdbsearch(qdb, StringValueCStr(word), NUM2INT(smode), &np);
    if (idlist == NULL)
        tc_error(tcqdbecode(qdb), tcqdberrmsg(tcqdbecode(qdb)));
    VALUE ret = rb_ary_new2(np);
    int i;
    for (i = 0; i < np; i++)
        rb_ary_push(ret, ULL2NUM(idlist[i]));
    free(idlist);
    return ret;
}

static VALUE qdb_sync(VALUE obj)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    QDB_CHK(tcqdbsync(qdb));
    return obj;
}

static VALUE qdb_optimize(VALUE obj)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    QDB_CHK(tcqdboptimize(qdb));
    return obj;
}

static VALUE qdb_vanish(VALUE obj)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    QDB_CHK(tcqdbvanish(qdb));
    return obj;
}

static VALUE qdb_copy(VALUE obj, VALUE path)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    FilePathValue(path);
    QDB_CHK(tcqdbcopy(qdb, RSTRING_PTR(path)));
    return obj;
}

static VALUE qdb_path(VALUE obj)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    const char *path = tcqdbpath(qdb);
    return path ? rb_tainted_str_new2(path) : Qnil;
}

static VALUE qdb_tnum(VALUE obj)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    return ULL2NUM(tcqdbtnum(qdb));
}

static VALUE qdb_fsiz(VALUE obj)
{
    TCQDB *qdb;
    Data_Get_Struct(obj, TCQDB, qdb);
    return ULL2NUM(tcqdbfsiz(qdb));
}

/* Simple */

static VALUE jdb_allocate(VALUE klass)
{
    TCJDB *jdb = tcjdbnew();
    return Data_Wrap_Struct(klass, NULL, tcjdbdel, jdb);
}

#define JDB_CHK(x) if (!(x)) tc_error(tcjdbecode(jdb), tcjdberrmsg(tcjdbecode(jdb)))

static VALUE jdb_tune(VALUE obj, VALUE ernum, VALUE etnum, VALUE iusiz, VALUE opts)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbtune(jdb, NUM2LL(ernum), NUM2LL(etnum), NUM2LL(iusiz), NUM2INT(opts)));
    return obj;
}

static VALUE jdb_setcache(VALUE obj, VALUE icsiz, VALUE lcnum)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbsetcache(jdb, NUL2LL(icsiz), NUL2INT(lcnum)));
    return obj;
}

static VALUE jdb_setfwmmax(VALUE obj, VALUE fwmmax)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbsetfwmmax(jdb, NUM2ULONG(fwmmax)));
    return obj;
}

static VALUE jdb_open(VALUE obj, VALUE path, VALUE omode)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    FilePathValue(path);
    JDB_CHK(tcjdbopen(jdb, RSTRING_PTR(path), NUM2INT(omode)));
    return obj;
}

static VALUE jdb_close(VALUE obj)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbclose(jdb));
    return obj;
}

static VALUE jdb_put(VALUE obj, VALUE id, VALUE words)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    VALUE ary = rb_check_array_type(words);
    TCLIST *tclist = tclistnew();
    int i;
    VALUE *ptr = RARRAY_PTR(ary);
    for (i = 0; i < RARRAY_LEN(ary); i++) {
        VALUE s = rb_check_string_type(ptr[i]);
        tclistpush(tclist, RSTRING_PTR(s), RSTRING_LEN(s));
    }
    JDB_CHK(tcjdbput(jdb, NUM2LL(id), tclist));
    tclistdel(tclist);
    return obj;
}

static VALUE jdb_put2(VALUE obj, VALUE id, VALUE text, VALUE delims)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbput2(jdb, NUM2LL(id), StringValueCStr(text), StringValueCStr(delims)));
    return obj;
}

static VALUE jdb_out(VALUE obj, VALUE id)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbout(jdb, NUM2LL(id)));
    return obj;
}

static VALUE jdb_get(VALUE obj, VALUE id)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbget(jdb, NUM2LL(id)));
    return obj;
}

static VALUE jdb_get2(VALUE obj, VALUE id)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbget2(jdb, NUM2LL(id)));
    return obj;
}

static VALUE jdb_search(VALUE obj, VALUE word, VALUE smode)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    int np;
    uint64_t *idlist;
    idlist = tcjdbsearch(jdb, StringValueCStr(word), NUM2INT(smode), &np);
    if (idlist == NULL)
        tc_error(tcjdbcode(jdb), tcjdberrmsg(tcjdbecode(jdb)));
    VALUE ret = rb_ary_new2(np);
    int i;
    for (i = 0; i < np; i++)
        rb_ary_push(ret, ULL2NUM(idlist[i]));
    free(idlist);
    return ret;
}

static VALUE jdb_search2(VALUE obj, VALUE expr)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    int np;
    uint64_t *idlist;
    idlist = tcjdbsearch2(jdb, StringValueCStr(expr), &np);
    if (idlist == NULL)
        tc_error(tcjdbcode(jdb), tcjdberrmsg(tcjdbecode(jdb)));
    VALUE ret = rb_ary_new2(np);
    int i;
    for (i = 0; i < np; i++)
        rb_ary_push(ret, ULL2NUM(idlist[i]));
    free(idlist);
    return ret;
}

static VALUE jdb_iterinit(VALUE obj)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbiterinit(jdb));
    return obj;
}

static VALUE jdb_iternext(VALUE obj)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbiternext(jdb));
    return obj;
}

static VALUE jdb_sync(VALUE obj)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbsync(jdb));
    return obj;
}

static VALUE jdb_optimize(VALUE obj)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdboptimize(jdb));
    return obj;
}

static VALUE jdb_vanish(VALUE obj)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    JDB_CHK(tcjdbvanish(jdb));
    return obj;
}

static VALUE jdb_copy(VALUE obj, VALUE path)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    FilePathValue(path);
    JDB_CHK(tcjdbcopy(jdb, RSTRING_PTR(path)));
    return obj;
}

static VALUE jdb_path(VALUE obj)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    const char *path = tcjdbpath(jdb);
    return path ? rb_tainted_str_new2(path) : Qnil;
}

static VALUE jdb_rnum(VALUE obj)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    return ULL2NUM(tcjdbrnum(jdb));
}

static VALUE jdb_fsiz(VALUE obj)
{
    TCJDB *jdb;
    Data_Get_Struct(obj, TCJDB, jdb);
    return ULL2NUM(tcjdbfsiz(jdb));
}

/* Word */

static VALUE wdb_allocate(VALUE klass)
{
    TCWDB *wdb = tcwdbnew();
    return Data_Wrap_Struct(klass, NULL, tcwdbdel, wdb);
}

#define WDB_CHK(x) if (!(x)) tc_error(tcwdbecode(wdb), tcwdberrmsg(tcwdbecode(wdb)))

static VALUE wdb_tune(VALUE obj, VALUE etnum, VALUE opts)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    WDB_CHK(tcwdbtune(wdb, NUM2LL(etnum), NUM2INT(opts)));
    return obj;
}

static VALUE wdb_setcache(VALUE obj, VALUE icsiz, VALUE lcnum)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    WDB_CHK(tcwdbsetcache(wdb, NUL2LL(icsiz), NUL2INT(lcnum)));
    return obj;
}

static VALUE wdb_setfwmmax(VALUE obj, VALUE fwmmax)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    WDB_CHK(tcwdbsetfwmmax(wdb, NUM2ULONG(fwmmax)));
    return obj;
}

static VALUE wdb_open(VALUE obj, VALUE path, VALUE omode)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    FilePathValue(path);
    WDB_CHK(tcwdbopen(wdb, RSTRING_PTR(path), NUM2INT(omode)));
    return obj;
}

static VALUE wdb_close(VALUE obj)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    WDB_CHK(tcwdbclose(wdb));
    return obj;
}

static VALUE wdb_put(VALUE obj, VALUE id, VALUE words)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    VALUE ary = rb_check_array_type(words);
    TCLIST *tclist = tclistnew();
    int i;
    VALUE *ptr = RARRAY_PTR(ary);
    for (i = 0; i < RARRAY_LEN(ary); i++) {
        VALUE s = rb_check_string_type(ptr[i]);
        tclistpush(tclist, RSTRING_PTR(s), RSTRING_LEN(s));
    }
    WDB_CHK(tcwdbput(wdb, NUM2LL(id), tclist));
    tclistdel(tclist);
    return obj;
}

static VALUE wdb_put2(VALUE obj, VALUE id, VALUE text, VALUE delims)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    WDB_CHK(tcwdbput2(wdb, NUM2LL(id), StringValueCStr(text), StringValueCStr(delims)));
    return obj;
}

static VALUE wdb_out(VALUE obj, VALUE id, VALUE words)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    VALUE ary = rb_check_array_type(words);
    TCLIST *tclist = tclistnew();
    int i;
    VALUE *ptr = RARRAY_PTR(ary);
    for (i = 0; i < RARRAY_LEN(ary); i++) {
        VALUE s = rb_check_string_type(ptr[i]);
        tclistpush(tclist, RSTRING_PTR(s), RSTRING_LEN(s));
    }
    WDB_CHK(tcwdbout(wdb, NUM2LL(id), tclist));
    tclistdel(tclist);
    return obj;
}

static VALUE wdb_out2(VALUE obj, VALUE id, VALUE text, VALUE delims)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    WDB_CHK(tcwdbout2(wdb, NUM2LL(id), StringValueCStr(text), StringValueCStr(delims)));
    return obj;
}

static VALUE wdb_search(VALUE obj, VALUE word)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    int np;
    uint64_t *idlist;
    idlist = tcwdbsearch(wdb, StringValueCStr(word), &np);
    if (idlist == NULL)
        tc_error(tcwdbcode(wdb), tcwdberrmsg(tcwdbecode(wdb)));
    VALUE ret = rb_ary_new2(np);
    int i;
    for (i = 0; i < np; i++)
        rb_ary_push(ret, ULL2NUM(idlist[i]));
    free(idlist);
    return ret;
}

static VALUE wdb_sync(VALUE obj)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    WDB_CHK(tcwdbsync(wdb));
    return obj;
}

static VALUE wdb_optimize(VALUE obj)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    WDB_CHK(tcwdboptimize(wdb));
    return obj;
}

static VALUE wdb_vanish(VALUE obj)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    WDB_CHK(tcwdbvanish(wdb));
    return obj;
}

static VALUE wdb_copy(VALUE obj, VALUE path)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    FilePathValue(path);
    WDB_CHK(tcwdbcopy(wdb, RSTRING_PTR(path)));
    return obj;
}

static VALUE wdb_path(VALUE obj)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    const char *path = tcwdbpath(wdb);
    return path ? rb_tainted_str_new2(path) : Qnil;
}

static VALUE wdb_tnum(VALUE obj)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    return ULL2NUM(tcwdbrnum(wdb));
}

static VALUE wdb_fsiz(VALUE obj)
{
    TCWDB *wdb;
    Data_Get_Struct(obj, TCWDB, wdb);
    return ULL2NUM(tcwdbfsiz(wdb));
}

/* Initialize */

void Init_tokyodystopia()
{
    mTD = rb_define_module("TokyoDystopia");
    rb_define_const(mTD, "VERSION", rb_usascii_str_new2(tdversion));
    eTD = rb_define_class_under(mTD, "Error", rb_eStandardError);

    const char *errstr[] = {
        "Success", "ThreadError", "InvlaidError", "NoFileError", "NoPermError",
        "MetaError", "RHeadError", "OpenError", "CloseError", "TruncError",
        "SyncError", "SeekError", "ReadError", "WriteError", "MmapError",
        "LockError", "UnlinkError", "RenameError", "MkdirError", "RmdirError",
        "KeepError", "NoRecError"
    };

    unsigned int i;
    for (i = 0; i < sizeof(errstr)/sizeof(*errstr); i++)
        errors[i] = rb_define_class_under(mTD, errstr[i], eTD);
    eMisc = rb_define_class_under(mTD, "MiscError", eTD);

    /* Core */

    cIDB = rb_define_class_under(mTD, "IDB", rb_cObject);

    rb_define_const(cIDB, "LARGE", INT2NUM(IDBTLARGE));
    rb_define_const(cIDB, "DEFLATE", INT2NUM(IDBTDEFLATE));
    rb_define_const(cIDB, "BZIP", INT2NUM(IDBTBZIP));
    rb_define_const(cIDB, "TCBS", INT2NUM(IDBTTCBS));
    rb_define_const(cIDB, "READER", INT2NUM(IDBOREADER));
    rb_define_const(cIDB, "WRITER", INT2NUM(IDBOWRITER));
    rb_define_const(cIDB, "CREAT", INT2NUM(IDBOCREAT));
    rb_define_const(cIDB, "TRUNC", INT2NUM(IDBOTRUNC));
    rb_define_const(cIDB, "NOLCK", INT2NUM(IDBONOLCK));
    rb_define_const(cIDB, "LCKNB", INT2NUM(IDBOLCKNB));
    rb_define_const(cIDB, "SUBSTR", INT2NUM(IDBSSUBSTR));
    rb_define_const(cIDB, "PREFIX", INT2NUM(IDBSPREFIX));
    rb_define_const(cIDB, "SUFFIX", INT2NUM(IDBSSUFFIX));
    rb_define_const(cIDB, "FULL", INT2NUM(IDBSFULL));
    rb_define_const(cIDB, "TOKEN", INT2NUM(IDBSTOKEN));
    rb_define_const(cIDB, "TOKPRE", INT2NUM(IDBSTOKPRE));
    rb_define_const(cIDB, "TOKSUF", INT2NUM(IDBSTOKSUF));

    rb_define_alloc_func(cIDB, idb_allocate);
    rb_define_method(cIDB, "tune", idb_tune, 4);
    rb_define_method(cIDB, "setcache", idb_setcache, 2);
    rb_define_method(cIDB, "setfwmmax",idb_setfwmmax, 1);
    rb_define_method(cIDB, "open", idb_open, 2);
    rb_define_method(cIDB, "close", idb_close, 0);
    rb_define_method(cIDB, "put", idb_put, 2);
    rb_define_method(cIDB, "out", idb_out, 1);
    rb_define_method(cIDB, "get", idb_get, 1);
    rb_define_method(cIDB, "search", idb_search, 2);
    rb_define_method(cIDB, "search2", idb_search2, 1);
    rb_define_method(cIDB, "iterinit", idb_iterinit, 0);
    rb_define_method(cIDB, "iternext", idb_iternext, 0);
    rb_define_method(cIDB, "sync", idb_sync, 0);
    rb_define_method(cIDB, "optimize", idb_optimize, 0);
    rb_define_method(cIDB, "vanish", idb_vanish, 0);
    rb_define_method(cIDB, "copy", idb_copy, 1);
    rb_define_method(cIDB, "path", idb_path, 0);
    rb_define_method(cIDB, "rnum", idb_rnum, 0);
    rb_define_method(cIDB, "fsiz", idb_fsiz, 0);

    /* Q-gram */

    cQDB = rb_define_class_under(mTD, "QDB", rb_cObject);
    rb_define_const(cQDB, "LARGE", INT2NUM(QDBTLARGE));
    rb_define_const(cQDB, "DEFLATE", INT2NUM(QDBTDEFLATE));
    rb_define_const(cQDB, "BZIP", INT2NUM(QDBTBZIP));
    rb_define_const(cQDB, "TCBS", INT2NUM(QDBTTCBS));
    rb_define_const(cQDB, "READER", INT2NUM(QDBOREADER));
    rb_define_const(cQDB, "WRITER", INT2NUM(QDBOWRITER));
    rb_define_const(cQDB, "CREAT", INT2NUM(QDBOCREAT));
    rb_define_const(cQDB, "TRUNC", INT2NUM(QDBOTRUNC));
    rb_define_const(cQDB, "NOLCK", INT2NUM(QDBONOLCK));
    rb_define_const(cQDB, "LCKNB", INT2NUM(QDBOLCKNB));
    rb_define_const(cQDB, "SUBSTR", INT2NUM(QDBSSUBSTR));
    rb_define_const(cQDB, "PREFIX", INT2NUM(QDBSPREFIX));
    rb_define_const(cQDB, "SUFFIX", INT2NUM(QDBSSUFFIX));
    rb_define_const(cQDB, "FULL", INT2NUM(QDBSFULL));

    rb_define_alloc_func(cQDB, qdb_allocate);
    rb_define_method(cQDB, "tune", qdb_tune, 2);
    rb_define_method(cQDB, "setcache", qdb_setcache, 2);
    rb_define_method(cQDB, "setfwmmax", qdb_setfwmmax, 1);
    rb_define_method(cQDB, "open", qdb_open, 2);
    rb_define_method(cQDB, "close", qdb_close, 0);
    rb_define_method(cQDB, "put", qdb_put, 2);
    rb_define_method(cQDB, "out", qdb_out, 2);
    rb_define_method(cQDB, "search", qdb_search, 2);
    rb_define_method(cQDB, "sync", qdb_sync, 0);
    rb_define_method(cQDB, "optimize", qdb_optimize, 0);
    rb_define_method(cQDB, "vanish", qdb_vanish, 0);
    rb_define_method(cQDB, "copy", qdb_copy, 1);
    rb_define_method(cQDB, "path", qdb_path, 0);
    rb_define_method(cQDB, "tnum", qdb_tnum, 0);
    rb_define_method(cQDB, "fsiz", qdb_fsiz, 0);

    /* Simple */

    cJDB = rb_define_class_under(mTD, "JDB", rb_cObject);
    rb_define_const(cJDB, "LARGE", INT2NUM(JDBTLARGE));
    rb_define_const(cJDB, "DEFLATE", INT2NUM(JDBTDEFLATE));
    rb_define_const(cJDB, "BZIP", INT2NUM(JDBTBZIP));
    rb_define_const(cJDB, "TCBS", INT2NUM(JDBTTCBS));
    rb_define_const(cJDB, "READER", INT2NUM(JDBOREADER));
    rb_define_const(cJDB, "WRITER", INT2NUM(JDBOWRITER));
    rb_define_const(cJDB, "CREAT", INT2NUM(JDBOCREAT));
    rb_define_const(cJDB, "TRUNC", INT2NUM(JDBOTRUNC));
    rb_define_const(cJDB, "NOLCK", INT2NUM(JDBONOLCK));
    rb_define_const(cJDB, "LCKNB", INT2NUM(JDBOLCKNB));
    rb_define_const(cJDB, "SUBSTR", INT2NUM(JDBSSUBSTR));
    rb_define_const(cJDB, "PREFIX", INT2NUM(JDBSPREFIX));
    rb_define_const(cJDB, "SUFFIX", INT2NUM(JDBSSUFFIX));
    rb_define_const(cJDB, "FULL", INT2NUM(JDBSFULL));

    rb_define_alloc_func(cJDB, jdb_allocate);
    rb_define_method(cJDB, "tune", jdb_tune, 4);
    rb_define_method(cJDB, "setcache", jdb_setcache, 2);
    rb_define_method(cJDB, "setfwmmax", jdb_setfwmmax, 1);
    rb_define_method(cJDB, "open", jdb_open, 2);
    rb_define_method(cJDB, "close", jdb_close, 0);
    rb_define_method(cJDB, "put", jdb_put, 2);
    rb_define_method(cJDB, "put2", jdb_put2, 3);
    rb_define_method(cJDB, "out", jdb_out, 1);
    rb_define_method(cJDB, "get", jdb_get, 1);
    rb_define_method(cJDB, "get2", jdb_get2, 1);
    rb_define_method(cJDB, "search", jdb_search, 2);
    rb_define_method(cJDB, "search2", jdb_search2, 1);
    rb_define_method(cJDB, "iterinit", jdb_iterinit, 0);
    rb_define_method(cJDB, "iternext", jdb_iternext, 0);
    rb_define_method(cJDB, "sync", jdb_sync, 0);
    rb_define_method(cJDB, "optimize", jdb_optimize, 0);
    rb_define_method(cJDB, "vanish", jdb_vanish, 0);
    rb_define_method(cJDB, "copy", jdb_copy, 1);
    rb_define_method(cJDB, "path", jdb_path, 0);
    rb_define_method(cJDB, "rnum", jdb_rnum, 0);
    rb_define_method(cJDB, "fsiz", jdb_fsiz, 0);

    /* Word */

    cWDB = rb_define_class_under(mTD, "WDB", rb_cObject);
    rb_define_const(cWDB, "LARGE", INT2NUM(WDBTLARGE));
    rb_define_const(cWDB, "DEFLATE", INT2NUM(WDBTDEFLATE));
    rb_define_const(cWDB, "BZIP", INT2NUM(WDBTBZIP));
    rb_define_const(cWDB, "TCBS", INT2NUM(WDBTTCBS));
    rb_define_const(cWDB, "READER", INT2NUM(WDBOREADER));
    rb_define_const(cWDB, "WRITER", INT2NUM(WDBOWRITER));
    rb_define_const(cWDB, "CREAT", INT2NUM(WDBOCREAT));
    rb_define_const(cWDB, "TRUNC", INT2NUM(WDBOTRUNC));
    rb_define_const(cWDB, "NOLCK", INT2NUM(WDBONOLCK));
    rb_define_const(cWDB, "LCKNB", INT2NUM(WDBOLCKNB));
    rb_define_const(cWDB, "SUBSTR", INT2NUM(WDBSSUBSTR));
    rb_define_const(cWDB, "PREFIX", INT2NUM(WDBSPREFIX));
    rb_define_const(cWDB, "SUFFIX", INT2NUM(WDBSSUFFIX));
    rb_define_const(cWDB, "FULL", INT2NUM(WDBSFULL));

    rb_define_alloc_func(cWDB, wdb_allocate);
    rb_define_method(cWDB, "tune", wdb_tune, 2);
    rb_define_method(cWDB, "setcache", wdb_setcache, 2);
    rb_define_method(cWDB, "setfwmmax", wdb_setfwmmax, 1);
    rb_define_method(cWDB, "open", wdb_open, 2);
    rb_define_method(cWDB, "close", wdb_close, 0);
    rb_define_method(cWDB, "put", wdb_put, 2);
    rb_define_method(cWDB, "put2", wdb_put2, 3);
    rb_define_method(cWDB, "out", wdb_out, 2);
    rb_define_method(cWDB, "out2", wdb_out2, 3);
    rb_define_method(cWDB, "search", wdb_search, 1);
    rb_define_method(cWDB, "sync", wdb_sync, 0);
    rb_define_method(cWDB, "optimize", wdb_optimize, 0);
    rb_define_method(cWDB, "vanish", wdb_vanish, 0);
    rb_define_method(cWDB, "copy", wdb_copy, 1);
    rb_define_method(cWDB, "path", wdb_path, 0);
    rb_define_method(cWDB, "tnum", wdb_tnum, 0);
    rb_define_method(cWDB, "fsiz", wdb_fsiz, 0);
}
