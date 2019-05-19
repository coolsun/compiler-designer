// $Id: cppstrtok.cpp,v 1.3 2019-04-05 14:28:09-07 - - $

// Use cpp to scan a file and print line numbers.
// Print out each input line read in, then strtok it for
// tokens.

#include <string>
using namespace std;
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
//#include "string_set.h"
#include <iostream>
#include "debug.h"
#include "unistd.h"

#include "astree.h"
#include "auxlib.h"
//#include "emitter.h"
#include "lyutils.h"
#include "string_set.h"

// ssun24: 20190420181500 nostdinc error
// const string CPP = "/usr/bin/cpp -nostdinc";
const string CPP = "/usr/bin/cpp";
constexpr size_t LINESIZE = 1024;

// ssun24
// refer to tokFile at lytuils.cpp
extern FILE* tokFile;
FILE* astFile;
astree* yyparse_astree;

// Chomp the last character from a buffer if it is delim.
void chomp(char* string, char delim) {
  size_t len = strlen(string);
  if (len == 0)
    return;
  char* nlpos = string + len - 1;
  if (*nlpos == delim)
    *nlpos = '\0';
}

// Print the meaning of a signal.
static void eprint_signal(const char* kind, int signal) {
  fprintf(stderr, ", %s %d", kind, signal);
  const char* sigstr = strsignal(signal);
  if (sigstr != nullptr)
    fprintf(stderr, " %s", sigstr);
}

// Print the status returned from a subprocess.
/*
 void eprint_status (const char* command, int status) {
 if (status == 0) return; 
 fprintf (stderr, "%s: status 0x%04X", command, status);
 if (WIFEXITED (status)) {
 fprintf (stderr, ", exit %d", WEXITSTATUS (status));
 }
 if (WIFSIGNALED (status)) {
 eprint_signal ("Terminated", WTERMSIG (status));
 #ifdef WCOREDUMP
 if (WCOREDUMP (status)) fprintf (stderr, ", core dumped");
 #endif
 }
 if (WIFSTOPPED (status)) {
 eprint_signal ("Stopped", WSTOPSIG (status));
 }
 if (WIFCONTINUED (status)) {
 fprintf (stderr, ", Continued");
 }
 fprintf (stderr, "\n");
 }
 */

// Run cpp against the lines of the file.
void cpplines(FILE* pipe, const char* filename) {
  int linenr = 1;
  for (;;) {
    char buffer[LINESIZE];
    const char* fgets_rc = fgets(buffer, LINESIZE, pipe);
    if (fgets_rc == nullptr)
      break;
    chomp(buffer, '\n');
    //printf ("%s:line %d: [%s]\n", filename, linenr, buffer);
    // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
    char inputname[LINESIZE];
    int sscanf_rc = sscanf(buffer, "# %d \"%[^\"]\"", &linenr,
        inputname);
    if (sscanf_rc == 2) {
      //printf ("DIRECTIVE: line %d file \"%s\"\n", linenr, inputname);
      continue;
    }
    char* savepos = nullptr;
    char* bufptr = buffer;
    for (int tokenct = 1;; ++tokenct) {
      char* token = strtok_r(bufptr, " \t\n", &savepos);
      bufptr = nullptr;
      if (token == nullptr)
        break;
      /*
       printf ("token %d.%d: [%s]\n",
       linenr, tokenct, token);
       */
      const string* str1 = string_set::intern(token);
      /*
       printf ("intern (\"%s\") returned %p->\"%s\"\n",
       token, (void*)str1, str1->c_str());
       */
    }
    ++linenr;
  }
}

char *removeExt(char* mystr) {
  char *retstr;
  char *lastdot;
  if (mystr == NULL)
    return NULL;
  if ((retstr = (char *) malloc(strlen(mystr) + 1)) == NULL)
    return NULL;
  strcpy(retstr, mystr);
  lastdot = strrchr(retstr, '.');
  if (lastdot != NULL)
    *lastdot = '\0';
  return retstr;
}

string d_option = " ";
void scan_options(int argc, char** argv) {
  opterr = 0;
  yy_flex_debug = 0;
  yydebug = 0;
  int option = 0;

  while ((option = getopt(argc, argv, "yl:@:D:")) != -1) {
    switch (option) {
    case '@':
      set_debugflags(optarg);
      break;
    //case 'D':
    //  flag = true;
    //  str = optind - 1;
    //  break;
    case 'l':
      yy_flex_debug = 1;
      break;
    case 'y':
      yydebug = 1;
      break;
    case '?':
      errprintf("%: invalid option (%c)\n", optopt);
      break;
    default:
      fprintf(stderr, "invalid file\n");
      exit(1);
      break;
    }
  }

  if (optind < argc) {
    // fprintf(stderr, "operand not permitted!\n");
  }
}

// Open a pipe from the C preprocessor.
// Exit failure if can't.
// Assigns opened pipe to FILE* yyin.
/*
 void cpp_popen (const char* filename) {
 cpp_command = cpp_name + " " + filename;
 yyin = popen (cpp_command.c_str(), "r");
 if (yyin == nullptr) {
 syserrprintf (cpp_command.c_str());
 exit (exec::exit_status);
 }else {
 if (yy_flex_debug) {
 fprintf (stderr, "-- popen (%s), fileno(yyin) = %d\n",
 cpp_command.c_str(), fileno (yyin));
 }
 lexer::newfilename (cpp_command);
 }
 }
 */

/*
 void cpp_pclose() {
 int pclose_rc = pclose (yyin);
 eprint_status (cpp_command.c_str(), pclose_rc);
 if (pclose_rc != 0) exec::exit_status = EXIT_FAILURE;
 }
 */

FILE* open_file(char * path, string extension) {

  string file_name = path;
  file_name = file_name.substr(0, file_name.find_last_of('.'));
  file_name.append(extension);
  FILE* open_file = fopen(file_name.c_str(), "w");
  return open_file;
}

void close_file(FILE* file) {
  fclose(file);
}

int main(int argc, char** argv) {
  scan_options(argc, argv);
  exec::execname = basename(argv[0]);
  int exit_status = EXIT_SUCCESS;
  string cpp_command = "";
  for (int argi = 1; argi < argc; ++argi) {
    char* filename = argv[argi];
    if (filename[0] == '-') {
      continue;
    }
    cpp_command = CPP + " " + d_option + " " + filename;
    //DEBUGF ('d', "command=\"" << command.c_str()<< "\"");
    // asg1 .str
    FILE* pipe = popen(cpp_command.c_str(), "r");
    if (pipe == nullptr) {
      exit_status = EXIT_FAILURE;
      fprintf(stderr, "%s: %s: %s\n", exec::execname,
          cpp_command.c_str(), strerror(errno));
    } else {
      cpplines(pipe, filename);
      int pclose_rc = pclose(pipe);
      eprint_status(cpp_command.c_str(), pclose_rc);
      if (pclose_rc != 0)
        exit_status = EXIT_FAILURE;
    }
    
    // asg1 .str
    char file_str[256];

    sprintf(file_str, "%s.str", removeExt(basename(filename)));
    FILE* pipe_out = fopen(file_str, "w");
    string_set::dump(pipe_out);
    int poutclose_rc = fclose(pipe_out);

    // asg2 & 3 .tok .ast
    tokFile = open_file(filename, ".tok");
    astFile = open_file(filename, ".ast");


    yyin = popen(cpp_command.c_str(), "r");
    if (yyin == NULL) {
      syserrprintf(cpp_command.c_str());
    } else {
      int yy_rtn = yyparse();
      int pclose_rc = pclose(yyin);
      eprint_status(cpp_command.c_str(), pclose_rc);
    }
    fclose (tokFile);
    astree::print(astFile, parser::root);
    //astree::dump(astFile, astree.);
    fclose (astFile);
    delete(parser::root);
  }
  return 0;

}

