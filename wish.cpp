#include <stdio.h>
#include <iostream>
// use fstream for getline()
#include <fstream>
#include <sstream>

#include <fcntl.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

using namespace std;

/*
takes in argc and argv to determine the mode
returns modes as char 
I = interactive
B = batch
e = error
------------------------
only cares about arg count being 1 or 2
doesn't check file specified for batch
 */
char GetMode(int arg_c, char* arg_v[]) {
    char mode;
    // no arguments 
    // = interactive
    if (arg_c == 1) {
        mode = 'I';

    // 1 file specified 
    // (supposed to have commands written in it)
    // =batch mode
    } else if(arg_c == 2) {
        mode = 'B';

    //otherwise >2 arguments 
    //doesn't correspond to a mode
    } else {
        mode = 'e';
    }
    return mode;
}


///////// INTERACTIVE /////////
/*
writes prompt on std out 
same returns as write
- -1 if write unsuccessful
- #bites read if success
*/
int AskPrompt() {
    string prompt_mssg = "wish> ";
    return write(STDOUT_FILENO, prompt_mssg.c_str(), 
prompt_mssg.length());
}

/*
(use getline(), when hit EOF exit(0))
(getline (file descrip, 
        buffer to put line, 
        length limit of buff))
FD = file dscriptor
- stdin_fileno for interactive
- provided from OpenFile() for batch
*/
int ReadCmd_I(string* line_read) {
    getline(cin, *line_read);
    return 1;
}

int ReadCmd_B(ifstream in_file_stream, string* line_read) {
    getline(in_file_stream, *line_read);
    return 1;
}

///////// BATCH /////////
/*
try to open file 
same returns as open()
- success = file descriptor (int)
- error = -1
---------------
assumes only 1 file provided
*/
int OpenFile(char* argv[]) {
    // cout<<argv[1]<<endl;
    return open(argv[1], O_RDONLY);
}







int main(int argc, char* argv[]) {

    char mode = GetMode(argc, argv);
    // cout<<mode<<endl;

    //interactive
    if (mode == 'I') {
        //ask prompt while exit is not typed
        AskPrompt();
        string line;
        ReadCmd_I(&line);
        cout<<endl<<line<<endl;

    } else if (mode == 'B') {
        OpenFile(argv);
        // cout<<OpenFile(argv)<<endl;
    }

    return 0;

}
