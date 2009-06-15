
#include <cstdlib>
#include "core.hh"
#include "prim.hh"

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

Any len(Any a) {
  Tuple t = Tuple::from(a);
  if (t.not_null()) return Int(t->size());
  List l = List::from(a);
  if (l.not_null()) return Int(l->size());
  throw TypeError("len: argument is not a sequence");
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
