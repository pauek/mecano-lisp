// -*- mode: c++ -*-

#ifndef core_h
#define core_h

#include <cassert>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <iterator>
#include <gc/gc_cpp.h>

namespace mc {

typedef std::string str;

struct VM;

// exceptions
struct NullPointer {};
struct TypeError {};

// Boxing //////////////////////////////////////////////////

struct _any_ : public gc {
  virtual void eval(VM& vm) = 0;
  virtual void print(std::ostream& o) = 0;
  virtual bool equal(_any_ *a) = 0;
};

template<typename T>
struct _box_ : public _any_ { 
  T t; 
  _box_(const T& _t) : t(_t) {}
  virtual void eval(VM& vm);
  virtual void print(std::ostream& o);
  virtual bool equal(_any_ *a);
};

template<typename T> class Box;

class Any {
  _any_ *_pany;
  
  template<typename T>
  _box_<T> *ptr_cast() const { 
    return dynamic_cast<_box_<T> *>(_pany);
  } 
  
public:
  Any() : _pany(NULL) {}
  explicit Any(_any_ *o) : _pany(o) {}

  template<typename T> 
  explicit Any(Box<T>& b);

  bool is_null() const  { return _pany == NULL; }
  bool not_null() const { return _pany != NULL; }

  template<typename T> bool   is() const; 
  template<typename T> Box<T> as() const;
  template<typename T> Box<T> cast() const;
  template<typename T> T&     unbox() const;

  bool operator==(Any a) const { 
    return (_pany == NULL 
	    ? a._pany == NULL 
	    : _pany->equal(a._pany));
  }

  bool operator!=(Any a) const {
    return ! operator==(a);
  }

  _any_ *operator->() { return _pany; }
  const _any_ *operator->() const { return _pany; }

  template<typename T> static Any box(const T& t);

  friend std::ostream& 
  operator<<(std::ostream&, const Any&);
};

extern Any Nil;
extern Any True;
extern Any False;

template<typename T>
class Box {
  friend class Any;
  _box_<T> *_pbox;

protected:
  explicit Box(_box_<T> *p) : _pbox(p) {}
  void init(const T& t) { _pbox = new _box_<T>(t); }

  void _ck() const { 
    if (!_pbox) throw TypeError();
  }
    
public:
  Box() : _pbox(NULL) {}
  Box(Any a) : _pbox(a.as<T>()._pbox) {}
  explicit Box(const T& t) { init(t); }

  const Box<T>& operator=(const T& t);
  T *operator->() { _ck(); return &(_pbox->t); }
  const T *operator->() const { _ck(); return &(_pbox->t); }
  const T& operator*()  const { _ck(); return _pbox->t; }

  // operator T() const { return _pbox->t; }

  bool is_null()  const { return _pbox == NULL; }
  bool not_null() const { return _pbox != NULL; }

  operator Any() const { return Any(_pbox); }

  const T& unbox() const;
        T& unbox();
};

template<typename T>
bool operator==(const Box<T>& b, Any a) {
  return Any(b) == a;
}

template<typename T>
inline Any::Any(Box<T>& b) 
  : _pany(b._pbox) {}

template<typename T>
inline Any Any::box(const T& t) {
  return Any(new _box_<T>(t));
}

template<>
inline Any Any::box(const Any& a) { return a; }

template<typename T>
inline bool Any::is() const { 
  return ptr_cast<T>() != 0; 
}

template<typename T>
inline Box<T> Any::as() const { 
  return Box<T>(ptr_cast<T>()); 
}

template<typename T>
inline Box<T> Any::cast() const {
  if (!is<T>()) throw TypeError();
  return as<T>();
}

template<typename T>
inline T& Any::unbox() const { 
  return cast<T>().unbox(); 
}

template<typename T>
inline const T& Box<T>::unbox() const { 
  if (not_null()) return _pbox->t; 
  throw NullPointer();
}

template<typename T>
inline T& Box<T>::unbox() {
  if (not_null()) return _pbox->t;
  throw NullPointer();
}

template<typename T>
inline const Box<T>& Box<T>::operator=(const T& t) {
  _ck(); _pbox->t = t;
  return *this;
}

inline std::ostream& 
operator<<(std::ostream& o, const Any& a) {
  if (a._pany == NULL) o << "Nil";
  else a._pany->print(o);
  return o;
}

template<typename T>
inline std::ostream& 
operator<<(std::ostream& o, const Box<T>& b) {
  return o << b.unbox();
}

// Types ///////////////////////////////////////////////////

struct _bool { 
  bool v; 
  _bool(bool _v) : v(_v) {}
  bool operator==(_bool b) const { return v == b.v; }
};

struct _str {
  str s;
  _str(str _s) : s(_s) {}
  _str(const char *_s) : s(_s) {}
  bool operator==(_str str) const { return s == str.s; }
};

inline std::ostream& 
operator<<(std::ostream& o, const _bool& b) {
  return o << (b.v ? "True" : "False");
}

inline std::ostream& 
operator<<(std::ostream& o, const _str& s) {
  return o << '"' << s.s << '"';
}

typedef Box<int>     Int;
typedef Box<_bool>   Bool;
typedef Box<char>    Char;
typedef Box<double>  Real;
typedef Box<_str>    Str;

class sym {
  static std::vector<str> _syms;
  static int get_id(str name);
  static str get_name(int id) { return _syms[id]; }
  int _id;

public:
  sym(str name) : _id(get_id(name)) {}
  int id()   const { return _id; }
  str name() const { return get_name(_id); }
  bool operator==(const sym& s) const { 
    return _id == s._id; 
  }
};

class Sym : public Box<sym> {
public:
  Sym(str name) : Box<sym>(sym(name)) {}
  Sym(Any a) : Box<sym>(a) {}
  Sym(sym s) : Box<sym>(s) {}
};

struct seq : public std::vector<Any> {
  void append(const std::vector<Any>& v) {
    copy(v.begin(), v.end(), back_inserter(*this));
  }
};
struct tup : seq {};
struct lst : seq {};

std::ostream& operator<<(std::ostream&, const _bool&);
std::ostream& operator<<(std::ostream&, const sym&);
std::ostream& operator<<(std::ostream&, const tup&);
std::ostream& operator<<(std::ostream&, const lst&);

template<typename seq>
class Seq : public Box<seq> {
  void init() { Box<seq>::init(seq()); }
  void add(Any v) { (*this)->push_back(v); }

  struct _dummy_ {};

  explicit Seq(_dummy_) {} // Nil
public:
  Seq() { init(); }

  Seq(const std::vector<Any>& v) 
    : Box<seq>(v) {}

  Seq(const std::list<Any>& l) 
    : Box<seq>(std::vector<Any>(l.begin(), l.end())) {}

  explicit Seq(seq s) : Box<seq>(s) {}

  explicit Seq(Any v1) { 
    init(), add(v1); 
  }
  Seq(Any v1, Any v2) { 
    init(), add(v1), add(v2); 
  }
  Seq(Any v1, Any v2, Any v3) { 
    init(), add(v1), add(v2), add(v3); 
  }
  Seq(Any v1, Any v2, Any v3, Any v4) { 
    init(), add(v1), add(v2), add(v3), add(v4);
  }

  const Any& operator[](int n) const { 
    return (*this)->at(n); 
  }
  Any& operator[](int n) { 
    return (*this)->at(n); 
  }

  static Seq from(Any a) {
    Box<seq> box(a);
    if (box.is_null()) return Seq(_dummy_());
    else return Seq(*box);
  }
};

typedef Seq<tup> Tuple;
typedef Seq<lst> List;

// Environment /////////////////////////////////////////////

class Env {
  Env  *_upper;
  Tuple _locals;
  std::map<int, int> _transl;

  bool _bind(sym s, Any v);
public:
  Env() : _upper(NULL) {}
  Env *extend(Any params, Tuple locals);
  
  int  size() const { return _locals->size(); }
  Any  operator[](int n) const { return _locals[n]; }
  Any& operator[](int n) { return _locals[n]; }
  Any  lookup(sym s) const;
  void bind(sym s, Any v);
};

struct VM;
struct Callable {
  virtual void call(VM&, Any) = 0;
};

struct Executable {
  virtual void exec(VM&, Tuple) = 0;
  virtual str  type() const = 0;
};

struct Continuation : public Callable {
  Continuation *prev;
};

// Virtual Machine /////////////////////////////////////////


struct VM {
  Any           val;
  Env          *env;
  Continuation *cont;
  bool          breturn, berror;
  str           errmsg;

  VM() { reset(); }
  
  void reset();

  void init();
  void error(str e)  { berror = true, errmsg = e; }
  void yield(Any a)  { val = a, breturn = true; }
  void push(Continuation *c) 
                     { c->prev = cont, cont = c; }
  void pop()         { cont = cont->prev; }
  void run()         { while (step()); }
  Any  eval(Any a)   { val = a; run(); return val; }
  bool step();
};

// Functions ///////////////////////////////////////////////

class func {
  Executable* _pfn;

public:
  func(Executable *p) : _pfn(p) {}
  void exec(VM& vm, Tuple locals) {
    _pfn->exec(vm, locals);
  }

  friend
  std::ostream& operator<<(std::ostream&, const func&);

  bool operator==(const func& f) const { 
    return _pfn == f._pfn; 
  }
};

typedef Box<func> Func;

class prim : public Executable {
public:
  typedef Any (*Fn)(Tuple args);
private:
  Fn _fn;
public:
  prim(Fn f) : _fn(f) {}
  void exec(VM& vm, Tuple locals) {
    vm.yield(_fn(locals));
  }
  str type() const { return "Prim"; }
  operator func() { return func(this); }

  bool operator==(const prim& p) const { 
    return _fn == p._fn; 
  }

  friend
  std::ostream& operator<<(std::ostream&, const prim&);
};

class Prim : public Box<func> {
public:
  Prim(prim::Fn f) : Box<func>(new prim(f)) {}
};

class closure : public Executable {
  Env *_env;
  Any  _prms, _body;

  struct Return : public Continuation {
    Env *_prev;
    Return(Env *e) : _prev(e) {}
    void call(VM& vm, Any a) {
      vm.pop();
      vm.env = _prev;
      vm.yield(a);
    }
  };

public:
  closure(Env *e, Any prms, Any body)
    : _env(e), _prms(prms), _body(body) {}

  void exec(VM& vm, Tuple locals) {
    vm.push(new Return(vm.env));
    vm.env = _env->extend(_prms, locals);
    vm.val = _body;
  }
  
  str type() const { return "Closure"; }
};

class Closure : public Box<func> {
public:
  Closure(Env *e, Any prms, Any body) 
    : Box<func>(new closure(e, prms, body)) {}
};

// Cont ////////////////////////////////////////////////////

template<typename T>
struct Cont : public Continuation {
  const T& form;
  Cont(const T& _form) : form(_form) {}
  void call(VM& vm, Any a) { T::cont(vm, *this, a); }
};

// Special Forms ///////////////////////////////////////////

// If

class cond {
  Any _expr, _then, _else;
public:
  cond(Any expr, Any then, Any els) 
    : _expr(expr), _then(then), _else(els) {}

  static void eval(VM& vm, const cond& c) {
    vm.push(new Cont<cond>(c));
    vm.val = c._expr;
  }
  static void cont(VM& vm, const Cont<cond>& c, Any a) {
    vm.pop();
    Bool expr = a;
    vm.yield(expr == True ? c.form._then : c.form._else);
  }
  
  bool operator==(const cond& c) const {
    return _expr == c._expr &&
      _then == c._then &&
      _else == c._else;
  }

  friend 
  std::ostream& operator<<(std::ostream&, const cond&);
};

class If : public Box<cond> {
public:
  If(Any expr, Any then, Any els)
    : Box<cond>(cond(expr, then, els)) {}
};

// Set

class set {
  Sym _place;
  Any _val;
public:
  set(Sym place, Any val) : _place(place), _val(val) {
    assert(_place.not_null());
  }
  
  static void eval(VM& vm, const set& s) {
    vm.push(new Cont<set>(s));
    vm.val = s._val;
  }

  static void cont(VM& vm, const Cont<set>& c, Any a) {
    vm.pop();
    vm.env->bind(*c.form._place, a);
    vm.yield(a);
  }

  bool operator==(const set& s) const {
    return _place == s._place && _val == s._val;
  }

  friend
  std::ostream& operator<<(std::ostream&, const set&);
};

class Set : public Box<set> {
public:
  Set(Sym place, Any val) 
    : Box<set>(set(place, val)) {}
};

// Call

class call {
  Tuple _form;
public:
  call(Tuple form) : _form(form) {}

  static void eval(VM& vm, const call& c) {
    vm.push(new Cont<call>(c));
    vm.val = c._form;
  }
  
  static void cont(VM& vm, const Cont<call>& c, Any a) {
    vm.pop();
    Tuple form = Tuple::from(a);
    std::cout << form << std::endl;
    if (form.is_null()) {
      std::cout << form << std::endl;
      vm.error("Call: not calling tuple!");
      return;
    }
    Func f = form[0];
    if (f.is_null()) {
      vm.error("Call: head is not a function");
      return;
    }
    f->exec(vm, form);
  }

  bool operator==(const call& c) const { 
    return _form == c._form; 
  }

  friend
  std::ostream& operator<<(std::ostream&, const call&);

};

class Call : public Box<call> {
public:
  Call(Tuple form) : Box<call>(call(form)) {}
};

// Lambda

class lambda {
  Tuple _args;
  Any   _body;
public:
  lambda(Tuple args, Any body) 
    : _args(args), _body(body) {}

  static void eval(VM& vm, const lambda& l) {
    vm.yield(Closure(vm.env, l._args, l._body));
  }

  bool operator==(const lambda& l) const {
    return _args == l._args && _body == l._body;
  }

  friend
  std::ostream& operator<<(std::ostream&, const lambda&);
};

class Lambda : public Box<lambda> {
public:
  Lambda(Tuple args, Any body)
    : Box<lambda>(lambda(args, body)) {}
};

// Eval ////////////////////////////////////////////////////

template<typename T>
void eval(VM& vm, const T& t) { T::eval(vm, t); }

#define EVAL_BASIC(TYPE, type)				\
  template<> inline					\
  void eval(VM& vm, const type& i) { vm.yield(TYPE(i)); }

EVAL_BASIC(Int,  int)
EVAL_BASIC(Bool, _bool)
EVAL_BASIC(Char, char)
EVAL_BASIC(Str,  _str)
EVAL_BASIC(Real, double)
EVAL_BASIC(Func, func)

inline void eval(VM& vm, const sym& s) {
  vm.yield(vm.env->lookup(s));
}

template<typename seq>
struct seqCont : public Continuation {
  const seq& l;
  unsigned int k;
  seqCont(const seq& _l) : l(_l), k(0) {}
  void call(VM& vm, Any a) {
    if (++k < l.size()) {
      vm.val = l[k];
    } else {
      vm.pop();
      vm.yield(a);
    }
  }
};

typedef seqCont<lst> lstCont;

struct tupCont : public seqCont<tup> {
  Tuple result;
  tupCont(const tup& t) : seqCont<tup>(t) {}
  void call(VM& vm, Any a) {
    result->push_back(a);
    seqCont<tup>::call(vm, result);
  }
};

inline void eval(VM& vm, const tup& t) {
  vm.push(new tupCont(t));
  vm.val = t[0];
};

inline void eval(VM& vm, const lst& l) {
  vm.push(new lstCont(l));
  vm.val = l[0];
}

// Here because VM is needed

template<typename T>
void _box_<T>::eval(VM& vm) {
  mc::eval(vm, t);
}

template<typename T>
bool _box_<T>::equal(_any_ *a) { 
  _box_<T> *p = dynamic_cast<_box_<T> *>(a);
  return p && p->t == t;
}

template<typename T>
void _box_<T>::print(std::ostream& o) { 
  o << t;
}

} // namespace

#endif
