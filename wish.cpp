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


/*
(use getline(), when hit EOF exit(0))
---------------------------
(getline (file descrip, 
        buffer to put line, 
        length limit of buff))
---------------------------
in_file_stream = input file stream
    stdin passed for standard in
    otherwise input file from fopen()
puts line read into line_read_buff
NNEED TO CHECK EEERRORS?
*/
int ReadCmd(FILE* in_file_stream, char** line_read_buff, size_t* buff_size) {
    //getline(char ** restrict linep, 
    //      size_t * restrict linecapp,
     //      FILE * restrict stream);
    // linep = buffer of line that was read
    // linepcapp = size of buffer in memory 
    //      both will get updated as needed
    getline(line_read_buff, buff_size, in_file_stream);
    cout<<"line "<<*line_read_buff<<endl;
    return 1;
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
    return write(STDOUT_FILENO, prompt_mssg.c_str(), prompt_mssg.length());
}

///////// BATCH /////////
/*
try to open file (doesn't check if successful)
Then associate stream with it
will return nullptr if couldn't associate stream
---------------
assumes only 1 file provided
*/
FILE* OpenFile(char* argv[]) {
    // cout<<argv[1]<<endl;

    //fdopen associates stream with opening
    //returns file* needed for getline()
    int FD = open(argv[1], O_RDONLY);
    //read mode = 'r'
    char mode = 'r';
    return fdopen(FD, &mode);
}



/*
dealloc buff?
*/
int main(int argc, char* argv[]) {

    char w_mode = GetMode(argc, argv);
    char *buff = (char*)malloc(10 * sizeof(char));
    size_t buff_size = 10*sizeof(char);
    // cout<<*buff<< " buff"<<endl;
    // cout<< w_mode<<" "<<buff_size<<endl;
    
    // cout<<mode<<endl;

    //interactive
    if (w_mode == 'I') {
        //ask prompt while exit is not typed
        AskPrompt();
        string line;
        ReadCmd(stdin, &buff, &buff_size);
        cout<<endl<<line<<endl;

    } else if (w_mode == 'B') {
        FILE* in_stream = OpenFile(argv);
        // cout<< in_stream <<endl;
        // int a =
         ReadCmd(in_stream, &buff, &buff_size);
        // cout<<" A "<<a<<endl; 
    }

    return 0;

}