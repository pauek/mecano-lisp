
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

struct SeqScanner {
  virtual void put(Any a, int col) = 0;
  
};

class ListScanner : public SeqScanner {
  
};

class TupleScanner : public SeqScanner {
  
};

class SeqScanner {
  typedef std::vector<TransFn> Trans;
  typedef std::vector<Vec> Acum;
  str   _seps;
  Trans _trans;
  int   _lev, _inicol;
  Acum  _acum;
  bool  _breakable;

  void _pop(int lev);

public:
  SeqScanner(bool breakable = false) 
    : _lev(-1), _inicol(-1), _breakable(breakable) {}

  void add_level(char sep, TransFn fn);

  bool is_sep(char c) { return _seps.find(c) != str::npos; }
  bool is_end(char c) { return _seps[0] == c; }

  void put(Any a, int col);
  void put_sep(char c);
  void put_break();

  bool breakable() const { return _breakable; }
  int  inicol()    const { return _inicol; }
  bool busy()      const { return _inicol != -1; }

  bool collect(Any& a);
};

class Scanner {
  enum Config {
    escape = 1,
    endln = 2,
    beginln = 4,
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
  void _put(char c);
  void _put_str(char c);
  void _put_normal(char c);
  void _put_sep(char c);
  void _put_break();
  void _pop();
  void _pop_all();
  void _reset();

public:
  Scanner();

  void put(char c);
  void putline(str line);
  bool get(Any& a);
  bool busy() const;

  struct Error {
    int lin, col;
    str msg;
    Error(int l, int c, str m)
      : lin(l), col(c), msg(m) {}
  };
};

// Parse
Any parse(Any in);

} // namespace

#endif
