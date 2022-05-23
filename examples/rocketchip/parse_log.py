import sys
import math

if __name__ == "__main__":
    
    file_flag = 0
    struct_flag = 0
    output_flag = 0
    verbose_flag = 0
    
    for i, arg in enumerate(sys.argv):
        if((sys.argv[i] == "--file") or (sys.argv[i] == "-f")):
            file_flag = 1
            file_name = sys.argv[i+1]
        elif((sys.argv[i] == "--verbose") or (sys.argv[i] == "-v")):
            verbose_flag = 1
        elif((sys.argv[i] == "--struct") or (sys.argv[i] == "-s")):
            struct_flag = 1
            struct_name = sys.argv[i+1]
        elif((sys.argv[i] == "--output") or (sys.argv[i] == "-o")):
            output_flag = 1
            output_name = sys.argv[i+1]
            
    if(file_flag == 0):
        print("Need to provide log")
        exit()
    if(struct_flag == 0):
        print("Need to provide struct_regs.txt")
        exit()
    if(output_flag == 0):
        print("Name of output file?")
        exit()
        
    sp = open(struct_name, 'r')
    
    magic_key = sp.readline() #confirm we're reading a proper .csv file
    if(magic_key != "18623009\n"):
        print("Not a proper signals file! (missing Magic Key)")
        fp.close()
        exit()
    else:
        if(verbose_flag): print("Found Magic Key!")
        
    line = sp.readline()
    
    reg_names = ["First Element"]
    reg_widths = [0]
    reg_array = [0]
    reg_total = [0]
    total_regs = 0
    if(verbose_flag): print("Reading file into memory")
    
    while line:
        if(line[0] == '#'):
            line = sp.readline() #skip line
        else:
            line.rstrip()
            reg_names.append(line) #reg name
            line = sp.readline()
            reg_widths.append(int(line)) #reg width
            line = sp.readline()
            total_regs = total_regs + int(line)
            reg_array.append(total_regs)
            reg_total.append(int(line))
            line = sp.readline()
    if(verbose_flag): print("Done reading file")
        
    fp = open(file_name, 'r')
    op = open(output_name, 'w')
    
    #skip first two lines
    line = fp.readline()
    line = fp.readline()
    
    count0 = 0
    
    line = fp.readline()
    line_list = line.split()
    
    while line:
        if(line[0] == "\n"): break
        if(int(line_list[0]) < total_regs):
            to_write = line_list[0] #unique index
            to_write = to_write + " " + line_list[1] + " " #cycle
            
            while(reg_array[count0] < int(line_list[0])): count0 = count0 + 1 #find where to be
            if(reg_total[count0] == 1): to_write = to_write + reg_names[count0]
            else:
                index = reg_array[count0] - int(line_list[0])
                to_write = to_write + reg_names[count0].split("[")[0]
                to_write = to_write + "["+ str(index) + "]" + "\n"
                
            op.write(to_write)
        
        while(line[0] != "\n"): line = fp.readline() #get next unique
        line = fp.readline()
        line_list = line.split()
        
    fp.close()
    sp.close()
    op.close()
            
            
            
        
        
    
    
