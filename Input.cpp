#include "Input.h"

using namespace std;

// ___________ constructors/destructors ____________ //
  
Input::Input(string input) {
  setShellInput(input); 
  set_foreground();
  setSTDIN();
  setSTDOUT();
  setSTDERR();
  this->processes = inputToProcesses();
} // constructor

Input::Input(const Input & job): Input::Input(job.getShellInput()) {
  this->JID = job.getJID();
  this->status = job.getStatus();
} // copy constructor

//_____________ setShellInput(string) _____________ //

void Input::setShellInput(string input) {
  const char * ws = " \t"; // chars to trim
  input = input.erase(input.find_last_not_of(ws) + 1); // trims end 
  input = input.erase(0, input.find_first_not_of(ws)); // trims beginning  
  this->shellInput = input;
} // setShellInput

//_____________ set_foreground() _____________ //

void Input::set_foreground() {
  this->foreground = (shellInput[shellInput.size()-1] == '&') ? false : true;
} // setShellInput

//_____________ setStatus(const char*) _____________ //

void Input::setStatus(const char * status) {
  if(status != nullptr) { this->status = status; } // if
} // setShellInput

//_____________ setSTDIN() _____________ //

void Input::setSTDIN() {
  string fd = "STDIN_FILENO";
  stringstream ss(this->shellInput);
  vector<string> argv;
  string arg;
  while(ss >> arg) {
    argv.push_back(arg);
  } // while
  vector<string> processed_argv = processArgv(argv);
  for(unsigned int i = 0; i < processed_argv.size(); i++) {
    if(i > 0) {
      if(processed_argv[i-1] == "<") {
	fd = processed_argv[i];
	break;
      } // if
    } // if 
  } // for
  this->fd_STDIN = fd;
} // setSTDIN

//_____________ setSTDOUT() _____________ //

void Input::setSTDOUT() {
  string fd = "STDOUT_FILENO";
  string type = "";
  stringstream ss(this->shellInput);
  vector<string> argv;
  string arg;
  while(ss >> arg) {
    argv.push_back(arg);
  } // while
  vector<string> processed_argv = processArgv(argv);
  for(unsigned int i = 0; i < processed_argv.size(); i++) {
    if(i > 0) {
      if(processed_argv[i-1] == ">>" || processed_argv[i-1] == ">") {
	fd = processed_argv[i];
	type = processed_argv[i-1];
	break;
      } // if
    } // if
  } // for
  this->fd_STDOUT = fd;
  this->type_STDOUT = type;
} // setSTDOUT

//_____________ setSTDERR() _____________ //

void Input::setSTDERR() {
  string fd = "STDERR_FILENO";
  string type = "";
  stringstream ss(this->shellInput);
  vector<string> argv;
  string arg;
  while(ss >> arg) {
    argv.push_back(arg);
  } // while
  vector<string> processed_argv = processArgv(argv);
  for(unsigned int i = 0; i < processed_argv.size(); i++) {
    if(i > 0) {
      if(processed_argv[i-1] == "e>" || processed_argv[i-1] == "e>>") {
	fd = processed_argv[i];
	type = processed_argv[i-1];
	break;
      } // if
    } // if
  } // for
  this->fd_STDERR = fd;
  this->type_STDERR = type;
} // setSTDERR

//_____________ setJID(pid_t) _____________ //

void Input::setJID(pid_t pgid) {
  this->JID = pgid;
  for(int i = 0, s = processes.size(); i < s; i++) {
    processes[i].PGID = pgid;
  } // for
} // setJID

//_____________ getNumPipes() _____________ //

const int Input::getNumPipes() const {
  int pipes = 0;
  for(unsigned int i = 0; i < processes.size(); i++) {
    if(processes[i].hasPipe) { // if not the last process in input
      pipes++;
    } // if
  } // for
  return pipes;
} // getNumPipes

//_____________ getNumProcesses() _____________ //

const int Input::getNumProcesses() const {
  return processes.size();
} // getNumProcesses

//_____________ isStopped() _____________ //

const bool Input::isStopped() const {
  for(unsigned int i = 0, s = processes.size(); i < s; i++) {
    if(!processes[i].stopped && !processes[i].completed) {
      return false;
    } // if
  } // for
  return true;
} // isStopped

//_____________ isCompleted() _____________ //

const bool Input::isCompleted() const {
  for(unsigned int i = 0, s = processes.size(); i < s; i++) {
    if(!processes[i].completed) {
      return false;
    } // if
  } // for
  return true;
} // isCompleted

//_____________ inputToProcesses() _____________ //

vector<Process> Input::inputToProcesses() { 
  vector<Process> p_vector;
  // if empty
  if(this->shellInput.length() == 0) { return p_vector; } // if
  // splits up original shell input
  stringstream ss(this->shellInput);
  vector<string> argv;
  string a;
  while(ss >> a) { 
    argv.push_back(a);
  } // while
  // deals with double-quotes
  vector<string> processed_argv = processArgv(argv);
  // finally, add trimmed, concatenated/processed, and sanitized args to p_vector
  for(unsigned int i = 0; i < processed_argv.size(); i++) {
    if(i == 0) { 
      Process p;
      p_vector.push_back(p); // create a new Process
      p_vector[i].args.push_back(processed_argv[i]); // push the name of the process to be the first argument of the Process vector<string>
    } else if(processed_argv[i-1] == "|") { 
      Process p;
      p_vector.push_back(p); // create a new Process
      p_vector.back().args.push_back(processed_argv[i]); // push the name of the process to be the first argument of the Process vector<string>	
    } else { 
      if(processed_argv[i] != "<" && processed_argv[i-1] != "<" &&
	 processed_argv[i] != ">" && processed_argv[i-1] != ">" &&
	 processed_argv[i] != ">>" && processed_argv[i-1] != ">>" &&
	 processed_argv[i] != "e>" && processed_argv[i-1] != "e>" &&
	 processed_argv[i] != "e>>" && processed_argv[i-1] != "e>>") {
	if(processed_argv[i] == "|") {
	  p_vector.back().hasPipe = true;
	} else if(processed_argv[i] != "&") {
	  p_vector.back().args.push_back(processed_argv[i]); // push the process arguments to the Process vector<string>
	} // if/else
      } // if
    } // if/else
  } // for
  return p_vector;
} // inputToProcesses

//_____________ member operator overloads _____________ //

// ---- =                 

Input& Input::operator=(string input) {
  setShellInput(input); 
  set_foreground();
  setSTDIN();
  setSTDOUT();
  setSTDERR();
  this->processes = inputToProcesses();
  return *this;
} // Ex. 'my_Input_obj = other_Input_obj'                 

//_____________ non-member operator overloads _____________ //

// ---- <<

ostream& operator<<(ostream& output, Input& rhs) { 
  output << "JID = " << rhs.getJID() << ", In foreground? " << rhs.isForeground() << endl;
  for(unsigned int i = 0, s = rhs.getProcesses().size(); i < s; i++) {
    output << "Process " << i << " (PID/PGID = " << rhs.getProcesses()[i].PID << "/" << rhs.getProcesses()[i].PGID << ") argv: ";
    for(unsigned int j = 0; j < rhs.getProcesses()[i].args.size(); j++) {
      output << rhs.getProcesses()[i].args[j] << " ";
    } // for
    if(i != s-1) { output << endl; } // if not last iter
  } // for
  return output;
} // Ex. 'cout << my_input_obj'

// _______________ non-member helper methods ______________ //

string trim(string input) {
  string s = input;
  const char * ws = " \t"; // chars to trim
  s = s.erase(s.find_last_not_of(ws) + 1); // trims end 
  s = s.erase(0, s.find_first_not_of(ws)); // trims beginning  
  return s;
} // trim

string sanitize(string input, string charsToRemove) {
  const string unwanted = charsToRemove ;
  std::string sanitized_str = "";
  for(char c : input) {
    if(unwanted.find(c) == string::npos) {
      sanitized_str += c;
    } // if
  } // for
  return sanitized_str;
} // sanitize

bool hasQuotes(string input) {
  for(unsigned int i = 0; i < input.length(); i++) {
    if(i == 0) {
      if(input[i] == '"') return true;
    } else {
      if(input[i] == '"' && input[i-1] != '\\') return true;
    } // if/else
  } // for
  return false;
} // hasQuotes

unsigned int pos_of_first_quote(string input) {
  int pos = 0;
  for(unsigned int i = 0; i < input.length(); i++) {
    if(i == 0) {
      if(input[i] == '"') {
	pos = i; 
	break;
      } // if
    } else {
      if(input[i] == '"' && input[i-1] != '\\') {
	pos = i;
	break;
      } // if
    } // if/else
  } // for
  return pos;
} // pos_of_first_quote

vector<string> processArgv(vector<string> argv) {
  vector<string> processed_argv;
  int quote_count = 0;
  string arg = "";
  for(unsigned int i = 0; i < argv.size(); i++) { 
    if(hasQuotes(argv[i])) { 
      if(quote_count == 0) { arg = ""; } // if
      quote_count++;
      arg = arg + " " + argv[i];
      unsigned int q_pos = pos_of_first_quote(argv[i]);
      if(argv[i].length() > 1 && q_pos != argv[i].length()-1 && hasQuotes(argv[i].substr(q_pos+1))) quote_count++;
      if(quote_count == 2) {
	quote_count = 0;
      } else {
	continue;
      } // if/else
    } else {
      if(quote_count % 2 == 0) {
	arg = argv[i];
      } else {
	arg = arg + " " + argv[i];
	continue;
      } // if/else
    } // if/else
    arg = trim(arg);
    // remove beginning/end quotes
    string proc_arg = "";
    for(unsigned int i = 0; i < arg.length(); i++) {
      if(arg[i] == '"') {
	if(i > 0) {
	  if(arg[i-1] == '\\') proc_arg += arg[i];
	} // if
      } else {
	proc_arg += arg[i];
      } // if/else
    } // for
    proc_arg = sanitize(proc_arg,"\\");
    processed_argv.push_back(proc_arg);
  } // for
  return processed_argv;
} // processArgv

