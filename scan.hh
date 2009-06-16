
#ifndef scan_h
#define scan_h

#include <queue>
#include "core.hh"
using std::queue;

namespace mc {

template<typename T>
class Queue {
  queue<T> _queue;
protected:
  void _push(const T& t) { 
    std::cout << t << std::endl;
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
};

class Scanner {
  Tokenizer _T;
public:
  void put(char c) { _T.put(c); }
  void putline(str s);
  bool get(Any& a);
  bool busy() const { return false; }
};

// Parse
Any parse(Any in);

} // namespace

#endif
