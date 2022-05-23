/***********************************************
 * Author: Alan Gardner
 * File name: header_parser.cpp
 * Build instructions: g++ header_parser.cpp -o <file name>
 * 
 * Parses an ESSENT header to prep it for fault injection
 ***********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>

#define PREFIX_SIZE 512
#define LINE_BACK_SIZE 2048
#define META_DATA_SIZE 64

using std::vector;
using std::string;
using std::unordered_map;

//Injects the faults into the header
void header_injection(FILE * header_ptr, FILE * out_fp, char * header_file);
//trims white space
char * trimspaces(char * str);

typedef struct struct_construct{
    string struct_name;
    vector<string> internal_structs;
    vector<string> var_names;
    bool used = 0;
    int declarations = 0;
} struct_t;

typedef struct struct_var{
    string struct_name;
    string inst_name;
    vector<int>indecies;
} struct_var_t;

typedef struct prefix_construct{
    string prefix;
    string struct_name;
    string inst_name;
    int declarations;
} prefix_t;

typedef struct glob_list{

    string var_name;
    struct glob_list * next;
} list_t;

char output_flag = 0;
char verbose_flag = 0;
bool skip_flag = 0;

vector<string> prefix_array;
unordered_map<string, struct_t> struct_array; //struct name, used/unused, number of declarations
void create_prefixes(struct_t struct_temp, string prefix);

int main(int argc, char * argv[]){

    int option = 0;
    int flags = 0;

    char * header_file = NULL;
    char * out_string = NULL;

    FILE * header_ptr;
    FILE * out_fp;

    option = argc;
    if(option == 1){
        printf("Missing arguments, call with -h for help\n");
        return 0;
    }

    while((option = getopt(argc, argv, "hvsf:o:i:")) != -1){

        switch(option){

            case 'h':
                printf("-i <ESSENT_header_file> input header file to inject faults into (required)\n-o output file (Parsed ESSENT header)\n-s skip non-global variable name generation\n-h help\n-v toggle verbose mode\n");
                printf("If no output file name is provided, one will be created under the name <fault_header.h>\n");
                printf("This program expects an UNMODIFIED ESSENT header, provide it with something else at your own risk\n");
                return 0;
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case 's':
                skip_flag = 1;
                break;
            case 'i':
                header_file = optarg;
                flags = 1;
                break;
            case 'o':
                out_string = optarg;
                output_flag = 1;
                break;
            case '?':
                if((optopt == 'i')) printf("You need to provide an ESSENT header file\n");
                else if((optopt == 'o')) printf("You don't need to specify an output file, but if you use the option specify one\n");
                else printf("Unknown option recieved\n");
                return 1;
                break;
            default:
                printf("How'd you get here?\n");
                return 1;
                break;
        }
    }

    if(flags == 0){
        printf("You need to provide a header\n");
        return 2;
    }

    //open files
    if((header_ptr = fopen(header_file, "r")) == NULL){
        printf("Could not open header file %s\n", header_file);
        return 3;
    }

    if(output_flag) out_fp = fopen(out_string, "w");
    else out_fp = fopen("fault_header.h", "w");

    header_injection(header_ptr, out_fp, header_file);

    if(verbose_flag) printf("Closing File Pointers and Returning...\n");

    //done with files
    fclose(header_ptr);
    fclose(out_fp);

    return 0;
}

void header_injection(FILE * header_ptr, FILE * out_fp, char * header_file){

    char * line = NULL;     //return of getline()
    char * token = NULL;    //return of strtok()
    size_t line_len = 0;    //length of line from getline()
    size_t line_size = 0;   //indicate that getline should d-allocate space for line
    char bracket_flag = 1;  //Are we inside a struct?
    char glob_flag = 0;     //does this header have globals?
    unsigned long count = 0;

    int struct_count = 0;   //how many structs are in the file
    int count0 = 0;
    int count1 = 0;
    char struct_flag = 0;   //when we're done checking the structs

    char temp_typedef[PREFIX_SIZE];  //temp buffer for if we need to write something before cur line

    char * search0 = NULL;   //return for strstr
    char * token0 = NULL;   //return for strtok if token is in use
    char name_and_update[LINE_BACK_SIZE];  //cat-ed name and call to name_update
    char line_backup[LINE_BACK_SIZE];  //reading "line" is destructive so we need a backup
    char line_backup_temp[LINE_BACK_SIZE]; //don't ask
    size_t tok_len = 0;     //return for strlen

    bool insertion_flag = 0;
    //struct faults fault;    //struct that holds details of current fault being processed

    char inst_count = 0;

    //struct data arrays
    prefix_t prefix_temp;
    struct_t struct_temp;

    string cur_name;
    string cur_prefix_name;
    string cur_struct_name;

    int dec_count = 0;
    int to_walk = 0;
    int place_to_increment = 0;

    //linked list of global variables to handle in function
    list_t * head = NULL;
    head = (list_t *) calloc(1, sizeof(list_t));
    if(head == NULL){

        printf("Memory error! Could not allocate global variable list\n");
        return;
    }
    head->next = NULL;
    list_t * current = head;
    //------------------------------------------------------

    //header_file = strrchr(header_file, '/');

    if(verbose_flag) printf("Processing structs...\n");

    if(verbose_flag) printf("Creating struct array and counting declarations\n");
    while((line_len = getline(&line, &line_size, header_ptr) != -1)){
        memset(line_backup, 0, LINE_BACK_SIZE);
        strcpy(line_backup, line); //reading line is destructive

        //ESCAPE CONDITIONS------------------------------------------------------------------------------------
        //if the name of the struct contains the file name
        //if(struct_count != 0) if(strstr(header_file, ) struct_flag = 1;
        //if we've passed the last struct and encountered a newline
        if((struct_flag == 1) && line_backup[1] == 0) break;

        if((line_backup[0] == '\n') || (line_backup[0] == '#')) continue; //skip newline
        else if((token = strstr(line_backup, "struct")) != NULL){ //new struct

            //register struct in struct map
            token = strchr(token, ' ');
            trimspaces(token);
            token = strtok(token, " ");
            cur_name = token;
            struct_temp.used = false;
            struct_temp.declarations = 0;
            struct_temp.struct_name = cur_name;
            struct_temp.internal_structs.clear();
            struct_temp.var_names.clear();

            struct_array[cur_name] = struct_temp;

            if(strstr(header_file, token)) struct_flag = 1;

        }
        //if it's a declaration (not UInt)
        else if(strchr(line_backup, '>') == NULL){
            //if it's a valid struct declaration
            if((!strchr(line_backup, '(') && !strchr(line_backup, '}')) != 0){
                struct_temp.declarations = struct_temp.declarations + 1;

                //isolate struct name-------------------------------------------------------
                strcpy(line_backup_temp, line_backup);
                token = strtok(line_backup, " ");
                trimspaces(token);
                cur_struct_name = token;
                strcpy(line_backup, line_backup_temp);  //working with C strings is suffering
                //---------------------------------------------------------------------------

                token = strtok(line_backup, ";");
                token = strrchr(token, ' ');
                trimspaces(token);
                cur_prefix_name = token;

                struct_temp.internal_structs.push_back(cur_struct_name);
                struct_temp.var_names.push_back(cur_prefix_name);
                struct_array[cur_name] = struct_temp;
            }
        }
    }

    struct_flag = 0;
    //struct_temp = struct_array["TestHarness"];
    int loop_count = 0;
    //rewind(header_ptr); //return to start of file
    string empty_string;
    create_prefixes(struct_temp, empty_string);

    place_to_increment = 0;

    //replace white space with .
    int i = 0;
    int j = 0;
    string buff;
    while(i<prefix_array.size()){
        prefix_array[i].erase(0,1); //erase leading space
        buff = prefix_array[i];
        while(buff[j] != 0){
            if(buff[j] == ' ') buff[j] = '.';
            j = j + 1;
        }
        prefix_array[i] = buff;
        j = 0;
        i = i + 1;
    }

    rewind(header_ptr); //return to start of file

    if(verbose_flag) printf("Writing struct data...\n");
    while((line_len = getline(&line, &line_size, header_ptr) != -1)){

        //check if we've passed the includes
        if((line[0] == '\n')) output_flag = 1; //passed the newline gap
        else if((line[0] != '#') && (output_flag)){
            strcpy(temp_typedef, line);
            break;
        }
        else fprintf(out_fp, "%s", line);
    }

    //Write out the stuff needed for the prefix map
    fprintf(out_fp, "//============================\n");
    fprintf(out_fp, "#include <string.h>\n");
    fprintf(out_fp, "#include <map>\n");
    fprintf(out_fp, "using std::map;\nusing std::string;\n");
    fprintf(out_fp, "string file_name = \"%s\";\n", header_file);
    fprintf(out_fp, "char prefix_count = 0;\n");
    fprintf(out_fp, "map<int, string> prefix_table{\n");

    i = 0;

    while(i <= prefix_array.size()){
        fprintf(out_fp, "{%d, \"%s\"},\n", (i+1), prefix_array[i].c_str());
        i = i + 1;
    }
    fprintf(out_fp, "};\n");
    fprintf(out_fp, "\n%s", temp_typedef);

    i = 0;
    j = 0;

    //fclose(out_fp);

    if(verbose_flag) printf("Checking for global variables...\n");
    while((line_len = getline(&line, &line_size, header_ptr) != -1)){

        memset(line_backup, 0, LINE_BACK_SIZE);
        strcpy(line_backup, line); //reading line is destructive

        //update the prefix we're pointing at
        if(bracket_flag && strchr(line_backup, '{') && !j){
            fprintf(out_fp, "%s", line_backup);
            fprintf(out_fp, "prefix_count++;\ncur_prefix = prefix_table[prefix_count];\n");
            i = i + 1;
            j = 1; //prevent for loops from double reporting
            continue;
        }
        else if(j && !bracket_flag) j = 0;

        if((search0 = strchr(line_backup, '{')) != NULL) bracket_flag = 1;
        else if((search0 = strchr(line_backup, '}')) != NULL) bracket_flag = 0;


        if((bracket_flag == 0) && (strchr(line_backup, '>'))){
            if(verbose_flag) printf("Found Global Variables!\n");
            strcpy(temp_typedef, line);
            glob_flag = 1;
            break;
        }

        if(strstr(line_backup, "void EVAL_") != NULL){
            if(verbose_flag) printf("No Globals Found\n");
            strcpy(temp_typedef, line);
            break;
        }
        else if((search0 = strstr(line_backup, "for")) != NULL){

            token = strtok(line_backup, ")");
            fprintf(out_fp, "%s){\n", token);
            token = strtok(NULL, ";");
            fprintf(out_fp, "%s;\n", token);
            strtok(token, ".");
            token = trimspaces(token);
            std::string str(token);
            if(str.find('=') != std::string::npos) {
                token = strtok(token, "=");
                token = strtok(NULL, "=");
            }
            fprintf(out_fp, "%s.name_update_num();}\n", token);
        }
        else if((search0 = strstr(line_backup, "rand_init")) != NULL){

            fprintf(out_fp, "%s", line_backup);
            token = strtok(line_backup, ".");
            token = trimspaces(token);
            fprintf(out_fp, "%s.name_update_prefix();\n", token);
        }

        else fprintf(out_fp, "%s", line);
    }
    fprintf(out_fp, "\n%s", temp_typedef);

    if(glob_flag){

        token = strtok(temp_typedef, ">");
        token = strtok(NULL, ">");
        token[strlen(token) - 2] = 0;

        head->var_name = token;

        while((line_len = getline(&line, &line_size, header_ptr) != -1)){

            memset(line_backup, 0, LINE_BACK_SIZE);
            strcpy(line_backup, line); //reading line is destructive

            if(strstr(line_backup, "void EVAL_") != NULL){

                strcpy(temp_typedef, line);

                //generate global name function
                current = head;

                fprintf(out_fp, "char global_init(){\n");
                while(current->next != NULL){

                    //printf("made it");
                    
                    //search0 = trimspaces(current->var_name.c_str());
                    //fprintf(out_fp, "   %s.name_update_glob(\"%s\");\n", search0, search0);
                    current->var_name.erase(0,1);
                    fprintf(out_fp, "   %s.name_update_glob();\n", current->var_name.c_str());
                    current = current->next;
                }
                fprintf(out_fp, "   return 1;\n}\n");
                current = head;
                while(current->next != NULL){

                    current = current->next;
                    free(head);
                    head = current;
                }
                free(head);
                break;
            }
            else if(strchr(line_backup, '>')){

                current->next = (list_t *) calloc(1, sizeof(list_t));
                current = current->next;
                current->next = NULL;

                token = strtok(line_backup, ">");
                token = strtok(NULL, ">");
                token[strlen(token) - 2] = 0;
                current->var_name = token;

                fprintf(out_fp, "%s", line);
            }
            else fprintf(out_fp, "%s", line);
        }
    }
    fprintf(out_fp, "\n%s", temp_typedef);


    if(verbose_flag) printf("Generating Variable Names...\n");

    if(!skip_flag){
        //Generate the rest of the variable names
        //Originally this section would've also generated the calls to
        //fault_injection(); however, this is now done at run time by
        //the ESSENT component
        while((line_len = getline(&line, &line_size, header_ptr) != -1)){
            
            if(strlen(line) > LINE_BACK_SIZE){
                std::cout << strlen(line) << "\n";
                std::cout << line;
                printf("Stack ERROR\n");
                return;
            }
            
            memset(line_backup, 0, LINE_BACK_SIZE);
            strcpy(line_backup, line); //reading line is destructive
            token0 = strtok(line_backup, "="); //make sure we're before the assignment operator
            search0 = strchr(token0, '>'); //check for >
            
            
            //look for the string "l(b" from "... eval(bool ..."
            if(strstr(line, "l(b") != NULL){
                fprintf(out_fp, "%s", line_backup);
                fprintf(out_fp, "//--------\n");
                fprintf(out_fp, "       if(!glob_init_flag){ glob_init_flag = global_init();\n");
                fprintf(out_fp, "       load_faults();}\n");
                fprintf(out_fp, "       check_faults();\n");
                fprintf(out_fp, "       update_cycle();\n");
                break;
            }
            else if(search0 != NULL){
                fprintf(out_fp, "%s", line);
                
                search0[0] = ' ';
                token0 = strchr(search0, '=');
                
                strcpy(line_backup_temp, line_backup);
                
                if((strstr(line_backup, "UInt<")!=NULL) && (strstr(line_backup_temp, ">(0x")==NULL)){
                    tok_len = strlen(search0);
                    if(search0[tok_len - 2] == ';') search0[tok_len - 2] = 0; //if there's a newline, squash it
                    strncpy(name_and_update, search0, (tok_len - 1));
                    std::string s = std::to_string(count);
                    char* c = const_cast<char*>(s.c_str());
                    strcat(name_and_update, ".name_update(");
                    count++;
                    //if(name_and_update[1] != '(') fprintf(out_fp, "%s\n", name_and_update);
                    search0 = trimspaces(search0);
                    strcat(name_and_update, c);
                    strcat(name_and_update, ");");
                    search0 = trimspaces(name_and_update);
                    if(name_and_update[1] != '(') fprintf(out_fp, "%s\n", search0);
                    memset(name_and_update, 0, LINE_BACK_SIZE);
                }
                memset(line_backup_temp, 0, LINE_BACK_SIZE);
            }
            else fputs(line, out_fp);
        }
    }

    if(verbose_flag) printf("Finishing up...\n");

    //finish up the file
    while((line_len = getline(&line, &line_size, header_ptr) != -1)){

        fprintf(out_fp, "%s", line);
        
        if(skip_flag){
            //look for the string "l(b" from "... eval(bool ..."
            if(strstr(line, "l(b") != NULL){
                //fprintf(out_fp, "%s", line_backup);
                fprintf(out_fp, "//--------\n");
                fprintf(out_fp, "       if(!glob_init_flag){ glob_init_flag = global_init();\n");
                fprintf(out_fp, "       load_faults();}\n");
                fprintf(out_fp, "       check_faults();\n");
                fprintf(out_fp, "       update_cycle();\n");
            }
        }
    }

    free(line);
//    free(prefix_array);
}

void create_prefixes(struct_t struct_temp, string prefix){
    int count0 = 0;
    struct_t to_pass;
    string prefix_to_pass;

    while(count0 != struct_temp.declarations){
        prefix_to_pass = prefix + struct_temp.var_names[count0];
        to_pass = struct_array[struct_temp.internal_structs[count0]];
        create_prefixes(to_pass, prefix_to_pass);
        count0 = count0 + 1;
    }

    prefix_array.push_back(prefix);
}

//TAKEN FROM STACKOVERFLOW USER ADAM ROSENFIELD
//EDITED BY USER DAVE GRAY
char * trimspaces(char * str){

    if(str == NULL){
        return NULL;
    }

    char * end;

    while(isspace((unsigned char)*str)) str++;

    if(*str == 0) return str; //all spaces

    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';

    return str;

}
