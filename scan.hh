
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
  int   _lev;
  Acum  _acum;
  Vec   _in;

  void _pop(int lev);

public:
  SeqScanner() : _lev(-1) {}

  void add_level(char sep, TransFn fn) {
    _seps += sep;
    _trans.push_back(fn);
    _acum.push_back(Vec());
    _lev = _seps.size() - 1;
  }

  bool is_sep(char c) { return _seps.find(c) != str::npos; }
  bool is_end(char c) { return _seps[0] == c; }

  void put(Any a)     { _in.push_back(a); }
  void put_sep(char c);

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
  int    _lin, _col, _inipos;
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

  struct Error {};
};


} // namespace

#endif
