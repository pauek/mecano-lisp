
#ifndef prim_h
#define prim_h

namespace mc {

typedef Any (*SimpleFn)(Tuple args);
typedef Any (*UnaryFn)(Any a);
typedef Any (*BinaryFn)(Any a, Any b);

template<SimpleFn fn>
void direct(VM& vm, Tuple args) {
  vm.yield(fn(args));
}

template<UnaryFn fn>
Any unary(Tuple args) {
  if (args->size() > 2) {
    throw TypeError("too many arguments");
  }
  return fn(args[1]);
}

template<int N>
Any nth(Any a) {
  Tuple t = Tuple::from(a);
  return (t.not_null() && t->size() >= N ? t[N-1] : Nil);
}

template<BinaryFn fn>
Any pairwise(Tuple args) {
  if (args->size() < 3) {
    throw TypeError("number of args must be > 2");
  }
  Any a, b = args[1];
  for (size_t k = 2; k < args->size(); k++) {
    a = b;
    b = args[k];
    if (fn(a, b) == False) return False;
  }
  return True;
}

template<typename Seq>
Any mkseq(Tuple args) {
  Seq s;
  s->append(args->begin() + 1, args->end());
  return s;
}

void quit(VM& vm, Tuple args);
void callcc(VM& vm, Tuple args);
void apply(VM& vm, Tuple args);
void pmap(VM& vm, Tuple args);
void peval(VM& vm, Tuple args);
Any  len(Any a);
Any  mksym(Any a);
Any  scan(Any a);
Any  less(Any a, Any b);
Any  equal(Any a, Any b);
Any  sum(Tuple args);

} // namespace

#endif
