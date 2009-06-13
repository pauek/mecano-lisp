
#ifndef util_h
#define util_h

template<typename Enum>
class flags {
  int _flags;
public:
  flags() : _flags(0) {}

  struct BoolRef {
    int& flags;
    Enum val;

    BoolRef(int& f, Enum v) 
      : flags(f), val(v) {}

    operator bool() {
      return flags & val;
    }

    bool operator=(bool b) {
      if (b) { flags |= val; }
      else   { flags &= ~val; }
      return b;
    }
  };

  BoolRef operator[](Enum e) {
    return BoolRef(_flags, e);
  }
  
  bool operator[](Enum e) const {
    return _flags & e;
  }
};

#endif
