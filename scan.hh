
#ifndef scan_h
#define scan_h

#include <vector>
#include <queue>
#include <string>
#include "core.hh"
#include "util.hh"

namespace mc {

typedef std::vector<Any> Vec;
typedef Any (*TransFn)(Vec& v);

class SeqScanner {
  typedef std::vector<TransFn> Trans;
  typedef std::vector<Vec> Acum;
  str   _seps;
  Trans _trans;
  int   _lev, _col;
  Acum  _acum;
  Vec   _in;
  bool  _breaks;

  void _pop(int lev);

public:
  SeqScanner(bool breaks = false) 
    : _lev(-1), _col(-1), _breaks(breaks) {}

  void add_level(char sep, TransFn fn);

  bool is_sep(char c) { return _seps.find(c) != str::npos; }
  bool is_end(char c) { return _seps[0] == c; }

  void put(Any a, int col);
  void put_sep(char c);

  bool breakable() const { return _breaks; }
  int  inicol()    const { return _col; }

  Any  collect();
};

class Scanner {
  enum Config {
    escape = 1,
    endl = 2,
  };
  enum Mode { normal, string, comment };

  typedef std::list<SeqScanner *> Stack;
  typedef std::queue<Any> Queue;
  typedef flags<Config> Flags;

  Mode   _mode;
  Flags  _flags;
  Stack  _stack;
  Queue  _queue;
  int    _lin, _col, _inicol;
  str    _acum;

  void _emit(Any a);
  void _collect();
  void _update_pos(char c);
  void _put_str(char c);
  void _put_normal(char c);
  void _put_sep(char c);

public:
  Scanner();

  void put(char c);
  void putline(str line);
  bool get(Any& a);

  struct Error {
    int lin, col;
    str msg;
    Error(int l, int c, str m)
      : lin(l), col(c), msg(m) {}
  };
};


} // namespace

#endif
