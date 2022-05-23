#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <string.h>
#include <ctype.h>

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <sstream>

#define LINE_BACK_SIZE 2048

using namespace std;

typedef struct struct_construct{
    string struct_name;
    vector<string> internal_structs;
    vector<string> var_names;
    vector<string> reg_names;
    vector<int>    reg_count;
    vector<int>    reg_width;
    bool used = 0;
    int declarations = 0;
    int regs = 0;
} struct_t;

/*typedef struct glob_list{

    string var_name;
    struct glob_list * next;
} list_t;*/

//trims white space
char * trimspaces(char * str);
ofstream to_write;
unordered_map<string, struct_t> struct_array;
string file_name;
void create_prefixes(struct_t struct_temp, string prefix);

int main(int argc, char * argv[]){

    char * header_file = NULL;
    FILE * header_ptr;
    bool file_flag = 0;
    int option = 0;

    while((option = getopt(argc, argv, "hi:")) != -1){

        switch(option){

            case 'h':
                printf("Welcome to the signal generation tool\nProvide a *CLEAN* ESSENT header and this will generate an output file called \"struct_regs.txt\" which contains all the struct variable names along with their prefixes\n");
                return 0;
                break;
            case 'i':
                header_file = optarg;
                file_flag = 1;
                break;
            case '?':
                if((optopt == 'i')) printf("You need to provide an ESSENT header file\n");
                return 1;
                break;
            default:
                printf("How'd you get here?\n");
                return 1;
                break;
        }
    }

    if(file_flag == 0){
        printf("You need an ESSENT header!\n");
        return 1;
    }

    //open files
    //open files
    if((header_ptr = fopen(header_file, "r")) == NULL){
        printf("Could not open header file %s\n", header_file);
        return 3;
    }
    file_name = header_file;
    to_write.open("struct_regs.txt");
    if(!to_write.is_open()){
        printf("Could not open struct_regs\n");
        return 2;
    }

    //set up variables
    char * line;
    size_t line_size = 0;   //indicate that getline should d-allocate space for line
    string cur_name;
    string cur_uint_name;
    string cur_prefix_name;
    string cur_struct_name;
    string count_isolate;
    string temp;
    char line_backup[LINE_BACK_SIZE];
    char line_backup_temp[LINE_BACK_SIZE];
    bool struct_flag = 0;
    char * token = NULL;
    size_t line_len = 0;    //length of line from getline()
    struct_t struct_temp;
    while((line_len = getline(&line, &line_size, header_ptr) != -1)){

        memset(line_backup, 0, LINE_BACK_SIZE);
        strcpy(line_backup, line); //don't want to rewrite code

        //loop_count = loop_count + 1;
        //printf("%d\n", loop_count);

        //ESCAPE CONDITIONS--------------------------------------------------------
        //if the name of the struct contains the file name
        //if(struct_count != 0) if(strstr(header_file, ) struct_flag = 1;
        //if we've passed the last struct and encountered a newline
        if((struct_flag == 1) && line[1] == 0) break;

        if((line[0] == '\n') || (line[0] == '#')) continue; //skip newline
        else if((token = strstr(line_backup, "struct")) != NULL){ //new struct
            //register struct in struct map
            token = strchr(token, ' ');
            trimspaces(token);
            token = strtok(token, " ");
            cur_name = token;
            struct_temp.used = false;
            struct_temp.declarations = 0;
            struct_temp.regs = 0;
            struct_temp.struct_name = cur_name;
            struct_temp.internal_structs.clear();
            struct_temp.var_names.clear();
            struct_temp.reg_count.clear();
            struct_temp.reg_names.clear();
            struct_temp.reg_width.clear();

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
                std::cout<<"Name: "<<cur_struct_name<<"\n";
                strcpy(line_backup, line_backup_temp);  //working with C strings is suffering
                //---------------------------------------------------------

                token = strtok(line_backup, ";");
                token = strrchr(token, ' ');
                trimspaces(token);
                cur_prefix_name = token;
                cout<<"Curr_Prefix_name: "<<cur_prefix_name<<"\n";

                struct_temp.internal_structs.push_back(cur_struct_name);
                struct_temp.var_names.push_back(cur_prefix_name);
                struct_array[cur_name] = struct_temp;
            }
        }
        //is a UInt
        else if(strchr(line_backup, '>') != NULL){

            //isolate struct name-------------------------------------------------------
            strcpy(line_backup_temp, line_backup);
            token = strtok(line_backup, ">");
            token = strchr(line_backup, '<');
            temp = token;
            temp.erase(0,1);
            struct_temp.reg_width.push_back(stoi(temp));
            strcpy(line_backup, line_backup_temp);  //working with C strings is suffering
            //---------------------------------------------------------


            //Isolate variable name
            token = strtok(line_backup, ";");
            token = strrchr(token, ' ');
            trimspaces(token);
            cur_uint_name = token;

            struct_temp.regs = struct_temp.regs + 1;
            struct_temp.reg_names.push_back(cur_uint_name);

            if(strchr(line_backup, '[') == NULL) struct_temp.reg_count.push_back(1);
            else{
                token = strchr(line_backup, '[');
                count_isolate = token;
                count_isolate.erase(0,1);
                struct_temp.reg_count.push_back(stoi(count_isolate));
            }

            struct_array[cur_name] = struct_temp;
        }
    }

    string empty_string;
    to_write << "18623009\n"; //magic key
    create_prefixes(struct_temp, empty_string);
	
	//globals
    /*list_t * head = NULL;
    head = (list_t *) calloc(1, sizeof(list_t));
    if(head == NULL){

        printf("Memory error! Could not allocate global variable list\n");
        return 1;
    }
    head->next = NULL;
    list_t * current = head;
	
	char * search0 = NULL;   //return for strstr
	char bracket_flag = 1;  //Are we inside a struct?
	
	rewind(header_ptr); //return to start of file
	
	while((line_len = getline(&line, &line_size, header_ptr) != -1)){
        memset(line_backup, 0, LINE_BACK_SIZE);
        strcpy(line_backup, line); //reading line is destructive
		
        if((search0 = strchr(line_backup, '{')) != NULL) bracket_flag = 1;
        else if((search0 = strchr(line_backup, '}')) != NULL) bracket_flag = 0;
		
        if((bracket_flag == 0) && (strchr(line_backup, '>'))){
            //strcpy(temp_typedef, line);
            //glob_flag = 1;
            break;
        }
	}
	
	to_write << "#Globals\n";
	
    while((line_len = getline(&line, &line_size, header_ptr) != -1)){

        memset(line_backup, 0, LINE_BACK_SIZE);
        strcpy(line_backup, line); //reading line is destructive

        if(strstr(line_backup, "void EVAL_") != NULL){

            //strcpy(temp_typedef, line);

            if (head == NULL)
                printf("\nHead is NULL");
            //generate global name function
            current = head;

		    while(current->next != NULL){

		        //printf("made it");
        
		        //search0 = trimspaces(current->var_name.c_str());
		        //fprintf(out_fp, "   %s.name_update_glob(\"%s\");\n", search0, search0);
		        current->var_name.erase(0,1);
				to_write << current->var_name.c_str() << "\n";
		        //fprintf(out_fp, "   %s.name_update_glob();\n", current->var_name.c_str());
		        current = current->next;
		    }
	
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

            //fprintf(out_fp, "%s", line);
        }
        //else fprintf(out_fp, "%s", line);
	}
	
	
*/    
	
    to_write.close();
    free(line);
    return 0;
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

void create_prefixes(struct_t struct_temp, string prefix){
    int count0 = 0;
    int count1 = 0;
    struct_t to_pass;
    string prefix_to_pass;
    string token;

    while(count0 != struct_temp.declarations){
        prefix_to_pass = prefix + struct_temp.var_names[count0];
        to_pass = struct_array[struct_temp.internal_structs[count0]];
        create_prefixes(to_pass, prefix_to_pass);
        count0 = count0 + 1;
    }
    count0 = 0;

    //get rid of spaces
    prefix.erase(0,1);
    while(count0 != prefix.size()){
        if(prefix[count0] == ' ') prefix[count0] = '.';
        count0 = count0 + 1;
    }
    count0 = 0;

    if(prefix.size() != 0) to_write << "#Struct Name: " << prefix << "\n";
    else to_write << "#Struct Name: " << file_name << "\n";
    while(count0 != struct_temp.regs){
        if(prefix.size() != 0){
            struct_temp.reg_names[count0].erase(0,1);
            to_write << prefix << "." << struct_temp.reg_names[count0] << "\n";
            to_write << struct_temp.reg_width[count0] << "\n" << struct_temp.reg_count[count0] << "\n";

        }
        else{
            struct_temp.reg_names[count0].erase(0,1);
            to_write << struct_temp.reg_names[count0] << "\n";
            to_write << struct_temp.reg_width[count0] << "\n" << struct_temp.reg_count[count0] << "\n";
        }
        count0 = count0 + 1;
    }
}
