
#include <cstdlib>
#include "core.hh"
#include "scan.hh"
using namespace std;

namespace mc {

const string seps = ":;(){}";
inline bool issep(char c) {
  return seps.find(c) != str::npos;
}

// Tokenizer ///////////////////////////////////////////////

void Tokenizer::_collect() {
  if (!_text.empty()) {
    const char *begin = _text.c_str();
    char *end;
    double d = strtod(begin, &end);
    if (end != begin) {
      _curr.val = Real(d);
    } else {
      long int i = strtold(begin, &end);
      if (end != begin) {
	_curr.val = Int(i);
      } else {
	_curr.val = Sym(_text);
      }
    }
    _enq(_curr);
    _text = "";
  }
  if (_dot) {
    _enq(Token(_lin, _col-1, Box<char>('.'))); 
    _dot = false;
  }
}

void Tokenizer::_put_normal(char c) {
  if (c == '\n') {
    if (_endl && !_2endls) {
      _enq(Token(_lin, _col, Box<char>('\n')));
      _2endls = true;
    }
    _endl = true;
  } else {
    _endl = _2endls = false;
  }

  bool bsep = issep(c);
  if (isspace(c) || bsep) {
    _collect();
    if (bsep) {
      _enq(Token(_lin, _col, Box<char>(c)));
    }
  }
  else if (c == '.') {
    if (_dot) _collect();
    _dot = true;
  }
  else if (c == '"') {
    _collect();
    _mode = string;
    _curr.lin = _lin;
    _curr.col = _col;
    _escape = false;
  }
  else if (c == '#') {
    _collect();
    _mode = comment;
  }
  else {
    if (_dot) {
      _text += '.';
      _dot = false;
    }
    if (_text.empty()) {
      _curr.lin = _lin;
      _curr.col = _col;
    }
    _text += c;
  }
}

void Tokenizer::_put_string(char c) {
  if (_escape) {
    switch (c) {
    case 'n':  _text += '\n'; break;
    case '\\': _text += '\\'; break;
    case '"':  _text += '"';  break;
    }
  } else {
    if (c == '\\') {
      _escape = true;
    } else if (c == '"') {
      _curr.val = Str(_text);
      _enq(_curr);
      _text = "";
      _mode = normal;
    } else {
      _text += c;
    }
  }
}

void Tokenizer::put(char c) {
  if (_endl) ++_lin, _col = 0;
  else ++_col;

  switch (_mode) {
  case normal: _put_normal(c); break;
  case string: _put_string(c); break;
  case comment: 
    if (c == '\n') {
      _mode = normal;
      _endl = true;
    }
  }
}

// Scanner /////////////////////////////////////////////////

void SeqScanner::put(Token& t) {
  if (_indents.empty()) {
    _inilin = t.lin;
    _inicol = t.col;
  }
  _indents.push_back(t.col);
  _acum->push_back(t.val); 
}

Token SeqScanner::collect() {
  return Token(_inilin, _inicol, _collect());
}

void ListScanner::_collect_tuple() {
  if (_acum->empty()) {
    _list->push_back(Nil);
  } 
  else if (_acum->size() == 1) {
    _list->push_back(_acum[0]);
  } 
  else {
    _list->push_back(_acum);
  }
  _acum = Tuple();
}

Any ListScanner::_collect() {
  if (!_acum->empty()) _collect_tuple();
  return _list;
}

Scanner::Scanner() {
  _init();
}

void Scanner::_init() {
  _stack.push_front(new ListScanner(';', '.'));
}

void Scanner::_pop() {
  Token t = _stack.front()->collect();
  delete _stack.front();
  _stack.pop_front();
  if (_stack.empty()) {
    _enq(t.val);
  } else {
    _stack.front()->put(t);
  }
}

void Scanner::_pop_all() {
  while (!_stack.empty()) _pop();
  _init();
}

void Scanner::_put(Token& t) {
  if (t.val.is<char>()) {
    char c = *t.val.as<char>();
    if (c == '\n' && busy()) {
      _pop_all();
    } else {
      switch (c) {
      case '(': _push(new TupleScanner(')'));     break;
      case ':': _push(new ListScanner(';', '.')); break;
      case '{': _push(new ListScanner(';', '}')); break;

      case ';': {
	if (!_stack.front()->is_sep(c))
	  throw ScanError("Unexpected separator");
	_stack.front()->put_sep();
	break;
      }

      case ')': 
      case '.': {
	while (!_stack.front()->is_end(c)) {
	  _pop();
	  if (_stack.empty()) {
	    throw ScanError("unexpected close");
	  }
	}
	_pop();
	if (_stack.empty()) _init();
	break;
      }
      }
    }
  } else {
    _stack.front()->put(t);
  }
}

void Scanner::put(char c) {
  _T.put(c);
  Token t;
  while (_T.get(t)) _put(t);
}


void Scanner::putline(str s) {
  for (size_t k = 0; k < s.size(); k++) put(s[k]);
  put('\n');
}

} // namespace
