import sys
import math

from random import seed
from random import randint

if __name__ == "__main__":
    
    file_flag = 0
    cycle_flag = 0
    verbose_flag = 0
    
    for i, arg in enumerate(sys.argv):
        if((sys.argv[i] == "--help") or (sys.argv[i] == "-h")): 
            print("--file <struct_regs.txt> (required)\n--cycle The number of cycles in the test program (required)\nIf -1 is entered here then every fault will be injected every cycle\n--verbose: print runtime details\n--help: show help, i.e. this menu\nInvalid options WILL BE IGNORED\n")
            exit()
        elif((sys.argv[i] == "--file") or (sys.argv[i] == "-f")):
            file_flag = 1
            file_name = sys.argv[i+1]
        elif((sys.argv[i] == "--verbose") or (sys.argv[i] == "-v")):
            verbose_flag = 1
        elif((sys.argv[i] == "--cycle") or (sys.argv[i] == "-c")):
            cycle_flag = 1
            cycle = int(sys.argv[i+1])
            
            
    #check for error conditions
    if(file_flag == 0):
        print("You need to specifiy a file!")
        exit()
    if(cycle_flag == 0):
        print("Specifiy an upper bounds for cycle to inject on")
        exit()
        
    fp = open(file_name, 'r')
    
    magic_key = fp.readline() #confirm we're reading a proper .csv file
    if(magic_key != "18623009\n"):
        print("Not a proper signals file! (missing Magic Key)")
        fp.close()
        exit()
    else:
        if(verbose_flag): print("Found Magic Key!")
        
    line = fp.readline()
    
    reg_names = ["First Element"]
    reg_widths = [0]
    reg_array = [0]
    reg_total = [0]
    total_regs = 0
    if(verbose_flag): print("Reading file into memory")
    
    while line:
        if(line[0] == '#'):
            line = fp.readline() #skip line
        else:
            line.rstrip()
            reg_names.append(line) #reg name
            line = fp.readline()
            reg_widths.append(int(line)) #reg width
            line = fp.readline()
            total_regs = total_regs + int(line)
            reg_array.append(total_regs)
            reg_total.append(int(line))
            line = fp.readline()
    if(verbose_flag): print("Done reading file")
        
    if(verbose_flag): print("Faultable Registers: ", str(total_regs))
        
    if(verbose_flag): print("Question time!")
    
    file_num = int(input("How many fault files?: "))
    if(file_num <= 0):
        print("Not funny")
        exit()
    fault_num = int(input("How many faults per file?: "))
    if(file_num <= 0):
        print("Less funny")
        exit()
    #bit_num = int(input("How many bits per fault? (Suggested: 1): "))
    #if(bit_num <= 0):
    #    print("Stop that")
    #    exit()
    fault_type = input("Type of faults? Permanent (p), Transient (t), or Both (b)?: ")
    op_type = input("What operation do you want? &,^,|,0? ('^' cannot be used with perm faults): ")
    
    if(((fault_type == 'b') or (fault_type == 'p')) and (op_type == '^')):
        print("Pay attention to the directions")
        exit()
    
    if(verbose_flag): print("Generating fault files")
    
    #seed(18041202) #what's the number mean?
    
    count = 0
    count0 = 0
    fault_per_file_count = 0
    
    while(count != file_num):
        #open fault file
        file_name = "fault_file_" + str(count) + ".txt"
        fp_w = open(file_name, "w")
        
        while(fault_per_file_count != fault_num):
            value = randint(1, total_regs)
            #reg name
            while(reg_array[count0] < value): count0 = count0 + 1 #find where to be
            if(reg_total[count0] == 1): to_write = reg_names[count0]
            else:
                index = reg_array[count0] - value
                to_write = reg_names[count0].split("[")[0]
                to_write = to_write + "["+ str(index) + "]"
            #to_write = reg_names[value]
            to_write = to_write.rstrip()
            #type of fault
            if(fault_type == 'p'): to_write = to_write + ", 1"
            elif(fault_type == 't'): to_write = to_write + ", 0"
            else: to_write = to_write + ", " + str(randint(0,1))
            #cycle
            if(cycle != -1): to_write = to_write + ", " + str(randint(1, cycle))
            else: to_write = to_write + ", -1"
            #width of location
            to_write = to_write + ", " + str(reg_widths[count0])
            #Value to inject
            if(op_type != '0'):
                #bit = 1
                bit = 1 << randint(0, reg_widths[count0]-1)

                if(op_type == '&'):
                    if(reg_widths[count0] <= 8):
                        bit = ((~bit) & (int(2 ** 8) - 1)) #only affect 1 bit
                    else:
                        bit = ((~bit) & (int(2 ** reg_widths[count0]) - 1)) #only affect 1 bit
                if(bit <= 0xF):
                    if(op_type != '&'): to_write = to_write + ", 0x0" + hex(bit).lstrip("0x")
                    else:
                        to_write = to_write + ", 0xF" + hex(bit).lstrip("0x")
                else:
                    to_write = to_write + ", " + hex(bit)
            else:
                to_write = to_write + ", 0"
            #Operation
            to_write = to_write + ", " + op_type + "\n"
            fp_w.write(to_write)
            fault_per_file_count = fault_per_file_count + 1
            count0 = 0
        fault_per_file_count = 0
        count = count + 1
    
    fp.close()
    exit()
        
        
        
        
    
