#include <bits/stdc++.h>
#include <iostream>
using namespace std;
class node {
	public:
		long int node_num;
		node * next;
		node * child_nodes;
		long total_faults;
		int p_factor;
		int parent_occurrence;
};

class graph {
	public: 
		node *begin;
		graph(node *);
		void add (long int, long int);
		long find_p_factor(long int , long int *, int );
		//node get_and_delete();
		//int search(int );
		void print_list(long int );
		void print_list_all_parents();
		void add_to_parent_node(long int , bool);
		void add_to_parent_node_with_occurrence(long int , int);
		int is_p_factor_calculated(long int);
		//void delete_list();
};

graph::graph (node *my_node) {
	this->begin=my_node;
	this->begin->next = NULL;
	this->begin->child_nodes = NULL;
}

void graph::add (long int my_node_num, long int parent_node_num) {
	// To avoid adding nodes which are feeding data to itself
	if (my_node_num == parent_node_num) {
		return;
	}
	node *x = NULL;
	node *y = NULL;
	node *z = NULL;
	x = this->begin;
	while (x!=NULL) {
		if (x->node_num==parent_node_num) {
			y = x;
			while (y->child_nodes!=NULL) {
        			y=y->child_nodes;
			}
		        node *new_node = NULL;
        		new_node = new node;
			new_node->node_num = my_node_num;
        		new_node->next = NULL;
			new_node->child_nodes = NULL;
        		new_node->p_factor = -1;
			y->child_nodes=new_node;
			//add_to_parent_node(my_node_num);
			return;
		}
		z = x;
		x=x->next;

	}
	x = z;
	// If the parent node itself not present, then I am creating a new node 
	// And putting the child in its leaf node linked list
	/*node *new_node = NULL;
	new_node = new node();
	new_node->node_num = parent_node_num;
	new_node->next = NULL;
	node *new_leaf_node = NULL;
	new_leaf_node = new node();
	new_leaf_node->node_num = my_node_num;
	new_leaf_node->next = NULL;
	new_leaf_node->child_nodes = NULL;
	x->next=new_node;
	x->next->child_nodes = new_leaf_node;
	add_to_parent_node(my_node_num);*/
	// add the child node in the parent node list 
	/*x = this->begin;
        
	while (x!=NULL) {
                if (x->node_num==my_node_num) {
                	// child node exist in the parent node list 
			return;        
		}
		y = x;
		x = x->next;
	
	}
        //cout << " adding " << my_node_num << " to the parent node list " << endl;
        node *new_node_1 = NULL;
        new_node_1 = new node;
        new_node_1->node_num = my_node_num;
        new_node_1->next = NULL;
        new_node_1->child_nodes = NULL;
        y->next=new_node_1;
        return;*/


}

void graph::add_to_parent_node(long int my_node_num , bool is_faulted) {
        node *x = this->begin;
	node *y = NULL;
        while (x!=NULL) {
                if (x->node_num==my_node_num) {
                        // parent node exist in the parent node list 
			if (is_faulted) {
				x->total_faults++;	
			}
                        return;
                }
                y = x;
                x = x->next;

        }
        ////cout << " adding " << my_node_num << " to the parent node list " << endl;
        node *new_node_1 = NULL;
        new_node_1 = new node;
	if (is_faulted) { 
		new_node_1->total_faults=1;
	} else {
		new_node_1->total_faults=0;
	}
        new_node_1->node_num = my_node_num;
	new_node_1->parent_occurrence = 0;
        new_node_1->next = NULL;
	new_node_1->p_factor = -1;
        new_node_1->child_nodes = NULL;
        y->next=new_node_1;
        return;	
}

void graph::add_to_parent_node_with_occurrence(long int my_node_num , int parent_occurrence) {
        node *x = this->begin;
        node *y = NULL;
        while (x!=NULL) {
                if (x->node_num==my_node_num) {
                        // parent node exist in the parent node list
                        x->parent_occurrence = parent_occurrence;
			return;
                }
                y = x;
                x = x->next;

        }
        ////cout << " adding " << my_node_num << " to the parent node list " << endl;
        node *new_node_1 = NULL;
        new_node_1 = new node;
        new_node_1->node_num = my_node_num;
        new_node_1->parent_occurrence = parent_occurrence;
        new_node_1->next = NULL;
	new_node_1->p_factor = -1;
        new_node_1->child_nodes = NULL;
	new_node_1->total_faults=0;
        y->next=new_node_1;
        return;
}


void graph::print_list(long int parent_node) {
        node *x = NULL;
	node *y = NULL;
        x = this->begin;
        while (x!=NULL) {
		////cout << x->node_num << endl;
		if (x->node_num == parent_node) {
			y=x;
			////cout << " inside print list " << endl;
			while(y!=NULL) {
				//printf ("%li \n", y->node_num);
				y=y->child_nodes;	
			}
		return;	
		}
                x=x->next;
        }
}

void graph::print_list_all_parents() {
        node *x = NULL;
        node *y = NULL;
        x = this->begin;
        while (x!=NULL) {
               cout << x->node_num << "  " << " total faults " << x->total_faults << " total occurrence " << x->parent_occurrence <<" p_factor "<< x->p_factor << endl;
	       x = x->next;
        }
}

int graph::is_p_factor_calculated(long int parent_node) {
        node *x = NULL;
        node *y = NULL;
        node *z = NULL;
        x = this->begin;
	while (x!=NULL) {
		if (x->node_num == parent_node) {
			return x->p_factor;
		} 
		x = x->next;
	}
}
long graph::find_p_factor(long int parent_node, long int *visited_list, int visited_list_length) {
        node *x = NULL;
        node *y = NULL;
	node *z = NULL;
        x = this->begin;
	long p_factor_temp=0;
	int i=0;
	int already_visited = 0;
	while (visited_list[i]!=-1) {
		if (visited_list[i] == parent_node) {
			////cout << parent_node << " already visited " << endl;
			already_visited = 1;
		}
		i++;	
	}
	////cout << "I am here " << endl;
	while (x!=NULL) {
		if (x->node_num == parent_node) {
			/*if (x->p_factor!= -1) {
				return x->p_factor;
			}*/
			y = x;
			z = y->child_nodes;
			visited_list[i] = parent_node;
			if (z == NULL) {

				return y->total_faults;
			}
			if (already_visited == 1) {
				return x->total_faults;
			}
			// if all child visited that means this node is actually a leaf node but it is feeding back to the parent node
			// ao, we cant make its p_factor 0 even though the p_factor from its child node would be zero
			// if all_child_visited true then this node is a leaf node. 
			bool all_child_visited = true;
			while (z!=NULL) {
				long p_factor = 0;
				// check whether child is in visited list 
				/*int j = 0;
				bool child_node_already_visited = false;
				while (visited_list[j]!=-1) {
					if (visited_list[j] == z->node_num) {
						//cout << z->node_num << " already visited " << endl;
						child_node_already_visited = true;
					}
					j++;
				}
				// if the child node is not visited earlier 
				if (child_node_already_visited == false) {
					all_child_visited = false;
					p_factor = find_p_factor(z->node_num , visited_list, visited_list_length);
				}*/
				p_factor = find_p_factor(z->node_num , visited_list, visited_list_length);
				p_factor_temp += p_factor;
				//cout <<" node number "<< z->node_num << " p_factor " << p_factor << endl;
				z = z->child_nodes;	
			}
			// if p_factor of all the child nodes are zero 
			if (p_factor_temp == 0) {
				//y->total_faults = 0;
				y->p_factor = 0;
				return 0;
			} else {
				p_factor_temp = p_factor_temp + y->total_faults + 2*y->parent_occurrence;
				//y->p_factor = p_factor_temp;
				//y->total_faults = p_factor_temp;
				return p_factor_temp;
			}
		}
		x = x->next;
	}
}



/*void graph::delete_list()
{

    node* current = this->begin;
    node* next = NULL;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

}

node graph::get_and_delete() 
{
	node * first = NULL;
	first = this->begin;
	node new_node;
	new_node.address=this->begin->address;
	new_node.time=this->begin->time;
	new_node.next = NULL;
	this->begin = this->begin->next;
	delete first;
	return new_node;	
}

int graph::search(int address) {
	node* current = this->begin;
	node* previous = NULL;
	//cout << "begining :: address  " << this->begin->address << endl;
 	if (current->address == address) {
		this->begin = current->next;
		int t = current->time;
		delete current;
		return t;
	}	
	while (current->next!=NULL) {
		////cout << "I am here12 :: address  " << current->address << endl;
		if(current->address==address) {
			////cout << "I am here1234" << endl;
			break;
		} else {
			////cout << "I am here123456" << endl;
			previous=current;
			current=current->next;
		}			
	}
	////cout << " I am here56 " << endl;
	previous->next=current->next;
	int t = current->time;
	////cout << "I am here12345689" << endl;
	delete current;
	return t;

}*/	

int main(int argc, char *argv[]) {
    //cout <<"I am here" << endl;
    FILE *FP;
    long int child;
    long int parent;
    char fault_string[20];
    int total_nodes = atoi(argv[1]);
    //cout << "Total nodes "  << total_nodes << endl;
    //long int node_to_evaluate_p_factor = atoi(argv[1]);
    // Open trace_file in read mode
    //cout << "Start: Loading the occurrence list" << endl;
    FP = fopen("final_occurance_list", "r");
    //cout << "End: Loading the occurrence list" << endl;
    fscanf(FP, "%li %s", &parent , fault_string);
    node *new_node = NULL;
    new_node = new node();
    new_node->node_num = parent;
    // total nodes needed to keep track of the visited nodes
    if (strcmp(fault_string,"FAULTED") == 0) {
	    new_node->total_faults = 1;
    } else {
	    new_node->total_faults = 0;
    }
    new_node->p_factor = -1;
    graph my_graph(new_node);
    //cout << "Start: parent nodes addition" << endl;   
    int count = 0;
    while(fscanf(FP, "%li %s", &parent , fault_string) != EOF)
    {      
	    ////cout << count << endl;
	    bool is_faulted;
	    if (strcmp(fault_string,"FAULTED") == 0) {
		    is_faulted = true;
	     } else {
		    is_faulted = false;
	     }  
            my_graph.add_to_parent_node(parent, is_faulted);
	    //count++;
    }
    // add parents node with occurrences
    long int my_node;
    int parent_occurrence;
    FP = fopen("final_parent_occurrence_list", "r");
    while (fscanf(FP, "%li %d", &my_node , &parent_occurrence)!= EOF) {
    	my_graph.add_to_parent_node_with_occurrence(my_node , parent_occurrence);
    }
    //cout << "End: Parent node addition" << endl;
    //my_graph.print_list_all_parents();
    FP = fopen("child_list", "r");
    //cout << "Start: Child node addition" << endl;
    count = 0;
    while(fscanf(FP, "%li %li", &child, &parent) != EOF)
    {
	    ////cout << count << endl;
    	    my_graph.add(child, parent);
    	    //count++;
    }
    //cout << "End: Child node addition" << endl;
    //cout << " Printing child nodes for node " << node_to_evaluate_p_factor << endl;
    //my_graph.print_list(node_to_evaluate_p_factor);
    //my_graph.print_list(7);
    //my_graph.print_list_all_parents();
    long int visited_node[total_nodes];
    for (int i =0; i< total_nodes; i++) {
	    visited_node[i] = -1;
    }
    //my_graph.print_list(0);
    //cout << "Printing all parent node before running p_factor" << endl;
    //my_graph.print_list_all_parents();
    //cout << "Start: calculating p factor for the node :: " << node_to_evaluate_p_factor << endl;
    //int final_p_factor = my_graph.find_p_factor(node_to_evaluate_p_factor , visited_node, total_nodes);
    //cout << " final_p_factor " << final_p_factor << endl;
    //my_graph.print_list_all_parents();
    // new addition for faster simulation 
    FP = fopen("unique_parent_list", "r");
    while(fscanf(FP, "%li", &parent) != EOF) {
    		//int my_p_factor = my_graph.is_p_factor_calculated(parent);
	    	//if (my_p_factor == -1) {
			long final_p_factor = my_graph.find_p_factor(parent , visited_node, total_nodes);
			cout << parent << "  " << final_p_factor << endl;
    		/*} else {
			cout << "node name from prev calculation " << parent << " final_p_factor " << my_p_factor << endl;
		}*/
	        for (int i =0; i< total_nodes; i++) {
            		visited_node[i] = -1;
    		}

	}
}
