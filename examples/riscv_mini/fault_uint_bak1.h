#ifndef UINT_H_
#define UINT_H_

#include <algorithm>
#include <array>
#include <cinttypes>
#include <iomanip>
#include <iostream>
#include <random>
#include <type_traits>

//---------------------
#include <string.h>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <map>
#define TWO_ARG_CHECK() {result.meta.faulty_new=this->meta.faulty_new|other.meta.faulty_new;\
if(result.meta.faulty_new){if(this->meta.faulty_new){\
    result.meta.memetics=this->meta.memetics;result.meta.genetics=this->meta.index;}\
    else{result.meta.memetics=other.meta.memetics;\
    result.meta.genetics=other.meta.index;}}}
/*//#define READ_STRUCT_UPDATE(){if(struct_var) struct_read_usage_count[struct_index]++;\
if(other.struct_var) struct_read_usage_count[other.struct_index]++;}
//#define READ_STRUCT_UPDATE_ONE(){if(struct_var) struct_read_usage_count[struct_index]++;}
//if(struct_var) struct_write_usage_count[struct_index] = struct_write_usage_count[struct_index] + 1;*/
using std::vector;
using std::map;
using std::string;

long int cycle = 0; //to be replaced with SST::now

map<string, int> u0; //name, index
map<int, string> u1; //index, name
map<int, int>    u2; //index, faulty_old
map<int, void*>  u3; //index, void pointer (to be cast to met_t*)

//map<string, short> struct_name;
//map<short, string> struct_name_string;
//map<short, int> struct_write_usage_count;
//map<short, int> struct_read_usage_count;

typedef struct faults{

    //char location[64];
    int cycle;
    int perm;
    int width;
    long unsigned int * value;
    int index;
    char op[2];
}fault_t;

string cur_prefix;
//char prefix_count = 0;

char glob_init_flag = 0;
//---------------------


//Code additions by Alan delineated by hyphen line
//Some notes: EVERY function that's called on a
//declaration line needs to check for propagation!
//because these don't use the assignment operator
//------------------------------------------------

// Internal RNG
namespace {
  std::mt19937_64 rng64(14);
  uint64_t rng_leftover;
  uint64_t rng_bits_left = 0;
}

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
    for (int i=0; i < n_; i++)
      words_[i] = 0;

    meta.faulty_new = 0;
    meta.faulty_old = 0;
    meta.perm_fault = 0;
    meta.index = 0;

  }

  //DESTRUCTOR-------------------------
  //Need to handle meta data allocated memory
  ~UInt(){
    if(meta.perm_bits != NULL) delete [] meta.perm_bits;
  }
  //-----------------------------------

  //---------------------------------
  //Alan's GREAT copy constructor
  //This probably won't be needed once the naming system works
  //Could be useful to keep around for keeping track of original data
  UInt(const UInt &other){

    //READ_STRUCT_UPDATE();

    for (int i=0; i < n_; i++)
      words_[i] = other.words_[i];

    if(other.meta.faulty_new == 1){
        meta.faulty_new = 1;
        meta.memetics = other.meta.memetics; //pass inheritance
        meta.genetics = other.meta.genetics;
    }
    else{
        meta.faulty_new = 0;
        meta.faulty_old = 0;
    }
  }
  //---------------------------------

  UInt(uint64_t initial) : UInt() {
    words_[0] = initial;
    mask_top_unused();
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
      if (word * kWordSize >= input_bits)
        words_[word] = 0;
      else {
        int word_start = std::max(static_cast<int>(initial.length())
                                  - 16*(word+1), 0);
        int word_len = std::min(16, last_start - word_start);
        last_start = word_start;
        const std::string substr = initial.substr(word_start, word_len);
        words_[word] = static_cast<uint64_t>(std::stoul(substr, nullptr, 16));
      }
    }
  }

  // NOTE: reads words right to left so literal appears to be concatted
  UInt(std::array<word_t, n_> raw_input_reversed) {
    for (int i = 0; i < n_; i++)
      words_[i] = raw_input_reversed[n_ - i - 1];
    mask_top_unused();
  }



  template<int other_w>
  explicit UInt(const UInt<other_w> &other) {
    static_assert(other_w <= w_, "Can't copy construct from wider UInt");
    for (int word=0; word < n_; word++) {
      if (word < UInt<other_w>::NW)
        words_[word] = other.words_[word];
      else
        words_[word] = 0;
    }
    meta.faulty_new = 0;
    meta.faulty_old = 0;

    //READ_STRUCT_UPDATE();
  }

  void rand_init() {
    core_rand_init();
    mask_top_unused();
  }

  template<int out_w>
  UInt<cmax(w_,out_w)> pad() const {
    //READ_STRUCT_UPDATE_ONE();
    //---------------------------------------------------------
    UInt<cmax(w_,out_w)> result;
    result = UInt<cmax(w_,out_w)>(*this); //result is an intermediate value
    result.meta.faulty_new = this->meta.faulty_new;
    result.meta.memetics = this->meta.memetics;
    result.meta.genetics = this->meta.index;
    return result;
    //----------------------------------------------------------
    //return UInt<cmax(w_,out_w)>(*this);
  }

  template<int other_w>
  UInt<w_ + other_w> cat(const UInt<other_w> &other) const {
    //READ_STRUCT_UPDATE();
    UInt<w_ + other_w> to_return(other);
    const int offset = other_w % kWordSize;
    for (int i = 0; i < n_; i++) {
      to_return.words_[word_index(other_w) + i] |= static_cast<uint64_t>(words_[i]) <<
                                                     cap(offset);
      if ((offset != 0) && (i + 1 < to_return.NW - word_index(other_w)))
        to_return.words_[word_index(other_w) + i + 1] |= static_cast<uint64_t>(words_[i]) >>
                                                           cap(kWordSize - offset);
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
    //------------------------------------------------------------

    return to_return;
  }


  UInt<w_ + 1> operator+(const UInt<w_> &other) const {
    UInt<w_ + 1> result = core_add_sub<w_+1, false>(other);

    //READ_STRUCT_UPDATE();

    //-------------------------------------------------------
    TWO_ARG_CHECK();
    //-------------------------------------------------------

    if ((kWordSize * n_ == w_) && (result.words_[n_-1] < words_[n_-1]))
      result.words_[word_index(w_ + 1)] = 1;
    return result;
  }

  UInt<w_> addw(const UInt<w_> &other) const {
    UInt<w_> result = core_add_sub<w_, false>(other);
    result.mask_top_unused();
    return result;
  }

  UInt<w_> subw(const UInt<w_> &other) const {
    UInt<w_> result(core_add_sub<w_, true>(other.ui));
    result.mask_top_unused();
    return result;
  }

  SInt<w_ + 1> operator-() const {
    return SInt<w_+1>(0).subw(SInt<w_+1>(pad<w_+1>()));
  }

  UInt<w_ + 1> operator-(const UInt<w_> &other) const {
    UInt<w_ + 1> result = core_add_sub<w_+1, true>(other);
    //READ_STRUCT_UPDATE();
    //-------------------------------------------------------
    TWO_ARG_CHECK();
    //-------------------------------------------------------

    if (kWordSize * n_ == w_) {
      if (result.words_[n_-1] < other.words_[n_-1])
        result.words_[word_index(w_ + 1)] = 1;
    } else {
      result.mask_top_unused();
    }
    return result;
  }

  UInt<w_ + w_> operator*(const UInt<w_> &other) const {
    UInt<w_ + w_> result(0);
    //READ_STRUCT_UPDATE();
    //-------------------------------------------------------
    TWO_ARG_CHECK();
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
    return result;
  }

//--------------------------------------------
//ALAN'S PATENTED ASSIGNMENT OPERATOR OVERLOAD
  UInt<w_> &operator=(const UInt<w_> &other){

    //if(struct_var) struct_write_usage_count[struct_index] = struct_write_usage_count[struct_index] + 1;

    this->meta.faulty_new = other.meta.faulty_new;
    if(this->meta.faulty_old != this->meta.faulty_new){
        if(this->meta.faulty_old == 0){
            this->meta.memetics = other.meta.memetics;
            this->meta.genetics = other.meta.genetics;
            //if(is_glob) std::cout << "*";
            //std::cout << "Fault At: " << u1[this->meta.index] << " by " << u1[this->meta.genetics] << " due to " << u1[this->meta.memetics] << " at "<< cycle << "\n";
        }
        else{
            if(!meta.perm_fault){
                if(is_glob) std::cout << "*";
                //std::cout << u1[this->meta.index] << " was corrected" << " at " << cycle <<"\n";
            }
        }
    }
    this->meta.faulty_old = this->meta.faulty_new;
    u2[this->meta.index] = this->meta.faulty_old;

    for (int i = 0; i < n_; i++) this->words_[i] = other.words_[i];
    if(meta.perm_fault == 1){
        meta.faulty_new = 1;
        meta.faulty_old = 1;

        switch(meta.operation){
        case '&':
            for (int i = 0; i < n_; i++) this->words_[i] = this->meta.perm_bits[i] & this->words_[i];
            break;
        case '|':
            for (int i = 0; i < n_; i++) this->words_[i] = this->meta.perm_bits[i] | this->words_[i];
            break;
        case '0':
            for (int i = 0; i < n_; i++) this->words_[i] = 0;
            break;
        case '^':
            std::cout << "Invalid operator for permanent fault '^' " << u1[this->meta.index] << "\n";
            break;
        default:
            std::cout << "You shouldn't be here (assignment operator perm effect) " << u1[this->meta.index] << "\n";
        }
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
    TWO_ARG_CHECK();
    //READ_STRUCT_UPDATE();
    //------------------------------------------------------------------
    //return UInt<w_>(as_single_word() / other.as_single_word());
  }

  // this % other
  template<int other_w>
  UInt<cmin(w_, other_w)> operator%(const UInt<other_w> &other) const {
    static_assert(w_ <= kWordSize, "Mod not supported beyond 64b");
    static_assert(other_w <= kWordSize, "Mod not supported beyond 64b");
    //------------------------------------------------------------------------
    UInt<w_> result = as_single_word() % other.as_single_word();
    TWO_ARG_CHECK();
    //READ_STRUCT_UPDATE();
    //------------------------------------------------------------------------
    //return UInt<cmin(w_, other_w)>(as_single_word() % other.as_single_word());
  }

  UInt<w_> operator~() const {
    UInt<w_> result;

    //-------------------------------------------------------
    //READ_STRUCT_UPDATE_ONE();
    result.meta.faulty_new = this->meta.faulty_new;
    result.meta.memetics = this->meta.memetics;
    result.meta.genetics = this->meta.index;
    //-------------------------------------------------------

    for (int i = 0; i < n_; i++) {
      result.words_[i] = ~words_[i];
    }
    result.mask_top_unused();
    return result;
  }

  UInt<w_> operator&(const UInt<w_> &other) const {
    UInt<w_> result;

    //-------------------------------------------------------
    //READ_STRUCT_UPDATE();
    TWO_ARG_CHECK();
    //-------------------------------------------------------

    for (int i = 0; i < n_; i++) {
      result.words_[i] = words_[i] & other.words_[i];
    }
    return result;
  }

  UInt<w_> operator|(const UInt<w_> &other) const {
    UInt<w_> result;

    //-------------------------------------------------------
    //READ_STRUCT_UPDATE();
    TWO_ARG_CHECK();
    //-------------------------------------------------------

    for (int i = 0; i < n_; i++) {
      result.words_[i] = words_[i] | other.words_[i];
    }
    return result;
  }

  UInt<w_> operator^(const UInt<w_> &other) const {
    UInt<w_> result;

    //-------------------------------------------------------
    //READ_STRUCT_UPDATE();
    TWO_ARG_CHECK();
    //-------------------------------------------------------

    for (int i = 0; i < n_; i++) {
      result.words_[i] = words_[i] ^ other.words_[i];
    }
    return result;
  }

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
    //--------------------------------------------
    //READ_STRUCT_UPDATE_ONE();
    result.meta.faulty_new = this->meta.faulty_new;
    result.meta.memetics = this->meta.memetics;
    result.meta.genetics = this->meta.index;
    //--------------------------------------------
    return result;
  }

  template<int n>
  UInt<n> head() const {
    static_assert(n <= w_, "Head n must be <= width");
    return bits<w_-1, w_-n>();
  }

  template<int n>
  UInt<w_ - n> tail() const {
    static_assert(n < w_, "Tail n must be < width");
    return bits<w_-n-1, 0>();
  }

  template<int shamt>
  UInt<w_ + shamt> shl() const {
    return cat(UInt<shamt>(0));
  }

  template<int shamt>
  UInt<w_> shlw() const {
    return shl<shamt>().template tail<shamt>();
  }

  template<int shamt>
  UInt<w_ - shamt> shr() const {
    return bits<w_-1, shamt>();
  }

  template<int other_w>
  UInt<w_> operator>>(const UInt<other_w> &other) const {
    UInt<w_> result(0);

    //-------------------------------------------------------
    //READ_STRUCT_UPDATE();
    TWO_ARG_CHECK();
    //-------------------------------------------------------

    uint64_t dshamt = other.as_single_word();
    uint64_t word_down = word_index(dshamt);
    uint64_t bits_down = dshamt % kWordSize;
    for (uint64_t i=word_down; i < n_; i++) {
      result.words_[i - word_down] = words_[i] >> bits_down;
      if ((bits_down != 0) && (i < n_-1))
        result.words_[i - word_down] |= words_[i + 1] << cap(kWordSize - bits_down);
    }
    return result;
  }

  template<int other_w>
  UInt<w_ + (1<<other_w) - 1> operator<<(const UInt<other_w> &other) const {
    UInt<w_ + (1<<other_w) - 1> result(0);

    //-------------------------------------------------------
    //READ_STRUCT_UPDATE();
    TWO_ARG_CHECK();
    //-------------------------------------------------------

    uint64_t dshamt = other.as_single_word();
    uint64_t word_up = word_index(dshamt);
    uint64_t bits_up = dshamt % kWordSize;
    for (uint64_t i=0; i < n_; i++) {
      result.words_[i + word_up] |= words_[i] << bits_up;
      if ((bits_up != 0) && (dshamt + w_ > kWordSize) && (i + word_up + 1 < result.NW))
        result.words_[i + word_up + 1] = words_[i] >> cap(kWordSize - bits_up);
    }
    return result;
  }

  template<int other_w>
  UInt<w_> dshlw(const UInt<other_w> &other) const {
    // return operator<<(other).template bits<w_-1,0>();
    UInt<w_> result(0);
    uint64_t dshamt = other.as_single_word();
    uint64_t word_up = word_index(dshamt);
    uint64_t bits_up = dshamt % kWordSize;
    for (uint64_t i=0; i + word_up < n_; i++) {
      result.words_[i + word_up] |= words_[i] << bits_up;
      if ((bits_up != 0) && (w_ > kWordSize) && (i + word_up + 1 < n_))
        result.words_[i + word_up + 1] = words_[i] >> cap(kWordSize - bits_up);
    }
    result.mask_top_unused();

    //-----------------------------------------------
    //READ_STRUCT_UPDATE();
    TWO_ARG_CHECK();
    //-----------------------------------------------

    return result;
  }

  UInt<1> operator<=(const UInt<w_> &other) const {
    //-----------------------------------------------------
    UInt<1>case_true = UInt<1>(1);
    UInt<1>case_false = UInt<1>(0);

    //READ_STRUCT_UPDATE();

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

    //READ_STRUCT_UPDATE();

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

    //READ_STRUCT_UPDATE();

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
    return UInt<w_>(*this);
  }

  SInt<w_> asSInt() const {
    SInt<w_> result(*this);

    //----------------------
    //READ_STRUCT_UPDATE_ONE();
    result.ui.meta.faulty_new = this->meta.faulty_new;
    result.ui.meta.faulty_old = this->meta.faulty_old;
    //----------------------

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

  // Reused math operators
  template<int out_w, bool subtract>
  UInt<out_w> core_add_sub(const UInt<w_> &other) const {
    UInt<out_w> result;

    //---------------------------------
    TWO_ARG_CHECK();
    //READ_STRUCT_UPDATE();
    //---------------------------------

    uint64_t carry = subtract;
    for (int i = 0; i < n_; i++) {
      uint64_t operand = subtract ? ~other.words_[i] : other.words_[i];
      result.words_[i] = words_[i] + operand + carry;
      carry = result.words_[i] < operand ? 1 : 0;
    }
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
      rng_leftover = rng_leftover >> cap(w_);
      rng_bits_left -= w_;
    } else {
      for (int word=0; word < n_; word++) {
        words_[word] = rng64();
      }
    }
  }

  template<int hi, int lo>
  UInt<hi - lo + 1> core_bits() const {
    static_assert(hi < w_, "Bit extract hi bigger than width");
    static_assert(hi >= lo, "Bit extract lo > hi");
    static_assert(lo >= 0, "Bit extract lo is negative");
    UInt<hi - lo + 1> result;
    //--------------------------------------
    result.meta.faulty_new = this->meta.faulty_new;
    result.meta.faulty_old = this->meta.faulty_old;
    //--------------------------------------
    int word_down = word_index(lo);
    int bits_down = lo % kWordSize;
    for (int i=0; i < result.NW; i++) {
      result.words_[i] = words_[i + word_down] >> bits_down;
      if ((bits_down != 0) && (i + word_down + 1 < n_))
        result.words_[i] |= words_[i + word_down + 1] << cap(kWordSize - bits_down);
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
  //NEW VARIABLES (need to be down here for some reason?)
  //Meta data struct, pointer to be passed stored in global vector
  typedef struct met{
    char faulty_new = 0;
    char faulty_old = 0;

    int index = 0;
    int memetics = 0; //when a value is faulted the original index that faulted it will be here
    int genetics = 0; //the index of the value that directly caused a fault

    bool perm_fault = 0;
    char operation = 0;
    long int * perm_bits = NULL; //if the fault is permanent store the bits here
    int n; //loop counter
    word_t * data_ptr;
  } met_t;

  bool struct_var = 0;
  //short struct_index = 0;

  met_t meta;     //holds meta data, in a struct so it can accessed by generic pointer
  bool is_glob = 0; //If a variable is global it should be noted as important to the user upon output
  short int width = 0;

  //name update function
  void name_update(string var_name){
    if(glob_init_flag) is_glob = 0;
    else is_glob = 1;

    meta.perm_fault = 0; //init this value

    width = (NW * WW) - (WW - bits_in_top_word_);

    auto got = u0.find(var_name);

    if(got != u0.end()){
        this->meta.index = got->second;
        this->meta.faulty_old = u2[this->meta.index];
    }
    else{
        this->meta.index = u0.size();
        this->meta.faulty_old = 0;
        u0[var_name] = this->meta.index;
        u1[this->meta.index] = var_name;

        //if it's global store its metadata pointer
        if(is_glob){
            meta.data_ptr = &words_[0];
            meta.n = n_;
            u3[meta.index] = &meta;
        }
    }

    if((this->meta.faulty_new == 1) && (this->meta.faulty_old == 0)){
        this->meta.faulty_old = 1; //prevent =operator from double reporting
        std::cout << "Fault At: " << u1[this->meta.index] << " by " << u1[this->meta.genetics] << " due to " << u1[this->meta.memetics] << " at " << cycle << "\n";

    }
    else if((this->meta.faulty_new == 0) && (this->meta.faulty_old == 1)){
        this->meta.faulty_old = 0;
        std::cout << u1[this->meta.index] << " was corrected at " << cycle << "\n";
    }
    else this->meta.faulty_old = 0;

    u2[this->meta.index] = this->meta.faulty_old;
  }



  void name_update_prefix(string file_name, string prefix, string var_name){
    this->meta.faulty_new = 0;
    this->meta.faulty_old = 0;
    string to_pass;

    if(prefix.size() != 0){
        to_pass = prefix;
        to_pass.append(".");

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

    }
    else{
        struct_var = 1;
        /*auto got = struct_name.find(prefix);
        if(got != struct_name.end()){
            this->struct_index = got->second;
        }
        else{
            this->struct_index = struct_name.size();
            struct_name[file_name] = struct_index;
            struct_name_string[struct_index] = file_name;
        }*/
    }

    to_pass.append(var_name);
    name_update(to_pass);

  }

  void name_update_num(string prefix, string var_name, int a){
    const char to_find[2] = "[";
    size_t location = 0;

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

    string temp;

    location = var_name.find(to_find);

    if(location != string::npos){

        temp = prefix;
        temp.append(".");
        string to_pass = var_name.substr(0, location);
        to_pass.append("[");
        to_pass.append(std::to_string(a));
        to_pass.append("]");
        temp.append(to_pass);
        name_update(temp);
    }
    else printf("An error occurred in the parser phase\n");
  }

  void name_update_glob(string var_name){
    this->meta.faulty_new = 0;
    this->meta.faulty_old = 0;

    name_update(var_name);
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
  void fault_injection(void * to_fault, fault_t meta_data){
  //void fault_injection(UInt<w> * to_fault, UInt<w> bits, char op, char perm){
    met_t * to_inject = (met_t *)to_fault;
    to_inject->faulty_new = 1;
    to_inject->faulty_old = 1;
    to_inject->memetics = meta_data.index;
    to_inject->genetics = meta_data.index;
    to_inject->operation = meta_data.op[0];
    //to_fault->meta.faulty_new = 1;
    //to_fault->meta.faulty_old = 1;
    //to_fault->meta.memetics = to_fault->meta.index;
    //to_fault->meta.genetics = to_fault->meta.index;
    //to_fault->meta.operation = op;

    if(meta_data.perm && (to_inject->perm_bits == NULL)){
        to_inject->perm_bits = new long int [(meta_data.width/64) + 1]; //allocate space for the bits to fault
        for (int i = 0; i < to_inject->n; i++) to_inject->perm_bits[i] = meta_data.value[i]; //copy the bits into the allocated space
        to_inject->perm_fault = 1;
        std::cout << "Permanent fault Injected in " << u1[to_inject->index] << " at " << cycle <<"\n";
    }
    else to_inject->perm_fault = 0;

    //TO SHUBHAM: you can put the copy from words -> origData operation HERE

    //----------------------------------------------------------------------

    //op_inject(meta_data.value, op);

    switch(to_inject->operation){
    case '^':
        for (int i = 0; i < n_; i++) to_inject->data_ptr[i] = to_inject->data_ptr[i] ^ meta_data.value[i];
        break;
    case '&':
        for (int i = 0; i < n_; i++) to_inject->data_ptr[i] = to_inject->data_ptr[i] & meta_data.value[i];
        break;
    case '0':
        for (int i = 0; i < n_; i++) to_inject->data_ptr[i] = 0;
        break;
    case '|':
        for (int i = 0; i < n_; i++) to_inject->data_ptr[i] = to_inject->data_ptr[i] | meta_data.value[i];
        break;
    default:
        printf("How'd you get here? (op_inject switch)\n");
        break;
    }

    if(!meta_data.perm) std::cout << "Fault Injected in " << u1[to_inject->index] << " at " << cycle <<"\n";
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

vector<fault_t> fault_list;
string fault_file;

void load_faults(){

    //in the final version this will be passed to the function as a string from the SST parameter list
    //char fault_file_path[] = "/media/adgardn2/3CCD-373B/fault_injection/essent_fault/essent_fault/fault_file.txt";
    char * trimmed_name;
    int index = 0;
    FILE * fault_ptr;
    fault_t fault;
    char location[64];
    size_t line_size = 0;   //indicate that getline should d-allocate space for line
    char * line = NULL;     //return of getline()
    char * token = NULL;    //return of strtok()
    size_t line_len = 0;    //length of line from getline()

    char buff[32];
    size_t str_size = 0;
    size_t i = 0;

    map<string, int>::iterator it;

    fault_ptr = fopen(fault_file.c_str(), "r");

    if(fault_ptr == NULL) {
        printf("Could not open fault file\n");
        return;
    }

    while((line_len = getline(&line, &line_size, fault_ptr) != -1)){

            //check if first character is '#' or newline (empty line)
            if((line[0] == '#') || (line[0] == '\n')) continue;
            //else read fault file and append instruction to header
            else{

                //get location
                token = strtok(line, ",");
                strcpy(location, token);
                trimmed_name = trimspaces(location);

                //get index of name
                it = u0.find(trimmed_name);
                if(it == u0.end()){
                    printf("BAD VARIABLE NAME %s\n", trimmed_name);
                    continue; //skip
                }
                fault.index = u0[trimmed_name];

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

                fault_list.push_back(fault);

            }
    }
    delete [] line;
    fclose(fault_ptr);
}

void check_faults(){

    int cnt = 0;

    while(cnt != fault_list.size()){

        if((fault_list[cnt].cycle == cycle) || fault_list[cnt].cycle == -1){
            //if the UInt object is less than 8 bits than the words_t type will be made of
            //uint8_ts, otherwise it will be uint64_ts. This affects how we cast the meta_t pointer
            if(fault_list[cnt].width > 8){
                UInt<32> temp_large;
                temp_large.fault_injection(u3[fault_list[cnt].index], fault_list[cnt]);
            }
            else{
                UInt<1> temp_small;
                temp_small.fault_injection(u3[fault_list[cnt].index], fault_list[cnt]);
            }
            delete [] fault_list[cnt].value;


        }

        cnt = cnt + 1;
    }
}


//-------------------------------------------------------------
//-------------------------------------------------------------

#endif  // UINT_H_

