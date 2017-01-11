#ifndef INPUT_H
#define INPUT_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

struct Process {
  pid_t PID = -1;
  pid_t PGID = -1;
  bool stopped = false;
  bool completed = false;
  bool hasPipe = false;
  std::vector<std::string> args;
}; // process

class Input {
 private:
  pid_t JID = -1;
  bool foreground;
  const char * status = "Running";
  std::string shellInput;
  std::vector<Process> processes;
  std::string fd_STDIN;
  std::string fd_STDOUT;
  std::string type_STDOUT;
  std::string fd_STDERR;
  std::string type_STDERR;

  /**
   * Called in constructor. Sets the original, trimmed, shell input string.
   *
   * @param string the shell input string
   */
  void setShellInput(std::string);
  /**
   * Called in constructor. Converts the shellInput member into a vector<Process>.
   *
   * @return the vector of Process structs
   */
  std::vector<Process> inputToProcesses();
  /**
   * Called in constructor. Sets the 'foreground' property of the Input obj to true if job was placed
   * in background. False if not.
   */
  void set_foreground();
  /**
   * Called in constructor. Using the shell input, Sets the STDIN output destination. Default is STDIN_FILENO.
   */
  void setSTDIN();
  /**
   * Called in constructor. Using the shell input, Sets the STDOUT output destination. Default is STDOUT_FILENO.
   * Also sets the type of STDOUT. Types are ">" or ">>".
   */
  void setSTDOUT();
  /**
   * Called in constructor. Using the shell input, Sets the STDERR output destination. Default is STDERR_FILENO.
   * Also sets the type of STDERR. Types are "e>" or "e>>".
   */
  void setSTDERR();
 public:
  /**
   * Constructor. The processes vector<Processes> will be initialized using toProcessVector(string).
   *
   * @param string the shell input from the user
   */
  Input(std::string); 
  /**
   * Copy Constructor.
   *
   * @param const Input& the Input object from which the calling Input object will copy
   */
  Input(const Input &);
  /**
   * Sets the JID of the Input obj/job and the PGID of all Processes in the job with the given pid.
   *
   * @param pid_t the pid of the first Process in the job
   */
  void setJID(pid_t);
  /**
   * Sets the status of the current job for bookkeeping purposes. Can be either "Running" or "Stopped."
   *
   * @param const char* the status of the current job
   */
  void setStatus(const char *);
  /**
   * Gets the JID (the PGID) of the Input obj.
   *
   * @return the JID of the Input obj
   */
  const pid_t getJID() const { return JID; }
  /**
   * Gets the job status of the Input obj.
   *
   * @return the status of the Input obj/job
   */
  const char * getStatus() const { return status; }
  /**
   * Determines if job was placed in foreground or background.
   *
   * @return true if in foreground, false if not.
   */
  const bool isForeground() const { return foreground; } 
  /**
   * Gets the original, trimmed, shell input string.
   *
   * @return std::string& the shell input string
   */
  const std::string& getShellInput() const { return shellInput; } 
  /**
   * Gets the destination for the job's STDIN.
   *
   * @return std::string the STDIN destination
   */
  const std::string getSTDIN_fd() const { return fd_STDIN; } 
  /**
   * Gets the destination for the job's STDOUT.
   *
   * @return std::string the STDOUT destination
   */
  const std::string getSTDOUT_fd() const { return fd_STDOUT; } 
  /**
   * Gets the type of STDOUT output.
   *
   * @return std::string the type of output. Either ">" or ">>".
   */
  const std::string getSTDOUT_type() const { return type_STDOUT; } 
  /**
   * Gets the destination for the job's STDERR.
   *
   * @return std::string the STDERR destination
   */
  const std::string getSTDERR_fd() const { return fd_STDERR; } 
  /**
   * Gets the type of STDERR output.
   *
   * @return std::string the type of output. Either "e>" or "e>>".
   */
  const std::string getSTDERR_type() const { return type_STDERR; } 
  /**
   * Gets the total number of pipes in the job.
   *
   * @return int the number of pipes
   */
  const int getNumPipes() const; 
  /**
   * Gets the total number of processes in the job.
   *
   * @return int the number of processes
   */
  const int getNumProcesses() const;
  /**
   * Gets the Process vector.
   *
   * @return std::vector<Process>& the reference to a vector of Process structs
   */
  std::vector<Process>& getProcesses() { return processes; }  
  /**
   * Determines if all the processes in the job have stopped or completed yet.
   *
   * @return true if all Processes have stopped/completed. false if not.
   */
  const bool isStopped() const;
  /**
   * Determines if all of the processes in the job have completed yet.
   *
   * @return true if all Processes have completed. false if not.
   */
  const bool isCompleted() const;
  /**
   * Copy assignment operator overload. Copies values from one Input object into another without copy constructor. 
   *
   * @param std::string& the reference to the input string being copied over, and from which class members will be updated
   * @return a reference to the Input object pointed to by 'this'
   */
  Input& operator=(std::string);

}; // Input

// ___________________ Non-member operator overload prototypes _____________________ //

// ---- <<

/**
 * Stream insertion operator overload.
 *
 * @param output the ostream reference into which the Input object output will be directed
 * @param rhs the Input object reference whose data will be directed into the ostream
 * @return reference to the ostream containing the desired Input object output
 */
std::ostream& operator<<(std::ostream& output, Input& rhs);

// ___________________ Non-member helper methods _____________________ //

/**
 * Trims the input string of all leading and trailing spaces and tabs.
 *
 * @param string the shell input to be trimmed of whitespace
 */
std::string trim(std::string);

/**
 * Sanitizes the input string of any chars given in the charsToRemove string.
 *
 * @param input the shell input to be trimmed of whitespace
 * @param charsToRemove the list of chars to be removed from input
 * @return the sanitized input string
 */
std::string sanitize(std::string input, std::string charsToRemove);

/**
 * Determines if shell input has any unescaped double-quotes.
 *
 * @param string the shell input to be checked for quotes
 * @return true if the input has any quotes, false if not
 */
bool hasQuotes(std::string);

/**
 * Gets the pos of the first unescaped double-quote in input string. Guaranteed to be one.
 *
 * @param string the shell input to be searched
 * @return pos of the first quote
 */
unsigned int pos_of_first_quote(std::string);

/**
 * Processes the raw argv vector<string> obtained from the stringstream
 * of the shell input. Deals with the double-quotes, trims each argument,
 * and sanitizes the backslashes.
 *
 * @param std::vector<string> the unprocessed args from the stringstream
 * @return the processed vector<string> used to determine i/o destinations/populate the vector<Process>
 */
std::vector<std::string> processArgv(std::vector<std::string>);

#endif

