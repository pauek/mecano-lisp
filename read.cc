
#include <cstdlib>
#include "core.hh"
#include "read.hh"
using namespace std;

namespace mc {

const string seps = ".:;(){}`'";
inline bool issep(char c) {
  return seps.find(c) != str::npos;
}

// Tokenizer ///////////////////////////////////////////////

void Tokenizer::_collect() {
  if (_text.empty()) return;

  const char *begin = _text.c_str();
  char *end;
  long int i = strtol(begin, &end, 10);
  if (size_t(end - begin) == _text.size()) {
    _curr.val = Int(i);
  } else {
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

void Tokenizer::_put_normal(char c) {
  if (isspace(c)) {
    _collect();
    if (c == '\n' && _last == '\n') {
      _enq(Token(Range(_pos), Box<char>('\n')));
    }
  } 
  else if (issep(c) || c == '@') {
    _collect();
    _enq(Token(Range(_pos), Box<char>(c)));
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
    if (_text.empty()) _curr.pos.ini = _pos;
    _text += c;
  }
}

void Tokenizer::_put_string(char c) {
  if (_escape) {
    _escape = false;
    switch (c) {
    case 'n':  _text += '\n'; break;
    case 't':  _text += '\t'; break;
    case 'r':  _text += '\r'; break;
    case 'b':  _text += '\b'; break;
    case '\\': 
    case '"':  _text += c;  break;
    default: throw ScanError("Unknown escape char.");
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
  switch (_mode) {
  case normal: _put_normal(c); break;
  case string: _put_string(c); break;
  case comment: 
    if (c == '\n') { _mode = normal; }
  }

  if (c == '\n') {
    _pos.newline(); 
    _last = (_last == '\n'? 0 : '\n');
  } else {
    ++_pos;
    _last = c;
  }
}

// Reader /////////////////////////////////////////////////

void SeqReader::put(Token& t) {
  positions.push_back(t.pos);
  Any v = t.val;
  while (unquote > 0) {
    v = Tuple(Sym("unquote"), v);
    unquote--;
  }
  acum->push_back(v); 
}

bool SeqReader::has_indent(Pos p) const {
  vector<Range>::const_reverse_iterator i;
  i = positions.rbegin();
  for (; i != positions.rend(); i++) {
    if (i->ini.col == p.col) return true;
  }
  return false;
}

Token SeqReader::collect(Pos fin) {
  return Token(Range(ini, fin), _collect());
}

void ListReader::_collect_tuple() {
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

Any ListReader::_collect() {
  if (!acum->empty()) _collect_tuple();
  return _list;
}

Any QuoteReader::_collect() {
  return Tuple(Sym("quote"), 
	       ListReader::_collect());
}

// Reader /////////////////////////////////////////////////

void Reader::_reset() { 
  _T.reset();
  _stack.clear(); 
}

void Reader::_pop() {
  Token t = _stack.front()->collect(_tok.pos.ini);
  delete _stack.front();
  _stack.pop_front();
  if (_stack.empty()) {
    _enq(t.val);
  } else {
    _stack.front()->put(t);
  }
}

void Reader::_pop_all() {
  while (!_stack.empty()) _pop();
}

void Reader::_pop_until(char end) {
  while (!_stack.front()->is_end(end)) {
    _pop();
    if (_stack.empty()) {
      throw ScanError("unexpected close");
    }
  }
  _pop();
}

void Reader::_maybe_break(Pos p) {
  if (!_stack.front()->is_lower(p)) return;

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

void Reader::_put() {
  if (_stack.empty()) {
    _stack.push_front(new ListReader(';', '.', _T.pos()));
  }
  if (_tok.val.is<char>()) {
    char c = *_tok.val.as<char>();
    if (c == '\n' && busy()) {
      _pop_all();
      _reset();
    } else if (c == '@') {
      _stack.front()->unquote++;
    } else {
      switch (c) {
      case '(': 
	_maybe_break(_tok.pos.ini);
	_push(new TupleReader(')', _tok.pos.ini));     
	break;
      case ':': 
	_maybe_break(_tok.pos.ini);
	_push(new ListReader(';', '.', _tok.pos.ini)); 
	break;
      case '{':
	_maybe_break(_tok.pos.ini);
	_push(new ListReader(';', '}', _tok.pos.ini)); 
	break;
      case '`':
	_maybe_break(_tok.pos.ini);
	_push(new QuoteReader(';', '\'', _tok.pos.ini)); 
	break;
      case ';': _stack.front()->put_sep(c); break;
      case ')': 
      case '\'':
      case '.':
	_pop_until(c); 
	if (_stack.empty()) _reset();
	break;
      }
    }
  } else {
    _maybe_break(_tok.pos.ini);
    _stack.front()->put(_tok);
  }
}

void Reader::put(char c) {
  _T.put(c);
  while (_T.get(_tok)) _put();
}

void Reader::putline(const str& s) {
  for (size_t k = 0; k < s.size(); k++) put(s[k]);
  put('\n');
}

} // namespace
