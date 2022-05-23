#ifndef UINT_H_
#define UINT_H_

#include <algorithm>
#include <array>
#include <cinttypes>
#include <iomanip>
#include <iostream>
#include <random>
#include <bitset>
#include <type_traits>
#include <string.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <stdlib.h>
#include <climits>


#define TWO_ARG_CHECKS() {result.meta.faulty_new=this->meta.faulty_new|other.meta.faulty_new;\
    if(result.meta.faulty_new){if(this->meta.faulty_new){\
    result.meta.memetics=this->meta.memetics;result.meta.genetics=this->meta.index;}\
    else{result.meta.memetics=other.meta.memetics;\
    result.meta.genetics=other.meta.index;}}}

std::unordered_map<int,long> callcount; // activity profiling
std::vector<unsigned long> fr; // vector of fault register indices
//for these maps, key is an index in fr^
std::map<unsigned long,std::vector<long int>> writes; // live range profiling
std::map<unsigned long, int> idx_width;

int profile_flag = 0;
//extern std::vector<int> wv;
//extern std::vector<int> g1v;
int w = 0; // These two flags are used in load_faults()
int g1 = 0;
int w2 = 0;
int g2 = 0;

#define mkstr(s) #s
typedef uint64_t Cycle_t;

using std::vector;
using std::map;
using std::string;
using namespace std;

long int cycle = 0; //to be replaced with SST::now
unsigned long index_count = 0;
unsigned long store_last_glob_index = 0;
long temp_index = -2;
int idx_flag = 0;

std::fstream golden;
int last_cycle = 0;

unsigned int chksum = 0;
bool chksum_flag = 0;


vector<unsigned long> local_index;  

//vector<int> u0; //index
//vector<string> u0;
//map<int, string> u1; //index, name
//map<int, int> idunsigned long idx, int start_cycle, int end_cycle, int widthx_map; //subs_index, old_index
//map<int, int>    u2; //index, faulty_old
//vector<void*>  u3; //index, void pointer (to be cast to met_t*)
vector<long int> perm_bits;
char perm_op;

bool first_var = 0;
map<int,pair<void*, void*>> to_fault_map;
map<int,void*> u3;

typedef struct faults{
    //char location[64];
    string var_name;
    int line_num;
    int cycle;
    int end_cycle;
    int perm;
    int width;
    long unsigned int * value;
    unsigned long index;
    char op[2];
}fault_t;


vector<fault_t> fault_list_inject;
string fault_file;
string fault_file_new;
string cur_prefix;

char glob_init_flag = 0;

void load_faults(unsigned long idx, int start_cycle = -1, int end_cycle = -1, int bit_pos = 0); //declaration
void load_faults(); //declaration

//Code additions by Alan delineated by hyphen line
//Some notes: EVERY function that's called on a
//declaration line needs to check for propagation!
//because these don't use the assignment operator

// Internal RNG
namespace {
  std::mt19937_64 rng64(14);
  uint64_t rng_leftover;
  uint64_t rng_bits_left = 0;
}

namespace faultTrack {  
    typedef enum {NEVER_FAULTED,
                  FAULTED, 
                  MATH_CORRECTED,
                  OVERWRITE_CORRECTED,
                  MEM_CORRECTED,
                  LAST_FAULT_STATUS,
                  CORRECTED,
                  AUTOCORRECTED
    } faultStatus_t;

    typedef enum {NO_FAULT, // should not be used
                  WRONG_ADDR_WRITTEN, // we wrote data the wrong place
                  WRONG_ADDR_NOT_WRITTEN, // we didn't write data
                                          // where we should have
                  RIGHT_ADDR_WRONG_DATA
    } memFaults_t;
}

//=========================================FAULT TRACKING DATA STRUCTURE=========================================================
struct faultDesc {    
    Cycle_t when;
    Cycle_t whenCorrected;


    //std::array<word_t, n_> bits;
    //int bit_index;
    //int num_bits;
    unsigned long src;
    bool is_glob = 0;
    bool struct_var = 0;
    //long origin;
    faultTrack::faultStatus_t status;

    // for injecting
    faultDesc(Cycle_t _when, bool _is_glob, bool _struct_var, faultTrack::faultStatus_t _stat)
        : when(_when), is_glob(_is_glob), struct_var(_struct_var), whenCorrected(0), status(_stat)
    {;}

    faultDesc(faultTrack::faultStatus_t _stat, Cycle_t _whenCorrected)
        : whenCorrected(_whenCorrected), status(_stat)
    {;}

};

//typedef struct faultDesc faultDesc;
/*template<int w_,
    typename word_t = typename std::conditional<(w_ <= 8),
        uint8_t, uint64_t>::type,
            int n_ = (w_ <= 8) ? 1 : (w_ + 64 - 1) / 64>
struct memFaultDesc {    
    faultTrack::memFaults_t type;

    std::list<faultDesc<w_>> origFaults;

    memFaultDesc() : type(faultTrack::NO_FAULT) {;}
    memFaultDesc(faultTrack::memFaults_t _type, 
                 const std::list<faultDesc<w_>> oFaults)
        : type(_type), origFaults(oFaults) {;}
};*/

std::unordered_map<unsigned long, std::vector<faultDesc>> fault_list; //fault list tracking faults propagated along with sources of faults


//=========================================FAULT TRACKING DATA STRUCTURE=========================================================

//=============================================FAULT METADATA UPDATE=============================================================

#define TWO_ARG_CHECK(OPNM, OP, width_) {                                                   \
    result.meta.faulty_new = this->meta.faulty_new | other.meta.faulty_new;                 \
    for(int i = 0; i < n_; i++)                                                             \
        result.Origdata[i] = this->Origdata[i] OP other.Origdata[i];                        \
    bool this_flag, other_flag;                                                             \
    if(result.meta.faulty_old && !result.meta.faulty_new) {                                 \
        /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                               \
       auto it = fault_list.find(result.meta.index);                                        \
       if(it != fault_list.end()) {                                                         \
           /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                            \
           faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::OVERWRITE_CORRECTED);  \
           fault_desc.src = result.meta.index;                                               \
           it->second.push_back(fault_desc);                                                \
       }                                                                                    \
    }                                                                                       \
    if(result.meta.faulty_new) {                                                            \
        faultDesc fault_desc_this(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);            \
        faultDesc fault_desc_other(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);           \
        vector<faultDesc> new_fault;                                                  \
        for(int i = 0; i < n_; i++) {                                                       \
            if(result.words_[i] != result.Origdata[i]) {                                    \
                /*std::cout<<"\nFAULT PROPAGATED Bitwise\n";*/                              \
                result.meta.faulty_new = 1;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    break;                                                                  \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
            }                                                                               \
            else {                                                                          \
                result.meta.faulty_new = 0;                                                 \
                /*std::cout<<"\nFAULT CORRECTED bitwise";*/                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
        if(this_flag && other_flag) {                                                       \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            ret.first->second.push_back(fault_desc_other);                                  \
            this_flag = false;                                                              \
            other_flag = false;                                                             \
        }                                                                                   \
        else if(this_flag) {                                                                \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            this_flag = false;                                                              \
        }                                                                                   \
        else if(other_flag) {                                                               \
            new_fault.push_back(fault_desc_other);                                          \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_other);                              \
            }                                                                               \
            other_flag = false;                                                             \
        }                                                                                   \
    }                                                                                       \
}                                                                                           \

#define TWO_ARG_CHECK_cat() {                                                               \
    bool this_flag, other_flag;                                                             \
    if(to_return.meta.faulty_old && !to_return.meta.faulty_new) {                                 \
       /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                               \
       auto it = fault_list.find(to_return.meta.index);                                        \
       if(it != fault_list.end()) {                                                         \
           /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                            \
           faultDesc fault_desc(cycle, to_return.is_glob, to_return.struct_var, faultTrack::OVERWRITE_CORRECTED);  \
           fault_desc.src = to_return.meta.index;                                               \
           it->second.push_back(fault_desc);                                                \
       }                                                                                    \
    }                                                                                       \
    if(to_return.meta.faulty_new) {                                                            \
        faultDesc fault_desc_this(cycle, to_return.is_glob, to_return.struct_var, faultTrack::FAULTED);   \
        faultDesc fault_desc_other(cycle, to_return.is_glob, to_return.struct_var, faultTrack::FAULTED);  \
        vector<faultDesc> new_fault;                                                  \
        for(int i = 0; i < n_; i++) {                                                       \
            if(to_return.words_[i] != to_return.Origdata[i]) {                                    \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    break;                                                                  \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
            }                                                                               \
            else {                                                                          \
                to_return.meta.faulty_new = 0;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
        if(this_flag && other_flag) {                                                       \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({this->meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            ret.first->second.push_back(fault_desc_other);                                  \
            this_flag = false;                                                              \
            other_flag = false;                                                             \
        }                                                                                   \
        else if(this_flag) {                                                                \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({this->meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            this_flag = false;                                                              \
        }                                                                                   \
        else if(other_flag) {                                                               \
            new_fault.push_back(fault_desc_other);                                          \
            auto ret = fault_list.insert({this->meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_other);                              \
            }                                                                               \
            other_flag = false;                                                             \
        }                                                                                   \
    }                                                                                       \
} 

#define TWO_ARG_CHECK_mod_div(OPNM, OP, width_) {                                                   \
    result.meta.faulty_new = this->meta.faulty_new | other.meta.faulty_new;                 \
    bool this_flag, other_flag;                                                             \
    if(result.meta.faulty_old && !result.meta.faulty_new) {                                 \
       auto it = fault_list.find(result.meta.index);                                        \
       if(it != fault_list.end()) {                                                         \
           faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::OVERWRITE_CORRECTED);  \
           fault_desc.src = result.meta.index;                                                           \
           it->second.push_back(fault_desc);                                                \
       }                                                                                    \
    }                                                                                       \
    if(result.meta.faulty_new) {                                                            \
        faultDesc fault_desc_this(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);            \
        faultDesc fault_desc_other(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);           \
                                                                                            \
        result.Origdata[0] = this->as_single_word_Origdata() OP other.as_single_word_Origdata(); \
        n_ = 1;                                                                         \
                                                                                            \
        vector<faultDesc> new_fault;                                                  \
        for(int i = 0; i < n_; i++) {                                                       \
            if(result.words_[i] != result.Origdata[i]) {                                    \
                result.meta.faulty_new = 1;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_this.src = this->meta.index;                                 \
                    /*auto it = fault_list.find(this->meta.index);*/                            \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    /*it = fault_list.find(other.meta.index);*/                                 \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_this.src = this->meta.index;                                 \
                    /*auto it = fault_list.find(this->meta.index);*/                            \
                    this_flag = true;                                                       \
                    break;                                                                  \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_other.src = other.meta.index;                                \
                    /*auto it = fault_list.find(other.meta.index);*/                            \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
            }                                                                               \
            else {                                                                          \
                result.meta.faulty_new = 0;                                                 \
                /*std::cout<<"\nFAULT CORRECTED mod_div";*/                                             \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_this.src = this->meta.index;                                 \
                    /*auto it = fault_list.find(this->meta.index);*/                            \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    /*it = fault_list.find(other.meta.index);*/                                 \
                    other_flag = true;                                                      \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_this.src = this->meta.index;                                 \
                    /*auto it = fault_list.find(this->meta.index);*/                            \
                    this_flag = true;                                                       \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_other.src = other.meta.index;                                \
                    /*auto it = fault_list.find(other.meta.index);*/                            \
                    other_flag = true;                                                      \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
        if(this_flag && other_flag) {                                                       \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            ret.first->second.push_back(fault_desc_other);                                  \
            this_flag = false;                                                              \
            other_flag = false;                                                             \
        }                                                                                   \
        else if(this_flag) {                                                                \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            this_flag = false;                                                              \
        }                                                                                   \
        else if(other_flag) {                                                               \
            new_fault.push_back(fault_desc_other);                                          \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_other);                              \
            }                                                                               \
            other_flag = false;                                                             \
        }                                                                                   \
    }                                                                                       \
}                                                                                           \


#define TWO_ARG_CHECK_conditional(op) {                                                     \
    result.meta.faulty_new = this->meta.faulty_new | other.meta.faulty_new;                 \
    bool this_flag, other_flag;                                                             \
    if(result.meta.faulty_old && !result.meta.faulty_new) {                                 \
        /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                               \
       auto it = fault_list.find(result.meta.index);                                        \
       if(it != fault_list.end()) {                                                         \
           /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                            \
           faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::OVERWRITE_CORRECTED);      \
           fault_desc.src = result.meta.index;                                                           \
           it->second.push_back(fault_desc);                                                \
       }                                                                                    \
    }                                                                                       \
    bool new_flag = 0;                                                                  \
    if(strcmp(op, "<=") == 0) {                                                                    \
        for(int i = n_-1; i >= 0; i++) {                                                \
            if (Origdata[i] < other.Origdata[i]) { result.Origdata[0] = case_true; new_flag = 1; break;}\
            if (Origdata[i] > other.Origdata[i]) { result.Origdata[0] = case_false; new_flag = 1; break;}\
        }                                                                               \
                                                                                            \
        if(!new_flag)                                                                   \
        result.Origdata[0] = case_true;                                                 \
                                                                                            \
    }                                                                                   \
    else if(strcmp(op,">=") == 0) {                                                                    \
        for(int i = n_-1; i >= 0; i++) {                                                \
            if (Origdata[i] > other.Origdata[i]) { result.Origdata[0] = case_true; new_flag = 1; break;}\
            if (Origdata[i] < other.Origdata[i]) { result.Origdata[0] = case_false; new_flag = 1; break;}\
        }                                                                               \
                                                                                            \
        if(!new_flag)                                                                   \
        result.Origdata[0] = case_true;                                                 \
                                                                                            \
    }                                                                                   \
    else if(strcmp(op, "==") == 0) {                                                               \
        for(int i = 0; i < n_; i++)                                                     \
            if (Origdata[i] != other.Origdata[i]) { result.Origdata[0] = case_false; new_flag = 1; break;}\
                                                                                            \
        if(!new_flag)                                                                   \
        result.Origdata[0] = case_true;                                                 \
    }                                                                                   \
                                                                                        \
    if(result.meta.faulty_new) {                                                            \
        faultDesc fault_desc_this(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);                 \
        faultDesc fault_desc_other(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);                \
                                                                                            \
        vector<faultDesc> new_fault;                                                  \
        if(result.words_[0] != result.Origdata[0]) {                                        \
            /*std::cout<<"\nCONDITIONAL FAULT PROPAGATED\n";*/                              \
            result.meta.faulty_new = 1;                                                     \
            if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                fault_desc_this.status = faultTrack::FAULTED;                           \
                fault_desc_other.status = faultTrack::FAULTED;                          \
                fault_desc_this.src = this->meta.index;                                 \
                this_flag = true;                                                       \
                fault_desc_other.src = other.meta.index;                                \
                other_flag = true;                                                      \
            }                                                                           \
            else if(this->meta.faulty_new) {                                            \
                fault_desc_this.status = faultTrack::FAULTED;                           \
                fault_desc_this.src = this->meta.index;                                 \
                this_flag = true;                                                       \
            }                                                                           \
            else {                                                                      \
                fault_desc_other.status = faultTrack::FAULTED;                          \
                fault_desc_other.src = other.meta.index;                                \
                other_flag = true;                                                      \
            }                                                                           \
        }                                                                               \
        else {                                                                          \
            result.meta.faulty_new = 0;                                                 \
            /*std::cout<<"\nCONDITIONAL FAULT CORRECTED";*/                             \
            if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                fault_desc_this.src = this->meta.index;                                 \
                this_flag = true;                                                       \
                fault_desc_other.src = other.meta.index;                                \
                other_flag = true;                                                      \
            }                                                                           \
            else if(this->meta.faulty_new) {                                            \
                fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                fault_desc_this.src = this->meta.index;                                 \
                this_flag = true;                                                       \
            }                                                                           \
            else {                                                                      \
                fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                fault_desc_other.src = other.meta.index;                                \
                other_flag = true;                                                      \
            }                                                                           \
        }                                                                               \
                                                                                        \
    if(this_flag && other_flag) {                                                       \
        new_fault.push_back(fault_desc_this);                                           \
        auto ret = fault_list.insert({result.meta.index, new_fault});                   \
        if(!ret.second) {                                                               \
            ret.first->second.push_back(fault_desc_this);                               \
        }                                                                               \
        ret.first->second.push_back(fault_desc_other);                                  \
        this_flag = false;                                                              \
        other_flag = false;                                                             \
    }                                                                                   \
    else if(this_flag) {                                                                \
        new_fault.push_back(fault_desc_this);                                           \
        auto ret = fault_list.insert({result.meta.index, new_fault});                   \
        if(!ret.second) {                                                               \
            ret.first->second.push_back(fault_desc_this);                               \
        }                                                                               \
        this_flag = false;                                                              \
    }                                                                                   \
    else if(other_flag) {                                                               \
        new_fault.push_back(fault_desc_other);                                          \
        auto ret = fault_list.insert({result.meta.index, new_fault});                   \
        if(!ret.second) {                                                               \
            ret.first->second.push_back(fault_desc_other);                              \
        }                                                                               \
        other_flag = false;                                                             \
    }                                                                                   \
  }                                                                                     \
}                                                                                       \


#define TWO_ARG_CHECK_ADD_SUB(width_) {                                                     \
    result.meta.faulty_new = this->meta.faulty_new | other.meta.faulty_new;                 \
    bool this_flag, other_flag;                                                             \
    if(result.meta.faulty_old && !result.meta.faulty_new) {                                 \
       auto it = fault_list.find(result.meta.index);                                        \
       if(it != fault_list.end()) {                                                         \
           faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::OVERWRITE_CORRECTED);  \
           fault_desc.src = result.meta.index;                                                           \
           it->second.push_back(fault_desc);                                                \
       }                                                                                    \
    }                                                                                       \
    carry = subtract;                                                                       \
    for(int i = 0; i < n_; i++) {                                                           \
        uint64_t operand = subtract ? ~other.Origdata[i] : other.Origdata[i];               \
        result.Origdata[i] = Origdata[i] + operand + carry;                                 \
        carry = result.Origdata[i] < operand ? 1 : 0;                                       \
    }                                                                                       \
                                                                                            \
    if(result.meta.faulty_new) {                                                            \
        faultDesc fault_desc_this(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);            \
        faultDesc fault_desc_other(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);           \
        vector<faultDesc> new_fault;                                                  \
        for(int i = 0; i < n_; i++) {                                                       \
            if(result.words_[i] != result.Origdata[i]) {                                    \
                result.meta.faulty_new = 1;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    break;                                                                  \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
            }                                                                               \
            else {                                                                          \
                result.meta.faulty_new = 0;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
        if(this_flag && other_flag) {                                                       \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            ret.first->second.push_back(fault_desc_other);                                  \
            this_flag = false;                                                              \
            other_flag = false;                                                             \
        }                                                                                   \
        else if(this_flag) {                                                                \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            this_flag = false;                                                              \
        }                                                                                   \
        else if(other_flag) {                                                               \
            new_fault.push_back(fault_desc_other);                                          \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_other);                              \
            }                                                                               \
            other_flag = false;                                                             \
        }                                                                                   \
    }                                                                                       \
} 

#define TWO_ARG_CHECK_MUL(width_) {                                                         \
    result.meta.faulty_new = this->meta.faulty_new | other.meta.faulty_new;                 \
    bool this_flag, other_flag;                                                             \
    if(result.meta.faulty_old && !result.meta.faulty_new) {                                 \
        /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                               \
       auto it = fault_list.find(result.meta.index);                                        \
       if(it != fault_list.end()) {                                                         \
           /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                         \
           faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::OVERWRITE_CORRECTED);  \
           fault_desc.src = result.meta.index;                                                           \
           it->second.push_back(fault_desc);                                                \
       }                                                                                    \
    }                                                                                       \
    carry = 0;                                                                              \
    for(int i = 0; i < n_; i++) {                                                           \
        carry = 0;                                                                          \
        for (int j=0; j < n_; j++) {                                                        \
            uint64_t prod_ll = lower(Origdata[i]) * lower(other.Origdata[j]);               \
            uint64_t prod_lu = lower(Origdata[i]) * upper(other.Origdata[j]);               \
            uint64_t prod_ul = upper(Origdata[i]) * lower(other.Origdata[j]);               \
            uint64_t prod_uu = upper(Origdata[i]) * upper(other.Origdata[j]);               \
            uint64_t lower_sum = lower(result.Origdata[i+j]) + lower(carry) +               \
                lower(prod_ll);                                                             \
            uint64_t upper_sum = upper(result.Origdata[i+j]) + upper(carry) +               \
                upper(prod_ll) + upper(lower_sum) +                                         \
                lower(prod_lu) + lower(prod_ul);                                            \
            result.Origdata[i+j] = (upper_sum << 32) | lower(lower_sum);                    \
            carry = upper(upper_sum) + upper(prod_lu) + upper(prod_ul) + prod_uu;           \
        }                                                                                   \
        if ((i+n_) < result.NW)                                                             \
            result.Origdata[i + n_] += carry;                                               \
                                                                                            \
    }                                                                                       \
                                                                                            \
    if(result.meta.faulty_new) {                                                            \
        faultDesc fault_desc_this(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);            \
        faultDesc fault_desc_other(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);           \
        vector<faultDesc> new_fault;                                                  \
        for(int i = 0; i < 2*n_; i++) {                                                       \
            if(result.words_[i] != result.Origdata[i]) {                                    \
                result.meta.faulty_new = 1;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    break;                                                                  \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
            }                                                                               \
            else {                                                                          \
                result.meta.faulty_new = 0;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
        if(this_flag && other_flag) {                                                       \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            ret.first->second.push_back(fault_desc_other);                                  \
            this_flag = false;                                                              \
            other_flag = false;                                                             \
        }                                                                                   \
        else if(this_flag) {                                                                \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            this_flag = false;                                                              \
        }                                                                                   \
        else if(other_flag) {                                                               \
            new_fault.push_back(fault_desc_other);                                          \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_other);                              \
            }                                                                               \
            other_flag = false;                                                             \
        }                                                                                   \
    }                                                                                       \
} 

#define TWO_ARG_CHECK_lshift(width_) {                                                            \
    result.meta.faulty_new = this->meta.faulty_new | other.meta.faulty_new;                 \
    bool this_flag, other_flag;                                                             \
    if(result.meta.faulty_old && !result.meta.faulty_new) {                                 \
        /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                               \
       auto it = fault_list.find(result.meta.index);                                        \
       if(it != fault_list.end()) {                                                         \
           /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                            \
           faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::OVERWRITE_CORRECTED);  \
           fault_desc.src = result.meta.index;                                                           \
           it->second.push_back(fault_desc);                                                \
       }                                                                                    \
    }                                                                                       \
    for (uint64_t i=0; i < n_; i++) {                                                   \
        result.Origdata[i + word_up] |= Origdata[i] << bits_up;                             \
        if ((bits_up != 0) && (dshamt + w_ > kWordSize) && (i + word_up + 1 < result.NW))   \
            result.Origdata[i + word_up + 1] = Origdata[i] >> cap(kWordSize - bits_up);     \
    }                                                                                   \
                                                                                            \
    if(result.meta.faulty_new) {                                                            \
        faultDesc fault_desc_this(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);            \
        faultDesc fault_desc_other(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);           \
        vector<faultDesc> new_fault;                                                  \
        for(int i = 0; i < n_; i++) {                                                       \
            if(result.words_[i] != result.Origdata[i]) {                                    \
                /*std::cout<<"\nrshift FAULT PROPAGATED\n";*/                               \
                result.meta.faulty_new = 1;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    break;                                                                  \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
            }                                                                               \
            else {                                                                          \
                result.meta.faulty_new = 0;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
        if(this_flag && other_flag) {                                                       \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            ret.first->second.push_back(fault_desc_other);                                  \
            this_flag = false;                                                              \
            other_flag = false;                                                             \
        }                                                                                   \
        else if(this_flag) {                                                                \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            this_flag = false;                                                              \
        }                                                                                   \
        else if(other_flag) {                                                               \
            new_fault.push_back(fault_desc_other);                                          \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_other);                              \
            }                                                                               \
            other_flag = false;                                                             \
        }                                                                                   \
    }                                                                                       \
} 

#define TWO_ARG_CHECK_rshift(width_) {                                                      \
    result.meta.faulty_new = this->meta.faulty_new | other.meta.faulty_new;                 \
    bool this_flag, other_flag;                                                             \
    if(result.meta.faulty_old && !result.meta.faulty_new) {                                 \
        /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                           \
       auto it = fault_list.find(result.meta.index);                                        \
       if(it != fault_list.end()) {                                                         \
           /*std::cout<<"\nOVERWRITE CORRECTED\n";*/                                        \
           faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::OVERWRITE_CORRECTED);  \
           fault_desc.src = result.meta.index;                                                           \
           it->second.push_back(fault_desc);                                                \
       }                                                                                    \
    }                                                                                       \
    for (uint64_t i=word_down; i < n_; i++) {                                               \
        result.Origdata[i - word_down] = Origdata[i] >> bits_down;                          \
        if ((bits_down != 0) && (i < n_-1))                                                 \
            result.Origdata[i - word_down] |= Origdata[i + 1] << cap(kWordSize - bits_down);\
    }                                                                                       \
                                                                                            \
    if(result.meta.faulty_new) {                                                            \
        faultDesc fault_desc_this(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);            \
        faultDesc fault_desc_other(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);           \
        vector<faultDesc> new_fault;                                                        \
        for(int i = 0; i < n_; i++) {                                                       \
            if(result.words_[i] != result.Origdata[i]) {                                    \
                result.meta.faulty_new = 1;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::FAULTED;                           \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    break;                                                                  \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::FAULTED;                          \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                    break;                                                                  \
                }                                                                           \
            }                                                                               \
            else {                                                                          \
                result.meta.faulty_new = 0;                                                 \
                if(this->meta.faulty_new && other.meta.faulty_new) {                        \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
                else if(this->meta.faulty_new) {                                            \
                    fault_desc_this.status = faultTrack::MATH_CORRECTED;                    \
                    fault_desc_this.src = this->meta.index;                                 \
                    this_flag = true;                                                       \
                }                                                                           \
                else {                                                                      \
                    fault_desc_other.status = faultTrack::MATH_CORRECTED;                   \
                    fault_desc_other.src = other.meta.index;                                \
                    other_flag = true;                                                      \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
        if(this_flag && other_flag) {                                                       \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            ret.first->second.push_back(fault_desc_other);                                  \
            this_flag = false;                                                              \
            other_flag = false;                                                             \
        }                                                                                   \
        else if(this_flag) {                                                                \
            new_fault.push_back(fault_desc_this);                                           \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_this);                               \
            }                                                                               \
            this_flag = false;                                                              \
        }                                                                                   \
        else if(other_flag) {                                                               \
            new_fault.push_back(fault_desc_other);                                          \
            auto ret = fault_list.insert({result.meta.index, new_fault});                   \
            if(!ret.second) {                                                               \
                ret.first->second.push_back(fault_desc_other);                              \
            }                                                                               \
            other_flag = false;                                                             \
        }                                                                                   \
    }                                                                                       \
} 

//=============================================FAULT METADATA UPDATE=============================================================



// Forward dec
template<int w_>
class SInt;

template<int w_,
         typename word_t = typename std::conditional<(w_ <= 8),
                                                     uint8_t, uint64_t>::type,
         int n_ = (w_ <= 8) ? 1 : (w_ + 64 - 1) / 64>
class UInt {
private:
  constexpr static int cmin(int wa, int wb) { return wa < wb ? wa : wb; }
  constexpr static int cmax(int wa, int wb) { return wa > wb ? wa : wb; }

public:

  UInt() {
    for (int i=0; i < n_; i++) {
      words_[i] = 0;
      Origdata[i] = 0;
    }

    meta.faulty_new = 0;
    meta.faulty_old = 0;
    meta.perm_fault = 0;
    meta.index = ULONG_MAX;

  }

  //DESTRUCTOR-------------------------
  //Need to handle meta data allocated memory
  /*~UInt(){
    if(meta.perm_bits != NULL) delete [] meta.perm_bits;
  }*/
  //-----------------------------------

  //---------------------------------
  //This probably won't be needed once the naming system works
  //Could be useful to keep around for keeping track of original data

  UInt(const UInt &other) {

    for (int i=0; i < n_; i++) {
      words_[i] = other.words_[i];
      Origdata[i] = other.Origdata[i];
    }
    if(!other.meta.faulty_new) {
        for(int i = 0; i < n_; i++)
            this->Origdata[i] = this->words_[i];
    }

    if (profile_flag == 1){
    if(this->is_glob || this->struct_var) {
	    ++callcount[this->meta.index];
    }

    if(other.meta.index != 0) {
	    if(other.is_glob || other.struct_var) {
		     ++callcount[other.meta.index];
	    }
    }
    }

    int cnt = 0;
    while(cnt != fault_list_inject.size()) {

//            std::cout<<"\nIfault_list_inject not empty";
            std::cout.flush();

        if((fault_list_inject[cnt].index == this->meta.index)) {
            u3[meta.index] = &meta;
            to_fault_map[meta.index] = make_pair(&words_[0], &Origdata[0]);
      //      std::cout<<"\nIndex added in UInt constructor";
            std::cout.flush();
        } 

        if((fault_list_inject[cnt].index == this->meta.index) && (fault_list_inject[cnt].cycle <= cycle) && (fault_list_inject[cnt].end_cycle > cycle)){

    //        std::cout<<"\nfault_list_inject condition satisfied in UInt() constructor";
            std::cout.flush();
            //if the UInt object is less than 8 bits than the words_t type will be made of
            //uint8_ts, otherwise it will be uint64_ts. This affects how we cast the meta_t pointer
            if(fault_list_inject[cnt].width > 8){
                UInt<32> temp_large;
  //              std::cout<<"\nfault is getting injected (temp_large)";
                std::cout.flush();
                temp_large.fault_injection(u3[fault_list_inject[cnt].index], fault_list_inject[cnt]);
            }
            else{
                UInt<1> temp_small;
   //             std::cout<<"\nfault is getting injected (temp_small)";
                std::cout.flush();
                temp_small.fault_injection(u3[fault_list_inject[cnt].index], fault_list_inject[cnt]);
            }
        }
        else if((fault_list_inject[cnt].index == this->meta.index) && (fault_list_inject[cnt].end_cycle == cycle)) {
            std::vector<fault_t>::iterator it = fault_list_inject.begin() + cnt;
            fault_list_inject.erase(it); 
 //           std::cout<<"\nErase called";
            std::cout.flush();
        }

        cnt = cnt + 1;
    }
    

//=====================================================
    if(other.meta.faulty_new == 1) {
        meta.faulty_new = 1;
        meta.memetics = other.meta.memetics; //pass inheritance
        meta.genetics = other.meta.genetics;
        vector<faultDesc> new_fault;                                              
        faultDesc fault_desc(cycle, this->is_glob, this->struct_var, faultTrack::FAULTED);
        fault_desc.src = other.meta.index;
        auto it = fault_list.find(other.meta.index);
        new_fault.push_back(fault_desc);
        auto ret = fault_list.insert({meta.index, new_fault});               
        if(!ret.second) {
            for(int i = 0; i < n_; i++) 
                ret.first->second.push_back(fault_desc); 
        }
    }

    //Add overwrite fault correction part
//=======================================================

    else{ meta.faulty_new = 0; }

    if(this->meta.faulty_old && !this->meta.faulty_new) {                                 
       auto it = fault_list.find(this->meta.index);                                        
       if(it != fault_list.end()) {                                                         
           faultDesc fault_desc(cycle, this->is_glob, this->struct_var, faultTrack::OVERWRITE_CORRECTED);      
           fault_desc.src = other.meta.index;                                                           
           it->second.push_back(fault_desc);                                                
       }                                                                                    
    }             
    this->meta.faulty_old = 0;

  }
  //---------------------------------

  UInt(uint64_t initial) : UInt() {
    words_[0] = initial;
    Origdata[0] = initial;
    mask_top_unused();
    mask_top_unused_origdata();
    
    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {  
        ++callcount[this->meta.index]; 
    }
    }
  }

  UInt(std::string initial) {
    if (initial.substr(0,2) != "0x") {
      std::cout << "ERROR: UInt string literal must start with 0x!" << std::endl;
      std::exit(-17);
    }
    initial.erase(0,2);
    // FUTURE: check that literal isn't too big
    int input_bits = 4*initial.length();
    int last_start = initial.length();
    for (int word=0; word < n_; word++) {
      if (word * kWordSize >= input_bits) {
        words_[word] = 0;
        Origdata[word] = 0;
      }
      else {
        int word_start = std::max(static_cast<int>(initial.length())
                                  - 16*(word+1), 0);
        int word_len = std::min(16, last_start - word_start);
        last_start = word_start;
        const std::string substr = initial.substr(word_start, word_len);
        words_[word] = static_cast<uint64_t>(std::stoul(substr, nullptr, 16));
        Origdata[word] = static_cast<uint64_t>(std::stoul(substr, nullptr, 16));
      }
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {  
        ++callcount[this->meta.index]; 
    }
    }

  }

  // NOTE: reads words right to left so literal appears to be concatted
  UInt(std::array<word_t, n_> raw_input_reversed) {
    for (int i = 0; i < n_; i++) {
      words_[i] = raw_input_reversed[n_ - i - 1];
      Origdata[i] = raw_input_reversed[n_ - i - 1];
    }
    mask_top_unused();
    mask_top_unused_origdata();

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    }

  }



  template<int other_w>
  explicit UInt(const UInt<other_w> &other) {
    static_assert(other_w <= w_, "Can't copy construct from wider UInt");

    for (int word=0; word < n_; word++) {
      if (word < UInt<other_w>::NW) {
        words_[word] = other.words_[word];
        Origdata[word] = other.Origdata[word];
      }
      else {
        words_[word] = 0;
        Origdata[word] = 0;
      }
    }
    meta.faulty_new = other.meta.faulty_new;
    meta.faulty_old = other.meta.faulty_old;

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
	    ++callcount[this->meta.index];
    }
    if(other.meta.index != 0) {
	    if(other.is_glob || other.struct_var) {
		    ++callcount[other.meta.index];
	    }
    }
    }

    if(meta.faulty_new == 1) {
        meta.memetics = other.meta.memetics; //pass inheritance
        meta.genetics = other.meta.genetics;
        vector<faultDesc> new_fault;                                              
        faultDesc fault_desc(cycle, this->is_glob, this->struct_var, faultTrack::FAULTED);
        fault_desc.src = other.meta.index;
        auto it = fault_list.find(other.meta.index);
        new_fault.push_back(fault_desc);
        auto ret = fault_list.insert({meta.index, new_fault});               
        if(!ret.second) {
            for(int i = 0; i < n_; i++) 
                ret.first->second.push_back(fault_desc); 
        }
    }
  }

  void rand_init() {
    core_rand_init();
    mask_top_unused();
    mask_top_unused_origdata();
  }

  template<int out_w>
  UInt<cmax(w_,out_w)> pad() const {

    //---------------------------------------------------------
    UInt<cmax(w_,out_w)> result;

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    result = UInt<cmax(w_,out_w)>(*this); //result is an intermediate value
    result.meta.faulty_new = this->meta.faulty_new;
    result.meta.memetics = this->meta.memetics;
    result.meta.genetics = this->meta.index;
    return result;
    //----------------------------------------------------------
    //return UInt<cmax(w_,out_w)>(*this);
    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {  
        ++callcount[this->meta.index]; 
    }
    }

  }

  template<int other_w>
  UInt<w_ + other_w> cat(const UInt<other_w> &other) const {

    UInt<w_ + other_w> to_return(other);

    if(to_return.meta.index == ULONG_MAX) {
        to_return.name_update(-2);
    }

    const int offset = other_w % kWordSize;
    for (int i = 0; i < n_; i++) {
      to_return.words_[word_index(other_w) + i] |= static_cast<uint64_t>(words_[i]) <<
                                                     cap(offset);
      to_return.Origdata[word_index(other_w) + i] |= static_cast<uint64_t>(Origdata[i]) <<
                                                     cap(offset);
      if ((offset != 0) && (i + 1 < to_return.NW - word_index(other_w)))
        to_return.words_[word_index(other_w) + i + 1] |= static_cast<uint64_t>(words_[i]) >>
                                                           cap(kWordSize - offset);
        to_return.Origdata[word_index(other_w) + i + 1] |= static_cast<uint64_t>(Origdata[i]) >>
                                                           cap(kWordSize - offset);
    }
       
    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {    
        ++callcount[this->meta.index];    
    }
    if(other.is_glob || other.struct_var) {
        ++callcount[other.meta.index];    
    }
    }

    //------------------------------------------------------------
    if((this->meta.faulty_new == 1) || (other.meta.faulty_new == 1)) to_return.meta.faulty_new = 1;
    if(to_return.meta.faulty_new){
        if(this->meta.faulty_new){
            to_return.meta.memetics = this->meta.memetics;
            if(this->meta.faulty_old == 1) to_return.meta.genetics = this->meta.index;
            else to_return.meta.genetics = this->meta.genetics;
        }
        else{
            to_return.meta.memetics = other.meta.memetics;
            if(other.meta.faulty_old == 1) to_return.meta.genetics = other.meta.index;
            else to_return.meta.genetics = other.meta.genetics;
        }
    }

    TWO_ARG_CHECK_cat();
    //------------------------------------------------------------

    return to_return;
  }


  UInt<w_ + 1> operator+(const UInt<w_> &other) const {
    UInt<w_ + 1> result = core_add_sub<w_+1, false>(other);


    //-------------------------------------------------------
    TWO_ARG_CHECKS();
    //-------------------------------------------------------

    if ((kWordSize * n_ == w_) && (result.words_[n_-1] < words_[n_-1]))
      result.words_[word_index(w_ + 1)] = 1;
    if ((kWordSize * n_ == w_) && (result.Origdata[n_-1] < Origdata[n_-1]))
      result.Origdata[word_index(w_ + 1)] = 1;

    return result;
  }

  UInt<w_> addw(const UInt<w_> &other) const {
    UInt<w_> result = core_add_sub<w_, false>(other);
    result.mask_top_unused();
    result.mask_top_unused_origdata();

    return result;
  }

  UInt<w_> subw(const UInt<w_> &other) const {
    UInt<w_> result(core_add_sub<w_, true>(other.ui));
    result.mask_top_unused();
    result.mask_top_unused_origdata();

    return result;
  }

  SInt<w_ + 1> operator-() const {
    return SInt<w_+1>(0).subw(SInt<w_+1>(pad<w_+1>()));
  }

  UInt<w_ + 1> operator-(const UInt<w_> &other) const {
    UInt<w_ + 1> result = core_add_sub<w_+1, true>(other);

    //-------------------------------------------------------
    TWO_ARG_CHECKS();
    //-------------------------------------------------------

    if (kWordSize * n_ == w_) {
      if (result.words_[n_-1] < other.words_[n_-1]) {
        result.words_[word_index(w_ + 1)] = 1;
        result.Origdata[word_index(w_ + 1)] = 1;
      }
    } else {
      result.mask_top_unused();
      result.mask_top_unused_origdata();
    }
    return result;
  }

  UInt<w_ + w_> operator*(const UInt<w_> &other) const {
    UInt<w_ + w_> result(0);

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {  
        ++callcount[this->meta.index]; 
    }

    if(other.is_glob || other.struct_var) { 
        ++callcount[other.meta.index]; 
    }
    }

    //-------------------------------------------------------
    TWO_ARG_CHECKS();
    //-------------------------------------------------------

    uint64_t carry = 0;
    for (int i=0; i < n_; i++) {
      carry = 0;
      for (int j=0; j < n_; j++) {
        uint64_t prod_ll = lower(words_[i]) * lower(other.words_[j]);
        uint64_t prod_lu = lower(words_[i]) * upper(other.words_[j]);
        uint64_t prod_ul = upper(words_[i]) * lower(other.words_[j]);
        uint64_t prod_uu = upper(words_[i]) * upper(other.words_[j]);
        uint64_t lower_sum = lower(result.words_[i+j]) + lower(carry) +
                             lower(prod_ll);
        uint64_t upper_sum = upper(result.words_[i+j]) + upper(carry) +
                             upper(prod_ll) + upper(lower_sum) +
                             lower(prod_lu) + lower(prod_ul);
        result.words_[i+j] = (upper_sum << 32) | lower(lower_sum);
        carry = upper(upper_sum) + upper(prod_lu) + upper(prod_ul) + prod_uu;
      }
      if ((i+n_) < result.NW)
        result.words_[i + n_] += carry;
    }
    TWO_ARG_CHECK_MUL(w_ + w_);
    return result;
  }

//--------------------------------------------
//ALAN'S PATENTED ASSIGNMENT OPERATOR OVERLOAD
  UInt<w_> &operator=(const UInt<w_> &other){

    this->meta.faulty_new = other.meta.faulty_new;
    if(this->meta.faulty_old != this->meta.faulty_new){
        if(this->meta.faulty_old == 0){
            this->meta.memetics = other.meta.memetics;
            this->meta.genetics = other.meta.genetics;
        }
        else{
            if(!meta.perm_fault){
                //if(is_glob) std::cout << "*";
                //std::cout << u1[this->meta.index] << " was corrected" << " at " << cycle <<"\n";
            }
        }
    }
    this->meta.faulty_old = this->meta.faulty_new;
    //u2[this->meta.index] = this->meta.faulty_old;
    
     if(!this->meta.faulty_new) {
        for(int i = 0; i < n_; i++)
            this->Origdata[i] = this->words_[i];
     }

    for (int i = 0; i < n_; i++) { this->words_[i] = other.words_[i]; this->Origdata[i] = other.Origdata[i]; }
    if(meta.perm_fault == 1){
        meta.faulty_new = 1;
        meta.faulty_old = 1;

        //Anything else to be added here for tracking?
        switch(perm_op){
        case '&':
            for (int i = 0; i < n_; i++) { this->words_[i] = perm_bits[i] & this->words_[i]; this->Origdata[i] = perm_bits[i] & this->Origdata[i]; }
            std::cout << "& Injection op= in " << this->meta.index << " at cycle " << cycle << endl;
            break;
        case '|':
            for (int i = 0; i < n_; i++) { this->words_[i] = perm_bits[i] | this->words_[i]; this->Origdata[i] = perm_bits[i] | this->Origdata[i]; }
            std::cout << "| Injection op= in " << this->meta.index << " at cycle " << cycle << endl;
            break;
        case '0':
            for (int i = 0; i < n_; i++) { this->words_[i] = 0; this->Origdata[i] = 0; }
            std::cout << "0 Injection op= in " << this->meta.index << " at cycle " << cycle << endl;
            break;
        case '^':
            std::cout << "Invalid operator for permanent fault '^' "; //<< u0[this->meta.index] << "\n";
            break;
        default:
            std::cout << "You shouldn't be here (assignment operator perm effect) ";// << u0[this->meta.index] << "\n";
        }
    }

    if(this->meta.faulty_new) {
        faultDesc fault_desc(cycle, this->is_glob, this->struct_var, faultTrack::FAULTED);
        fault_desc.src = other.meta.index;
        vector<faultDesc> new_fault;
        new_fault.push_back(fault_desc);
        auto ret = fault_list.insert({this->meta.index, new_fault}); 
        if(!ret.second) 
            ret.first->second.push_back(fault_desc);
    }

    if(this->meta.faulty_old && !this->meta.faulty_new) {                                 
       auto it = fault_list.find(this->meta.index);                                        
       if(it != fault_list.end()) {                                                         
           faultDesc fault_desc(cycle, this->is_glob, this->struct_var, faultTrack::OVERWRITE_CORRECTED);      
           fault_desc.src = other.meta.index;                                                           
           it->second.push_back(fault_desc);                                                
       }                                                                                    
    }             


     if (profile_flag ==1){
     if(this->is_glob || this->struct_var) {writes[this->meta.index].push_back(cycle);}
     if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
     }
     if(other.meta.index != 0) {
     if(other.is_glob || other.struct_var) {
      ++callcount[other.meta.index];
     }
     }
     }

    int cnt = 0;
    while(cnt != fault_list_inject.size()) {

        if((fault_list_inject[cnt].index == this->meta.index)) {
            u3[this->meta.index] = &meta;
            to_fault_map[this->meta.index] = make_pair(&words_[0], &Origdata[0]);
      //      std::cout<<"\nMeta added in operator=";
          //  std::cout.flush();
        } 

        if((fault_list_inject[cnt].index == this->meta.index) && (cycle >= fault_list_inject[cnt].cycle) && (cycle < fault_list_inject[cnt].end_cycle)){
            //if the UInt object is less than 8 bits than the words_t type will be made of
            //uint8_ts, otherwise it will be uint64_ts. This affects how we cast the meta_t pointer
    //        std::cout<<"\nfault_list_inject condition satisfied in UInt() constructor";
          //  std::cout.flush();
            if(fault_list_inject[cnt].width > 8){
                UInt<32> temp_large;
  //              std::cout<<"\nfault is getting injected (temp_small)";
        //        std::cout.flush();
                std::cout << "Large injection F_I in " << fault_list_inject[cnt].index << " at cycle " << cycle << endl;
                temp_large.fault_injection(u3[fault_list_inject[cnt].index], fault_list_inject[cnt]);
            }
            else{
                UInt<1> temp_small;
//                std::cout<<"\nfault is getting injected (temp_small)";
      //          std::cout.flush();
                std::cout << "Small  injection F_I in " << fault_list_inject[cnt].index << " at cycle " << cycle << endl;
                temp_small.fault_injection(u3[fault_list_inject[cnt].index], fault_list_inject[cnt]);
            }

        }
        else if((fault_list_inject[cnt].index == this->meta.index) && (fault_list_inject[cnt].end_cycle == cycle)) {
            std::vector<fault_t>::iterator it = fault_list_inject.begin() + cnt;
            fault_list_inject.erase(it); 
        //    std::cout<<"\nErase called";
    //        std::cout.flush();
        }

        cnt = cnt + 1;
    }


    return *this;
  }
//--------------------------------------------

  // this / other
  template<int other_w>
  UInt<w_> operator/(const UInt<other_w> &other) const {
    static_assert(w_ <= kWordSize, "Div not supported beyond 64b");
    static_assert(other_w <= kWordSize, "Div not supported beyond 64b");
    //------------------------------------------------------------------
    UInt<w_> result = as_single_word() / other.as_single_word();

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }
    if(other.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) { ++callcount[this->meta.index]; }
    if(other.is_glob || other.struct_var) { ++callcount[other.meta.index]; }
    }

    TWO_ARG_CHECKS();
    TWO_ARG_CHECK_mod_div(operator/, /, w_); 
    //------------------------------------------------------------------
    //return UInt<w_>(as_single_word() / other.as_single_word());
    return result;
  }

  // this % other
  template<int other_w>
  UInt<cmin(w_, other_w)> operator%(const UInt<other_w> &other) const {
    static_assert(w_ <= kWordSize, "Mod not supported beyond 64b");
    static_assert(other_w <= kWordSize, "Mod not supported beyond 64b");
    //------------------------------------------------------------------------
    UInt<cmin(w_, other_w)> result = as_single_word() % other.as_single_word();

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }
    if(other.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {  
        ++callcount[this->meta.index]; 
    }
    if(other.is_glob || other.struct_var) { 
        ++callcount[other.meta.index]; 
    }
    }

    TWO_ARG_CHECKS();
    TWO_ARG_CHECK_mod_div(operator%, %, cmin(w_, other_w)); 

    return result;
  }

  UInt<w_> operator~() const {
    UInt<w_> result;

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    }

    //-------------------------------------------------------
    result.meta.faulty_new = this->meta.faulty_new;
    result.meta.memetics = this->meta.memetics;
    result.meta.genetics = this->meta.index;
    //-------------------------------------------------------

    for (int i = 0; i < n_; i++) {
      result.words_[i] = ~words_[i];
      result.Origdata[i] =  ~Origdata[i];
    }

    if(result.meta.faulty_new) {
        vector<faultDesc> new_fault;
        faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);
        for(int i = 0; i < n_; i++) { 
            if(result.words_[i] != result.Origdata[i]) { 
                fault_desc.src = this->meta.index;
            }
            else {
                fault_desc.status = faultTrack::MATH_CORRECTED;                    
                fault_desc.src = this->meta.index;                                 
            } 
        }
        new_fault.push_back(fault_desc);                                           
        auto ret = fault_list.insert({result.meta.index, new_fault});                  
        if(!ret.second) {                                                               
            ret.first->second.push_back(fault_desc);                               
        }                                                       
        /*if(find(autocorrect.begin(), autocorrect.end(), result.meta.index) != autocorrect.end()) {  
            for(int i = 0; i < n_; i++)                                                             
                result.words_[i] = result.Origdata[i];                                              
            result.meta.faulty_new = 0;
            result.meta.faulty_old = 0;
            faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::AUTOCORRECTED); 
            auto it = fault_list.find(result.meta.index);                                   
            if (it != fault_list.end()) {                                                   
                fault_desc.src = it->second.back().src;                                     
                it->second.push_back(fault_desc);                                           
            }                                                                               
        }*/                                                                                   
    }

    result.mask_top_unused();
    result.mask_top_unused_origdata();
    return result;
  }

  UInt<w_> operator&(const UInt<w_> &other) const {
    UInt<w_> result;

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }

    if(other.is_glob || other.struct_var) {
        ++callcount[other.meta.index];
    }
    }

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    //-------------------------------------------------------
    TWO_ARG_CHECKS();
    //-------------------------------------------------------

    for (int i = 0; i < n_; i++) {
      result.words_[i] = words_[i] & other.words_[i];
    }
    TWO_ARG_CHECK(operator&, &, w_);
    return result;
  }

  UInt<w_> operator|(const UInt<w_> &other) const {
    UInt<w_> result;

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    if(other.is_glob || other.struct_var) {
        ++callcount[other.meta.index];
    }
    }

    //-------------------------------------------------------
    TWO_ARG_CHECKS();
    //-------------------------------------------------------

    for (int i = 0; i < n_; i++) {
      result.words_[i] = words_[i] | other.words_[i];
    }
    TWO_ARG_CHECK(operator|, |, w_);
    return result;
  }

  UInt<w_> operator^(const UInt<w_> &other) const {
    UInt<w_> result;
    
    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    if(other.is_glob || other.struct_var) {
        ++callcount[other.meta.index];
    }
    }

    //-------------------------------------------------------
    TWO_ARG_CHECKS();
    //-------------------------------------------------------

    for (int i = 0; i < n_; i++) {
      result.words_[i] = words_[i] ^ other.words_[i];
    }
    TWO_ARG_CHECK(operator^, ^, w_);
    return result;
  }

//=====================NOT BEING TRACKED CURRENTLY===========================
  UInt<1> andr() const {
    return *this == ~UInt<w_>(0);
  }

  UInt<1> orr() const {
    return *this != UInt<w_>(0);
  }

  UInt<1> xorr() const {
    word_t result = 0;
    for (int i = 0; i < n_; i++) {
      word_t word_parity_scratch = words_[i] ^ (words_[i] >> 1);
      word_parity_scratch ^= (word_parity_scratch >> 2);
      word_parity_scratch ^= (word_parity_scratch >> 4);
      if (WW > 8) {
        word_parity_scratch ^= (word_parity_scratch >> 8);
        word_parity_scratch ^= (word_parity_scratch >> 16);
        word_parity_scratch ^= (word_parity_scratch >> 32);
      }
      result ^= word_parity_scratch;
    }
    return UInt<1>(result & 1);
  }

  template<int hi, int lo>
  UInt<hi - lo + 1> bits() const {
    UInt<hi - lo + 1> result;
    result = core_bits<hi,lo>();
    result.mask_top_unused();
    result.mask_top_unused_origdata();
    //--------------------------------------------
    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    }



    result.meta.faulty_new = this->meta.faulty_new;
    result.meta.memetics = this->meta.memetics;
    result.meta.genetics = this->meta.index;
    //--------------------------------------------

    /*if(result.meta.faulty_new) {
        vector<faultDesc> new_fault;
        faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);
        for(int i = 0; i < n_; i++) { 
            if(result.words_[i] != result.Origdata[i]) { 
                fault_desc.src = this->meta.index;
            }
            else {
                fault_desc.status = faultTrack::MATH_CORRECTED;                    
                fault_desc.src = this->meta.index;                                 
            } 
        }
        new_fault.push_back(fault_desc);                                           
        auto ret = fault_list.insert({result.meta.index, new_fault});                  
        if(!ret.second) {                                                               
            ret.first->second.push_back(fault_desc);                               
        }                                                       
        if(find(autocorrect.begin(), autocorrect.end(), result.meta.index) != autocorrect.end()) {  
            for(int i = 0; i < n_; i++)                                                             
                result.words_[i] = result.Origdata[i];                                              
            result.meta.faulty_new = 0;
            result.meta.faulty_old = 0;
            faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::AUTOCORRECTED); 
            auto it = fault_list.find(result.meta.index);                                   
            if (it != fault_list.end()) {                                                   
                fault_desc.src = it->second.back().src;                                     
                it->second.push_back(fault_desc);                                           
            }                                                                               
        }                                                                                   
    }*/

    return result;
  }

//=====================NOT BEING TRACKED CURRENTLY===========================
  template<int n>
  UInt<n> head() const {
    static_assert(n <= w_, "Head n must be <= width");

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    }

    return bits<w_-1, w_-n>();
  }

  template<int n>
  UInt<w_ - n> tail() const {
    static_assert(n < w_, "Tail n must be < width");

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    }

    return bits<w_-n-1, 0>();
  }

  template<int shamt>
  UInt<w_ + shamt> shl() const {
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    return cat(UInt<shamt>(0));
  }

  template<int shamt>
  UInt<w_> shlw() const {

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    }

    return shl<shamt>().template tail<shamt>();
  }

  template<int shamt>
  UInt<w_ - shamt> shr() const {

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    }

    return bits<w_-1, shamt>();
  }

  template<int other_w>
  UInt<w_> operator>>(const UInt<other_w> &other) const {
    UInt<w_> result(0);

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {
        ++callcount[this->meta.index];
    }
    if(!other.meta.index) {
        if(other.is_glob || other.struct_var) {
            ++callcount[other.meta.index];
        }
    }
    }

    //-------------------------------------------------------
    TWO_ARG_CHECKS();
    //-------------------------------------------------------

    uint64_t dshamt = other.as_single_word();
    uint64_t word_down = word_index(dshamt);
    uint64_t bits_down = dshamt % kWordSize;
    for (uint64_t i=word_down; i < n_; i++) {
      result.words_[i - word_down] = words_[i] >> bits_down;
      if ((bits_down != 0) && (i < n_-1))
        result.words_[i - word_down] |= words_[i + 1] << cap(kWordSize - bits_down);
    }
    int width_ =  w_;
    TWO_ARG_CHECK_rshift(width_);

    return result;
  }

  template<int other_w>
  UInt<w_ + (1<<other_w) - 1> operator<<(const UInt<other_w> &other) const {
    UInt<w_ + (1<<other_w) - 1> result(0);

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    //-------------------------------------------------------
    TWO_ARG_CHECKS();
    //-------------------------------------------------------

    uint64_t dshamt = other.as_single_word();
    uint64_t word_up = word_index(dshamt);
    uint64_t bits_up = dshamt % kWordSize;
    for (uint64_t i=0; i < n_; i++) {
      result.words_[i + word_up] |= words_[i] << bits_up;
      if ((bits_up != 0) && (dshamt + w_ > kWordSize) && (i + word_up + 1 < result.NW))
        result.words_[i + word_up + 1] = words_[i] >> cap(kWordSize - bits_up);
    }
    int width_ =  w_ + (1<<other_w) - 1;
    TWO_ARG_CHECK_lshift(width_);

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {  
        ++callcount[this->meta.index]; 
    }
    if(other.meta.index > 0) {
        if(other.is_glob || other.struct_var) { 
            ++callcount[other.meta.index]; 
        }
    }
    }

    return result;
  }

  template<int other_w>
  UInt<w_> dshlw(const UInt<other_w> &other) const {
    // return operator<<(other).template bits<w_-1,0>();
    UInt<w_> result(0);

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    uint64_t dshamt = other.as_single_word();
    uint64_t word_up = word_index(dshamt);
    uint64_t bits_up = dshamt % kWordSize;
    for (uint64_t i=0; i + word_up < n_; i++) {
      result.words_[i + word_up] |= words_[i] << bits_up;
      if ((bits_up != 0) && (w_ > kWordSize) && (i + word_up + 1 < n_))
        result.words_[i + word_up + 1] = words_[i] >> cap(kWordSize - bits_up);
    }
    result.mask_top_unused();

    if (profile_flag ==1){
    if(this->meta.index > 0 || this->meta.index != ULONG_MAX) {
        if(this->is_glob || this->struct_var) {
            ++callcount[this->meta.index];
        }
    }
    if(other.meta.index > 0  || other.meta.index != ULONG_MAX) {
        if(other.is_glob || other.struct_var) {  
            ++callcount[other.meta.index]; 
        }
    }
    }

    //-----------------------------------------------
    TWO_ARG_CHECKS();
    //-----------------------------------------------

    return result;
  }

  UInt<1> operator<=(const UInt<w_> &other) const {
    //-----------------------------------------------------
    UInt<1>case_true = UInt<1>(1);
    UInt<1>case_false = UInt<1>(0);
    UInt<1> result;

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }
    if(case_true.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }
    if(case_false.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {  
        ++callcount[this->meta.index]; 
    }
    if(other.is_glob || other.struct_var) { 
        ++callcount[other.meta.index]; 
    }
    }

    bool flag = 0;

    if((other.meta.faulty_new == 1) || (this->meta.faulty_new == 1)){
        case_false.meta.faulty_new = 1;
        case_true.meta.faulty_new = 1;

        if(this->meta.faulty_new){
            case_false.meta.memetics = this->meta.memetics;
            case_false.meta.genetics = this->meta.index;
            case_true.meta.memetics = this->meta.memetics;
            case_true.meta.genetics = this->meta.index;
        }
        else{
            case_false.meta.memetics = other.meta.memetics;
            case_false.meta.genetics = other.meta.index;
            case_true.meta.memetics = other.meta.memetics;
            case_true.meta.genetics = other.meta.index;
        }
    }

    for(int i = n_-1; i >= 0; i--) {
        if (words_[i] < other.words_[i]) { result = case_true; flag = 1; break;}
        if (words_[i] > other.words_[i]) { result = case_false; flag = 1; break;}
    }
    if(!flag) 
        result = case_true;

    TWO_ARG_CHECK_conditional(mkstr(<=));

    for (int i=n_-1; i >= 0; i--) {
      if (words_[i] < other.words_[i]) return case_true;
      if (words_[i] > other.words_[i]) return case_false;
    }

    return case_true;
    //-----------------------------------------------------

    /*for (int i=n_-1; i >= 0; i--) {
      if (words_[i] < other.words_[i]) return UInt<1>(1);
      if (words_[i] > other.words_[i]) return UInt<1>(0);
    }
    return UInt<1>(1);*/
  }

  UInt<1> operator>=(const UInt<w_> &other) const {
    //-----------------------------------------------------
    UInt<1>case_true = UInt<1>(1);
    UInt<1>case_false = UInt<1>(0);
    UInt<1> result;
    bool flag = 0;

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }
    if(case_true.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }
    if(case_false.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {  
        ++callcount[this->meta.index]; 
    }
    if(other.is_glob || other.struct_var) { 
        ++callcount[other.meta.index]; 
    }
    }

    if((other.meta.faulty_new == 1) || (this->meta.faulty_new == 1)){
        case_false.meta.faulty_new = 1;
        case_true.meta.faulty_new = 1;

        if(this->meta.faulty_new){
            case_false.meta.memetics = this->meta.memetics;
            case_false.meta.genetics = this->meta.index;
            case_true.meta.memetics = this->meta.memetics;
            case_true.meta.genetics = this->meta.index;
        }
        else{
            case_false.meta.memetics = other.meta.memetics;
            case_false.meta.genetics = other.meta.index;
            case_true.meta.memetics = other.meta.memetics;
            case_true.meta.genetics = other.meta.index;
        }
    }

    for(int i = n_-1; i >= 0; i--) {
        if (words_[i] < other.words_[i]) { result = case_true; flag = 1; break;}
        if (words_[i] > other.words_[i]) { result = case_false; flag = 1; break;}
    }
    if(!flag) 
        result = case_true;

    TWO_ARG_CHECK_conditional(mkstr(>=));

    for (int i=n_-1; i >= 0; i--) {
      if (words_[i] > other.words_[i]) return case_true;
      if (words_[i] < other.words_[i]) return case_false;
    }
    return case_true;
    //-----------------------------------------------------

    /*for (int i=n_-1; i >= 0; i--) {
      if (words_[i] > other.words_[i]) return UInt<1>(1);
      if (words_[i] < other.words_[i]) return UInt<1>(0);
    }
    return UInt<1>(1);*/
  }

  UInt<1> operator<(const UInt<w_> &other) const {
    return ~(*this >= other);
  }

  UInt<1> operator>(const UInt<w_> &other) const {
    return ~(*this <= other);
  }

  UInt<1> operator==(const UInt<w_> &other) const {
    UInt<1> case_true = UInt<1>(1);
    UInt<1> case_false = UInt<1>(0);
    UInt<1> result;
    bool flag = 0;

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }
    if(case_true.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }
    if(case_false.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {  
        ++callcount[this->meta.index]; 
    }
    if(other.is_glob || other.struct_var) { 
        ++callcount[other.meta.index]; 
    }
    }

    if((other.meta.faulty_new == 1) || (this->meta.faulty_new == 1)){
        case_false.meta.faulty_new = 1;
        case_true.meta.faulty_new = 1;

        if(this->meta.faulty_new){
            case_false.meta.memetics = this->meta.memetics;
            case_false.meta.genetics = this->meta.index;
            case_true.meta.memetics = this->meta.memetics;
            case_true.meta.genetics = this->meta.index;
        }
        else{
            case_false.meta.memetics = other.meta.memetics;
            case_false.meta.genetics = other.meta.index;
            case_true.meta.memetics = other.meta.memetics;
            case_true.meta.genetics = other.meta.index;
        }
    }
    for(int i = 0; i < n_; i++) 
        if (words_[i] != other.words_[i]) { result = case_false; flag = 1; break;}

    if(!flag) 
        result = case_true;

    TWO_ARG_CHECK_conditional(mkstr(==));

    for (int i = 0; i < n_; i++) {
      if (words_[i] != other.words_[i])
          return case_false;
    }
        //return UInt<1>(0);
    //return UInt<1>(1);
    return case_true;
  }

  UInt<1> operator!=(const UInt<w_> &other) const {
      return ~(*this == other);
  }

  operator bool() const {
    static_assert(w_ == 1, "conversion to bool only allowed for width 1");
    return static_cast<bool>(words_[0]);
  }

  UInt<w_> asUInt() const {

    UInt<w_> result(*this);

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    //----------------------
    result.ui.meta.faulty_new = this->meta.faulty_new;
    result.ui.meta.faulty_old = this->meta.faulty_old;
    //----------------------
     if(result.meta.faulty_new) {
        vector<faultDesc> new_fault;
        faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);
        for(int i = 0; i < n_; i++) { 
            if(result.words_[i] != result.Origdata[i]) { 
                fault_desc.src = this->meta.index;
            }
            else {
                fault_desc.status = faultTrack::MATH_CORRECTED;                    
                fault_desc.src = this->meta.index;                                 
            } 
        }
        new_fault.push_back(fault_desc);                                           
        auto ret = fault_list.insert({result.meta.index, new_fault});                  
        if(!ret.second) {                                                               
            ret.first->second.push_back(fault_desc);                               
        }                                                       
    }

    return result;
  }

  SInt<w_> asSInt() const {
    SInt<w_> result(*this);

    //----------------------
    result.ui.meta.faulty_new = this->meta.faulty_new;
    result.ui.meta.faulty_old = this->meta.faulty_old;
    //----------------------
     if(result.ui.meta.faulty_new) {
        vector<faultDesc> new_fault;
        faultDesc fault_desc(cycle, result.ui.is_glob, result.ui.struct_var, faultTrack::FAULTED);
        for(int i = 0; i < n_; i++) { 
            if(result.ui.words_[i] != result.ui.Origdata[i]) { 
                fault_desc.src = this->meta.index;
            }
            else {
                fault_desc.status = faultTrack::MATH_CORRECTED;                    
                fault_desc.src = this->meta.index;                                 
            } 
        }
        new_fault.push_back(fault_desc);                                           
        auto ret = fault_list.insert({result.ui.meta.index, new_fault});                  
        if(!ret.second) {                                                               
            ret.first->second.push_back(fault_desc);                               
        }                                                       
        /*if(find(autocorrect.begin(), autocorrect.end(), result.ui.meta.index) != autocorrect.end()) {  
            for(int i = 0; i < n_; i++)                                                             
                result.ui.words_[i] = result.ui.Origdata[i];                                              
            result.ui.meta.faulty_new = 0;
            result.ui.meta.faulty_old = 0;
            faultDesc fault_desc(cycle, result.ui.is_glob, result.ui.struct_var, faultTrack::AUTOCORRECTED); 
            auto it = fault_list.find(result.ui.meta.index);                                   
            if (it != fault_list.end()) {                                                   
                fault_desc.src = it->second.back().src;                                     
                it->second.push_back(fault_desc);                                           
            }                                                                               
        }*/                                                                                   
    }

    result.sign_extend();
    return result;
  }

  SInt<w_ + 1> cvt() const {
    return pad<w_+1>().asSInt();
  }

  // Direct access for ops that only need small signals
  uint64_t as_single_word() const {
    static_assert(w_ <= kWordSize, "UInt too big for single uint64_t");
    return words_[0];
  }
  // Direct access for ops that only need small signals
  uint64_t as_single_word_Origdata() const {
    static_assert(w_ <= kWordSize, "UInt too big for single uint64_t");
    return Origdata[0];
  }

protected:

  template<int other_w>
  friend class uint_wrapper_t;

  void raw_copy_in(uint64_t *src) {
    for (int word=0; word < n_; word++)
      words_[word] = *src++;
  }

  void raw_copy_out(uint64_t *dst) {
    for (int word=0; word < n_; word++)
      *dst++ = words_[word];
  }

public:

  // Internal state
  std::array<word_t, n_> words_ = {0};
  std::array<word_t, n_> Origdata = {0};

  // Access array word type
  typedef word_t WT;
  // Access array length
  const static int NW = n_;
  // Access array word type bit width
  const static int WW = std::is_same<word_t,uint64_t>::value ? 64 : 8;

  const static int bits_in_top_word_ = w_ % WW == 0 ? WW : w_ % WW;

  // Friend Access
  template<int other_w, typename other_word_t, int other_n>
  friend class UInt;

  template<int other_w>
  friend class SInt;

  template<int w>
  friend std::ostream& operator<<(std::ostream& os, const UInt<w>& ui);

  // Bit Addressing
  const static int kWordSize = 64;

  int static word_index(int bit_index) { return bit_index / kWordSize; }

  uint64_t static upper(uint64_t i) { return i >> 32; }
  uint64_t static lower(uint64_t i) { return i & 0x00000000ffffffff; }

  // Hack to prevent compiler warnings for shift amount being too large
  int static cap(int s) { return s % kWordSize; }

  // Clean up high bits
  void mask_top_unused() {
    if (bits_in_top_word_ != WW) {
      words_[n_-1] = words_[n_-1] & ((1l << cap(bits_in_top_word_)) - 1l);
    }
  }
  // Clean up high bits from Origdata
  void mask_top_unused_origdata() {
    if (bits_in_top_word_ != WW) {
      Origdata[n_-1] = Origdata[n_-1] & ((1l << cap(bits_in_top_word_)) - 1l);
    }
  }


  // Reused math operators
  template<int out_w, bool subtract>
  UInt<out_w> core_add_sub(const UInt<w_> &other) const {
    UInt<out_w> result;

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    //---------------------------------
    TWO_ARG_CHECKS();
    //---------------------------------

    if (profile_flag ==1){
    if(this->is_glob || this->struct_var) {  
        ++callcount[this->meta.index]; 
    }
    if(other.is_glob || other.struct_var) { 
        ++callcount[other.meta.index]; 
    }
    }

    uint64_t carry = subtract;
    for (int i = 0; i < n_; i++) {
      uint64_t operand = subtract ? ~other.words_[i] : other.words_[i];
      result.words_[i] = words_[i] + operand + carry;
      carry = result.words_[i] < operand ? 1 : 0;
    }
    TWO_ARG_CHECK_ADD_SUB(out_w);
    return result;
  }

  __attribute__((noinline))
  void core_rand_init() {
    // trusting mask_top_unused() will be called afterwards
    if (w_ < 64) {
      if (w_ > rng_bits_left) {
        rng_leftover = rng64();
        rng_bits_left = 64;
      }
      words_[0] = rng_leftover;
      Origdata[0] = words_[0];
      rng_leftover = rng_leftover >> cap(w_);
      rng_bits_left -= w_;
    } else {
      for (int word=0; word < n_; word++) {
        words_[word] = rng64();
        Origdata[word] = words_[word];
      }
    }
  }

  template<int hi, int lo>
  UInt<hi - lo + 1> core_bits() const {
    static_assert(hi < w_, "Bit extract hi bigger than width");
    static_assert(hi >= lo, "Bit extract lo > hi");
    static_assert(lo >= 0, "Bit extract lo is negative");
    UInt<hi - lo + 1> result;

    if(result.meta.index == ULONG_MAX) {
        result.name_update(-2);
    }

    //--------------------------------------
    result.meta.faulty_new = this->meta.faulty_new;
    result.meta.faulty_old = this->meta.faulty_old;
    //--------------------------------------
    int word_down = word_index(lo);
    int bits_down = lo % kWordSize;
    for (int i=0; i < result.NW; i++) {
      result.words_[i] = words_[i + word_down] >> bits_down;
      result.Origdata[i] = Origdata[i + word_down] >> bits_down;
      if ((bits_down != 0) && (i + word_down + 1 < n_)) {
        result.words_[i] |= words_[i + word_down + 1] << cap(kWordSize - bits_down);
        result.Origdata[i] |= Origdata[i + word_down + 1] << cap(kWordSize - bits_down);
      }
    }
    if(result.meta.faulty_new) {
        vector<faultDesc> new_fault;
        faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::FAULTED);

        for(int i = 0; i < result.NW; i++) {
            if(result.words_[i] != result.Origdata[i]) { 
                fault_desc.src = this->meta.index;
              
            }
            else {
                fault_desc.status = faultTrack::MATH_CORRECTED;                    
                fault_desc.src = this->meta.index;                                 
            } 
        }
        new_fault.push_back(fault_desc);                                           
        auto ret = fault_list.insert({result.meta.index, new_fault});                  
        if(!ret.second) {                                                               
            ret.first->second.push_back(fault_desc);                               
        }                                                                 
        /*if(find(autocorrect.begin(), autocorrect.end(), result.meta.index) != autocorrect.end()) {  
            for(int i = 0; i < n_; i++)                                                             
                result.words_[i] = result.Origdata[i];                                              
            result.meta.faulty_new = 0;
            faultDesc fault_desc(cycle, result.is_glob, result.struct_var, faultTrack::AUTOCORRECTED); 
            auto it = fault_list.find(result.meta.index);                                   
            if (it != fault_list.end()) {                                                   
                fault_desc.src = it->second.back().src;                                     
                it->second.push_back(fault_desc);                                           
            }                                                                               
        }*/                                                                                   
    }

    return result;
  }


  void print_to_stream(std::ostream& os) const {
    os << "0x" << std::hex << std::setfill('0');
    int top_nibble_width = (bits_in_top_word_ + 3) / 4;
    os << std::setw(top_nibble_width);
    uint64_t top_word_mask = bits_in_top_word_ == kWordSize ? -1 :
                               (1l << cap(bits_in_top_word_)) - 1;
    os << (static_cast<uint64_t>(words_[n_-1]) & top_word_mask);
    for (int word=n_ - 2; word >= 0; word--) {
     os << std::hex << std::setfill('0') << std::setw(16) << words_[word];
    }
    os << std::dec;
  }
public:
  //NEW VARIABLES
  //Meta data struct, pointer to be passed stored in global vector
typedef struct met{
    bool faulty_new = 0;
    bool faulty_old = 0;

    unsigned long index = ULONG_MAX;
    int memetics = 0; //when a value is faulted the original index that faulted it will be here
    int genetics = 0; //the index of the value that directly caused a fault

    bool perm_fault = 0;
    //char operation = 0;
    //long int * perm_bits = NULL; //if the fault is permanent store the bits here
    short n; //loop counter
    //word_t * data_ptr;
} met_t;


  bool struct_var = 0;
  //short struct_index = 0;

  met_t meta;     //holds meta data, in a struct so it can accessed by generic pointer
  bool is_glob = 0; //If a variable is global it should be noted as important to the user upon output
  //short int width = 0;
public:
  //name update function
  //void name_update(string var_name){
  
  void name_update(int increment){
    if(glob_init_flag) is_glob = 0;
    else is_glob = 1;

    if(glob_init_flag && idx_flag == 0) { 
        store_last_glob_index = index_count;
        idx_flag = 1;
    }

    meta.perm_fault = 0; //init this value

    //width = (NW * WW) - (WW - bits_in_top_word_);

    //auto got = u0.find(var_name);

    /*if(got != u0.end()){
        this->meta.index = got->second;
        this->meta.faulty_old = u2[this->meta.index];
    }*/
    if(is_glob){
        this->meta.index = index_count;
        //this->meta.subs_idx = new_index;
        //idx_map[new_index] = index_count;
        /*if(increment == 1) {
            std::cout<<index_count<<"\n";
            std::cout.flush();
        }*/
        //u0.push_back(this->meta.index);

        //check if the pointer to words should be stored in u3 and to_fault_map
        int cnt = 0;

        while(cnt != fault_list_inject.size()) {
            if((fault_list_inject[cnt].index == this->meta.index)) {
                u3[meta.index] = &meta;
                //std::cout<<"\nmeta_data is:"<<meta.index;
                std::cout.flush();
                to_fault_map[meta.index] = make_pair(&words_[0], &Origdata[0]);
            }
            cnt = cnt + 1;
        }
        index_count = index_count + 1;
    }

    else if(increment == -2){
    
        //Temporary index
        //temp_index--;
        
        if(index_count == ULONG_MAX) {
            index_count = std::max(store_last_glob_index,index_count) + 1;
        }
        index_count++;
        meta.index = index_count;

        //meta.index = temp_index;
    }

    else if(increment != -1){
    
        //recoverable index
        if(local_index.size() > increment){
        
            meta.index = local_index[increment];
        }
        else{
            meta.index = index_count;
            //u0.push_back(this->meta.index);
            local_index.push_back(index_count);
            index_count = index_count + 1;
        }
    }

    this->meta.faulty_old = 0;

        //u0.push_back(var_name);
        //u0[var_name] = this->meta.index;
        //u1[this->meta.index] = var_name;

        //if it's global store its metadata pointer

        //meta.data_ptr = &words_[0];
        meta.n = n_;
        //u3.push_back(&meta);
    //}

    if((this->meta.faulty_new == 1) && (this->meta.faulty_old == 0)){
        this->meta.faulty_old = 1; //prevent =operator from double reporting
        //std::cout << "Fault At: " << u1[this->meta.index] << " by " << u1[this->meta.genetics] << " due to " << u1[this->meta.memetics] << " at " << cycle << "\n";

    }
    else if((this->meta.faulty_new == 0) && (this->meta.faulty_old == 1)){
        this->meta.faulty_old = 0;
        //std::cout << u1[this->meta.index] << " was corrected at " << cycle << "\n";
    }
    else this->meta.faulty_old = 0;

    if(is_glob || struct_var) {
        idx_width[this->meta.index] = w_;
    }

    //u2[this->meta.index] = this->meta.faulty_old;
  }



  //void name_update_prefix(string file_name, string prefix, string var_name){
  void name_update_prefix(int a = -1){

    if(!first_var){
        load_faults();
        first_var = 1;
    }
    this->meta.faulty_new = 0;
    this->meta.faulty_old = 0;
    //string to_pass;

    //if(prefix.size() != 0){
        //to_pass = prefix;
        //to_pass.append(".");

        struct_var = 1;
        /*auto got = struct_name.find(prefix);
        if(got != struct_name.end()){
            this->struct_index = got->second;
        }
        else{
            this->struct_index = struct_name.size();
            struct_name[prefix] = struct_index;
            struct_name_string[struct_index] = prefix;
        }*/

    //}
    //else{
        //struct_var = 1;
        /*auto got = struct_name.find(prefix);
        if(got != struct_name.end()){
            this->struct_index = got->second;
        }
        else{
            this->struct_index = struct_name.size();
            struct_name[file_name] = struct_index;
            struct_name_string[struct_index] = file_name;
        }*/
    //}

    //to_pass.append(var_name);
    //name_update(to_pass);
    name_update(a);

  }

  //void name_update_num(string prefix, string var_name, int a){
  void name_update_num(){
    //const char to_find[2] = "[";
    //size_t location = 0;

    if(!first_var){
        load_faults();
        first_var = 1;
    }

    struct_var = 1;
    /*auto got = struct_name.find(prefix);
    if(got != struct_name.end()){
        this->struct_index = got->second;
    }
    else{
        this->struct_index = struct_name.size();
        struct_name[prefix] = struct_index;
        struct_name_string[struct_index] = prefix;
    }*/

    this->meta.faulty_new = 0;
    this->meta.faulty_old = 0;

    //string temp;

    //location = var_name.find(to_find);

    /*if(location != string::npos){

        temp = prefix;
        temp.append(".");
        string to_pass = var_name.substr(0, location);
        to_pass.append("[");
        to_pass.append(std::to_string(a));
        to_pass.append("]");
        temp.append(to_pass);
        name_update(temp);
    }
    else printf("An error occurred in the parser phase\n");*/

    name_update(-1);
  }

  //void name_update_glob(string var_name){
  void name_update_glob(int a = -1){
    this->meta.faulty_new = 0;
    this->meta.faulty_old = 0;

    //name_update(var_name);
    name_update(a);
  }

  //Only perform the injection op on the data
  //AND NOT origData
  /*void op_inject(met_t * to_inject, char op){

    switch(op){
    case '^':
        for (int i = 0; i < n_; i++) this->words_[i] = this->words_[i] ^ other.words_[i];
        break;
    case '&':
        for (int i = 0; i < n_; i++) this->words_[i] = this->words_[i] & other.words_[i];
        break;
    case '0':
        for (int i = 0; i < n_; i++) this->words_[i] = 0;
        break;
    case '|':
        for (int i = 0; i < n_; i++) this->words_[i] = this->words_[i] | other.words_[i];
        break;
    case 'p':
        for (int i = 0; i < n_; i++) this->perm_bits[i] = other.words_[i];
        break;
    default:
        printf("How'd you get here? (op_inject switch)\n");
        break;
    }
  }*/

  //template<int w>
  void fault_injection(void * to_fault, fault_t meta_data) {
    met_t * to_inject = (met_t *)to_fault;
    word_t * data_ptr = (word_t*)to_fault_map[meta_data.index].first;
    word_t * data_ptr_origdata = (word_t*)to_fault_map[meta_data.index].second;
    to_inject->faulty_new = 1;
    to_inject->faulty_old = 1;
    to_inject->memetics = meta_data.index;
    to_inject->genetics = meta_data.index;
//    std::cout<<"fault variable index: "<<meta_data.index;
    //to_inject->operation = meta_data.op[0];
    //to_fault->meta.faulty_new = 1;
    //to_fault->meta.faulty_old = 1;
    //to_fault->meta.memetics = to_fault->meta.index;
    //to_fault->meta.genetics = to_fault->meta.index;
    //to_fault->meta.operation = op;

    char op = meta_data.op[0];
    if(meta_data.perm) perm_op = meta_data.op[0];

    //get the var name
    int line_count = 1;
    int index_temp = 0;

    /*std::fstream signals;
    string r_line;
    std::string::size_type sz;
    signals.open("struct_regs.txt", std::ios::in);
    if(signals.is_open()){
        getline(signals, r_line);
        if(r_line != "18623009") printf("Opened struct_regs.txt but could not find magic key\n");
    }
    else printf("Could not open struct_regs.txt!\n");
    while(getline(signals, r_line)){
        line_count = line_count + 1;
        if(line_count == (meta_data.line_num-1)) break;
    }*/


    if(meta_data.perm && (perm_bits.size() == 0)){
        //to_inject->perm_bits = new long int [(meta_data.width/64) + 1]; //allocate space for the bits to fault
        for (int i = 0; i < to_inject->n; i++) perm_bits.push_back(meta_data.value[i]); //copy the bits into the allocated space
        to_inject->perm_fault = 1;
        std::cout << "Permanent fault Injected in " << meta_data.index << " at " << cycle <<"\n";
    }
    else to_inject->perm_fault = 0;

    //TO SHUBHAM: you can put the copy from words -> origData operation HERE
    
    for(int i = 0; i < n_; i++) {
        data_ptr_origdata[i] = data_ptr[i]; 
    }
    faultDesc fault_desc(cycle, this->is_glob, this->struct_var, faultTrack::FAULTED);
    fault_desc.src = meta_data.index;
    for(int i = 1; i < n_; i++) 
        cout<<"Faulty metadata value is: "<<*meta_data.value<<"\n";
    vector<faultDesc> new_fault;
    new_fault.push_back(fault_desc);
    auto ret = fault_list.insert({meta_data.index, new_fault});
    if(!ret.second) 
        ret.first->second.push_back(fault_desc);
    //----------------------------------------------------------------------

    //op_inject(meta_data.value, op);

    switch(op){
    case '^':
        for (int i = 0; i < n_; i++) data_ptr[i] = data_ptr[i] ^ meta_data.value[i];
        break;
    case '&':
        //std::cout << "\nAND Original Data:\n ";
        //for (int i = 0; i < n_; i++) std::cout << data_ptr[i] << " ";

        for (int i = 0; i < n_; i++) data_ptr[i] = data_ptr[i] & meta_data.value[i];

        //std::cout << "\nAND Fault Data:\n ";
        //for (int i = 0; i < n_; i++) std::cout << data_ptr[i] << " ";
        break;
    case '0':
        for (int i = 0; i < n_; i++) data_ptr[i] = 0;
        break;
    case '1':
        for (int i = 0; i < n_; i++) data_ptr[i] = 1;
        break;
    case '|':

        //std::cout << "\nOR Original Data:\n ";
        //for (int i = 0; i < n_; i++) std::cout << data_ptr[i] << " ";

        for (int i = 0; i < n_; i++) data_ptr[i] = data_ptr[i] | meta_data.value[i];

        //std::cout << "\nOR Fault Data:\n ";
        //for (int i = 0; i < n_; i++) std::cout << data_ptr[i] << " ";
        break;


    default:
        printf("How'd you get here? (op_inject switch)\n");
        break;
    }

    if(!meta_data.perm) std::cout << "Fault Injected in " << meta_data.index << " at " << cycle <<"\n";
}

  //-----------------------------------------------------
};



template<int w>
std::ostream& operator<<(std::ostream& os, const UInt<w>& ui) {
  ui.print_to_stream(os);
  os << "<U" << w << ">";
  return os;
}

//------------------------------------------------------------

//TO BE REPLACED WITH SST::NOW
void update_cycle(){
    cycle = cycle + 1;
}



template<int w>
void fault_injection(SInt<w> & to_fault, UInt<w> bits, char op, char perm){
    to_fault.debug_faulty_update(bits, op, perm);
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


/*void load_faults(std::vector<unsigned long> idx, std::vector<int> start_cycle, std::vector<int> end_cycle, std::vector<int> bit_position) {

    std::ifstream frin("fr.txt");
    std::string r;
    while (std::getline(frin,r)){
        fr.push_back(std::strtoul(r.c_str(),NULL,0));
    }

    //in the final version this will be passed to the function as a string from the SST parameter list
    char * trimmed_name;
    int index = 0;
    FILE * fault_ptr;
    fault_t fault, fault_;
    char location[256];
    size_t line_size = 0;   //indicate that getline should d-allocate space for line
    char * line = NULL;     //return of getline()
    char * token = NULL;    //return of strtok()
    size_t line_len = 0;    //length of line from getline()
    string trimmed_name_to_find;
    char buff[32];
    size_t str_size = 0;
    size_t i = 0;

    int line_count = 1;
    std::fstream signals;
    string r_line;
    std::string::size_type sz;


    //map<string, int>::iterator it;

    fault_ptr = fopen(fault_file.c_str(), "r");
    if(fault_ptr == NULL) {
        printf("Could not open fault file\n");
        return;
    }

  for (int i = 0; i < idx.size(); ++i){

    printf("FI in %lu between cycle %d and cycle %d.\n", idx[i], start_cycle[i], end_cycle[i]);
    if(start_cycle[i] != -1) {
        fault_.perm = 1;
        fault_.cycle = start_cycle[i];
        fault_.end_cycle = end_cycle[i];
        if (g1v[i] == 0){
            auto it = idx_width.find(idx[i]);
            if(it == idx_width.end()){
                return;

            }
            else{ 
                fault_.width = it->second;
                wv[i] = fault_.width;
                if (bit_position[i] > wv[i]-1) { printf("\nEXIT: You provided bit position %d but the specified register %lu is %d bits.\nPlease specify a position between 0 and %d inclusive.",bit_position[i],idx[i],wv[i],wv[i]-1); exit(1); }
                wv[i] = bit_position[i];
                g1v[i] = 1;
            }
        }
        printf("\nIndex width of %lu is %d bits.", idx[i], fault_.width);


// ************************************ 
// This determines how faults are manifest. This is not necessarily correct (05/10/22) and this (and dependencies) should be removed, and how FI is done should be completely within fault_injection().
// ie Within the switch-case of fault_injection(), operate directly on bits of data[] to change value  according to what is called
// fault_.op[0] immediately below (which itself should be turned into a program argument). e.g. & is supposed to be stuckat0 and | is supposed to be sa1.
        fault_.op[0] = '&';
        int rand_width = wv[i];
        if(wv[i] % 64 == 0) {
            fault_.value  =  new long unsigned int [(wv[i]/64)];
            for(int i = 0; i < (wv[i]/64); i++) {
                if(wv[i]/64 == i){ 
                    fault_.value[i] = ~((long unsigned int)(1 << wv[i]));//0
//                    fault_.value[i] = (long unsigned int)(1 << (wv[i]-1));//1
               }
              else {
                    long unsigned int t = 0;
                    fault_.value[i] = ~t;
                }
            }
        }
        else {
            fault_.value =  new long unsigned int [(wv[i]/64) + 1];          
            for(int i = 0; i < (wv[i]/64 + 1); i++) {
                if(wv[i]/64 == i){
                    fault_.value[i] = ~((long unsigned int)(1 << wv[i]));//0
//                    fault_.value[i] = (long unsigned int)(1 << (wv[i]-1));//1
                }
                else {
                    long unsigned int t = 0;
                    fault_.value[i] = ~t;
                }
            }
        }      
        fault_.index = idx[i];
        fault_.var_name = "";
        printf("\nFault at bit %d of %lu pushed to fault list.", wv[i], idx[i]);
        //fault_list_inject.push_back(fault_);
    }
// ************************************ 
  } //end fault loop (loop over idx.size())

}*/

void load_faults(unsigned long idx, int start_cycle, int end_cycle, int bit_position) {

    std::ifstream frin("fr.txt");
    std::string r;
    while (std::getline(frin,r)){
        fr.push_back(std::strtoul(r.c_str(),NULL,0));
    }

    //in the final version this will be passed to the function as a string from the SST parameter list
    char * trimmed_name;
    int index = 0;
    FILE * fault_ptr;
    fault_t fault, fault_;
    char location[256];
    size_t line_size = 0;   //indicate that getline should d-allocate space for line
    char * line = NULL;     //return of getline()
    char * token = NULL;    //return of strtok()
    size_t line_len = 0;    //length of line from getline()
    string trimmed_name_to_find;
    char buff[32];
    size_t str_size = 0;
    size_t i = 0;

    int line_count = 1;
    std::fstream signals;
    string r_line;
    std::string::size_type sz;


    //map<string, int>::iterator it;

    fault_ptr = fopen(fault_file.c_str(), "r");
    if(fault_ptr == NULL) {
        printf("Could not open fault file\n");
        return;
    }
    printf("FI between cycle %d and cycle %d.", start_cycle, end_cycle);
    if(start_cycle != -1) {
        fault_.perm = 1;
        fault_.cycle = start_cycle;
        fault_.end_cycle = end_cycle;
        if (g1 == 0){
            auto it = idx_width.find(idx);
            if(it == idx_width.end()){
                return;

            }
            else{ 
                fault_.width = it->second;
                w = fault_.width;
                if (bit_position > w-1) { printf("\nEXIT: You provided bit position %d but the specified register %lu is %d bits.\nPlease specify a position between 0 and %d inclusive.",bit_position,idx,w,w-1); exit(1); }
                w = bit_position;
                g1 = 1;
            }
        }
        printf("\nIndex width is %d bits.", fault_.width);



// ************************************ 
// This determines how faults are manifest. This is not necessarily correct (05/10/22) and this (and dependencies) should be removed, and how FI is done should be completely within fault_injection().
// ie Within the switch-case of fault_injection(), operate directly on bits of data[] to change value  according to what is called
// fault_.op[0] immediately below (which itself should be turned into a program argument). e.g. & is supposed to be stuckat0 and | is supposed to be sa1.
        fault_.op[0] = '&';
        int rand_width = w;
        if(w % 64 == 0) {
            fault_.value  =  new long unsigned int [(w/64)];
            for(int i = 0; i < (w/64); i++) {
                if(w/64 == i){ 
                    fault_.value[i] = ~((long unsigned int)(1 << w));//0
//                    fault_.value[i] = (long unsigned int)(1 << (w-1));//1
               }
              else {
                    long unsigned int t = 0;
                    fault_.value[i] = ~t;
                }
            }
        }
        else {
            fault_.value =  new long unsigned int [(w/64) + 1];          
            for(int i = 0; i < (w/64 + 1); i++) {
                if(w/64 == i){
                    fault_.value[i] = ~((long unsigned int)(1 << w));//0
//                    fault_.value[i] = (long unsigned int)(1 << (w-1));//1
                }
                else {
                    long unsigned int t = 0;
                    fault_.value[i] = ~t;
                }
            }
        }      
        fault_.index = idx;
        fault_.var_name = "";
        printf("\nFault at bit %d pushed to fault list.", w);
        //fault_list_inject.push_back(fault_);
    }
// ************************************ 


}

void load_faults(unsigned long idx, int start_cycle, int end_cycle, int bit_position, unsigned long idx2, int start_cycle2, int end_cycle2, int bit_position2) {

    std::ifstream frin("fr.txt");
    std::string r;
    while (std::getline(frin,r)){
        fr.push_back(std::strtoul(r.c_str(),NULL,0));
    }

    //in the final version this will be passed to the function as a string from the SST parameter list
    char * trimmed_name;
    int index = 0;
    FILE * fault_ptr;
    fault_t fault, fault_, fault2_;
    char location[256];
    size_t line_size = 0;   //indicate that getline should d-allocate space for line
    char * line = NULL;     //return of getline()
    char * token = NULL;    //return of strtok()
    size_t line_len = 0;    //length of line from getline()
    string trimmed_name_to_find;
    char buff[32];
    size_t str_size = 0;
    size_t i = 0;

    int line_count = 1;
    std::fstream signals;
    string r_line;
    std::string::size_type sz;


    //map<string, int>::iterator it;

    fault_ptr = fopen(fault_file.c_str(), "r");
    if(fault_ptr == NULL) {
        printf("Could not open fault file\n");
        return;
    }

    printf("FI in %lu between cycle %d and cycle %d.\n", idx, start_cycle, end_cycle);
    printf("FI in %lu between cycle %d and cycle %d.\n", idx2, start_cycle2, end_cycle2);

    if(start_cycle != -1) {
        fault_.perm = 1;
        fault2_.perm = 1;
        fault_.cycle = start_cycle;
        fault_.end_cycle = end_cycle;
        fault2_.cycle = start_cycle2;
        fault2_.end_cycle = end_cycle2;

        if (g1 == 0){
            auto it = idx_width.find(idx);
            if(it == idx_width.end()){
                return;

            }
            else{ 
                fault_.width = it->second;
                w = fault_.width;
                if (bit_position > w-1) { printf("\nEXIT: You provided bit position %d but the specified register %lu is %d bits.\nPlease specify a position between 0 and %d inclusive.",bit_position,idx,w,w-1); exit(1); }
                w = bit_position;
                g1 = 1;
            }
        }
        if (g2 == 0){
            auto it2 = idx_width.find(idx2);
            if(it2 == idx_width.end()){
                return;

            }
            else{ 
                fault2_.width = it2->second;
                w2 = fault2_.width;
                if (bit_position2 > w2-1) { printf("\nEXIT: You provided bit position %d but the specified register %lu is %d bits.\nPlease specify a position between 0 and %d inclusive.",bit_position2,idx2,w2,w2-1); exit(1); }
                w2 = bit_position2;
                g2 = 1;
            }

        }
        printf("\nIndex width of %lu is %d bits.", idx, fault_.width);
        printf("\nIndex width of %lu is %d bits.", idx2, fault2_.width);


// ************************************ 
// This determines how faults are manifest. This is not necessarily correct (05/10/22) and this (and dependencies) should be removed, and how FI is done should be completely within fault_injection().
// ie Within the switch-case of fault_injection(), operate directly on bits of data[] to change value  according to what is called
// fault_.op[0] immediately below (which itself should be turned into a program argument). e.g. & is supposed to be stuckat0 and | is supposed to be sa1.
        fault_.op[0] = '&';
        int rand_width = w;
        if(w % 64 == 0) {
            fault_.value  =  new long unsigned int [(w/64)];
            for(int i = 0; i < (w/64); i++) {
                if(w/64 == i){ 
                    fault_.value[i] = ~((long unsigned int)(1 << w));//0
//                    fault_.value[i] = (long unsigned int)(1 << (w-1));//1
               }
              else {
                    long unsigned int t = 0;
                    fault_.value[i] = ~t;
                }
            }
        }
        else {
            fault_.value =  new long unsigned int [(w/64) + 1];          
            for(int i = 0; i < (w/64 + 1); i++) {
                if(w/64 == i){
                    fault_.value[i] = ~((long unsigned int)(1 << w));//0
//                    fault_.value[i] = (long unsigned int)(1 << (w-1));//1
                }
                else {
                    long unsigned int t = 0;
                    fault_.value[i] = ~t;
                }
            }
        }      
        fault_.index = idx;
        fault_.var_name = "";
        printf("\nFault at bit %d of %lu pushed to fault list.", w, idx);
        fault_list_inject.push_back(fault_);


        fault2_.op[0] = '&';
        int rand_width2 = w2;
        if(w2 % 64 == 0) {
            fault2_.value  =  new long unsigned int [(w2/64)];
            for(int i = 0; i < (w2/64); i++) {
                if(w2/64 == i){ 
                    fault2_.value[i] = ~((long unsigned int)(1 << w2));//0
//                    fault2_.value[i] = (long unsigned int)(1 << (w2-1));//1
               }
              else {
                    long unsigned int t2 = 0;
                    fault2_.value[i] = ~t2;
                }
            }
        }
        else {
            fault2_.value =  new long unsigned int [(w2/64) + 1];          
            for(int i = 0; i < (w2/64 + 1); i++) {
                if(w2/64 == i){
                    fault2_.value[i] = ~((long unsigned int)(1 << w2));//0
//                    fault2_.value[i] = (long unsigned int)(1 << (w2-1));//1
                }
                else {
                    long unsigned int t2 = 0;
                    fault_.value[i] = ~t2;
                }
            }
        }      
        fault2_.index = idx2;
        fault2_.var_name = "";
        printf("\nFault at bit %d of %lu pushed to fault list.", w2, idx2);
        //fault_list_inject.push_back(fault2_);

    }
// ************************************ 

}

void load_faults(){

    std::ifstream frin("fr.txt");
    std::string r;
    while (std::getline(frin,r)){
        fr.push_back(std::strtoul(r.c_str(),NULL,0));
    }

    //in the final version this will be passed to the function as a string from the SST parameter list
    char * trimmed_name;
    int index = 0;
    FILE * fault_ptr;
    fault_t fault, fault_;
    char location[256];
    size_t line_size = 0;   //indicate that getline should d-allocate space for line
    char * line = NULL;     //return of getline()
    char * token = NULL;    //return of strtok()
    size_t line_len = 0;    //length of line from getline()
    string trimmed_name_to_find;
    char buff[32];
    size_t str_size = 0;
    size_t i = 0;

    int line_count = 1;
    std::fstream signals;
    string r_line;
    std::string::size_type sz;


    //map<string, int>::iterator it;

    fault_ptr = fopen(fault_file.c_str(), "r");
    if(fault_ptr == NULL) {
        printf("Could not open fault file\n");
        return;
    }

    signals.open("struct_regs.txt", std::ios::in);
    if(signals.is_open()){
        getline(signals, r_line);
        if(r_line != "18623009") printf("Opened struct_regs.txt but could not find magic key\n");
    }
    else printf("Could not open struct_regs.txt!\n");

    while((line_len = getline(&line, &line_size, fault_ptr) != -1)){
            std::cout << line << std::endl;
            //check if first character is '#' or newline (empty line)
            if((line[0] == '#') || (line[0] == '\n')) continue;
            //else read fault file and append instruction to header
            else{

                //get location
                token = strtok(line, ",");
                strcpy(location, token);
                trimmed_name = trimspaces(location);
                trimmed_name_to_find = trimmed_name;

                fault.var_name = trimmed_name_to_find;

                size_t found = trimmed_name_to_find.find("["); //check for array
                if(found != std::string::npos){

                    std::stringstream check1(trimmed_name_to_find);
                    string intermediate;

                    getline(check1, intermediate, '[');
                    trimmed_name_to_find = intermediate + "[";
                    while(getline(signals, r_line)){
                        line_count = line_count + 1;
                        if(r_line[0] == '#') continue;
                        if(r_line.find(trimmed_name_to_find) != std::string::npos){
                            getline(check1, intermediate, ']');
                            fault.line_num = line_count;
                            getline(signals, r_line); //skip width
                            getline(signals, r_line); //get array count
                            //getline(signals, r_line); //get occurence count
                            //getline(signals, r_line); //index
                            fault.index = stoi(r_line, &sz);
                            //auto got = idx_map.find(stoi(r_line, &sz));
                            //if(got != idx_map.end()) {
                            //fault.index = stoi(r_line, &sz);
                            //}
                            //else { std::cout<<"\nIndex mapping not available! Exiting.."; exit(0);}
                            break;
                        }
                        getline(signals, r_line); //skip width
                        getline(signals, r_line); //get array count
                        //getline(signals, r_line); //get occurence count
                        //getline(signals, r_line); //index
                        line_count = line_count + 2;

                    }
                }
                else{
                    while(getline(signals, r_line)){
                        line_count = line_count + 1;
                        if(r_line[0] == '#') continue;
                        if(trimmed_name_to_find == r_line){
                            fault.line_num = line_count;
                            getline(signals, r_line); //skip width
                            getline(signals, r_line); //get array count
                            //getline(signals, r_line); //get occurence count
                            //getline(signals, r_line); //index
                            //auto got = idx_map.find(stoi(r_line, &sz));
                            //if(got != idx_map.end()) {
                            fault.index = stoi(r_line, &sz);
                            //}
                            //else { std::cout<<"\nIndex mapping not available! Exiting.."; exit(0);}
                            //fault.index = stoi(r_line, &sz);
                            break;
                        }
                        getline(signals, r_line); //skip width
                        getline(signals, r_line); //get array count
                        //getline(signals, r_line); //get occurence count
                        //getline(signals, r_line); //index
                        line_count = line_count + 2;
                    }
                }

                signals.clear();
                signals.seekg(0);
                trimmed_name_to_find.clear();
                line_count = 1;



                //get temporal flag
                token = strtok(NULL, ",");
                fault.perm = (int)strtol(token, NULL, 10);

                //get cycle
                token = strtok(NULL, ",");
                fault.cycle = (int)strtol(token, NULL, 10);

                //get width
                token = strtok(NULL, ",");
                fault.width = (int)strtol(token, NULL, 10);

                //get value
                token = strtok(NULL, ",");
                fault.value = new long unsigned int [(fault.width/64) + 1];

                char * to_trim;
                to_trim = token;
                to_trim = trimspaces(to_trim);

                str_size = strlen(to_trim);

                //op is zero
                if(str_size < 3){
                    delete [] fault.value;
                }
                //otherwise
                else{
                    if(((str_size-3)% 16) == 0) str_size = str_size - 1;
                    str_size = ((str_size-3)/16) + 1;

                    i = 0;

                    while(i<str_size){
                        memset(buff, 0, 32);
                        strncpy(buff, &to_trim[(16*i)+2], 16);
                        strncpy(buff, &to_trim[(16*i)+2], 16);
                        fault.value[i] = strtol(buff, NULL, 16);
                        i++;
                    }
                }

                //a flag should be used to skip the last part if the above condition was true

                //fault.value = strtol(token, NULL, 16);
                //get op
                token = strtok(NULL, ",");
                if(token[0] == ' ') fault.op[0] = token[1];
                else fault.op[0] = token[0];
                fault.end_cycle = -1;
                fault_list_inject.push_back(fault);
            }
    }
    delete [] line;
    fclose(fault_ptr);
    signals.close();
}

void check_faults() {

    int cnt = 0;

    while(cnt != fault_list_inject.size()) {
//        std::cout<<"\nfault_list_inject is not empty";
        std::cout.flush();

        if(((fault_list_inject[cnt].cycle <= cycle) && (fault_list_inject[cnt].end_cycle > cycle)) || (fault_list_inject[cnt].cycle == -1)) {
            //if the UInt object is less than 8 bits than the words_t type will be made of
            auto it = u3.find(fault_list_inject[cnt].index);
            //uint8_ts, otherwise it will be uint64_ts. This affects how we cast the meta_t pointer
  //          std::cout<<"\nfault_list_inject condition satisfied in check_faults()";
            std::cout.flush();
            if(fault_list_inject[cnt].width > 8){
                UInt<32> temp_large;
    //            std::cout<<"\nBefore u3 in check_faults";
                std::cout.flush();
      //          if(!fault_list_inject.empty()) 
      //              printf("\nFault list not empty");
                auto it = u3.find(fault_list_inject[cnt].index);
                if(it != u3.end()) { 
        //            std::cout<<"\nfault is getting injected (temp_small)";
                    std::cout.flush();
                    temp_large.fault_injection(u3[fault_list_inject[cnt].index], fault_list_inject[cnt]);
                }
            }
            else{
                UInt<1> temp_small;
        //        std::cout<<"\nBefore u3 in check_faults";
                std::cout.flush();
                auto it = u3.find(fault_list_inject[cnt].index);
    //            if(!fault_list_inject.empty()) 
      //              printf("\nFault list not empty");
                if(it != u3.end()) {
  //                  std::cout<<"\nfault is getting injected (temp_small)";
                    std::cout.flush();
                    temp_small.fault_injection(u3[fault_list_inject[cnt].index], fault_list_inject[cnt]);
                }
            }
            delete [] fault_list_inject[cnt].value;

        }

        cnt = cnt + 1;
    }
}

void print_fault_list() {
    ofstream fout;
    system("rm fault_tracking.log");
    fout.open("fault_tracking.log");
    fout<<"***************FAULT_METADATA*****************\n";
    for(auto it = fault_list.begin(); it != fault_list.end(); ++it) {
        for(auto v = it->second.begin(); v != it->second.end(); ++v) {
            if((*v).is_glob || (*v).struct_var) {
                fout<<it->first<<" ";
                fout<<(*v).when <<" "<<(*v).src<<" ";
                if((*v).status == faultTrack::FAULTED) 
                    fout<<" FAULTED";
                else if((*v).status == faultTrack::MATH_CORRECTED) 
                    fout<<" MATH_CORRECTED";
                else if((*v).status == faultTrack::OVERWRITE_CORRECTED) 
                    fout<<" OVERWRITE_CORRECTED";
                else if((*v).status == faultTrack::AUTOCORRECTED) 
                    fout<<" AUTOCORRECTED";
                fout<<"\n";
            }
        }
        fout<<"\n";
    }
    fout.close();

    if (profile_flag == 1){
    fout.open("bit_widths.log");
    for (const auto& elt: idx_width){
        fout << elt.first << "," << elt.second << "\n";
    }
    fout.close();

    fout.open("activity.log");
    for(auto it = callcount.begin(); it != callcount.end(); ++it){
        fout << it->first << "," << it->second <<"\n";
    }
    fout.close();

    fout.open("writes.log");
    for (const auto& v : writes){
        fout << v.first << "\n";
        for(const auto& p : v.second) {
            fout << p << "\n";
        }
    }
    fout.close();
    }
 }


#if 0
 void open_golden(){
     golden.open("golden.txt", std::fstream::out);
 }
 void golden_update() {
     if((!chksum_flag)&&((cycle%1000==0) || (cycle==last_cycle))){
         golden << std::dec << cycle << " ";
         golden << chksum << " ";
         golden << "\n";
         chksum_flag = 1;
     }
     else if((chksum_flag)&&(cycle%1000 == 1)) chksum_flag = 0;
     
 }
#endif
//-------------------------------------------------------------
//-------------------------------------------------------------

#endif  // UINT_H_


