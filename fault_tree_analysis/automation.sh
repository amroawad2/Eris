#!/bin/bash
## remove the first line 
grep -v "FAULT_METADATA" fault_tracking.log | awk '{print $1" "$4}' > occurance_list_1
echo "occurance_list_1 computed"
grep -v "FAULT_METADATA" fault_tracking.log | awk '{print $1" "$3}'| sort -u > child_list
echo "child_list computed"
## To remove undesired spaces in the file
rm final_occurance_list
awk 'NF' occurance_list_1 > occurance_list_2
echo "occurance_list_2 computed"
node_counts=$(awk '{print $1}' occurance_list_2 | sort -u | wc -l)
echo "node_counts computed"
unique_nodes=$(awk '{print $1}' occurance_list_2 | sort -u)
echo "unique_nodes computed"
echo "$unique_nodes" > unique_nodes.txt
lines=(`cat "unique_nodes.txt"`)
length=0
echo "I am here"
while [ $length -le $node_counts ]; do
	node=${lines[$length]}
	echo "$node"
	length=$(($length+1))
	is_math_corrected=$(grep "^$node " occurance_list_2 | head -n 1 | grep "CORRECTED" | wc -l)
	if [ $is_math_corrected -eq 1 ]; then 
		#node_list=$(grep "^$node " occurance_list_2 | head -n 1)
		grep "^$node " occurance_list_2 | head -n 1 >> final_occurance_list
	fi
	if [ $is_math_corrected -eq 0 ]; then
		#node_list=$(grep "^$node " occurance_list_2)
		grep "^$node " occurance_list_2 >> final_occurance_list
	fi
        #echo "$node_list" >> final_occurance_list	
done
#prev_clock_cycle=${clock_cycle_for_nodes[$index]}
echo "dumped all neccessary files "

## calculating occurrence as parent 
grep -v "FAULT_METADATA" fault_tracking.log | awk 'NF' | awk '{print $3}'  > parent_list
unique_parent_list=$(sort -u parent_list)
echo "$unique_parent_list" > unique_parent_list
unique_parent_node_count=$(cat unique_parent_list | wc -l)
lines=(`cat "unique_parent_list"`)
length=0
echo "computing occurrence list of each node as parent"
rm final_parent_occurrence_list
while [ $length -lt $unique_parent_node_count ]; do
        node=${lines[$length]}
        length=$(($length+1))
        count=$(grep "^$node$" parent_list | wc -l)
        echo "$node $count" >> final_parent_occurrence_list
done
#./graph $node_counts $1
./run_new.sh

