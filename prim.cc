
#include <cstdlib>
#include "core.hh"
#include "prim.hh"
#include "scan.hh"

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

Any scan(Any a) {
  Str s = a;
  if (s.is_null()) 
    throw TypeError("scan: argument must be string");

  Scanner S;
  S.putline(*s);
  S.put('\n');
  List l;
  while (S.get(a)) {
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

Any mklist(Tuple args) {
  List l;
  l->append(args->begin() + 1, args->end());
  return l;
}

} // namespace
