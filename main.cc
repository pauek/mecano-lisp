
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <readline/readline.h>
#include <readline/history.h>
#include "core.hh"
#include "read.hh"
using namespace mc;
using namespace std;

struct Config {
  string file;
  bool scan_only;
  Config() 
    : file(""), scan_only(false) 
  {}
};

void parse_args(int argc, char *argv[], Config& conf) {
  for (int k = 1; k < argc; k++) {
    const string argk(argv[k]);
    if (argk == "-s" || argk == "--scan-only")
      conf.scan_only = true;
    else 
      conf.file = argv[k];
  }
}

struct Input {
  virtual bool end() const = 0;
  virtual string read_line(string prompt) = 0;
};

struct FileInput : public Input {
  ifstream fin;
  FileInput(string filename) : fin(filename.c_str()) {}
  bool end() const { return fin.eof(); }
  string read_line(string prompt) {
    char line[1000];
    fin.getline(line, 1000);
    return string(line);
  }
};

struct StandardInput : public Input {
  bool _end;
  StandardInput() : _end(false) {
    using_history();
    cout << "Mecano v0.1" << endl;
  }
  bool end() const { return _end; }
  string read_line(string prompt) {
    char *line = readline(prompt.c_str());
    if (line == NULL) {
      _end = true;
      cout << endl;
      return "";
    } else {
      add_history(line);
      string ret(line);
      free(line);
      return ret;
    }
  }
};

Input *setup_input(const Config& conf) {
  return (conf.file != "" 
	  ? (Input *) new FileInput(conf.file)
	  : (Input *) new StandardInput());
}

int main(int argc, char *argv[]) {
  Config conf;
  string prompt1 = "> ", prompt2 = "_ ";
  parse_args(argc, argv, conf);

  Input *in = setup_input(conf);

  VM vm;
  Reader R;
  string line = in->read_line(prompt1);
  while (!in->end()) {
    R.putline(line);
    Any a;
    while (R.get(a)) {
      if (conf.scan_only) {
	cout << a << endl;
      } else {
	vm.eval(a);
	cout << vm.val << endl;
      }
    }
    line = in->read_line(R.busy() ? prompt2 : prompt1);
  }
}
