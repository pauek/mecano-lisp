
#include <cstdlib>
#include "core.hh"
#include "scan.hh"
using namespace std;

namespace mc {

// separator ///////////////////////////////////////////////

struct sep {
  str text;
  operator str() { return text; }
  sep(char c) { text += c; }
  sep(const char* t) : text(t) {}
  bool operator==(const sep& s) const { 
    return text == s.text;
  }
};

EVAL_BASIC(Box<sep>, sep);

ostream& operator<<(ostream& o, const sep& s) { 
  return o << '\'' << s.text << '\'';
}

const string seps = ":;()";
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
    _push(_curr);
    _text = "";
  }
  if (_dot) {
    _push(Token(_lin, _col-1, Box<sep>('.'))); 
    _dot = false;
  }
}


void Tokenizer::_put_normal(char c) {
  if (c == '\n') {
    if (_endl && !_2endls) {
      _push(Token(_lin, _col, Box<sep>("\n\n")));
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
      _push(Token(_lin, _col, Box<sep>(c)));
    }
  }
  else if (c == '.') {
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
      _push(_curr);
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

void Scanner::putline(str s) {
  for (size_t k = 0; k < s.size(); k++) put(s[k]);
  put('\n');
}

bool Scanner::get(Any& a) {
  Token t;
  if (_T.get(t)) {
    a = t.val;
    return true;
  }
  return false;
}

} // namespace
