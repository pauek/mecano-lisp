
#include <iostream>
#include "core.hh"
#include "util.hh"
using namespace std;
using namespace mc;

enum Config { faemino = 1, cansado = 2 };

int main() {
  flags<Config> f;
  f[faemino] = true;
  f[cansado] = false;
  if (f[faemino]) {
    cout << "true" << endl;
  } else {
    cout << "false" << endl;
  }

  Any a = Str("hola");
  cout << a << endl;
}

// Local variables:
// compile-command: "gcc -o t test.cc -lstdc++ -lgc"
// End:
