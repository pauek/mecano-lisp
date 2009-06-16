
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
    long int i = strtold(begin, &end);
    if (size_t(end - begin) == _text.size()) {
      _curr.val = Int(i);
    } else{
      double d = strtod(begin, &end);
      if (size_t(end - begin) == _text.size()) {
	_curr.val = Real(d);
      } else {
	_curr.val = Sym(_text);
      }
    }
    _curr.pos.fin = _pos - 1;
    _enq(_curr);
    _text = "";
  }
  if (_dot) {
    _enq(Token(Range(_pos), Box<char>('.'))); 
    _dot = false;
  }
}

void Tokenizer::_put_normal(char c) {
  if (c == '\n') {
    if (_endl && !_2endls) {
      _enq(Token(Range(_pos), Box<char>('\n')));
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
      _enq(Token(Range(_pos), Box<char>(c)));
    }
  }
  else if (c == '.') {
    if (_dot) _collect();
    _dot = true;
  }
  else if (c == '"') {
    _collect();
    _mode = string;
    _curr.pos.ini = _pos;
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
    if (_text.empty()) _curr.pos.ini = _pos;
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
      _curr.pos.fin = _pos;
      _enq(_curr);
      _text = "";
      _mode = normal;
    } else {
      _text += c;
    }
  }
}

void Tokenizer::put(char c) {
  if (_endl) ++_pos.lin, _pos.col = 0;
  else ++_pos.col;

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
  indents.push_back(t.pos);
  acum->push_back(t.val); 
}

bool SeqScanner::has_indent(Pos p) const {
  vector<Range>::const_reverse_iterator i;
  i = indents.rbegin();
  for (; i != indents.rend(); i++) {
    if (i->ini.col == p.col) return true;
  }
  return false;
}

Token SeqScanner::collect(Pos fin) {
  return Token(Range(ini, fin), _collect());
}

void ListScanner::_collect_tuple() {
  if (acum->empty()) {
    _list->push_back(Nil);
  } 
  else if (acum->size() == 1) {
    _list->push_back(acum[0]);
  } 
  else {
    _list->push_back(acum);
  }
  acum = Tuple();
}

Any ListScanner::_collect() {
  if (!acum->empty()) _collect_tuple();
  return _list;
}

Scanner::Scanner() {
  _init(0);
}

void Scanner::_init(int lin) {
  _stack.push_front(new ListScanner(';', '.', Pos(lin, 0)));
}

void Scanner::_pop() {
  Token t = _stack.front()->collect(_tok.pos.ini);
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
}

void Scanner::_pop_until(char end) {
  while (!_stack.front()->is_end(end)) {
    _pop();
    if (_stack.empty()) {
      throw ScanError("unexpected close");
    }
  }
  _pop();
}

void Scanner::_maybe_break(Pos p) {
  if (_stack.front()->is_lower(p)) {
    bool has_it = _stack.front()->has_indent(p);
    while (!has_it) {
      if (!_stack.front()->can_break) {
	throw ScanError("Unexpected indentation");
      }
      _pop();
      if (_stack.empty()) {
	throw ScanError("Unexpected indentation");
      }
      has_it = _stack.front()->has_indent(p);
    }
    if (_stack.front()->is_initial(p) && 
	_stack.front()->can_break) {
      _stack.front()->put_break();
    }
  }
}

void Scanner::_put() {
  if (_tok.val.is<char>()) {
    char c = *_tok.val.as<char>();
    if (c == '\n' && busy()) {
      _pop_all();
      _init(_tok.pos.fin.lin);
    } else {
      switch (c) {
      case '(': 
	_maybe_break(_tok.pos.ini);
	_push(new TupleScanner(')', _tok.pos.ini));     
	break;
      case ':': 
	_maybe_break(_tok.pos.ini);
	_push(new ListScanner(';', '.', _tok.pos.ini)); 
	break;
      case '{':
	_maybe_break(_tok.pos.ini);
	_push(new ListScanner(';', '}', _tok.pos.ini)); 
	break;
      case ';': _stack.front()->put_sep(c); break;
      case ')': 
      case '.': {
	_pop_until(c); 
	if (_stack.empty()) _init(_tok.pos.fin.lin);
	break;
      }
      }
    }
  } else {
    _maybe_break(_tok.pos.ini);
    _stack.front()->put(_tok);
  }
}

void Scanner::put(char c) {
  _T.put(c);
  while (_T.get(_tok)) _put();
}


void Scanner::putline(const str& s) {
  for (size_t k = 0; k < s.size(); k++) put(s[k]);
  put('\n');
}

} // namespace
