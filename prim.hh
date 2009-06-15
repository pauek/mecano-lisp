
#ifndef prim_h
#define prim_h

namespace mc {

typedef Any (*SimpleFn)(Tuple args);

template<SimpleFn fn>
void direct(VM& vm, Tuple args) {
  vm.yield(fn(args));
}

typedef Any (*UnaryFn)(Any a);

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

void quit(VM& vm, Tuple args);
void apply(VM& vm, Tuple args);
Any  len(Any a);
Any  sum(Tuple args);

} // namespace

#endif
