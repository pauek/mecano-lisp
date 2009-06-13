
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
      if (a.not_null()) 
	cout << a << endl;
    }
    line = read_line("> ");
  }
}

/*
  Any inc = Lambda(Tuple(Sym("x")), 
		   Call(Tuple(Sym("+"), Sym("x"), Int(1))));
  cout << vm.eval(Call(Tuple(inc, Int(4)))) << endl;
*/

/*
  cout << vm.eval(Call(Tuple(Sym("+"), Int(1), Int(3)))) 
       << endl;
*/

/*
  Tuple t(Int(1), Char('a'), Str("hola"), Real(4.55));
  List l(Int(10), Tuple(Int(2), Int(3)), List(Char('a')));
  
  cout << t << endl;
  cout << l << endl;
*/


/*
  Int a = l[0];
  cout << a << endl;
  Bool b = t[0];
  cout << b.is_null() << endl;
  
  Bool b2 = True;
  cout << b2 << endl;
  cout << (b2 == False) << endl;
*/

/*
  Sym a("hi"), b("hi");
  Sym c("he"), d("ho");
  cout << (a == b) << endl;
  cout << c << ' ' << d << ' ' << (c == d) << endl;
  cout << a->id() << ' ' << c->id() << endl;
*/

/*
  vm.val = If(Int(0), Int(5), Int(13));
  vm.run();
  cout << vm.val << endl;
*/

/* 
   vm.val = Tuple(Int(1), If(False, Char('a'), Str("argh")));
   vm.run();
   cout << vm.val << endl;
*/

/* 
   vm.val = List(Int(1), If(False, Char('a'), Str("argh")));
   vm.run();
   cout << vm.val << endl;
*/

/* 
   cout << vm.eval(Set(Sym("a"), Int(15))) << endl;
   cout << vm.eval(Set(Sym("a"), Str("hola"))) << endl;
   cout << vm.eval(Tuple(Int(1), Sym("a"))) << endl;
*/

