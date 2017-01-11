#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <map>
#include <iomanip>
#include <pwd.h>
#include <limits.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Input.h"

using namespace std;

// PROTOTYPES

/** 
 * Prints out the latest errno error and exits the process with EXIT_FAILURE.
 *
 * @param const string& the name of the system call which failed
 */
inline void nope_out(const string &);

/**
 * Displays the shell prompt to the user. 
 */
void prompt();

/**
 * Determines if shell input is valid or not.
 *
 * @param string the shell input to be checked for validity
 * @return true if the input is valid, false if not
 */
bool isValidInput(string);

/**
 * Determines if shell input has an even number of unescaped double-quotes.
 *
 * @param string the shell input to be checked for quote pairs
 * @return true if the input has valid quote pairs, false if not
 */
bool hasEvenQuotes(string);

/**
 * @source Mike's pipe2.cpp
 *
 * Converts the given Process's arg vector<string> into a vector<char *> to be used by 
 * execvp(). Uses dynamic memory allocation, so the vector<char*> must be deleted later 
 * with dl_cstrvec().
 *
 * @param vector<string>& the given Process's arg vector<string> 
 * @return the vector<char*> to be used by execvp() in nice_exec()
 */
vector<char *> mk_cstrvec(vector<string> &);

/**
 * @source Mike's pipe2.cpp
 *
 * Frees the memory allocated to store the temporary c-string vector.
 * Called inside of nice_exec().
 *
 * @param vector<char*>& the c-string vector created from the Process's arg vector<string>
 */
void dl_cstrvec(vector<char *> &);

/**
 * @source Mike's pipe2.cpp
 *
 * Attempts to execvp() the given Process using its arg vector<string>. 
 * Exits with failure if execvp() fails. 
 *
 * @param vector<string> the given Process's arg vector<string>
 * @param int** the dynamically allocated pipes which must be deleted if exec fails
 * @param int the length of the pipes array
 */
void nice_exec(vector<string>, int**, int);

/**
 * Returns a dynamically-allocated, multidimensional array of pipefd[2],
 * the size of which is determined by the given numPipes value, which is 
 * taken from the current Input obj.
 *
 * @param int the number of pipefd[2] to allocate
 * @return int** the pipes array
 */
int** makePipes(int);

/**
 * Deletes the dynamically allocated pipefd[2] array used in the job pipeline.
 *
 * @param int** the dynamically-allocated pipefd[2] array
 * @param int the size of the array
 */
void deletePipes(int**, int);

/*
 * @source Mike's pipe2.cpp
 *
 * Closes the read/write ends of the given pipe. Exits with EXIT_FAILURE
 * if unsuccessful.
 *
 * @param int[2] the pipe (array of fd's) to be closed
 * @param bool set to true if user wants to exit upon sys call failure. false if not.
 */
void close_pipe(int[2], bool);

/**
 * Sets the file descriptor destinations of any i/o redirection.
 *
 * @param job the Input obj from which the i/o redirect destinations will be pulled
 * @param fdSTDIN the fd of the stdin source. default is STDIN_FILENO
 * @param fdSTDOUT the fd of the stdout destination. default is STDOUT_FILENO
 * @param fdSTDERR the fd of the stderr destination. default is STDERR_FILENO
 * @return -1 upon any failure to open file descriptors. 0 otherwise
 */
int set_redirects(Input* job, int& fdSTDIN, int& fdSTDOUT, int& fdSTDERR);

/**
 * Do the dup2 i/o redirects for the given file descriptors. If any of the provided
 * values are negative, the dup2 for that redirect will not be performed.
 *
 * @param STDIN the chosen fd for std input redirection. If negative, this dup2 is skipped
 * @param STDOUT the chosen fd for std output redirection. If negative, this dup2 is skipped
 * @param STDERR the chosen fd for std error redirection. If negative, this dup2 is skipped
 */
void do_redirects(int STDIN, int STDOUT, int STDERR);

/**
 * Closes the descriptors of the specified i/o redirects. Exits with
 * EXIT_FAILURE if unsuccessful.
 *
 * @param STDIN the chosen fd for std input redirection. Default is STDIN_FILENO.
 * @param STDOUT the chosen fd for std output redirection. Default is STDOUT_FILENO.
 * @param STDERR the chosen fd for std error redirection. Default is STDERR_FILENO.
 */
void close_redirects(int STDIN, int STDOUT, int STDERR);

/**
 * Sets the disposition of all the job control signals in the parent process.
 */
void parent_signals();

/**
 * Sets the disposition of all the job control signals to in the child process.
 */
void child_signals();

/**
 * Prints job status information for the user.
 *
 * @param Input* the job for which the info will be printed
 * @param const char* the status message of the job (Ex. launched, stopped, etc.)
 */
void format_job_info(Input*, const char*);

/**
 * Calls waitpid() on processes in the given Input obj. The boolean param controls how this is done.
 *
 * @param Input* the given job to be waited on
 * @param bool true if waiting on any job (WNOHANG is on and pid == -1), false if waiting on current 
 *        job (WNOHANG is off and pid == -job->getJID())
 */
void wait_for_job(Input*, bool);

/**
 * Puts the given job into the foreground. If bool arg is true, send SIGCONT
 * signal to wake it back up before blocking with waitpid.
 *
 * @param Input* the job which will be put in the foreground
 * @param bool true if SIGCONT should be sent to all the Processes in the job's PGID
 */
void put_job_in_foreground(Input*, bool);

/**
 * Puts the given job into the background. If bool arg is true, send SIGCONT
 * signal to wake it up.
 *
 * @param Input* the job which will be put in the background
 * @param bool true if SIGCONT should be sent to all the Processes in the job's PGID
 */
void put_job_in_background(Input*, bool);

/**
 * Checks for status changes on any currently running jobs, which are contained in the 
 * global vector<Input> current_jobs. Each time one is detected, a call to format_job_info() 
 * is made to print the status info to the user. 
 */
void check_current_jobs();

/**
 * Searches the current_jobs vector<Input*> for a job with the same JID as the given
 * input. If it finds one, it deletes it from memory and replaces its value in the 
 * current_jobs vector with nullptr. 
 *
 * @param Input* the given Input obj/job to delete from the heap/current_jobs vector
 */
void delete_from_current_jobs(Input*);

/**
 * Parent SIGCHLD handler. Calls check_current_jobs().
 *
 * @param int the SIGCHLD signal received by the handler
 */
void handler_SIGCHLD(int);

/**
 * Changes the cwd to the user's home dir. Called at shell init.
 */
void chdir_home();

/**
 * Prints ASCII art shell logo.
 */
void print_logo();

/**
 * Determines whether or not a given command is built into the 1730sh.
 *
 * @param string& the given commmand to check
 * @return true if the given command is a built-in, false if not
 */
bool isBuiltIn(string&);

/**
 * Calls the given built-in shell function.
 *
 * @param string& the name of the command to call
 * @param const vector<string>& the args with which to call 'exit'
 * @param Input* the dynamically allocated job which will need to be deleted
 */
void callBuiltIn(string&, const vector<string>&, Input*);

/**
 * Exits the program with specified status, or, if unspecified, with exit 
 * status of last process called.
 *
 * @param const vector<string>& the args with which to call 'exit'
 * @return the status with which to exit the shell. -1 if invalid syntax.
 */
int exit_builtin(const vector<string>&);

/**
 * Displays some helpful information about the implemented built-ins of the shell.
 *
 * @param const vector<string>& the args with which to call 'help'
 * @return -1 if invalid syntax, 0 otherwise
 */
int help_builtin(const vector<string>&);

/**
 * Changes the current working directory to the specified path. The default is the 
 * user's home directory.
 *
 * @param const vector<string>& the args with which to call 'cd'
 * @return -1 if invalid syntax or system call failure, 0 otherwise
 */
int cd_builtin(const vector<string>&);

/**
 * Changes the specified environment variable to given value. If the variable doesn't
 * exist yet, it is created and given the value of an empty string. 
 *
 * @param const vector<string>& the args with which to call 'export'
 * @return -1 if invalid syntax or system call failure, 0 otherwise
 */
int export_builtin(const vector<string>&);

/**
 * Puts the job specified by the pid_t arg into the foreground.
 *
 * @param const vector<string>& the args with which to call 'fg'
 * @return -1 if invalid syntax or system call failure, 0 otherwise
 */
int fg_builtin(const vector<string>&);

/**
 * Puts the job specified by the pid_t arg into the background.
 *
 * @param const vector<string>& the args with which to call 'bg'
 * @return -1 if invalid syntax or system call failure, 0 otherwise
 */
int bg_builtin(const vector<string>&);

/**
 * Prints the status of each currently running/stopped job.
 *
 * @param const vector<string>& the args with which to call 'bg'
 * @return -1 if invalid syntax or system call failure, 0 otherwise
 */
int jobs_builtin(const vector<string>&);

/**
 * Sends a signal to a specified PID, if PID > 0. If PID == 0, the signal is sent to
 * every process in the current process group. If PID == -1, the signal is sent to
 * every PID to which the utility has permission to send signals. If PID < -1, the signal 
 * is sent to every process in the process group whose PGID == |PID|. 
 *
 * @param const vector<string>& the args with which to call 'export'
 * @return -1 if invalid syntax or system call failure, 0 otherwise
 */
int kill_builtin(const vector<string>&);

// GLOBALS

int last_exit_status = EXIT_SUCCESS;
int shell_terminal = STDIN_FILENO;
pid_t shell_pgid = getpgrp();
vector<Input*> current_jobs{};

// MAIN

int main(int argc, char * argv[]) {

  // prints shell logo
  print_logo();

  // set job control signal dispositions to SIG_IGN
  parent_signals();

  cout.setf(std::ios::unitbuf);
  cin.setf(std::ios::unitbuf);

  // changes to user's home dir upon shell init
  chdir_home();

  string input = "";
  bool hangingPipe = false;
  bool hangingQuote = false;
  
  // begin REPL loop
  while(1) { // exits when ^C

    // polls all of the currently running jobs for status changes
    check_current_jobs();
    
    // prompt
    if(!hangingPipe && !hangingQuote) {
      prompt();
    } else {
      cout << "> ";
    } // if/else

    // only reset input if not waiting on more (due to hanging pipe OR hanging quote). otherwise, append to it.
    if(!hangingPipe && !hangingQuote) {
      input = "";
      getline(cin,input);
      input = trim(input);
    } else {
      string tmp = "";
      getline(cin,tmp);
      tmp = trim(tmp);
      if(hangingQuote) {
	input = input + tmp;
      } else if(hangingPipe) {
	input = input + " " + tmp;
      } // if/else
    } // if/else

    // user just hit [enter]
    if(input == "") continue;

    // if user input actually contains something, check if valid
    if(isValidInput(input)) {
      if(hasQuotes(input) && !hasEvenQuotes(input)) {
	hangingQuote = true;
	continue;
      } // if
      hangingQuote = false;
      if(input[input.length()-1] == '|') {
	hangingPipe = true;
	continue;
      } // if
      hangingPipe = false;

      // if finally have valid input AND no hanging pipes/quotes, make Input obj and do stuff
      Input * job = new Input(input);
      int ** pipes = makePipes(job->getNumPipes());
      int fd_STDIN = STDIN_FILENO;
      int fd_STDOUT = STDOUT_FILENO;
      int fd_STDERR = STDERR_FILENO;
      int pid; 
      
      // sets and/or creates the destinations for any i/o redirection. default is STD[IN/OUT/ERR]_FILENO
      if(set_redirects(job,fd_STDIN,fd_STDOUT,fd_STDERR) == -1) { delete job; continue; } // if

      // command is either built-in || has no pipes
      if(job->getProcesses().size() == 1) {
	string command = job->getProcesses()[0].args[0];
	if(isBuiltIn(command)) { // does not involve fork/exec
	  do_redirects(fd_STDIN,fd_STDOUT,fd_STDERR);
	  close_redirects(fd_STDIN,fd_STDOUT,fd_STDERR);
	  callBuiltIn(command, job->getProcesses()[0].args, job);
	  continue;
	} else { // involves fork/exec
	  if((pid = fork()) == -1) {
	    nope_out("fork");
	  } else if(pid == 0) { // in child
	    child_signals(); // resets signal dispositions back to default
	    job->getProcesses()[0].PID = getpid(); // sets pid for bookkeeping
	    job->setJID(getpid()); // sets JID/PGID of current Input obj/Processes for bookkeeping
	    if(setpgid(getpid(),job->getJID()) == -1) { nope_out("setpgid"); } // sets pgid of process in system
	    if(job->isForeground()) {
	      if(tcsetpgrp(shell_terminal, job->getJID()) == -1) { nope_out("tcsetpgrp"); } // makes JID the foreground pgrp of the terminal
	    } // if
	    do_redirects(fd_STDIN,fd_STDOUT,fd_STDERR);
	    close_redirects(fd_STDIN,fd_STDOUT,fd_STDERR);
	    nice_exec(job->getProcesses()[0].args, pipes, job->getNumPipes());
	  } else { // in parent
	    job->getProcesses()[0].PID = pid; // sets pid for bookkeeping
	    job->setJID(pid); // sets JID/PGID of current Input obj/Processes for bookkeeping
	    if(setpgid(pid,job->getJID()) == -1) { nope_out("setpgid"); } // sets pgid of process in system
	  } // if/else
	  // after job has been launched
	  current_jobs.push_back(job); // add to vector of currently running jobs
	} // if/else
      } else { // command is a pipelined job
	for(unsigned int i = 0, size = job->getProcesses().size(); i < size; i++) {
	  if(i != size-1) { // not last process
	    if(pipe(pipes[i]) == -1) { nope_out("pipe"); } // if
	  } // if
	  if((pid = fork()) == -1) {
	    nope_out("fork");
	  } else if(pid == 0) { // in child
	    job->getProcesses()[i].PID = getpid(); // sets pid for bookkeeping
	    if(i == 0) { job->setJID(getpid()); } // sets JID/PGID of current Input obj/Processes for bookkeeping
	    if(setpgid(getpid(),job->getJID()) == -1) { nope_out("setpgid"); } // sets pgid of process in system
	    if(job->isForeground()) {
	      if(tcsetpgrp(shell_terminal, job->getJID()) == -1) { nope_out("tcsetpgrp"); } // makes JID the foreground pgrp of the terminal
	    } // if
	    child_signals(); // reset signal dispositions back to default
	    if(i == 0) { // first process
	      do_redirects(fd_STDIN,pipes[i][1],-1);
	      close_pipe(pipes[i],true);
	    } else if(i != size-1) { // some middle process
	      do_redirects(pipes[i-1][0],pipes[i][1],-1);
	      close_pipe(pipes[i-1],true);
	      close_pipe(pipes[i],true);
	    } else if(i == size-1) { // last process
	      do_redirects(pipes[i-1][0],fd_STDOUT,fd_STDERR);
	      close_pipe(pipes[i-1],true);
	    } // if/else
	    close_redirects(fd_STDIN,fd_STDOUT,fd_STDERR);
	    nice_exec(job->getProcesses()[i].args, pipes, job->getNumPipes());
	  } else { // in parent
	    job->getProcesses()[i].PID = pid; // sets pid for bookkeeping
	    if(i == 0) { job->setJID(pid); } // sets JID/PGID of current Input obj/Processes
	    if(setpgid(pid,job->getJID()) == -1) { nope_out("setpgid"); } // sets pgid of process in system
	    if(i != 0) {
	      close_pipe(pipes[i-1],true);
	    } // if
	  } // if/else	
	} // for	
	// after job has been launched
	current_jobs.push_back(job); // add to vector of currently running jobs
      } // if/else
      
      close_redirects(fd_STDIN,fd_STDOUT,fd_STDERR); // close i/o redirect fd's
      deletePipes(pipes,job->getNumPipes()); // delete dynamically-allocated pipefd[2] array

      // waits on last child of job if in foreground. if in background, it doesnt.
      if(job->isForeground()) {
	put_job_in_foreground(job,false);
      } else {
	put_job_in_background(job,false);
      } // if/else
    } else { // invalid syntax
      cout << "./1730sh: Invalid command syntax" << endl;
    } // if/else
  } // while
  return EXIT_SUCCESS;
} // main

// DEFINITIONS

inline void nope_out(const string & sc_name) {
  perror(sc_name.c_str());
  exit(EXIT_FAILURE);
} // nope_out

void prompt() {
  char cwd[PATH_MAX];
  const char * homedir = nullptr;
  string s_cwd = "";
  string s_homedir = "";
  string prompt = "";
  // home dir stuff
  if((homedir = getenv("HOME")) == NULL) {
    homedir = getpwuid(getuid())->pw_dir;
  } // if
  s_homedir = string(homedir);
  // current dir stuff
  if(getcwd(cwd,sizeof(cwd)) == nullptr) { nope_out("getcwd"); } // if
  s_cwd = string(cwd);
  // prompt stuff
  prompt = s_cwd;
  size_t pos;
  if((pos = prompt.find(s_homedir)) != string::npos) {
    prompt.replace(pos,s_homedir.size(),"~");
  } // if
  cout << "1730sh:" << prompt << "$ ";
} // prompt

bool isValidInput(string input) {
  if(input.length() == 0) return true;
  bool redirect_in_flag = false;
  bool redirect_out_flag = false;
  bool redirect_err_flag = false;
  stringstream ss(input);
  vector<string> argv;
  string arg;
  while(ss >> arg) {
    argv.push_back(arg);
  } // while
  // can not begin input with a pipe
  if(argv[0] == "|") return false;
  // only allowed to have one redirect of each kind (STDIN, STDOUT, STDERR)
  for(unsigned int i = 0; i < argv.size(); i++) {
    if(argv[i] == "<") {
      if(!redirect_in_flag) { 
	redirect_in_flag = true; 
      } else {
	return false;
      } // if/else
    } else if(argv[i] == ">" || argv[i] == ">>") {
      if(!redirect_out_flag) { 
	redirect_out_flag = true; 
      } else {
	return false;
      } // if/else
    } else if(argv[i] == "e>" || argv[i] == "e>>") {
      if(!redirect_err_flag) { 
	redirect_err_flag = true; 
      } else {
	return false;
      } // if/else
    } // if/else
  } // for  
  return true;
} // isValidInput

bool hasEvenQuotes(string input) {
  int count = 0;
  for(unsigned int i = 0; i < input.length(); i++) {
    if(i == 0) {
      if(input[i] == '"') count++;
    } else {
      if(input[i] == '"' && input[i-1] != '\\') count++;
    } // if/else
  } // for
  if(count % 2 == 0) { 
    return true;
  } else {
    return false;
  } // if/else
} // hasQuotes

vector<char *> mk_cstrvec(vector<string> & strvec) {
  vector<char *> cstrvec;
  for (unsigned int i = 0, s = strvec.size(); i < s; i++) {
    cstrvec.push_back(new char [strvec.at(i).size() + 1]);
    strcpy(cstrvec.at(i), strvec.at(i).c_str());
  } // for
  cstrvec.push_back(nullptr);
  return cstrvec;
} // mk_cstrvec

void dl_cstrvec(vector<char *> & cstrvec) {
  for (unsigned int i = 0; i < cstrvec.size(); ++i) {
    delete[] cstrvec.at(i);
  } // for
} // dl_cstrvec

void nice_exec(vector<string> strargs, int** pipes, int numPipes) {
  vector<char *> cstrargs = mk_cstrvec(strargs);
  execvp(cstrargs.at(0), &cstrargs.at(0));
  // only makes it here if execvp fails
  cout << "1730sh: " << cstrargs.at(0) << ": command not found" << endl;
  // close pipes
  for(int i = 0; i < numPipes; i++) {
    close_pipe(pipes[i],false);
  } // for
  // delete dynamically-allocated members of current_jobs vector<Input*>
  for(unsigned int i = 0, s = current_jobs.size(); i < s; i++) {
    if(current_jobs[i] != nullptr) { delete current_jobs[i]; current_jobs[i] = nullptr; } // if
  } // for
  // delete dynamically-allocated pipes
  deletePipes(pipes,numPipes);
  dl_cstrvec(cstrargs);
  exit(EXIT_FAILURE);
} // nice_exec

int** makePipes(int numPipes) {
  int** pipes = nullptr;
  if(numPipes != 0) {
    pipes = new int * [numPipes];
    for(int i = 0; i < numPipes; i++) {
      pipes[i] = new int[2];
    } // for
  } // if
  return pipes;
} // makePipes

void deletePipes(int** pipes, int numPipes) {
  if(pipes != nullptr) {
    for(int i = 0; i < numPipes; i++) {
      delete[] pipes[i];
    } // for
    delete[] pipes;
  } // if
} // deletePipes

void close_pipe(int pipefd[2], bool exit_on_failure) {
  if(close(pipefd[0]) == -1) {
    if(exit_on_failure) { nope_out("close"); } // if
  } // if
  if(close(pipefd[1]) == -1) {
    if(exit_on_failure) { nope_out("close"); } // if
  } // if
} // close_pipe

int set_redirects(Input * job, int& fdSTDIN, int& fdSTDOUT, int& fdSTDERR) {
  fdSTDIN = STDIN_FILENO;
  fdSTDOUT = STDOUT_FILENO;
  fdSTDERR = STDERR_FILENO;
  // if user input specified any i/o redirection, open/create the given fd's
  if(job->getSTDIN_fd() != "STDIN_FILENO") {
    if((fdSTDIN = open(job->getSTDIN_fd().c_str(), O_RDONLY)) == -1) {
      cout << "1730sh: " << job->getSTDIN_fd() << ": No such file or directory" << endl;
      return -1;
    } // if
  } // if
  if(job->getSTDOUT_fd() != "STDOUT_FILENO") {
    if(job->getSTDOUT_type() == ">") {
      if((fdSTDOUT = open(job->getSTDOUT_fd().c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1) {
	cout << "1730sh: `" << job->getSTDOUT_fd() << "' cannot be opened" << endl;
	return -1;
      } // if
    } else {
      if((fdSTDOUT = open(job->getSTDOUT_fd().c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666)) == -1) {
	cout << "1730sh: `" << job->getSTDOUT_fd() << "' cannot be opened" << endl;
	return -1;
      } // if
    } // if/else
  } // if
  if(job->getSTDERR_fd() != "STDERR_FILENO") {
    if(job->getSTDERR_type() == "e>") {
      if((fdSTDERR = open(job->getSTDERR_fd().c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1) {
	cout << "1730sh: `" << job->getSTDERR_fd() << "' cannot be opened" << endl;
	return -1;
      } // if
    } else {
      if((fdSTDERR = open(job->getSTDERR_fd().c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666)) == -1) {
	cout << "1730sh: `" << job->getSTDERR_fd() << "' cannot be opened" << endl;
	return -1;
      } // if
    } // if/else
  } // if
  return 0;
} // set_redirects

void do_redirects(int STDIN, int STDOUT, int STDERR) {
  if(STDIN >= 0) {
    if(dup2(STDIN, STDIN_FILENO) == -1) nope_out("dup2");
  } // if
  if(STDOUT >= 0) {
    if(dup2(STDOUT, STDOUT_FILENO) == -1) nope_out("dup2");
  } // if
  if(STDERR >= 0) {
    if(dup2(STDERR, STDERR_FILENO) == -1) nope_out("dup2");
  } // if
} // do_redirects

void close_redirects(int STDIN, int STDOUT, int STDERR) {
  if(STDIN != STDIN_FILENO) {
    if(close(STDIN) == -1) nope_out("close");
  } // if
  if(STDOUT != STDOUT_FILENO) {
    if(close(STDOUT) == -1) nope_out("close");
  } // if
  if(STDERR != STDERR_FILENO) {
    if(close(STDERR) == -1) nope_out("close");
  } // if
} // close_redirects

void parent_signals() {
  if(signal(SIGINT, SIG_IGN) == SIG_ERR) nope_out("signal");
  if(signal(SIGQUIT, SIG_IGN) == SIG_ERR) nope_out("signal");
  if(signal(SIGTSTP, SIG_IGN) == SIG_ERR) nope_out("signal");
  if(signal(SIGTTIN, SIG_IGN) == SIG_ERR) nope_out("signal");
  if(signal(SIGTTOU, SIG_IGN) == SIG_ERR) nope_out("signal");
  if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) nope_out("signal");
} // parent_signals

void child_signals() {
  if(signal(SIGINT, SIG_DFL) == SIG_ERR) nope_out("signal");
  if(signal(SIGQUIT, SIG_DFL) == SIG_ERR) nope_out("signal");
  if(signal(SIGTSTP, SIG_DFL) == SIG_ERR) nope_out("signal");
  if(signal(SIGTTIN, SIG_DFL) == SIG_ERR) nope_out("signal");
  if(signal(SIGTTOU, SIG_DFL) == SIG_ERR) nope_out("signal");
  if(signal(SIGCHLD, SIG_DFL) == SIG_ERR) nope_out("signal");
  if(signal(SIGPIPE, SIG_DFL) == SIG_ERR) nope_out("signal");
} // child_signals

void format_job_info(Input * job, const char * status) {
  if(job != nullptr) {
    cout << job->getJID() << " " << string(status) << " " << job->getShellInput() << endl;
  } // if
} // format_job_info

void wait_for_job(Input * job) { 
  if(job != nullptr) {
    int wpid, pstatus;
    if((wpid = waitpid(job->getProcesses().back().PID, &pstatus, WUNTRACED)) > 0) {
      if(WIFEXITED(pstatus)) {
	cout << job->getJID() << " " 
	     << "Exited (" << WEXITSTATUS(pstatus) << ")" << " " 
	     << job->getShellInput() << endl;
	delete_from_current_jobs(job);	
	last_exit_status = WEXITSTATUS(pstatus);
      } else if(WIFSIGNALED(pstatus)) {
	cout << job->getJID() << " " 
	     << "Exited (" << strsignal(WTERMSIG(pstatus)) << ")" << " " 
	     << job->getShellInput() << endl;
	delete_from_current_jobs(job);
	last_exit_status = WTERMSIG(pstatus);
      } else if(WIFSTOPPED(pstatus)) {
	job->setStatus("Stopped");
	format_job_info(job,"Stopped");
      } else if(WIFCONTINUED(pstatus)) {
	job->setStatus("Running");
	format_job_info(job,"Continued");
      } // if/else
    } // if
  } // if
} // wait_for_job

void put_job_in_foreground(Input * job, bool cont) {
  if(job != nullptr) {
    // makes JID the foreground pgrp of the terminal
    if(tcsetpgrp(shell_terminal, job->getJID()) == -1) { nope_out("tcsetpgrp"); } 
    // Send the job a continue signal, if necessary
    if(cont) {
      if(kill(-job->getJID(), SIGCONT) < 0) {
	nope_out("kill(SIGCONT)");
      } // if
    } // if
    // wait for job to finish
    wait_for_job(job); 
    // makes the shell the foreground pgrp again
    if(tcsetpgrp(shell_terminal, shell_pgid) == -1) { nope_out("tcsetpgrp"); }
  } // if
} // put_job_in_foreground

void put_job_in_background(Input * job, bool cont) {
  if(job != nullptr) {
    if(cont) {
      if(kill(-job->getJID(), SIGCONT) < 0) {
	nope_out("kill(SIGCONT)");  
      } // if
    } // if
  } // if
} // put_job_in_background

void check_current_jobs() {
  pid_t wpid;
  int pstatus;
  for(unsigned int i = 0, s = current_jobs.size(); i < s; i++) {
    if(current_jobs[i] != nullptr) {
      pid_t pid = current_jobs[i]->getProcesses().back().PID;
      while((wpid = waitpid(pid, &pstatus, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
	if(WIFEXITED(pstatus)) {
	  cout << current_jobs[i]->getJID() << " "
	       << "Exited (" << WEXITSTATUS(pstatus) << ")" << " " 
	       << current_jobs[i]->getShellInput() << endl;
	  last_exit_status = WEXITSTATUS(pstatus);
	  delete_from_current_jobs(current_jobs[i]);
	  break;
	} else if(WIFSIGNALED(pstatus)) {
	  cout << current_jobs[i]->getJID() << " "
	       << "Exited (" << strsignal(WTERMSIG(pstatus)) << ")" << " " 
	       << current_jobs[i]->getShellInput() << endl;
	  last_exit_status = WTERMSIG(pstatus);
	  delete_from_current_jobs(current_jobs[i]);
	  break;
	} else if(WIFSTOPPED(pstatus)) {
	  current_jobs[i]->setStatus("Stopped");
	  format_job_info(current_jobs[i],"Stopped");
	  break;
	} else if(WIFCONTINUED(pstatus)) {
	  current_jobs[i]->setStatus("Running");
	  format_job_info(current_jobs[i],"Continued");
	  break;
	} // if/else	
      } // while
    } // if
  } // for
} // check_current_jobs

void delete_from_current_jobs(Input * job) {
  if(job != nullptr) {
    for(unsigned int i = 0, s = current_jobs.size(); i < s; i++) {
      if(current_jobs[i] != nullptr) {
	if(job->getJID() == current_jobs[i]->getJID()) { 
	  delete job; 
	  current_jobs[i] = nullptr; 
	  break;
	} // if
      } // if
    } // for
  } // if
} // delete_from_current_jobs

void handler_SIGCHLD(int signo) {
  check_current_jobs();
} // handler_SIGCHLD

void chdir_home() {
  // home dir retrieval
  const char * homedir = nullptr;
  string s_homedir = "";
  if((homedir = getenv("HOME")) == nullptr) {
    homedir = getpwuid(getuid())->pw_dir;
  } // if
  s_homedir = string(homedir);
  chdir(homedir);
} // chdir_home

void print_logo() {
  cout << ":  ....,.......,..,...  ..,,,.,:::~::::::::+~=~:,,., ..,,.,.,:,,,.,,..........+Z" << endl;
  cout << "....... ...,,..,...........,~OOOO88N$$$ZO$$$$$$7=,...........~:,,...,.........,Z" << endl;
  cout << ".. .........~.........,:=$8DMNDD8DNNDNDDDNNDDNNDZI+:.......,...................," << endl;
  cout << "....................,~7ODDDMD8ODNNNDDD88ND8DDNNDNNO?:,.,,,,,. ..,:... .........:" << endl;
  cout << "..................,=$MNDDDDDDDD8O88DND8OO88OD8DMNNNNI~,...,,,........  ..... .~?" << endl;
  cout << "...............,,~?NDN8DN8O$$Z77I?IIIII777$$Z8DNDDNDN7=:....,.............. ..+I" << endl;
  cout << ".................$DDDNDDO$$$77IIIIIIII?I??IIII7$$Z7ODN7+.................  .. .," << endl;
  cout << "................?8NDNDDOZ7$777II???III?????III77$$Z$Z88I~............. ..  ....." << endl;
  cout << ".... ..........,DNDNDD8Z$77777III?+++???????II777$$ZOO87~....... ... ..........," << endl;
  cout << " .............,~DNNDD8OZ77$77IIIIII=++++?++??I7777$ZZO8D~,,.,.. ....... .......:" << endl;
  cout << "...... ...,...:$NDDD8OO$7$7$77I??+~+====?=IIIII77$$ZOO8O+,..... ....:. .. ......" << endl;
  cout << "...... ...,...=ZNNNN88OZ$777$7I?I++==+++++=??+??I7$ZZOOO7...,......:............" << endl;
  cout << "....... ......:DNDNDD8O$$$$$$77I??+===+?+I??II?II77ZZZOO8,.. .,......... ......." << endl;
  cout << ".........,.. .:8NDDDDDZ$7$$$$7???+==~~~==?????+??I7$ZZOOO+...::,...............:" << endl;
  cout << "..... ........~7NNDD8O$$$ZZZ777O7$$$Z$7?+~~=++?+II7$$ZZOO=, ..,,,.....:... . ..?" << endl;
  cout << "....  .. .....~7DDDD8Z$$$Z8MNMMNNMMMNMMMMMNND8DNNMMMMMMNMN7I?...,... .~.. ....,=" << endl;
  cout << "..............+?ZDDD8OZ$ZNMMMMMMMMMNMMNMMMMNMMMMMMMMMMMMMNMMM~...... ,........,=" << endl;
  cout << ".. . .........,=ZDD8OZZ$$MNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNNMI=..,.    .  .   . ." << endl;
  cout << ". . . .  .....,INNDNMMNMNNMMMMMMMMMMMMMMMMD~MMNMMMMMMMMMMMDMDI.....             " << endl;
  cout << "..   .......,=Z77ODDOO$ZZMMMMMMMMMMNMMMMNMM$?$DMMMMMMMMMMNNMN~...,.. ..  ......," << endl;
  cout << ".    ........:D8$I7OOZZ$$MNMMMMMMMMMMMNMMMZ?+IOMMMMMMMMMMNMM7?.....   .  ......." << endl;
  cout << "..   ........+$$777ZOZZ$$MNMMMNNNNNNMNMMMN?+?I$NMMMMMMMMMNMZ?~....    ...   ...." << endl;
  cout << ". .  ........~I7IOZ$OZ$$$MNMNNNNNNMNNNMMM7?+??$OMMMMMMMMMMM?=... .  . ... ..  .~" << endl;
  cout << ".    ........~III$$7ZOZ$7$DMMNNNNNNNNNND$II???IZ8MMMMMMMMMM=.,....   ..  .... .Z" << endl;
  cout << ".    ........~III$$7ZOZ$7$DMMNNNNNNNNNND$II???IZ8MMMMMMMMMM=.,....   ..  .... .Z" << endl;
  cout << "....... .....~77??7$OZZZ$7III?++++++++7II7ZZ$7Z8OZOODDDND8=.... ... ...       .." << endl;
  cout << "...  . .,,...:II$$Z$OZZZ$777??++==++++$8NMOOMMMMNDZ$$$ZOO?....... ..,,..     . ." << endl;
  cout << ". . . ........:+I7I7OZ$Z$$77I??+++????III$IZ7788OZZZ$$ZOO+..,,.. .........  ...." << endl;
  cout << ". ... .........,:=$8OZZ$$$777IIII?I?+??$$$I?ZZ788O$$Z$ZOOI,......  .......  . .." << endl;
  cout << "... .............:~Z8Z$$$777777I7I+?O$7+I7I?77$$$88ZZOOO8=,,..,,... ...... ....." << endl;
  cout << "... . ......,~.....~ZOZ$$$777777$7?7Z7OOZOZZ7ZO8OOOOZOOOZ,.....,:,. ....,.....:I" << endl;
  cout << "..  ........,,.....,ZOZZ$7$777777778DOII?I??I??ZNN88ZOOZ?,:...,.. .....,...  .?Z" << endl;
  cout << "+,..........:.... .,ZZOZZ$$$$$777$ZI??I?????I?7$ZZ7$ZO$I:,.. ..,... ....,. ...$O" << endl;
  cout << "..... ... .........:ZZZ8OZZZ$$$$7$ZI?III77O88DD8$Z$$O$=:.,....,,=~............,7" << endl;
  cout << ".. ... ..... ....,::ZZZZ8OOZZZZ$OO87$7$777I787ZZOOO8O$?,,. .,.:,......,.....,.. " << endl;
  cout << "... ........:...,=~NOZ$ZZ888OZOZZO8ZZZ$$$$ZOZ8O88DNDDDDO...:......,,:.... .?...." << endl;
  cout << "..............,,:NM8OZZ$ZOONDO8OZOD$$Z7777I7ZZOO8DNNMMMND$N..,.............. .:+" << endl;
  cout << ".  .... ......::NMNMOZZ$$ZO8DMD88ODOD888ZO8ZDD88NMMMMN8D++7M?~...,,,..:,......=I" << endl;
  cout << ".............,DDNOZM8ZZZ$$ZZODMNDO88ODODO88DDDNNMMMND8O7$NMNN:...,,...........~I" << endl;
  cout << ".....,......,NDNDOZ8DZ$ZZ$ZZZOOOMM8NDNN88O8DNNNNMMNO877ZOMNMNN.,..... .......,,?" << endl;
  cout << "~. .......,:DDDDDDD+8Z$$$ZZZZZOZZZMMMMMMMMMMNMMMM8OO8NNNNMMNMNN+~.............:+" << endl;
  cout << ".. ......,+DDDDNDDD88O$$$77$ZZOZOOZDMMMMMMMMMMMMOOZO8NMMMDMNNNDN7=,,..........:+" << endl;
  cout << "..,.. .,.ZDDDDDND88N8OZ$$77$Z$ZOOOOOONMMMMMMMMMOZZOO8NMN88MNNNNNNNN$=~:......,,=" << endl;
  cout << ".......:ODDDDDNODD8D8OZ$$$$$7$ZOOOO888DMMMMMMMOZOOOOD8NNIDMNDNNNNNNNZ8?:.. . ..=" << endl;
  cout << "..  .+O8DDNDDDO8DDD8DDZ$$$$77$$ZZOO888DDDMMMNOZZZOOOM+NMDD8NNDNNDNNDNDODD$+,...." << endl;
  cout << "...:7Z8DDDDDDDDDDDDO88=$$$$$7777$Z$ZO8OOOZZZZZZZZOONMDDN$$DMNDDNNNDDDDD8DDD8=,,," << endl;
  cout << "?$ZO8DDDDDNDDDNDDND7$888Z$$$$$77$$$$$$$$$$777$ZZONNM$DDN$Z7MDDNNNNNN8DDDDDDD8ZII" << endl;
  cout << "D88DD8DDDDDNDDNDDNN8D88DD88$$$7$7$$$$$7$7777$8NNNNNZ88DM$DOMNDNNNNNNDDDDNNDDNDN8" << endl;
  cout << "DDDD8DDDDNNDDDN88NN8DN8:$888Z$$77777$77$8DDNNNNNNNN~DDNNDNMMNDDNNNNNDNDDDNNDDDND" << endl;
  cout << "DDD8NDDNDNDNDNND8DM?8D+OI~OD88D8DDNDDNNNNNNNNNNNDDZDDDNN=NDDMDNNNNNNNNDNDDNDDNDD" << endl;
  cout << "DDDNDNDDDNNDDDND88D788N?~D88888O8DNNDDNDDDNDDDDDDND8DD8I?DDOMNNNNNNNDNNDNDDND8DD" << endl;
  cout << "DDDDDDDDNDNNNDDD88D$DDDD8DD8788888DDDDDDDDDDDDDDDNDD8D:D8D?IMDDNNNNNNNDDNDDDNDDD" << endl;
  cout << "" << endl;
} // print_logo

// BUILT-IN STUFF

bool isBuiltIn(string & command) {
  bool isBuiltIn = false;
  if(command == "cd") {
    isBuiltIn = true;
  } else if(command == "exit") {
    isBuiltIn = true;
  } else if(command == "help") {
    isBuiltIn = true;
  } else if(command == "bg") {
    isBuiltIn = true;
  } else if(command == "fg") {
    isBuiltIn = true;
  } else if(command == "export") {
    isBuiltIn = true;
  } else if(command == "jobs") {
    isBuiltIn = true;
  } else if(command == "kill") {
    isBuiltIn = true;
  } // if/else
  return isBuiltIn;
} // isBuiltIn

void callBuiltIn(string & command, const vector<string>& args, Input * job) {
  if(command == "cd") { // changes current working dir
    last_exit_status = (cd_builtin(args) == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
  } else if(command == "exit") { // exits shell with specified status
    int status;
    last_exit_status = ((status = exit_builtin(args)) != -1) ? status : EXIT_FAILURE;
    delete job;
    if(status != -1) {
      for(unsigned int i = 0, s = current_jobs.size(); i < s; i++) {
	if(current_jobs[i] != nullptr) delete current_jobs[i]; current_jobs[i] = nullptr;
      } // for
      exit(status);
    } // if
  } else if(command == "help") { // prints built-in command options
    last_exit_status = (help_builtin(args) == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
  } else if(command == "bg") { // puts and resumes job in background
    last_exit_status = (bg_builtin(args) == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
  } else if(command == "fg") { // puts and resumes job in foreground
    last_exit_status = (fg_builtin(args) == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
  } else if(command == "export") { // changes env variables
    last_exit_status = (export_builtin(args) == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
  } else if(command == "jobs") { // prints status of currently running jobs
    last_exit_status = (jobs_builtin(args) == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
  } else if(command == "kill") { // sends specified signal to specified pid/pgid
    last_exit_status = (kill_builtin(args) == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
  } // if/else
  delete job;
} // callBuiltIn

int exit_builtin(const vector<string> & args) {
  int status = -1;
  if(args.size() == 1) { // 'exit'
    status = last_exit_status;
    cout << "1730sh: Exited with status: " << status << endl;
    return status;
  } else if(args.size() == 2) { // 'exit [int]'
    bool isNumber = true;
    for(int i = 0, size = args[1].size(); i < size; i++) {
      if(!isdigit(args[1][i])) { // if (number[i] > '9' || number[i] < '0')
	isNumber = false;
	break;
      } // if
    } // for
    if(isNumber) {
      try {
	status = stoi(args[1],0,10);
	cout << "1730sh: Exited with status: " << status << endl;
      } catch(invalid_argument invalidargument) {
	cout << "1730sh: exit: Invalid exit value" << endl;
	status = -1;
      } catch(out_of_range outofrange) {
	cout << "1730sh: exit: Invalid exit value" << endl;
	status = -1;
      } // try/catch
      return status;
    } else {
      cout << "1730sh: exit: Invalid exit value" << endl;
      status = -1;
    } // if/else
    return status;
  } else { // invalid syntax
    cout << "Usage: exit [N]" << endl;
    status = -1;
    return status;
  } // if/else
} // exit_builtin

int help_builtin(const vector<string> & args) {
  if(args.size() != 1) {
    cout << "Usage: help" << endl;
    return -1;
  } else {
    cout << "Here are some useful commands to make navigating the shell a bit easier:" << endl;
    cout << endl;
    cout << "bg JID – Resume the stopped job JID in the background, as if it had been started with &." << endl;
    cout << endl;
    cout << "cd [PATH] – Change the current directory to PATH. The environmental variable HOME is the default PATH." << endl;
    cout << endl;
    cout << "exit [N] – Cause the shell to exit with a status of N. If N is omitted, the exit status is that of the last job executed." << endl;
    cout << endl;
    cout << "export NAME[=WORD] – the variable NAME is automatically included in the environment of subsequently executed jobs." << endl;
    cout << endl;
    cout << "fg JID – Resume job JID in the foreground, and make it the current job." << endl;
    cout << endl;
    cout << "help – Display helpful information about builtin commands." << endl;
    cout << endl;
    cout << "jobs – List current jobs. Here is an example of the desired output:" << endl;
    cout << endl;
    cout << "          "; cout << "JID  STATUS      COMMAND" << endl;
    cout << "          "; cout << "1137 Stopped     less &" << endl;
    cout << "          "; cout << "2245 Running     cat /dev/urandom | less &" << endl;
    cout << "          "; cout << "2343 Running     ./jobcontrol &" << endl;
    cout << endl;
    cout << "kill [-s SIGNAL] PID – The kill utility sends the specified signal to the specified process or process group PID" << endl;
    cout << "(see kill(2)). If no signal is specified, the SIGTERM signal is sent. The SIGTERM signal will kill processes which do" << endl;
    cout << "not catch this signal. For other processes, it may be necessary to use the SIGKILL signal, since this signal cannot" << endl;
    cout << "be caught. If PID is positive, then the signal is sent to the process with the ID specified by PID. If PID equals 0," << endl;
    cout << "then the signal is sent to every process in the current process group. If PID equals -1, then the signal is sent to" << endl;
    cout << "every process for which the utility has permission to send signals to, except for process 1 (init). If PID is less than" << endl;
    cout << "-1, then the signal is sent to every process in the process group whose ID is -PID. When the -s SIGNAL option is" << endl;
    cout << "used, instead of sending SIGTERM, the specified signal is sent instead. SIGNAL can be provided as a signal number" << endl;
    cout << "or a constant (e.g., SIGTERM)." << endl;
    cout << endl;
    cout << "-- End help --" << endl;
    return 0;  
  } // if/else
} // help_builtin

int cd_builtin(const vector<string> & args) {
  if(args.size() > 2) { // too many args
    cout << "1730sh: Usage: cd [PATH]" << endl;
    return -1;
  } // if/else
  // home dir retrieval
  const char * homedir = nullptr;
  string s_homedir = "";
  if((homedir = getenv("HOME")) == nullptr) {
    homedir = getpwuid(getuid())->pw_dir;
  } // if
  s_homedir = string(homedir);
  // actual chdir stuff
  if(args.size() == 1) { // 'cd'
    if(chdir(homedir) == -1) { 
      int err = errno; 
      cout << "1730sh: cd: " << homedir << ": " << strerror(err) << endl;
      return -1;
    } // if
  } else if(args.size() == 2) { // cd [PATH]
    string dest = args[1];
    // replaces "~" with full path of HOME
    size_t pos;
    if((pos = dest.find("~")) != string::npos) {
      dest.replace(pos,1,s_homedir);
    } // if    
    // current dir retrieval
    char cwd[PATH_MAX];
    string s_cwd = "";
    if(getcwd(cwd,sizeof(cwd)) == nullptr) { 
      int err = errno; 
      cout << "1730sh: cd: " << strerror(err) << endl;
      return -1;      
    } // if
    s_cwd = string(cwd);
    if(dest[0] == '/') { // if dest already started with '/', no need to append to cwd
      if(chdir(dest.c_str()) == -1) { 
	int err = errno; 
	cout << "1730sh: cd: " << dest << ": " << strerror(err) << endl;
	return -1;
      } // if
    } else { // dest was a relative path, so need to append '/' + dest to cwd
      s_cwd = s_cwd + "/" + dest;
      if(chdir(s_cwd.c_str()) == -1) { 
	int err = errno; 
	cout << "1730sh: cd: " << dest << ": " << strerror(err) << endl;
	return -1;
      } // if
    } // if/else
  } // if/else
  return 0;
} // cd_builtin

int export_builtin(const vector<string> & args) {
  if(args.size() != 2) {
    cout << "1730sh: Usage: export NAME[=WORD]" << endl;
    return -1;
  } else {
    string str = args[1];
    size_t pos;
    if((pos = str.find("=")) != string::npos) {
      if(pos == 0) { // user entered '=NAME' or '='
	cout << "1730sh: Usage: export NAME[=WORD]" << endl;
	return -1;
      } else { // user entered 'NA=ME' or 'NAME='
	string name = str.substr(0,pos);
	string value = (pos == str.length()-1) ? "" : str.substr(pos+1,str.length()-1-pos); 
	if(setenv(name.c_str(),value.c_str(),1) == -1) {
	  int err = errno; 
	  cout << "1730sh: export: " << str << ": " << strerror(err) << endl;
	  return -1;
	} // if
      } // if/else
    } else { // user entered something like 'NAME'
      string name = str;
      string value = "";
      if(setenv(name.c_str(),value.c_str(),1) == -1) { 
	int err = errno; 
	cout << "1730sh: export: " << str << ": " << strerror(err) << endl;
	return -1;
      } // if
    } // if/else
  } // if/else
  return 0;
} // export_builtin

int fg_builtin(const vector<string> & args) {
  if(args.size() != 2) {
    cout << "1730sh: Usage: fg JID" << endl;
  } else {
    pid_t JID;
    bool isNumber = true;
    for(int i = 0, size = args[1].size(); i < size; i++) {
      if(!isdigit(args[1][i])) { // if (number[i] > '9' || number[i] < '0')
	isNumber = false;
	break;
      } // if
    } // for
    if(isNumber) {
      try {
	JID = stoi(args[1],0,10);
	for(unsigned int i = 0, s = current_jobs.size(); i < s; i++) {
	  if(current_jobs[i] != nullptr) {
	    if(current_jobs[i]->getJID() == JID) {
	      put_job_in_foreground(current_jobs[i],true);
	      return 0;
	    } // if
	  } // if
	} // for	
      } catch(invalid_argument invalidargument) {
	cout << "1730sh: fg: Invalid JID" << endl;
      } catch(out_of_range outofrange) {
	cout << "1730sh: fg: Invalid JID" << endl;
      } // try/catch
    } else {
      cout << "1730sh: fg: Invalid JID" << endl;
    } // if/else
  } // if/else
  return -1;
} // fg_builtin

int bg_builtin(const vector<string> & args) {
  if(args.size() != 2) {
    cout << "1730sh: Usage: bg JID" << endl;
  } else {
    pid_t JID;
    bool isNumber = true;
    for(int i = 0, size = args[1].size(); i < size; i++) {
      if(!isdigit(args[1][i])) { // if (number[i] > '9' || number[i] < '0')
	isNumber = false;
	break;
      } // if
    } // for
    if(isNumber) {
      try {
	JID = stoi(args[1],0,10);
	for(unsigned int i = 0, s = current_jobs.size(); i < s; i++) {
	  if(current_jobs[i] != nullptr) {
	    if(current_jobs[i]->getJID() == JID) {
	      put_job_in_background(current_jobs[i],true);
	      return 0;
	    } // if
	  } // if
	} // for	
      } catch(invalid_argument invalidargument) {
	cout << "1730sh: bg: Invalid JID" << endl;
      } catch(out_of_range outofrange) {
	cout << "1730sh: bg: Invalid JID" << endl;
      } // try/catch
    } else {
      cout << "1730sh: bg: Invalid JID" << endl;
    } // if/else
  } // if/else
  return -1;
} // bg_builtin

int jobs_builtin(const vector<string> & args) {
  if(args.size() != 1) {
    cout << "1730sh: Usage: jobs" << endl;
  } else {
    cout << std::left << setw(8) << "JID" << setw(13) << "STATUS" << "COMMAND" << endl;
    for(unsigned int i = 0, s = current_jobs.size(); i < s; i++) {
      if(current_jobs[i] != nullptr) {
	pid_t JID = current_jobs[i]->getJID();
	string status = current_jobs[i]->getStatus();
	string command = current_jobs[i]->getShellInput();
	cout << std::left << setw(8) << JID << setw(13) << status << command << endl;
      } // if
    } // for
    return 0;
  } // if/else
  return -1;
} // jobs_builtin

int kill_builtin(const vector<string> & args) {
  if(args.size() != 2 && args.size() != 4) {
    cout << "1730sh: Usage: kill [-s SIGNAL] PID" << endl;
  } else {
    // creates key-value pairs for posix signals
    const map<string, int> signal_map { 
      #ifdef SIGHUP
        {"SIGHUP", SIGHUP},
      #endif
      #ifdef SIGINT
	{"SIGINT", SIGINT},
      #endif
      #ifdef SIGTERM
	{"SIGTERM", SIGTERM},
      #endif
      #ifdef SIGKILL
	{"SIGKILL", SIGKILL},
      #endif
      #ifdef SIGSTOP
	{"SIGSTOP", SIGSTOP},
      #endif
      #ifdef SIGCONT
	{"SIGCONT", SIGCONT},
      #endif
      #ifdef SIGQUIT
	{"SIGQUIT", SIGQUIT},
      #endif
      #ifdef SIGALRM
	{"SIGALRM", SIGALRM},
      #endif
      #ifdef SIGTSTP
	{"SIGTSTP", SIGTSTP},
      #endif
    };
    // checks to see if last arg is a number
    bool isNumber = true;
    for(int i = 0, size = args.back().size(); i < size; i++) {
      if(i == 0) {
	if(args.back()[0] == '-') continue;
      } // if
      if(!isdigit(args.back()[i])) { // if (number[i] > '9' || number[i] < '0')
	isNumber = false;
	break;
      } // if
    } // for
    if(isNumber) {
      try {
	pid_t PID = stoi(args.back(),0,10);
	if(args.size() == 2) { // SIGTERM sent to PID
	  try {
	    int signo = signal_map.at("SIGTERM");
	    if(kill(PID,signo) != -1) return 0; // if
	  } catch (exception & e) {
	    cout << "1730sh: kill: Invalid signal" << endl;
	  } // try/catch
	} else { // custom signal sent to PID
	  if(args[1] == "-s") {
	    // checks to see if SIGNAL arg is a number or a string
	    bool sig_is_num = true;
	    for(int i = 0, size = args[2].size(); i < size; i++) {
	      if(!isdigit(args[2][i])) { // if (number[i] > '9' || number[i] < '0')
		sig_is_num = false;
		break;
	      } // if
	    } // for
	    if(sig_is_num) { // sig given as number
	      try {
		int signo = stoi(args[2],0,10);
		if(kill(PID,signo) != -1) return 0; // if
	      } catch(invalid_argument invalidargument) {
		cout << "1730sh: kill: `" << args.back() << "': Invalid signal" << endl;
	      } catch(out_of_range outofrange) {
		cout << "1730sh: kill: `" << args.back() << "': Invalid signal" << endl;
	      } // try/catch
	    } else { // sig given as string
	      try {
		int signo = signal_map.at(args[2]);
		if(kill(PID,signo) != -1) return 0; // if
	      } catch (exception & e) {
		cout << "1730sh: kill: `" << args.back() << "': Invalid signal" << endl;
	      } // try/catch
	    } // if/else
	  } else {
	    cout << "1730sh: Usage: kill [-s SIGNAL] PID" << endl;	
	  } // if/else
	} // if/else
      } catch(invalid_argument invalidargument) {
	cout << "1730sh: kill: `" << args.back() << "': Invalid PID" << endl;
      } catch(out_of_range outofrange) {
	cout << "1730sh: kill: `" << args.back() << "': Invalid PID" << endl;
      } // try/catch
    } else {
      cout << "1730sh: kill: `" << args.back() << "': Invalid PID" << endl;
    } // if/else
  } // if/else
  return -1;
} // jobs_builtin

