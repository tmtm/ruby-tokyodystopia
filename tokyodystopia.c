#include <ruby.h>
#include <tcqdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

static VALUE mTD;
static VALUE cQDB;
static VALUE eQDB;

static VALUE qdb_allocate(VALUE klass)
{
    TCQDB *qdb = tcqdbnew();
    return Data_Wrap_Struct(klass, NULL, tcqdbdel, qdb);
}

#define QDB_CHK(x) if (!(x)) rb_raise(eQDB, "%s", tcqdberrmsg(tcqdbecode(qdb)))

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
    QDB_CHK(tcqdbsetfwmmax(qdb, NUM2LONG(fwmmax)));
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
        rb_raise(eQDB, "%s", tcqdberrmsg(tcqdbecode(qdb)));
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

void Init_tokyodystopia()
{
    mTD = rb_define_module("TokyoDystopia");
    cQDB = rb_define_class_under(mTD, "QDB", rb_cObject);
    eQDB = rb_define_class_under(cQDB, "Error", rb_eStandardError);

    rb_define_const(mTD, "VERSION", rb_usascii_str_new2(tdversion));
    rb_define_const(mTD, "QDBTLARGE", INT2NUM(QDBTLARGE));
    rb_define_const(mTD, "QDBTDEFLATE", INT2NUM(QDBTDEFLATE));
    rb_define_const(mTD, "QDBTBZIP", INT2NUM(QDBTBZIP));
    rb_define_const(mTD, "QDBTTCBS", INT2NUM(QDBTTCBS));
    rb_define_const(mTD, "QDBOREADER", INT2NUM(QDBOREADER));
    rb_define_const(mTD, "QDBOWRITER", INT2NUM(QDBOWRITER));
    rb_define_const(mTD, "QDBOCREAT", INT2NUM(QDBOCREAT));
    rb_define_const(mTD, "QDBOTRUNC", INT2NUM(QDBOTRUNC));
    rb_define_const(mTD, "QDBONOLCK", INT2NUM(QDBONOLCK));
    rb_define_const(mTD, "QDBOLCKNB", INT2NUM(QDBOLCKNB));
    rb_define_const(mTD, "QDBSSUBSTR", INT2NUM(QDBSSUBSTR));
    rb_define_const(mTD, "QDBSPREFIX", INT2NUM(QDBSPREFIX));
    rb_define_const(mTD, "QDBSSUFFIX", INT2NUM(QDBSSUFFIX));
    rb_define_const(mTD, "QDBSFULL", INT2NUM(QDBSFULL));

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
}
