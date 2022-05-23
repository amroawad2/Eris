import sys
import math
import mmap
import re

if __name__ == "__main__":
    
    header_flag = 0
    freq_flag = 0
    str_idx_width_map_flag = 0
    verbose_flag = 0
    
    for i, arg in enumerate(sys.argv):
        if((sys.argv[i] == "--str_idx") or (sys.argv[i] == "-i")):
            str_idx_width_map_flag = 1
            str_idx_width_name = sys.argv[i+1]
        elif((sys.argv[i] == "--header") or (sys.argv[i] == "-h")):
            header_flag = 1
            header_name = sys.argv[i+1]
        elif((sys.argv[i] == "--freq") or (sys.argv[i] == "-f")):
            freq_flag = 1
            freq_name = sys.argv[i+1]
            
    if(str_idx_width_map_flag == 0):
        print("Need to str idx mapping")
        exit()
    if(header_flag == 0):
        print("Need to provide header file")
        exit()
    if(freq_flag == 0):
        print("Name of freq file?")
        exit()
        
    sp = open("struct_regs.txt", 'w')
    fp = open(freq_name, 'r')
        
    line = fp.readline()
    
    if(verbose_flag): print("Reading file into memory")
    
    while line:
        if '#' in line:
            line = fp.readline() #skip line
            continue
        line.rstrip()
        line_list = line.split()
        index = line_list[0]
        print(index)
        freq = line_list[1]
        print(freq)
        reg_width = line_list[2]
        print(reg_width)
        st_name = line_list[3]
        print(st_name)
        st = st_name
        if "[" in st_name:
            st = st_name.split('[')[0] + '['
        if "." not in st[0] and "[" not in st:
            sp.write(st+'\n')
            #print("Write to struct_reg "+ st)
            sp.write(reg_width+'\n')
            sp.write('1'+'\n')
            sp.write(freq+'\n')
            line = fp.readline()
            continue
        with open(header_name, 'r') as hF:
            idx_written = 0
            size_set = 0
            nocore = 0
            found = 0
            for st_line in hF:
                if st.split('.')[-1] in st_line and '[' in st_line and size_set == 0:
                    arr_size = st_line.split()[-1].split('[')[1].split(']')[0]
                    print(arr_size)
                    size_set = 1
                if st in st_line:
                    found = 1
                    if "[" in st and "." in st[0]:
                        st_line_list = st_line.split()
                        i = 0
                        while(1):
                            if st in st_line_list[i]:
                                final_str_list = st_line_list[i].split("[")
                                if size_set == 1:
                                    final_str = final_str_list[0] + '[' + arr_size + ']'
                                    if "core" not in final_str:
                                        print("Core not in "+ final_str)
                                        nocore = 1
                                        break
                                else:
                                    print("Array size not set yet")
                                sp.write(final_str+'\n')
                                print("Writing name" + final_str)
                                sp.write(reg_width+'\n')
                                print("Writing width" + reg_width)
                                sp.write(arr_size+'\n')
                                print("Writing arr_size" + arr_size)
                                sp.write(freq+'\n')
                                print("Writing freq" + freq)
                                idx_written = 1
                                break
                            i=i+1
                    elif "[" in st and "." in st:
                        st_line_list = st_line.split()
                        i = 0
                        while(1):
                            if st in st_line_list[i]:
                                final_str_list = st_line_list[i].split("[")
                                if size_set == 1:
                                    final_str = final_str_list[0] + '[' + arr_size + ']'
                                    if "core" not in final_str:
                                        print("Core not in "+ final_str)
                                        nocore = 1
                                        break
                                else:
                                    print("Array size not set yet")
                                sp.write(final_str+'\n')
                                print("Writing name" + final_str)
                                sp.write(reg_width+'\n')
                                print("Writing width" + reg_width)
                                sp.write(arr_size+'\n')
                                print("Writing arr_size" + arr_size)
                                sp.write(freq+'\n')
                                print("Writing freq" + freq)
                                idx_written = 1
                                break
                            i=i+1
                    elif "[" in st:
                        st_line_list = st_line.split()
                        i = 0
                        while(1):
                            if st in st_line_list[i]:
                                final_str_list = st_line_list[i].split("[")
                                if size_set == 1:
                                    final_str = final_str_list[0] + '[' + arr_size + ']'
                                    if "core" not in st_name:
                                        print("Core not in "+ st_name)
                                        nocore = 1
                                        break
                                else:
                                    print("Array size not set yet")
                                sp.write(final_str+'\n')
                                print("Writing name" + final_str)
                                sp.write(reg_width+'\n')
                                print("Writing width" + reg_width)
                                sp.write(arr_size+'\n')
                                print("Writing arr size" + arr_size)
                                sp.write(freq+'\n')
                                print("Writing freq" + freq)
                                idx_written = 1
                                break
                            i=i+1
                if nocore == 1 or idx_written == 1:
                    break
            if found == 0:
                print("String "+ st +" not found in header")
        line = fp.readline()
    sp.close()
    fp.close()
