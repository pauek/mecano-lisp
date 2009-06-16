
#include <sstream>
#include "core.hh"
#include "scan.hh"
using std::stringstream;

namespace mc {

// Parse
template<class Seq>
Seq parse_each(const Seq& s, int from = 0) {
  Seq res;
  for (size_t k = from; k < s->size(); k++)
    res->push_back(parse(s[k]));
  return res;
}

bool parse_list(Any in, Any& out) {
  List lin = List::from(in);
  if (lin.is_null()) return false;
  out = parse_each(lin);
  return true;
}

bool parse_tuple(Any in, Any& out) {
  Tuple t = Tuple::from(in);
  if (t.is_null()) return false;
  
  // Catch special forms
  if (t->size() == 0) {
    out = Nil;
  } else if (t[0] == Sym("\\")) {
    Tuple params;
    for (size_t k = 1; k < t->size() - 1; k++) {
      params->push_back(t[k]);
    }
    out = Lambda(params, parse(t->back()));
  } 
  else if (t[0] == Sym("tuple")) {
    out = parse_each(t, 1);
  }
  else if (t[0] == Sym("quote")) {
    if (t->size() != 2) return false;
    out = Sym("unimplemented");
    // out = Quote(parse(t[1]));
  } 
  else if (t[0] == Sym("unquote")) {
    if (t->size() != 2) return false;
    out = Sym("unimplemented");
    // out = UnQuote(parse(t[1]));
  } 
  else if (t[0] == Sym("if")) {
    if (t->size() != 4) return false;
    out = If(parse(t[1]), parse(t[2]), parse(t[3]));
  } 
  else if (t->size() > 1 and t[1] == Sym("=")) {
    if (t->size() != 3) return false;
    out = Set(parse(t[0]), parse(t[2]));
  } 
  else {
    out = Call(parse_each(t));
  }
  return true;
}

Any parse(Any in) {
  Any out; 
  if (in == Sym("nil")) return Nil;
  if (in == Sym("true")) return True;
  if (in == Sym("false")) return False;
  if (parse_list(in, out)) return out;
  if (parse_tuple(in, out)) return out;
  return in;
}

// parse2 //////////////////////////////////////////////////

void parse2(VM& vm, Tuple a);

void call_parse2(VM& vm, Any a) {
  vm.val = Call(Tuple(Prim(parse2), a));
}

// list

struct ParseList : public Continuation {
  List data;
  size_t k;
  ParseList() : k(0) {}
  void call(VM& vm, Any a) {
    data[k] = a;
    k++;
    if (k < data->size()) {
      call_parse2(vm, data[k]);
    } else {
      vm.yield(data);
    }
  }
};

void parse2_list(VM& vm, List l) {
  ParseList *C = new ParseList;
  C->data->append(*l);
  call_parse2(vm, C->data[0]);
  vm.push(C);
}

// lambda

struct ParseLambda : public Continuation {
  Tuple params;
  void call(VM& vm, Any body) {
    vm.yield(Lambda(params, body));
  }
};

void parse2_lambda(VM& vm, Tuple t) {
  ParseLambda *C = new ParseLambda;
  for (size_t k = 1; k < t->size()-1; k++) {
    if (!t[k].is<sym>()) {
      throw ParseError("Parameters in Lambda have to be symbols");
    }
    C->params->push_back(t[k]);
  }
  call_parse2(vm, t->back()); // body
  vm.push(C);
}

// tuple

void parse2_tuple(VM& vm, Tuple t) {
  if (t[0] == Sym("quote")) {
    vm.yield(t[1]);
    return;
  }
  else if (t[0] == Sym("\\")) {
    parse2_lambda(vm, t);
    return;
  }
  
  stringstream out;
  out << "Unable to parse " << t;
  throw ParseError(out.str());
}

void parse2(VM& vm, Tuple args) {
  if (args->size() != 2) {
    throw TypeError("parse: need exactly one argument");
  }
  Any val = args[1];
  if (val == Sym("nil")) {
    vm.yield(Nil);
  } else if (val == Sym("true")) {
    vm.yield(True);
  } else if (val == Sym("false")) {
    vm.yield(False);
  } else if (val.is<lst>()) {
    parse2_list(vm, List::from(val));
  } else if (val.is<tup>()) {
    parse2_tuple(vm, Tuple::from(val));
  } else {
    vm.yield(val);
  }
}

} // namespace
