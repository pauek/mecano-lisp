
#include <cstdlib>
#include "core.hh"
#include "prim.hh"
#include "read.hh"
using namespace std;

namespace mc {

void quit(VM& vm, Tuple args) {
  exit(0);
}

void apply(VM& vm, Tuple args) {
  if (args->size() != 3) {
    throw TypeError("apply: need exactly two arguments");
  }
  Tuple form(args[1]);
  Tuple prms = Tuple::from(args[2]);
  if (prms.is_null()) {
    throw TypeError("apply: second argument must be a tuple");
  }
  form->append(*prms);
  eval(vm, call(form));
}

void peval(VM& vm, Tuple args) {
  if (args->size() != 2) {
    throw TypeError("eval: need exactly one argument");
  }
  vm.val = args[1];
}

struct savecc : public Executable {
  Continuation *saved;
  savecc(Continuation *s) : saved(s) {}
  void exec(VM& vm, Tuple args) {
    if (args->size() != 2) {
      throw TypeError("savecc: need exactly one argument");
    }
    vm.cont = saved;
    vm.val = args[1];
  }
  str type() const { return "SaveCC"; }
};

void callcc(VM& vm, Tuple args) {
  if (args->size() != 2) {
    throw TypeError("call/cc: need exactly one argument");
  }
  vm.val = Call(Tuple(args[1], Func(new savecc(vm.cont))));
}

template<typename Seq>
struct MapCont : public Continuation {
  size_t k;
  Func   fn;
  Seq    result;
  MapCont(Func f, Seq seq)
    : k(0), fn(f), result(*seq) {}

  void call(VM& vm, Any a) {
    result[k] = a;
    k++;
    if (k < result->size()) {
      vm.val = Call(Tuple(fn, result[k]));
    } else {
      vm.pop();
      vm.yield(result);
    }
  }
};

void pmap(VM& vm, Tuple args) {
  if (args->size() != 3) {
    throw TypeError("map: need exactly two arguments");
  }
  Func f = args[1];
  if (f.is_null()) {
    throw TypeError("map: first argument must be a function");
  }
  Tuple t = Tuple::from(args[2]);
  if (t.not_null()) {
    vm.val = Call(Tuple(f, t[0]));
    vm.push(new MapCont<Tuple>(f, t));
    return;
  }
  List l = List::from(args[2]);
  if (l.not_null()) {
    vm.val = Call(Tuple(f, l[0]));
    vm.push(new MapCont<List>(f, l));
    return;
  }
  throw TypeError("map: second argument must be a sequence");
}

Any mksym(Any a) {
  Str s = a;
  if (s.is_null()) 
    throw TypeError("argument must be a string");

  return Sym(*s);
}

Any scan(Any a) {
  Str s = a;
  if (s.is_null()) 
    throw TypeError("scan: argument must be string");

  Reader R;
  R.putline(*s);
  R.put('\n');
  List l;
  while (R.get(a)) {
    List ll = List::from(a);
    l->append(*ll);
  }
  return l;
}

Any len(Any a) {
  Tuple t = Tuple::from(a);
  if (t.not_null()) return Int(t->size());
  List l = List::from(a);
  if (l.not_null()) return Int(l->size());
  throw TypeError("len: argument is not a sequence");
}

Any equal(Any a, Any b) {
  return Bool(a == b);
}

Any less(Any a, Any b) {
  Int i(a);
  if (i.not_null()) {
    Int j(b);
    if (j.is_null()) {
      throw TypeError("less: args of different type");
    }
    return Bool(*i < *j);
  }
  Str sa(a);
  if (sa.not_null()) {
    Str sb = b;
    if (sb.is_null()) {
      throw TypeError("less: args of different type");
    }
    return Bool(*sa < *sb);
  }
  throw TypeError("less: wrong type");
}

Any sum(Tuple args) {
  // TODO: Handle reals, strings, lists, tuples, etc.
  int sum = 0;
  if (!args->empty()) {
    tup::iterator i = args->begin();
    while (i != args->end()) {
      Int n = *i++;
      if (n.not_null()) sum += *n;
    }
  }
  return Int(sum);
}


} // namespace
