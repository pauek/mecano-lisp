
#include <cassert>
#include <algorithm>
#include <iostream>
#include "core.hh"
#include "prim.hh"
using namespace std;

namespace mc {

Any Nil;
Any True = Bool(_bool(true));
Any False = Bool(_bool(false));

vector<string> sym::_syms;

int sym::get_id(string name) {
  vector<string>::iterator i = 
    find(_syms.begin(), _syms.end(), name);

  if (i != _syms.end())
    return i - _syms.begin();
  else {
    int k = _syms.size();
    _syms.push_back(name);
    return k;
  }
}

ostream& operator<<(ostream& o, const tup& t) {
  o << "(";
  tup::const_iterator i = t.begin();
  if (i != t.end()) o << *i++;
  while (i != t.end()) o << ' ' << *i++;
  return o << ")";
}

ostream& operator<<(ostream& o, const lst& l) {
  o << "{";
  tup::const_iterator i = l.begin();
  if (i != l.end()) o << *i++;
  while (i != l.end()) o << "; " << *i++;
  return o << "}";
}

ostream& operator<<(ostream& o, const cond& c) {
  return o << "<If " 
	   << c._expr << ' ' 
	   << c._then << ' ' 
	   << c._else << '>';
}

ostream& operator<<(ostream& o, const set& s) {
  return o << "<Set " << s._place << ' ' << s._val << '>';
}


ostream& operator<<(ostream& o, const sym& s) {
  return o << s.name();
}

ostream& operator<<(ostream& o, const prim& p) {
  return o << "<Prim " << p._fn << '>';
}

ostream& operator<<(ostream& o, const func& p) {
  return o << '<' << p._pfn->type() << ' ' << p._pfn << '>';
}

ostream& operator<<(ostream& o, const call& c) {
  return o << "<Call " << c._form << '>';
}

ostream& operator<<(ostream& o, const lambda& l) {
  return o << "<Lambda " 
	   << l._args << ' ' 
	   << l._body << '>';
}

// Environment /////////////////////////////////////////////

Env *Env::extend(Any params, Tuple locals) {
  Env *E = new Env;
  E->_upper = this;
  E->_locals = locals;

  Tuple transl = Tuple::from(params);
  if (transl.not_null()) {
    assert(transl->size() <= locals->size());
    for (unsigned int k = 0; k < transl->size(); k++) {
      Sym s = transl[k];
      assert(s.not_null());
      E->_transl[s->id()] = k + 1;
    }
  }
  return E;
}

Any Env::lookup(sym s) const {
  map<int, int>::const_iterator i = _transl.find(s.id());
  if (i != _transl.end()) {
    return _locals[i->second];
  } else {
    return (_upper ? _upper->lookup(s) : Nil);
  }
}

bool Env::_bind(sym s, Any a) {
  map<int,int>::iterator i = _transl.find(s.id());
  if (i != _transl.end()) {
    _locals[i->second] = a;
    return true;
  } else {
    return (_upper ? _upper->_bind(s, a) : false);
  }
}

void Env::bind(sym s, Any a) {
  if (!_bind(s, a)) {
    _transl[s.id()] = _locals->size();
    _locals->push_back(a);
  }
}


// Virtual Machine /////////////////////////////////////////

void VM::reset() {
  berror = breturn = false;
  cont = NULL;
  val = Nil;
  env = new Env();
  init();
}

void VM::init() {
  env->bind(sym("quit"), Prim(quit));
  env->bind(sym("apply"), Prim(apply));
  env->bind(sym("+"), Prim(direct<sum>));
  env->bind(sym("<"), Prim(direct< pairwise<less> >));
  env->bind(sym("=="), Prim(direct< pairwise<equal> >));
  env->bind(sym("1st"), Prim(direct< unary< nth<1> > >));
  env->bind(sym("2nd"), Prim(direct< unary< nth<2> > >));
  env->bind(sym("3rd"), Prim(direct< unary< nth<3> > >));
  env->bind(sym("4th"), Prim(direct< unary< nth<4> > >));
  env->bind(sym("5th"), Prim(direct< unary< nth<5> > >));
  env->bind(sym("len"), Prim(direct< unary<len> >));
}

bool VM::step() {
  try {
    if (breturn) {
      breturn = false;
      if (!cont) return false;
      cont->call(*this, val);
    }
    else {
      if (val.is_null()) return false;
      val->eval(*this);
    }
    return true;
  }
  catch (Error& e) {
    cerr << "Error: " << e.msg << endl;
    val = Nil;
    return false;
  }
}

} // namespace
