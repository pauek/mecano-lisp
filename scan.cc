
#include <cassert>
#include <string>
#include <iostream>
#include <list>
#include <queue>
#include "scan.hh"

using namespace std;

namespace mc {

bool _is_space(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}

enum sep_type { any, open, close, middle };

inline bool _is(str& tab, char c) {
  return tab.find(c) != str::npos;
}

bool _is_sep(sep_type typ, char c) {
  static str _open =   "({[`:";
  static str _close =  ")}]'.";
  static str _middle = ";,";
  static str _any = _open + _close + _middle;

  switch (typ) {
  case any:    return _is(_any, c);
  case open:   return _is(_open, c);
  case close:  return _is(_close, c);
  case middle: return _is(_middle, c);
  }
  return false;
}

inline bool _scan_int(str s, Any& res) {
  int v = 0;
  for (unsigned int k = 0; k < s.size(); k++) {
    if (s[k] < '0' || s[k] > '9') return false;
    v = v*10 + int(s[k] - '0');
  }
  res = Int(v);
  return true;
}

Any _scan_atom(str s) {
  Any v;
  return (_scan_int(s, v) ? v : Any(Sym(s)));
}

// SeqScanner /////////////////////////////////////////////

// In TransFn's the vector 'v' has to be cleared.

template<typename Seq>
Any id(Vec& v) {
  Seq s;
  s->swap(v);
  return s;
}

template<TransFn f>
Any rmv_singles(Vec& v) {
  Any res;
  if (v.size() == 1) {
    res = v[0], v.clear();
  } else if (v.size() > 1) {
    res = f(v);
  }
  return res;
}

Any add_tuple(Vec& v) {
  Tuple t;
  t->push_back(Sym("tuple"));
  for (size_t k = 0; k < v.size(); k++) {
    t->push_back(v[k]);
  }
  return t;
}

Any normal(Vec& v) {
  return rmv_singles< id<Tuple> >(v);
}

// SeqScanner //////////////////////////////////////////////

void SeqScanner::add_level(char sep, TransFn fn) {
  _seps += sep;
  _trans.push_back(fn);
  _acum.push_back(Vec());
  _lev = _seps.size() - 1;
}

void SeqScanner::_pop(int lev) {
  assert(lev >= 1 && lev < int(_acum.size()));
  _acum[lev-1].push_back(_trans[lev](_acum[lev]));
}

Any SeqScanner::collect() {
  if (!_in.empty()) {
    _acum[_lev].push_back(normal(_in));
  } 
  while (_lev > 0) _pop(_lev--);
  return _trans[0](_acum[0]);
}

void SeqScanner::put(Any a, int col) {
  _in.push_back(a); 
  if (_inicol == -1) _inicol = col;
}

void SeqScanner::put_sep(char c) {
  int newlev = _seps.find(c);
  assert(newlev >= 0 && newlev < int(_seps.size()));
  if (newlev >= _lev) {
    _acum[newlev].push_back(normal(_in));
    _lev = newlev;
  } else {
    _acum[_lev].push_back(normal(_in));
    while (_lev > newlev) _pop(_lev--);
  }
}

class ListScanner : public SeqScanner {
public:
  ListScanner() : SeqScanner(true) {
    add_level('.', rmv_singles< id<List> >);
    add_level(';', rmv_singles< id<List> >);
    add_level(',', rmv_singles<add_tuple>);
  }
};

// Scanner ////////////////////////////////////////////////

Scanner::Scanner () {
  _flags[endl] = true;
  _lin = -1, _col = -1;
  _mode = normal;
  _acum = "";
  _stack.push_back(new ListScanner());
}

void Scanner::_update_pos(char c) {
  if (_flags[endl]) {
    ++_lin, _col = 0;
    if (c == '\n') {
      // _pop_all();
    } else {
      _flags[endl] = false;
    }
  } else {
    ++_col;
    _flags[endl] = (c == '\n');
  }
}

void Scanner::_put_str(char c) {
  if (_flags[escape]) {
    _flags[escape] = false;
    switch (c) {
    case 'a': _acum += '\a'; break;
    case 'b': _acum += '\b'; break;
    case 'n': _acum += '\n'; break;
    case 'r': _acum += '\r'; break;
    case 't': _acum += '\t'; break;
    case '"': _acum += '"';  break;
    default:
      _acum += '\\', _acum += c;
    }
  } 
  else if (c == '\\') {
    _flags[escape] = true;
  }
  else if (c == '"') {
    _mode = normal; 
    _emit(Str(_acum));
    _acum = "";
  }
  else {
    _acum += c;
  }
}

void Scanner::_emit(Any a) {
  _stack.front()->put(a, _inicol);
}

void Scanner::_collect() {
  if (_acum != "") {
    _emit(_scan_atom(_acum));
    _acum = "";
  }
}

void Scanner::_put_sep(char c) {
  if (_is_sep(open, c)) {
  }
  else if (_is_sep(middle, c)) {
    _stack.front()->put_sep(c);
  }
  else if (_is_sep(close, c)) {
    if (_stack.front()->is_end(c)) {
      _queue.push(_stack.front()->collect());
    }
    else 
      throw Error(_lin, _col, "Unexpected close");
  }
}

void Scanner::_put_normal(char c) {
  if (c == '#') {
    _mode = comment;
  }
  else if (c == '"') {
    _mode = string;
  }
  else if (_is_sep(any, c)) {
    _collect();
    _put_sep(c);
  }
  else if (_is_space(c)) {
    _collect();
  }
  else {
    if (_acum == "") _inicol = _col;
    _acum += c;
  }
}

void Scanner::put(char c) {
  try {
    _update_pos(c);
    switch (_mode) {
    case normal: _put_normal(c); break;
    case string: _put_str(c); break;
    case comment: 
      if (c == '\n') _mode = normal;
    }
  }
  catch (Error& e) {
    cout << e.lin << ':' << e.col << ": "
	 << e.msg << endl;
  }
}

void Scanner::putline(str line) {
  for (size_t k = 0; k < line.size(); k++) {
    put(line[k]);
  }
  put('\n');
}

bool Scanner::get(Any& a) {
  if (_queue.empty()) return false;
  a = _queue.front();
  _queue.pop();
  return true;
}

} // namespace
