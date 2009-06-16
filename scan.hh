
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
    // std::cout << t << std::endl;
    _queue.push(t); 
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

struct Token {
  int lin, col;
  Any val;

  Token() : lin(-1), col(-1) {}

  Token(int l, int c, Any v)
    : lin(l), col(c), val(v) {}
};

inline std::ostream& 
operator<<(std::ostream& o, const Token& t) {
  return o << '[' << t.lin << ", " << t.col << ": " << t.val << ']';
}

class Tokenizer : public Queue<Token> {
  enum Mode { normal, comment, string };
  int   _lin, _col;
  str   _acum;
  Mode  _mode;
  bool  _endl, _2endls, _escape, _dot;
  Token _curr;
  str   _text;

  void _put_normal(char c);
  void _put_string(char c);

  void _collect();

public:
  Tokenizer() 
    : _lin(-1), _col(-1), _mode(normal), _endl(true) {}

  void put(char c);
  bool busy() const { 
    return _mode == string || 
      _mode == comment ||
      (_mode == normal && _text != ""); 
  }
};

struct SeqScanner {
  std::vector<int> _indents;
  int   _inilin, _inicol;
  Tuple _acum;
  char  _end;
  bool  _can_break;
  
  SeqScanner(char end) 
    : _end(end), _can_break(false) {}
  
  bool  busy() const { return !_acum->empty(); }
  void  put(Token& t);
  bool  is_end(char c) const { return _end == c; }
  Token collect();

  virtual bool is_sep(char c) const { return false; }
  virtual void put_sep()  { assert(false); }
  virtual Any  _collect() { return _acum; }
};

typedef SeqScanner TupleScanner;

struct ListScanner : public SeqScanner {
  char _sep;
  List _list;

  void _collect_tuple();

public:
  ListScanner(char sep, char end) 
    : SeqScanner(end), _sep(sep) {}
  
  bool is_sep(char c) const { return _sep == c; }
  void put_sep() { _collect_tuple(); }
  Any  _collect();
};

class Scanner : public Queue<Any> {
  Tokenizer _T;
  list<SeqScanner *> _stack;

  void _push(SeqScanner *s) {
    _stack.push_front(s);
  }

  void _put(Token& t);
  void _init();
  void _pop();
  void _pop_all();

public:
  Scanner();
  void put(char c);
  void putline(str s);

  bool busy() const { 
    return _stack.front()->busy() || _T.busy(); 
  }
};

// Parse
Any parse(Any in);

} // namespace

#endif
