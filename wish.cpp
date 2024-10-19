/*
Issue with B mode; fork is trying command too many times
trying for parent class???*/


#include <stdio.h>
#include <iostream>
// use fstream for getline()
#include <fstream>
#include <sstream>
//use. for strcmp()
#include<string.h>
//use for vector of strings for lines GetWords()
#include <vector>
//use for find() with vector of strings

#include <algorithm>


#include <fcntl.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sys/wait.h> // for wait()

using namespace std;

void PrintError() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}


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
returns num char written or -1 if hit EOF
NEED TO CHECK ERRORS?
*/
int ReadLine(FILE* in_file_stream, char** line_read_buff, size_t* 
buff_size) {
    return getline(line_read_buff, buff_size, in_file_stream);
    
    // cout<<"line "<<*line_read_buff<<endl;
}


/*
takes line read in as char array 'buff', 
and its length
converts it to a vector of strings and returns
the vector
----------------
no error handling
*/
vector<string> GetWords(char* buff, int buff_length) {
    string curr_word = "";
    vector<string> words;

    //loop through each character in the line
    for(int i=0; i<buff_length; i++) {

        if( ((buff[i] == ' ') || (buff[i] == '\n')) && 
(curr_word.length() > 0) ) {
            words.push_back(curr_word);
            curr_word = "";

        } else if(buff[i] != ' '){
            curr_word+=buff[i];
        }
    }
    return words;
}



/*
seearch for word in vector of strings
returnn index if found
reeturn-1 if not found
-----------------
can also change to return the 
vector<string>::iterator that points
to the found instance
-----------------
no error handling
*/
int SearchWord(string sw, vector<string> words) {
    vector<string>::iterator found = find(words.begin(), 
words.end(), sw);
    if(found != words.end()) {
        int ind = found-words.begin();
        return ind;  
    } 
    return -1;
}

/*
searches cmd vector words for the sc char
if one of the words has sc in it, splits the word
into source/first half, sc as a word, target/second half
and updates params to have these values
----------
returns -1 if not found, 
returns index of the word with the char if found
works with CharSplit to replace the word with <source, sc, target>
*/
int SearchChar(char sc, vector<string> words, string* first_half, 
string* sec_half) {
    int found = -1;

    string sc_word = "";
    sc_word+=sc;

    *first_half = "";
    *sec_half = "";

    for(int i=0; i< (int)words.size(); i++ ){

        if(((*first_half).length()>0) | ((*sec_half).length()>0)) {
            break;
        }

        string curr_w = words[i];

        string word_so_far = "";
        string remain_word = curr_w;
        for(int x=0; x<(int)curr_w.length(); x++) {
            char curr_c = curr_w[x];
            remain_word.erase(0, 1);

            if(curr_c == sc) {
                found = i;
                *first_half = word_so_far;
                *sec_half = remain_word;
                break;

            } else {
                word_so_far+=curr_c;
            }
        }
    }

    return found;
}

/*
replaces a word in cmd with split up word
source, char, target
at an index with the word that contains the char*/
void CharSplit(vector<string>* cmd, int ind, string source, string 
target, string sc_word) {
    vector<string> copy;
    for(int i=0; i<ind; i++) {
        copy.push_back((*cmd)[i]);
    }
    if(source.length()>0) {
        copy.push_back(source);
    }

    copy.push_back(sc_word);

    if(target.length()>0) {
        copy.push_back(target);
    }
    
    for(int i=ind+1; i<(int)(*cmd).size(); i++) {
        copy.push_back((*cmd)[i]);
    }
    *cmd = copy;
}

/*
Redirection = use dup2
takes in command
gets file we want to redirect to
changes STDOUT to be that file
closes original file descriptor
------------
returns file descrip if successfully redirected
returns -2 if there was an error
returns -1 if there wasn't a ">" found in the cmd
*/
int Redirect(vector<string>* curr_cmd, int still_running) {
    string source = "";
    string target = "";

    int foundC = SearchChar('>', *curr_cmd, &source, &target);
    if(foundC>=0) {
        CharSplit(curr_cmd, foundC, source, target, ">");
    }

    // vector<string>::iterator it2;
    // cout<<"cmd vec2::";
    // for(it2 = (*curr_cmd).begin(); it2 != (*curr_cmd).end(); 
it2++) {
    //     cout<<" "<<*it2;
    // }
    // cout<<endl<<endl;

    int found = SearchWord(">", *curr_cmd);
    // cout<<"found "<<found<<endl;

    //no source
    if(found == 0) {
        return -2;
    } else if(found > 0) {
        
        // value after carrot = file we want to redirect to
        // clear out redirection from actual command
        vector<string> output_fname;
        
        for(int i=found+1; i< (int)(*curr_cmd).size(); i++) {
            output_fname.push_back( (*curr_cmd)[i]);
            // cout<<"curr "<< (*curr_cmd)[i]<<endl;
            // (*curr_cmd).erase((*curr_cmd).begin() + (i-1));
        } 
        //get rid of the redirection from the cmd
        (*curr_cmd).erase((*curr_cmd).begin() + found, 
(*curr_cmd).begin() + (*curr_cmd).size());

        if( (output_fname.size()!=1) | (SearchWord(">", 
output_fname)>=0) ) {
            return -2;
        }
        
        
        const char* o_fname = output_fname[0].c_str();

        //try to open output file, won't work if no file specified
        //if still in program, append to end; otherwise truncate
        int output_fd = 0;
        if(still_running == 1) {
            // cout<<"open and append"<<endl;
            output_fd = open(o_fname, O_WRONLY | O_CREAT | 
O_APPEND);
            // cout<<"run 1"<<endl;
        } else {
            // cout<<"open new "<<endl;
            output_fd = open(o_fname, O_WRONLY | O_CREAT | 
O_TRUNC);
            // cout<<"run 2: "<<output_fd<<endl;
        }
        
        if(output_fd<0) {
            return -2;
        }
        // try to replace STDOUT with the output file
        int stdout_fd = STDOUT_FILENO;
        dup2(STDOUT_FILENO, stdout_fd);
        dup2(output_fd, STDOUT_FILENO);
        //now use STDOUT_FILENO to mean the new output
        // cout<<"2: "<<output_fd<<" "<<STDOUT_FILENO<<endl;
        close(output_fd);
        
        return stdout_fd;
    } else {
        return -1;
    }
    return -2;
}

/*
takes in 'words' = line with ? # of cmds
    2d array pointer that stores all the cmds 
returns number of commands 
updates all_cmds to have 2d vec of each cmd
*/
int NumCmds(vector<string> words, vector<vector<string>>* all_cmds) 
{
    //remaining = part of 'words' we haven't taken cmds from yet
    // cout<<"words::"<<endl;
    // for(string i:words){
    //     cout<<i<<" ";
    // }
    // cout<<endl<<endl;

    vector<string> remaining = words;
    vector<string> curr_cmd;
    int num_cmds = 1;
    (*all_cmds).clear();
    
    //ind = index of first appearance of &
    int ind = SearchWord("&", remaining);

    while(ind>0) {
        num_cmds+=1;
        vector<string> new_remain;
        curr_cmd.clear();
        
        //take in the cmd before the &
        for(int i=0; i<ind; i++) {
            curr_cmd.push_back(remaining[i]);
        }
        (*all_cmds).push_back(curr_cmd);

        //r_ind = index of first char after &
        //(start of the next cmd, excludes &)
        int r_ind = ind+1;
        
        //new_rem = the part of remaining that we didn't touch yet
        //will become remaining for the next loop
        while(r_ind < (int)remaining.size()) {
            new_remain.push_back(remaining[r_ind]);
            r_ind+=1;
        }
    
        ind = SearchWord("&", new_remain);
        remaining = new_remain;

    }

    curr_cmd.clear();
    for(int i=0; i<(int)remaining.size(); i++) {
        curr_cmd.push_back(remaining[i]);
    }
    (*all_cmds).push_back(curr_cmd);

// // TO PRINT 2d ARRAY!!!!!!!!
//     // Iterator for the 2-D vector
// 	    vector<vector<string>>::iterator it1;

// 	// Iterator for each vector inside the 2-D vector
// 	    vector<string>::iterator it2;

// 	// Traversing a 2-D vector using iterators
//     cout<<"NM. cmds ";
// 	    for(it1 = (*all_cmds).begin(); it1 != 
(*all_cmds).end(); it1++){
//             for(it2 = it1->begin();it2 != it1->end();it2++) {
//                 cout<<*it2<<" ";
//             }
//             cout<<endl;
// 	    }	

 
    return num_cmds;
}




char Exit(vector<string> vec, int* EX) {
   
    int found = SearchWord("exit", vec);
    if (found>=0) {
        if (vec.size()==1)  {
            // cout<<"exit!!"<<endl;
            *EX = -1;
            
            exit(0);
        } 
        else {
            return 'e';
        }
    } 
    return 'n';
    // exit(0);
}


char CD(vector<string> vec) {
    
    int found = SearchWord("cd", vec);
    if(found>=0) {
        if(vec.size() == 2) {
            char path[vec[1].length()];
            strcpy(path, vec[1].c_str());

            int CD = chdir(path);

            if(CD < 0) {
                return 'e';
            } else {
                // cout<<"success"<<endl;
                char cwd[4000];
                if (getcwd(cwd, 4000) != NULL) {
                    // cout<<"Current working directory:  "<< 
cwd<<endl;
                }
                return 'a';
            }
        } else {
            return 'e';
        }
    }
    return 'n';
}


char Path(vector<string> vec, vector<string>* paths) {
    int found = SearchWord("path", vec);
    if(found>=0) {
        
        (*paths).clear();
        
        //assumes vec[i=0] is the command "path"
        for(int i=1; i<(int)vec.size(); i++) {
            (*paths).push_back(vec[i]);
        }
        // cout<<" path : "<< (*paths)[0]<<endl;
        // for(string i:(*paths)) {
        //     // cout<< "path "<<i << endl;
        // }
        return 'a';
        
    } 
    return 'n';
    
    // return 'e';
    
}

char BuiltInCmd(vector<string> vec, int* EX, vector<string>* paths) 
{

    char cd =  CD(vec);
    char pa = Path(vec, paths);
    char ee = Exit(vec, EX);

    if((cd == 'e') | (ee == 'e')) {
        return 'e';
    } 
    
    if((cd == 'a') | (pa == 'a')) {
        return 'a';
    }
    return 'n';
}


/*
code borrowed from class
cmd_vec = single cmd we want to execute
-----------------
uses execvp()
either returns 0 or execvp worked
*/
int Execute(char* file_name, vector<string> cmd_vec) {
   
    int argv_size =((int)cmd_vec.size()+1);
    //argv = null terminated cmd as char* arr
    char* argv[argv_size];  

    // cout<<endl<<"execvp arg in: ";
    for(int i=0; i<(int)cmd_vec.size(); i++) {
        argv[i] = (char*) cmd_vec[i].c_str();
        // cout<<" "<<argv[i]<<endl;
    } 
    // cout<<endl<<endl;
    // cout<<"argv[0] "<<argv[0]<<endl;

    argv[cmd_vec.size()] = NULL;
    //execvp for using file name = argv[0] (ls)
    execvp(file_name, argv);
    
    return 0;
}

/*
Tries non built in commands (everything not defined above)
*/
char TryCmd(vector<string> cmd_vec, vector<string> paths) {
    // cout<<"trying "<<endl;
    string cmd = cmd_vec[0];
    // cout<<"path: "<<paths[0]<<endl;

    for(int i=0; i<(int)paths.size(); i++) {
        string total_path = paths[i];
        total_path += "/";
        total_path += cmd;
        // cout<<"total path "<<total_path<<endl;

        //file_name includes the path to the file
        //formatting for access
        char file_name[total_path.length()];
        strcpy(file_name, total_path.c_str());
       
        //X_OK means it's executable
        int works =  access(file_name, X_OK);

        if(works == 0) {
            // cout<<"access worked "<<endl;
            int succ = Execute(file_name, cmd_vec);
            if(succ == 0) {
                return 'a';
            }
            // execv()
        } else {
            // cout<<"oth cmd didn't work"<<endl;
        }
    }
    return 'e';
}




/*
try using select?
can't usee fork for this if nnonne of themcann write to parent 
values
*/
char ForkCmds(int num_cmd, vector<vector<string>> all_cmds, int* 
EX, vector<string>* path) {
    vector<pid_t> PIDs;


    for(int i=0; i<num_cmd; i++) {
        vector<string> curr_cmd = all_cmds[i];

        char bi = BuiltInCmd(curr_cmd, EX, path);
        // cout<<"exit. val "<<*EX<<endl;
        if(bi== 'e') {
            return 'e';
        } else if(bi == 'a') {
            return 'b';
        }

        int curr_pid = fork();
        // cout<<"after fork "<<endl;
        
        if(curr_pid < 0) {
            // cout<<"ERROR"<<endl;
            return 'e';

        //child
        } else if(curr_pid == 0) {
            // cout<<"child"<<endl;
            
            // if(BuiltInCmd(curr_cmd, EX, path) == 'e') {
            //     PrintError();
            // }
            //run commands
            char tc = TryCmd(curr_cmd, *path); 
            if(tc == 'e') {
                PrintError();
                exit(0);

                // return 'e';
            }
            // cout<< "trieed cmd "<<tc<<endl;
            
            // cout<<"child curr ";
            // for(string i:curr_cmd) {
            //     cout<<" "<<i;
            // }
            // cout<<endl;

        //parent
        } else {
            // cout<<"parent "<<endl<<endl;
            PIDs.push_back(curr_pid);
            // cout<<"pid size "<<(int)PIDs.size()<<endl;
        }
        // cout<<"finnal PID sizee "<<(int)PIDs.size()<<endl;
        // cout<<"very end"<<endl;
        for(int idx = 0; idx < (int)PIDs.size(); idx++) {
            int status;
            // cout<<"start waiting"<<endl;
            waitpid(PIDs[idx], &status, 0);
            // cout<<"done waiting"<<endl;
            // if (WIFEXITED(status)) {
            //     int childReturnValue = WEXITSTATUS(status);
            //     // cout<<"child return 
"<<childReturnValue<<endl;

            //     if(childReturnValue !=0) {
            //         *EX = -1;
            //     }
            // }
            
        }
    
    }
    return 'a';
    
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
    int fd = open(argv[1], O_RDONLY);
    //read mode = 'r'
    char mode = 'r';
    return fdopen(fd, &mode);
}



/*
MAIN--------------------------
*/
int main(int argc, char* argv[]) {

    char w_mode = GetMode(argc, argv);
    // int output_FD = STDOUT_FILENO;
    //ex = exit
    int ex = 0;
    vector<string> paths;
    paths.push_back("/bin");

    
    char *buffer;
    //buff_size = character count of buffer
    size_t buffer_size;
    vector<vector<string>> all_cmds;
    int num_cmds = 0;
    vector<string> words_in_line;

    //these variables are used to make prompt show up
    //in stdin, and to append to end of file if still running
    int orig_fd = STDOUT_FILENO;
    int still_running = 0;

    //interactive
    if (w_mode == 'I') {
        while((ex == 0) ) {
            
            char *buff = (char*)malloc(1 * sizeof(char));
            //buff_size = character count of buffer
            size_t buff_size = 1*sizeof(char);

            dup2(orig_fd, STDOUT_FILENO);

            //ask prompt while exit is not typed
            if(AskPrompt() < 0) {
                PrintError();
            }

            // string line;
            ReadLine(stdin, &buff, &buff_size);
            
            buffer = buff;
            buffer_size = buff_size;

            words_in_line = GetWords(buffer, (int)buffer_size); 

            /*
            orig_fd should always point to original output 
(terminal)
            */
            int orig_fd = Redirect(&words_in_line, still_running);
            // cout<<"orig fd :"<<orig_fd<<endl;
            if(orig_fd == -2) {
                PrintError();
                exit(0);
            }
            //if redirection worked to a new file, append to the 
end
            if(orig_fd >= 0) {
                still_running = 1;
            }

            // for(string i:words_in_line) {
            //     cout<< i << endl;
            // }

            num_cmds = NumCmds(words_in_line, &all_cmds);
            char fc = ForkCmds(num_cmds, all_cmds, &ex, &paths);
            if(fc == 'e') {
                PrintError();
            } 

            //clean up for the next loop
            all_cmds.clear();
            words_in_line.clear();
            free(buff);
            buff = NULL;
            buff_size = 0;

            
        }
        if(ex == -1) {

            exit(0);
        }

    } else if (w_mode == 'B') {
        // cout<<"B mode"<<endl;
        int has_next_read = 1; 

        FILE* in_stream = OpenFile(argv);
        if(in_stream == NULL) {
            PrintError();
            has_next_read = -1;
        }
        
        while( has_next_read > 0) {

            // cout<<"in loop"<<endl;
            char *buff = (char*)malloc(1 * sizeof(char));
            //buff_size = character count of buffer
            size_t buff_size = 1*sizeof(char);

            has_next_read = ReadLine(in_stream, &buff, &buff_size);
            // cout<<"next raed "<<has_next_read<<endl;

            if(has_next_read>0) {
                buffer = buff;
                buffer_size = buff_size;

                // cout<<endl<<"buff vals: "<<endl;
                // for(int i=0; i< (int)buffer_size;  i++) {
                //     cout<<" "<<buffer[i];
                // }
                // cout<<endl<<endl;

                words_in_line = GetWords(buffer, (int)buffer_size);

                int new_fd = Redirect(&words_in_line, 
still_running);
                if(new_fd == -2) {
                    PrintError();
                    exit(0);
                }
                //if redirection worked to a new file, append to 
the end
                if(new_fd >= 0) {
                    still_running = 1;
                }
                // cout<<endl;
                // for(string i:words_in_line) {
                //     cout<<i<<" ";
                // }
                // cout<<endl;
                


                num_cmds=NumCmds(words_in_line, &all_cmds);
                char fc = ForkCmds(num_cmds, all_cmds, &ex, 
&paths);
                if(  fc == 'e') {
                    PrintError();
                } 
                
                
                
            }
            

            all_cmds.clear();
            words_in_line.clear();
            free(buff);
            buff = NULL;
            buff_size = 0;
            buffer = NULL;
            buffer_size = 0;
            // cout<<"end of loop"<<endl;
        }


    } else {
        //runnning the program didn't work with either mode = error
        PrintError();
    }

    return 0;

}for(int 
idx 
= 0; 
idx < (int)PIDs.size(); idx++) {
            //     cout<<"waiting "<<endl;
            //     waitpid(PIDs[idx], NULL, 0);
            //     PIDs.erase(PIDs.begin() + idx);
            //     cout<<"donee wait"<<endl;
            // }
