
#include "core.hh"
#include "scan.hh"

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
  else if (t->size() > 1 && t[1] == Sym("=")) {
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

} // namespace
