
#ifndef scan_h
#define scan_h

#include <queue>
#include <list>
#include "core.hh"
using std::queue;
using std::list;

namespace mc {

template<typename T>
class Queue {
  queue<T> _queue;
protected:
  void _enq(const T& t) { 
    _queue.push(t); 
  }
  void _reset() { 
    // no 'clear' in queue<T>... sigh.
    while (!_queue.empty()) _queue.pop(); 
  }
public:
  bool get(T& t);
};

template<typename T>
bool Queue<T>::get(T& t) {
  if (_queue.empty()) return false;
  t = _queue.front();
  _queue.pop();
  return true;
}

struct Pos {
  int lin, col;
  Pos() : lin(-1), col(-1) {}
  Pos(int l, int c) 
    : lin(l), col(c) {}

  Pos operator+(int i) const { return Pos(lin, col + i); }
  Pos operator-(int i) const { return Pos(lin, col - i); }
  void newline() { lin++, col = 0; }
};

struct Range { 
  Pos ini, fin; 
  Range() {}
  explicit Range(Pos p) : ini(p), fin(p + 1) {}
  Range(Pos i, Pos f) : ini(i), fin(f) {}
};

struct Token {
  Range pos;
  Any val;
  Token() {}
  Token(Range r, Any v)
    : pos(r), val(v) {}
};

inline std::ostream& 
operator<<(std::ostream& o, const Token& t) {
  return o << '[' << t.pos.ini.lin << ", " << t.pos.ini.col << ": " << t.val << ']';
}

class Tokenizer : public Queue<Token> {
  enum Mode { normal, comment, string };
  Pos   _pos;
  str   _acum;
  Mode  _mode;
  bool  _endl, _2endls, _escape, _dot;
  Token _curr;
  str   _text;

  void _put_normal(char c);
  void _put_string(char c);

  void _collect();
  
  void _reset(int lin) {
    _acum = "";
    _mode = normal;
    _endl = _2endls = true;
    _dot = false;
    Queue<Token>::_reset();
  }
  
public:
  Tokenizer()  { _reset(-1); }

  void reset() { _reset(_pos.lin + 1); }

  Pos  pos() const { return _pos; }
  void put(char c);
  bool busy() const { 
    return _mode == string || 
      _mode == comment ||
      (_mode == normal && _text != ""); 
  }
};

struct SeqReader {
  std::vector<Range> indents;
  Pos   ini;
  Tuple acum;
  char  end;
  bool  can_break;
  int   unquote;
  
  SeqReader(char c, Pos p) 
    : ini(p), end(c), can_break(false), unquote(0) {}
  
  bool  busy() const { return !acum->empty(); }
  bool  has_indent(Pos p) const;
  bool  is_lower(Pos p) const { 
    return !indents.empty() && indents.back().fin.lin < p.lin;
  }
  bool  is_initial(Pos p) const { 
    return !indents.empty() && indents.front().ini.col == p.col;
  }
  void  put(Token& t);
  bool  is_end(char c) const { return end == c; }
  Token collect(Pos fin);

  virtual void put_sep(char c) {
    throw ScanError("Unexpected separator"); 
  }

  virtual void put_break() { assert(false); }

  virtual Any  _collect()  { return acum; }
};

typedef SeqReader TupleReader;

struct ListReader : public SeqReader {
  char _sep;
  List _list;

  void _collect_tuple();

public:
  ListReader(char sep, char end, Pos pos) 
    : SeqReader(end, pos), _sep(sep) {
    can_break = true;
  }
  
  void put_sep(char c) { 
    if (_sep == c) _collect_tuple();
    else throw ScanError("Unexpected separator"); 
  }
  void put_break() { _collect_tuple(); }
  Any  _collect();
};

struct QuoteReader : public ListReader {
public:
  QuoteReader(char sep, char end, Pos pos)
    : ListReader(sep, end, pos) {}
  Any _collect();
};

class Reader : public Queue<Any> {
  Tokenizer _T;
  Token _tok;
  list<SeqReader *> _stack;

  void _reset();
  void _put();

  void _push(SeqReader *s) { _stack.push_front(s); }
  void _pop();
  void _pop_until(char end);
  void _pop_all();

  void _maybe_break(Pos p);

public:
  Reader()     { _reset(); }
  void reset() { _reset(); Queue<Any>::_reset(); }

  void put(char c);
  void putline(const str& s);

  bool busy() const { 
    return (!_stack.empty() && _stack.front()->busy()) || _T.busy(); 
  }
};

} // namespace

#endif
