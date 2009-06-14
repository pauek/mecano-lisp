
#include <cstdlib>
#include <iostream>
#include <readline/readline.h>
#include "core.hh"
#include "scan.hh"
using namespace mc;
using namespace std;

string read_line(string prompt) {
  char *line = readline("> ");
  if (line == NULL) {
    cout << endl;
    exit(0);
  }
  string ret(line);
  free(line);
  return ret;
}

int main() {
  VM vm;
  Scanner S;
  string line = read_line("> ");
  while (true) {
    S.putline(line);
    Any a;
    while (S.get(a)) {
      a = parse(a);
      vm.eval(a);
      if (vm.val.not_null()) 
	cout << vm.val << endl;
    }
    line = read_line("> ");
  }
}
