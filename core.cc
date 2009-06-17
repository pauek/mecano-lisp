
#include <cassert>
#include <algorithm>
#include <iostream>
#include "core.hh"
#include "scan.hh"
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
  while (i != t.end()) o << " " << *i++;
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

bool Env::lookup(sym s, Any& a) const {
  map<int, int>::const_iterator i = _transl.find(s.id());
  if (i != _transl.end()) {
    a = _locals[i->second];
    return true;
  } else {
    if (!_upper) return false;
    else return _upper->lookup(s, a);
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

// Eval ////////////////////////////////////////////////////

struct RestoreQuoteLevel : public Continuation {
  int level;
  RestoreQuoteLevel(int l) : level(l) {}
  void call(VM& vm, Any a) {
    vm.pop();
    vm.lquote = level;
    vm.yield(a);
  }
};

struct Invoke : public Continuation {
  void call(VM& vm, Any a) {
    Tuple t = Tuple::from(a);
    assert(t.not_null());
    vm.pop();
    cout << "invoke" << endl;
    vm.val = Call(t);
  }
};

void eval(VM& vm, const tup& t) {
  if (t.size() == 0) {
    vm.yield(Nil);
  }
  else if (t[0] == Sym("unquote")) {
    if (t.size() != 2) {
      throw ParseError("Unquote needs only one argument");
    }
    vm.push(new RestoreQuoteLevel(vm.lquote));
    vm.lquote--;
    if (vm.lquote < 0) {
      throw ParseError("Quote level below zero");
    }
    vm.val = t[1];
  } 
  else if (t[0] == Sym("quote")) {
    if (t.size() != 2) {
      throw ParseError("Quote needs only one argument");
    }
    vm.push(new RestoreQuoteLevel(vm.lquote));
    vm.lquote++;
    vm.val = t[1];
  } 
  else {
    if (vm.lquote > 0) {
      vm.push(new tupCont(t));
      vm.val = t[0];
      return;
    }

    if (t.size() > 1 and t[1] == Sym("=")) {
      if (t.size() != 3) {
	throw ParseError("Set needs exactly 2 arguments");
      }
      vm.val = Set(t[0], t[2]);
    } 
    else if (t[0] == Sym("\\")) {
      Tuple params;
      for (size_t k = 1; k < t.size() - 1; k++) {
	params->push_back(t[k]);
      }
      vm.val = Lambda(params, t.back());
    } 
    else if (t[0] == Sym("if")) {
      if (t.size() != 4) {
	throw ParseError("If needs 3 arguments");
      }
      vm.val = If(t[1], t[2], t[3]);
    } 
    else {
      vm.push(new Invoke);
      vm.push(new tupCont(t));
      vm.val = t[0];
    }
  }
};

// Virtual Machine /////////////////////////////////////////

void VM::reset() {
  berror = breturn = false;
  lquote = 0;
  cont = NULL;
  val = Nil;
  env = new Env();
  init();
}

void VM::init() {
  env->bind(sym("quit"),    Prim(quit));
  env->bind(sym("apply"),   Prim(apply));
  env->bind(sym("sym"),     Prim(direct< unary<mksym> >));
  env->bind(sym("call/cc"), Prim(callcc));
  env->bind(sym("scan"),    Prim(direct< unary<scan> >));
  env->bind(sym("list"),    Prim(direct< mkseq<List> >));
  env->bind(sym("tuple"),   Prim(direct< mkseq<Tuple> >));
  env->bind(sym("+"),       Prim(direct<sum>));
  env->bind(sym("<"),       Prim(direct< pairwise<less> >));
  env->bind(sym("=="),      Prim(direct< pairwise<equal> >));
  env->bind(sym("1st"),     Prim(direct< unary< nth<1> > >));
  env->bind(sym("2nd"),     Prim(direct< unary< nth<2> > >));
  env->bind(sym("3rd"),     Prim(direct< unary< nth<3> > >));
  env->bind(sym("4th"),     Prim(direct< unary< nth<4> > >));
  env->bind(sym("5th"),     Prim(direct< unary< nth<5> > >));
  env->bind(sym("len"),     Prim(direct< unary<len> >));
}

bool VM::step() {
  try {
    if (breturn) {
      breturn = false;
      if (!cont) return false;
      cout << "cont[" << lquote << "] " << val << endl;
      cont->call(*this, val);
    }
    else {
      if (val.is_null()) return false;
      cout << "eval[" << lquote << "] " << val << endl;
      val->eval(*this);
    }
    return true;
  }
  catch (Error& e) {
    cerr << "Error: " << e.msg << endl;
    val = Nil;
    cont = NULL;
    return false;
  }
}

} // namespace
