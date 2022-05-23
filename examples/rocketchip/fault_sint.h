#ifndef SINT_H_
#define SINT_H_

#include <cinttypes>

#include "fault_uint.h"

template<int w_>
class SInt {
  private: // copied from uint.h
  constexpr static int cmin(int wa, int wb) { return wa < wb ? wa : wb; }
  constexpr static int cmax(int wa, int wb) { return wa > wb ? wa : wb; }

public:
  SInt() : ui(0) {}

  SInt(int64_t i) : ui(i) {
    if (w_ > kWordSize)
      sign_extend(kWordSize - 1);
    else
      sign_extend();
  }

  SInt(std::string initial) : ui(initial) {
    sign_extend();
  }

  // TODO: make array's template parameters somehow inherit from ui
  SInt(std::array<uint64_t, (w_+63)/64> raw_input_reversed) : ui(raw_input_reversed) {
    sign_extend();
  }

  template<int other_w>
  explicit SInt(const SInt<other_w> &other) {
    static_assert(other_w <= w_, "Can't copy construct from wider SInt");
    ui = UInt<w_>(other.ui);
    sign_extend(other_w - 1);
  }

  SInt(const UInt<w_> &other) : ui(other) {

    if (profile_flag == 1){
    if(other.meta.index != 0) {
        if(other.is_glob || other.struct_var) { 
            ++callcount[other.meta.index]; 
        }
    }
    }
  }


  void rand_init() {
    ui.core_rand_init();
    sign_extend();
  }

  template<int out_w>

  SInt<cmax(w_,out_w)> pad() const {
    //---------------------------------------------------------
    SInt<cmax(w_,out_w)> result;

    if(result.ui.meta.index == ULONG_MAX) {
      result.ui.name_update(-2);
    }

   if (profile_flag == 1){
   if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    }

    result = SInt<cmax(w_,out_w)>(*this); //result is an intermediate value
    SInt<cmax(w_,out_w)> tmp_res = SInt<cmax(w_,out_w)>(*this);
    int n = (cmax(w_,out_w) <= 8) ? 1 : (cmax(w_,out_w) + 64 - 1) / 64;

    result.ui.meta.faulty_new = this->ui.meta.faulty_new;
    result.ui.meta.memetics = this->ui.meta.memetics;
    result.ui.meta.genetics = this->ui.meta.index;

    /*for(int i = 0; i < n; i++) {
        if(tmp_res.ui.Origdata[i] != tmp_res.ui.words_[i]) {
            faultDesc fault_desc(cycle, this->ui.is_glob, this->ui.struct_var, faultTrack::FAULTED);
            fault_desc.src = this->ui.meta.index;
            auto it = fault_list.find(this->ui.meta.index);
            vector<faultDesc> new_fault;
            new_fault.push_back(fault_desc);
            auto ret = fault_list.insert({this->ui.meta.index, new_fault}); 
            if(!ret.second) 
                ret.first->second.push_back(fault_desc);
        }
        else {
            faultDesc fault_desc(cycle, this->ui.is_glob, this->ui.struct_var, faultTrack::MATH_CORRECTED);
            fault_desc.src = this->ui.meta.index;
            auto it = fault_list.find(this->ui.meta.index);
            vector<faultDesc> new_fault;
            new_fault.push_back(fault_desc);
            auto ret = fault_list.insert({this->ui.meta.index, new_fault}); 
            if(!ret.second) 
                ret.first->second.push_back(fault_desc);
        }

    }*/

    return result;
    //----------------------------------------------------------
    //return SInt<cmax(w_,out_w)>(*this);
  }

  template<int other_w>
  SInt<w_ + other_w> cat(const SInt<other_w> &other) const {
    UInt<other_w> other_ui = other.ui;
    other_ui.mask_top_unused();
    SInt<w_ + other_w> result(ui.cat(other_ui));
    result.sign_extend();

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {
        ++callcount[this->ui.meta.index];
    }
    if(other.ui.is_glob || other.ui.struct_var) {
        ++callcount[other.ui.meta.index];
    }
    }

    return result;
  }

  SInt<w_ + 1> operator+(const SInt<w_> &other) const {
    SInt<w_+1> result(ui.template core_add_sub<w_+1, false>(other.ui));


    if(result.ui.meta.index == ULONG_MAX) {
      result.ui.name_update(-2);
    }

    if (w_ % kWordSize == 0) {
      if (negative() == other.negative()) {
        result.ui.words_[ui.word_index(w_)] = negative() ? -1 : 0;
      } else {
        bool is_neg = static_cast<int64_t>(result.ui.words_[ui.word_index(w_-1)]) < 0;
        result.ui.words_[ui.word_index(w_)] = is_neg ? -1 : 0;
      }
    }
    return result;
  }

  SInt<w_> addw(const SInt<w_> &other) const {
    return ui.template core_add_sub<w_, false>(other.ui);
  }

  SInt<w_> subw(const SInt<w_> &other) const {
    SInt<w_> result(ui.template core_add_sub<w_, true>(other.ui));
    if(result.ui.meta.index == ULONG_MAX) {
      result.ui.name_update(-2);
    }

    result.sign_extend();
    return result;
  }

  SInt<w_ + 1> operator-() const {

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {
        ++callcount[this->ui.meta.index];
    }
    }

    return SInt<w_>(0) - *this;
  }

  SInt<w_ + 1> operator-(const SInt<w_> &other) const {
    SInt<w_ + 1> result(ui.template core_add_sub<w_+1, true>(other.ui));

    /*if(result.ui.meta.index == ULONG_MAX) {
      result.ui.name_update(-2);
    }*/

    if (w_ % kWordSize == 0) {
      if (negative() != other.negative()) {
        result.ui.words_[ui.word_index(w_)] = negative() ? -1 : 0;
      } else {
        bool is_neg = static_cast<int64_t>(result.ui.words_[ui.word_index(w_-1)]) < 0;
        result.ui.words_[ui.word_index(w_)] = is_neg ? -1 : 0;
      }
    }
    return result;
  }

  SInt<w_ + w_> operator*(const SInt<w_> &other) const {
    SInt<4*w_> product(pad<w_ + w_>().ui * other.pad<w_ + w_>().ui);
    SInt<w_ + w_> result = (product.template tail<w_ + w_>()).asSInt();

    if(result.ui.meta.index == ULONG_MAX) {
      result.ui.name_update(-2);
    }

    result.sign_extend();

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.ui.is_glob || other.ui.struct_var) { 
        ++callcount[other.ui.meta.index]; 
    }
    }

    return result;
  }

  template<int other_w>
  SInt<w_+1> operator/(const SInt<other_w> &other) const {

    static_assert(w_ <= kWordSize, "Div not supported beyond 64b");
    static_assert(other_w <= kWordSize, "Div not supported beyond 64b");

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.ui.is_glob || other.ui.struct_var) { 
        ++callcount[other.ui.meta.index]; 
    }
    }

    return SInt<w_+1>(as_single_word() / other.as_single_word());
  }

  template<int other_w>
  SInt<cmin(w_, other_w)> operator%(const SInt<other_w> &other) const {
    static_assert(w_ <= kWordSize, "Mod not supported beyond 64b");
    static_assert(other_w <= kWordSize, "Mod not supported beyond 64b");

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.ui.is_glob || other.ui.struct_var) { 
        ++callcount[other.ui.meta.index]; 
    }
    }

    return SInt<cmin(w_, other_w)>(as_single_word() % other.as_single_word());
  }

  UInt<w_> operator~() const {

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    }

    return ~ui;
  }

  UInt<w_> operator&(const SInt<w_> &other) const {
    UInt<w_> result = ui & other.ui;
    /*if(result.meta.index == ULONG_MAX) {
      result.name_update(-2);
    }*/

    result.mask_top_unused();

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.ui.is_glob || other.ui.struct_var) { 
        ++callcount[other.ui.meta.index]; 
    }
    }

    return result;
  }

  UInt<w_> operator|(const SInt<w_> &other) const {
    UInt<w_> result = ui | other.ui;
    if (profile_flag == 1){
    /*if(result.meta.index == ULONG_MAX) {
      result.name_update(-2);
    }*/
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.ui.is_glob || other.ui.struct_var) { 
        ++callcount[other.ui.meta.index]; 
    }
    }

    result.mask_top_unused();

    return result;
  }

  UInt<w_> operator^(const SInt<w_> &other) const {
    UInt<w_> result = ui ^ other.ui;

    /*if(result.meta.index == ULONG_MAX) {
      result.name_update(-2);
    }*/

    result.mask_top_unused();

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.ui.is_glob || other.ui.struct_var) { 
        ++callcount[other.ui.meta.index]; 
    }
    }

    return result;
  }

  UInt<1> andr() const {
    UInt<w_> upper_bits_clear = ui;
    upper_bits_clear.mask_top_unused();
    return upper_bits_clear.andr();
  }

  UInt<1> orr() const {
    UInt<w_> upper_bits_clear = ui;
    upper_bits_clear.mask_top_unused();
    return upper_bits_clear.orr();
  }

  UInt<1> xorr() const {
    UInt<w_> upper_bits_clear = ui;
    upper_bits_clear.mask_top_unused();
    return upper_bits_clear.xorr();
  }

  template<int hi, int lo>
  UInt<hi - lo + 1> bits() const {

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    }
    return ui.template bits<hi,lo>();

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
  SInt<w_ + shamt> shl() const {
    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index];
    }
    }
    return cat(SInt<shamt>(0));
  }

  template<int shamt>
  SInt<w_> shlw() const {
    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    }
    SInt<w_> result(ui.template shlw<shamt>());
    /*if(result.ui.meta.index == ULONG_MAX) {
      result.ui.name_update(-2);
    }*/

    result.sign_extend();
    return result;
  }

  template<int shamt>
  SInt<w_ - shamt> shr() const {

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    }
    SInt<w_ - shamt> result(ui.template core_bits<w_-1, shamt>());
    result.sign_extend(w_ - shamt - 1);
    return result;
  }

  template<int other_w>
  SInt<w_> operator>>(const UInt<other_w> &other) const {
    uint64_t dshamt = other.as_single_word();
    SInt<w_> result(ui >> other);
    /*if(result.ui.meta.index == ULONG_MAX) {
      result.ui.name_update(-2);
    }*/
    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.meta.index != ULONG_MAX) {
        if(other.is_glob || other.struct_var) { 
            ++callcount[other.meta.index]; 
        }   
    }
    }

    result.sign_extend(w_ - dshamt - 1);

    return result;
  }

  template<int other_w>
  SInt<w_ + (1<<other_w) - 1> operator<<(const UInt<other_w> &other) const {
    uint64_t dshamt = other.as_single_word();
    SInt<w_ + (1<<other_w) - 1> result(ui << other);
    /*if(result.ui.meta.index == ULONG_MAX) {
      result.ui.name_update(-2);
    }*/

    result.sign_extend(w_ + dshamt - 1);
    
    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.meta.index != 0) {
        if(other.is_glob || other.struct_var) { 
            ++callcount[other.meta.index]; 
        }   
    }
    }
    return result;
  }

  template<int other_w>
  SInt<w_> dshlw(const UInt<other_w> &other) const {
    SInt<w_> result(ui.dshlw(other));
    result.sign_extend();

    if (profile_flag == 1){
    if(other.is_glob || other.struct_var) { 
        ++callcount[other.meta.index]; 
    }
    }

    return result;
  }

  UInt<1> operator<=(const SInt<w_> &other) const {
    // if (ui.NW == 1)
    //   return as_single_word() <= other.as_single_word();

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.ui.meta.index != 0) {
        if(other.ui.is_glob || other.ui.struct_var) { 
            ++callcount[other.ui.meta.index]; 
        }   
    }
    }

    if (negative()) {
      if (other.negative())
        return ui >= other.ui;
      else
        return UInt<1>(1);
    } else {
      if (other.negative())
        return UInt<1>(0);
      else
        return ui <= other.ui;
    }
  }

  UInt<1> operator>=(const SInt<w_> &other) const {
    // if (ui.NW == 1)
    //   return as_single_word() >= other.as_single_word();

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.ui.meta.index != 0) {
        if(other.ui.is_glob || other.ui.struct_var) { 
            ++callcount[other.ui.meta.index]; 
        }   
    }
    }


    if (negative()) {
      if (other.negative())
        return ui <= other.ui;
      else
        return UInt<1>(0);
    } else {
      if (other.negative())
        return UInt<1>(1);
      else
        return ui >= other.ui;
    }
  }

  UInt<1> operator<(const SInt<w_> &other) const {

    return ~(*this >= other);
  }

  UInt<1> operator>(const SInt<w_> &other) const {

    return ~(*this <= other);
  }

  UInt<1> operator==(const SInt<w_> &other) const {

    if (profile_flag == 1){
    if(this->ui.is_glob || this->ui.struct_var) {  
        ++callcount[this->ui.meta.index]; 
    }
    if(other.ui.meta.index != 0) {
        if(other.ui.is_glob || other.ui.struct_var) { 
            ++callcount[other.ui.meta.index]; 
        }   
    }
    }

   return ui == other.ui;
  }

  UInt<1> operator!=(const SInt<w_> &other) const {

    return ~(*this == other);
  }

  UInt<w_> asUInt() const {
    UInt<w_> result(ui);
    result.mask_top_unused();
    return result;
  }

  SInt<w_> asSInt() const {
    return SInt<w_>(*this);
  }

  SInt<w_> cvt() const {
    return SInt<w_>(*this);
  }

  // Direct access for ops that only need small signals
  int64_t as_single_word() const {
    static_assert(w_ <= kWordSize, "SInt too big for single int64_t");
    return ui.words_[0];
  }


protected:
  template<int other_w>
  friend class sint_wrapper_t;

  void raw_copy_in(uint64_t *src) {
    ui.raw_copy_in(src);
  }

  void raw_copy_out(uint64_t *dst) {
    ui.raw_copy_out(dst);
  }


private:
  UInt<w_> ui;

  const static int kWordSize = UInt<w_>::kWordSize;

  bool negative() const {
    return static_cast<int64_t>(ui.words_[ui.word_index(w_ - 1)]) < 0;
    // return (ui.words_[ui.word_index(w_ - 1)] >> ((w_-1) % kWordSize)) & 1;
  }

  void sign_extend(int sign_index = (w_-1)) {
    int sign_offset = sign_index % kWordSize;
    int sign_word = ui.word_index(sign_index);
    bool is_neg = (ui.words_[sign_word] >> sign_offset) & 1;
    ui.words_[sign_word] = (static_cast<int64_t>(ui.words_[sign_word]) <<
                             (kWordSize - sign_offset - 1)) >>
                             (kWordSize - sign_offset - 1);
    for (int i = sign_word+1; i < ui.NW; i++) {
      ui.words_[i] = is_neg ? -1 : 0;
    }
  }

  void print_to_stream(std::ostream& os) const {
    ui.print_to_stream(os);
  }

  template<int other_w, typename other_word_t, int other_n>
  friend class UInt;

  template<int w>
  friend class SInt;

  template<int w>
  friend std::ostream& operator<<(std::ostream& os, const SInt<w>& ui);

//---------------------------------------------------------------------------
public:
  //NEW VARIABLES
  //int faulty_new;
  //int faulty_old;

  //int index;
  //int fault_index;

  //name update function
  //void name_update(string var_name){
  void name_update(int increment){

    if(glob_init_flag) this->ui.is_glob = 0;
    else this->ui.is_glob = 1;

    ui.meta.perm_fault = 0;

    /*auto got = u0.find(var_name);

    if(got != u0.end()){
        this->ui.meta.index = got->second;
        this->ui.meta.faulty_old = u2[this->ui.meta.index];
    }
    else{
        this->ui.meta.index = u0.size();
        this->ui.meta.faulty_old = 0;
        u0[var_name] = this->ui.meta.index;
        u1[this->ui.meta.index] = var_name;

        //if it's global store its metadata pointer
        if(ui.is_glob){
            ui.meta.data_ptr = &ui.words_[0];
            ui.meta.n = ui.NW;
            u3[ui.meta.index] = &ui.meta;
        }
    }*/

    //this->ui.meta.index = u0.size();
        this->ui.meta.faulty_old = 0;
    //    u0.push_back(var_name);
        //u0[var_name] = this->meta.index;
        //u1[this->meta.index] = var_name;

        //if it's global store its metadata pointer
        if(ui.is_glob){
            this->ui.meta.index = index_count;
            //u0.push_back(index_count);
            if(increment == 1) {
                std::cout<<index_count<<"\n";
                std::cout.flush();
            }

            //check if the pointer to words should be stored in u3 and to_fault_map
            int cnt = 0;

            while(cnt != fault_list_inject.size()){
                if((fault_list_inject[cnt].index == this->ui.meta.index)){
                    u3[ui.meta.index] = &ui.meta;
                    to_fault_map[ui.meta.index] = make_pair(&ui.words_[0], &ui.Origdata[0]);
                }
                cnt = cnt + 1;
            }

            index_count = index_count + 1;
        }

        else if(increment == -2){
    
            //Temporary index
            //temp_index--;
            if(index_count == ULONG_MAX) {
                index_count = store_last_glob_index+1;
            }
            index_count++;
            this->ui.meta.index = index_count;
        }

        else if(increment != -1){
    
            //recoverable index
            if(local_index.size() > increment){
            
                ui.meta.index = local_index[increment];
            }
            else{
                ui.meta.index = index_count;
                //u0.push_back(index_count);
                local_index.push_back(index_count);
                index_count = index_count + 1;
            }
        }
        /*if(ui.is_glob){
            this->ui.meta.index = index_count;
            index_count = index_count + 1;
            this->ui.meta.faulty_old = 0;

            ui.meta.data_ptr = &ui.words_[0];
            ui.meta.n = ui.NW;
            //u3.push_back(&ui.meta);
        }*/

    this->ui.meta.faulty_old = 0;

    if((this->ui.meta.faulty_new == 1) && (this->ui.meta.faulty_old == 0)){
        this->ui.meta.faulty_old = 1; //prevent =operator from double reporting
        //std::cout << "Signed Fault at: " << u1[ui.meta.index] << " by " << u1[ui.meta.genetics] << " due to " << u1[ui.meta.memetics] << " at " << cycle << "\n";
    }
    else if((this->ui.meta.faulty_new == 0) && (this->ui.meta.faulty_old == 1)){
        this->ui.meta.faulty_old = 0;
        //std::cout << u1[this->ui.meta.index] << " signed value was corrected at " << cycle <<"\n";
    }
    else this->ui.meta.faulty_old = 0;
    if(ui.is_glob || ui.struct_var) {
        idx_width[this->ui.meta.index] = w_;
    }

    //u2[this->ui.meta.index] = this->ui.meta.faulty_old;
  }

  //void name_update_prefix(string file_name, string prefix, string var_name){
  void name_update_prefix(int a = -1){
    //string to_pass;

    this->ui.meta.faulty_new = 0;
    this->ui.meta.faulty_old = 0;

    ui.struct_var = 1;

    //to_pass = prefix;
    //to_pass.append(".");
    //to_pass.append(var_name);

    //name_update(to_pass);
    name_update(a);
  }

  //void name_update_num(string prefix, string var_name, int a){
  void name_update_num(){

    //const char to_find[2] = "[";
    //size_t location = 0;

    this->ui.meta.faulty_new = 0;
    this->ui.meta.faulty_old = 0;

    //string temp;
    ui.struct_var = 1;

    /*location = var_name.find(to_find);

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
    else printf("Error occurred in the parser phase (signed)\n");*/
    //name_update(temp);
    name_update(-1);
  }

  //void name_update_glob(string var_name){
  void name_update_glob(int a = -1){
    this->ui.is_glob = 1;

    this->ui.meta.faulty_new = 0;
    this->ui.meta.faulty_old = 0;

    name_update(a);
  }

  void debug_faulty_update(UInt<w_> & bits, char op, char perm){
    this->ui.meta.faulty_new = 1;
    this->ui.meta.faulty_old = 1;
    this->ui.meta.memetics = this->ui.meta.index;
    this->ui.meta.genetics = this->ui.meta.index;
    this->ui.meta.operation = op;

    if(perm && (this->ui.meta.perm_bits != NULL)){
        this->ui.meta.perm_bits = new long int [(w_/64) + 1];
        this->ui.op_inject(bits, 'p');
        this->ui.meta.perm_fault = 1;
    }

    //TO SHUBHAM: you can put the copy from words -> origData operation HERE

    for(int i = 0; i < ui.n_; i++) {
        this->ui.Origdata[i] = this->ui.words_[i]; 
    }

   faultDesc fault_desc(cycle, this->is_glob, this->struct_var, faultTrack::FAULTED);
    fault_desc.src = ui.meta.index;
    //for(int i = 1; i < n_; i++) 
        //cout<<"Faulty metadata value is: "<<*ui.meta.value<<"\n";
    vector<faultDesc> new_fault;
    new_fault.push_back(fault_desc);
    auto ret = fault_list.insert({ui.meta.index, new_fault});
    if(!ret.second) 
        ret.first->second.push_back(fault_desc);


    this->ui.op_inject(bits, op);

    //std::cout << "Fault Injected in " << u1[this->ui.meta.index] << " at " << cycle+1 << "\n";
  }
};

template<int w>
std::ostream& operator<<(std::ostream& os, const SInt<w>& si) {
  // static_assert(w <= SInt<w>::kWordSize, "SInt too big to print");
  // os << si.as_single_word() << "<S" << w << ">";
  // return os;
  si.print_to_stream(os);
  os << "<S" << w << ">";
  return os;
}

#endif  // SINT_H_
