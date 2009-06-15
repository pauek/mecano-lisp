
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <readline/readline.h>
#include "core.hh"
#include "scan.hh"
using namespace mc;
using namespace std;

struct Config {
  string file;
  bool parse_only;
  bool scan_only;
  Config() 
    : file(""), parse_only(false), scan_only(false) 
  {}
};

void parse_args(int argc, char *argv[], Config& conf) {
  for (int k = 1; k < argc; k++) {
    const string argk(argv[k]);
    if (argk == "-p" || argk == "--parse-only") 
      conf.parse_only = true;
    else if (argk == "-s" || argk == "--scan-only")
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
  Scanner S;
  string line = in->read_line(prompt1);
  while (!in->end()) {
    S.putline(line);
    Any a;
    while (S.get(a)) {
      if (conf.scan_only) {
	cout << a << endl;
      } else {
	a = parse(a);
	if (conf.parse_only) {
	  cout << a << endl;
	} else {
	  vm.eval(a);
	  cout << vm.val << endl;
	}
      }
    }
    line = in->read_line(S.busy() ? prompt2 : prompt1);
  }
}
