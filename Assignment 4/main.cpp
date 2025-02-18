#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <chrono>


// Format checker just assumes you have Alarm.bif and Solved_Alarm.bif (your file) in current directory
using namespace std;

// Our graph consists of a list of nodes where each node is represented as follows:
class Graph_Node{

private:
	string Node_Name;  // Variable name 
	vector<int> Children; // Children of a particular node - these are index of nodes in graph.
	vector<int> Parents; // Parents of a particular node- note these are names of parents
	int nvalues;  // Number of categories a variable represented by this node can take
	vector<string> values; // Categories of possible values
	vector<float> CPT; // conditional probability table as a 1-d array . Look for BIF format to understand its meaning

public:
	// Constructor- a node is initialised with its name and its categories
    Graph_Node(string name,int n,vector<string> vals){
		Node_Name=name;
		nvalues=n;
		values=vals;
	}
	string get_name(){
		return Node_Name;
	}
	vector<int> get_children(){
		return Children;
	}
	vector<int> get_Parents(){
		return Parents;
	}
	vector<float> get_CPT(){
		return CPT;
	}
	int get_nvalues(){
		return nvalues;
	}
	vector<string> get_values(){
		return values;
	}
	void set_CPT(vector<float> new_CPT){
		CPT.clear();
		CPT=new_CPT;
	}
    void set_Parents(vector<int> Parent_Nodes){
        Parents.clear();
        Parents=Parent_Nodes;
    }
    int add_child(int new_child_index ){
        for(int i=0;i<Children.size();i++){
            if(Children[i]==new_child_index)
                return 0;
        }
        Children.push_back(new_child_index);
        return 1;
    }
};

class network{
    int total_variables = 0;
	vector<Graph_Node> Pres_Graph;
    vector<string> index2name;
    unordered_map<string,int> name2index;

public:
	int addNode(Graph_Node node){
		Pres_Graph.push_back(node);
        index2name.push_back(node.get_name());
        name2index[node.get_name()] = total_variables;
        total_variables++;
		return 0;
	}
	int netSize(){
		return Pres_Graph.size();
	}
    int get_index(string val_name){
        return name2index[val_name];
    }
    Graph_Node* get_nth_node(int n){   
        Graph_Node* g = &Pres_Graph[n];
        return g;
    }
    Graph_Node* search_node(string val_name){
        int i = name2index[val_name];
        Graph_Node* g = &Pres_Graph[i];
        return g;
    }
};

network read_network(string alarmfile){
	network Alarm;
	string line;
	int find=0;
  	ifstream myfile(alarmfile); 
  	string temp;
  	string name;
  	vector<int> values; 	
    vector<string> val;
    if (myfile.is_open()){
    	while (! myfile.eof() ){
    		stringstream ss;
      		getline (myfile,line);
      		ss.str(line);
     		ss>>temp;
     		if(temp.compare("variable")==0){
     				ss>>name;
     				getline (myfile,line);
     				stringstream ss2;
     				ss2.str(line);
     				for(int i=0;i<4;i++){
     					ss2>>temp;
     				}
     				val.clear();
     				while(temp.compare("};")!=0){
     					val.push_back(temp);
     					ss2>>temp;
    				}
     				Graph_Node new_node(name,val.size(),val);
     				int pos=Alarm.addNode(new_node);
     		}
     		else if(temp.compare("probability")==0){
     				ss>>temp;
     				ss>>temp;
     				Graph_Node* currNode = Alarm.search_node(temp);
                    int index=Alarm.get_index(temp);
                    ss>>temp;
                    values.clear();
     				while(temp.compare(")")!=0){   
                        Graph_Node* parent=Alarm.search_node(temp);
                        int pi = Alarm.get_index(parent->get_name());
                        parent->add_child(index);
     					values.push_back(pi);
     					ss>>temp;
    				}
                    currNode->set_Parents(values);
    				getline (myfile,line);
     				stringstream ss2;
     				ss2.str(line);
     				ss2>> temp;
     				ss2>> temp;
     				vector<float> curr_CPT;
                    string::size_type sz;
     				while(temp.compare(";")!=0){ 
     					curr_CPT.push_back(atof(temp.c_str()));
     					ss2>>temp;
    				}
                    currNode->set_CPT(curr_CPT);
     		}
    	}
    	if(find==1)
    	myfile.close();
    }
  	return Alarm;
}

vector<float> weights;
class Data_Row{
    public:
        int question_mark = -1;
        vector<int> actual_data;
};
class DataSet{
    public:
        vector<Data_Row> rows;
        vector<vector<int>> possible_data;
};
DataSet read_data(network* alarm, string dataset){
    DataSet alldata;
    string l;
    ifstream data_file(dataset);
    string temp;
    string name;
    if(data_file.is_open()){
        while(! data_file.eof()){
            stringstream ss;
            getline(data_file,l);
            ss.str(l);
            vector<int> val;
            Data_Row row;
            for(int i=0;i<alarm->netSize();i++){
                ss>>temp;
                if(temp.compare("\"?\"") == 0){
                    row.question_mark = i;
                    val.push_back(-1);
                    continue;
                }
                int it = 0;
                int t = alarm->get_nth_node(i)->get_nvalues();
                vector<string> pval = alarm->get_nth_node(i)->get_values();
                while(it<t){
                    if(pval[it].compare(temp)==0) break;
                    it +=1;
                }
                val.push_back(it);
            }
            row.actual_data = val;
            alldata.rows.push_back(row);
            if(row.question_mark !=-1){
                int var = row.question_mark;
                int p = alarm->get_nth_node(var)->get_nvalues();
                int jt = 0;
                for(int jt=0; jt<p; jt++){
                    vector<int> possible_row(val.begin(),val.end());
                    possible_row[var] = jt;
                    alldata.possible_data.push_back(possible_row);
                    weights.push_back(1/p);
                }
            }
            else{
                alldata.possible_data.push_back(val);
                weights.push_back(1);
            }
        }
        data_file.close();
    }
    return alldata;
}

pair<float,int> cal_prob( int row_num,int id, int val, network* alarm, DataSet* alldata, bool flag, int pos_index){
    int index = 0;
    int x= 1;
    Graph_Node* n = alarm->get_nth_node(id);
    vector<int> parents = n->get_Parents();
    for(int i=parents.size()-1; i>=0; i--){
        // string pname = parents[i];
        Graph_Node* p = alarm->get_nth_node(parents[i]);
        if(flag==true){
            Data_Row* row = &alldata->rows[row_num];
            index += x*row->actual_data[parents[i]];
            x*= p->get_nvalues();
        }
        else{
            index += x*alldata->possible_data[pos_index][parents[i]];
            x*= p->get_nvalues();
        }
    }
    index += x*val;
    pair<float,int> q = make_pair(n->get_CPT()[index],index);
    return q;
}



void update_data(network* alarm, DataSet* alldata){
    weights.clear();
    for(int i=0; i<alldata->rows.size(); i++){
        Data_Row* row = &alldata->rows[i];
        if(row->question_mark!=-1){
            float nf = 0.0;
            vector<float> prob;
            Graph_Node* g = alarm->get_nth_node(row->question_mark);
            for(int j=0; j<g->get_nvalues(); j++){
                pair<float,int> ans  = cal_prob(i,row->question_mark, j, alarm, alldata, true, -1);
                float p = ans.first;
                vector<int> child = g->get_children();
                for(int k=0 ; k<child.size(); k++){
                    row->actual_data[row->question_mark] = j;
                    int c = child[k];
                    float p2 = cal_prob(i, c,row->actual_data[c], alarm, alldata, true, -1).first;
                    p = p*p2;
                }
                nf += p;
                prob.push_back(p);
            }
            int max_val = max_element(prob.begin(),prob.end()) - prob.begin();
            row->actual_data[row->question_mark] = max_val;
            for(int j=0; j<prob.size(); j++){
                weights.push_back(prob[j]/nf);
            }
        }
        else{
            weights.push_back(1);
            continue;
        }
    }
}

void update_cpt(network* alarm, DataSet* alldata){
    for(int i=0; i<alarm->netSize(); i++){
        Graph_Node* g = alarm->get_nth_node(i);
        vector<float> oldcpt = g->get_CPT();
        int set_size =oldcpt.size()/g->get_nvalues();
        vector<float> initial(set_size, 0.0008*g->get_nvalues()); //smoothing factor
        vector<float> newcpt = vector<float>(oldcpt.size(), 0.0008);
        for(int j=0; j<alldata->possible_data.size(); j++){
            int di = cal_prob(0,i,alldata->possible_data[j][i],alarm,alldata,false,j).second;
            newcpt[di] += weights[j];
            initial[di%set_size] += weights[j];
        }
        for(int j=0; j<g->get_CPT().size(); j++){
            if(initial[j%set_size] == 0) newcpt[j] = 0.0;
            else newcpt[j] = max((float)0.01,newcpt[j]/initial[j%set_size]);
        }
        g->set_CPT(newcpt);
    }
}

void solve(network* alarm, DataSet* alldata, string file_alarm, string out, chrono::milliseconds total_time, chrono::time_point<std::chrono::system_clock>& start_time){
    int total_size = alarm->netSize();
    for(int i=0 ; i<total_size ; i++){
        Graph_Node* g  = alarm->get_nth_node(i);
        vector<float> cpt = g->get_CPT();
        // int set_size =cpt.size()/g->get_nvalues();
        int num_sets = g->get_nvalues();
        int set_size =cpt.size()/num_sets;  //number of entries in CPT that sum to 1 Eg: P(a|b,e), P(!a|b,e)
        // vector<float> cpt = g->get_CPT();
        
        // unordered_map<int, pair<float,int>> mp;
        // for(int k = 0; k<set_size; k++){
        //     mp.insert({k,make_pair(0.0,num_sets)});
        // }
        // for(int j=0; j<cpt.size(); j++){
        //     if(cpt[j] != -1){
        //         mp[j%set_size].first += cpt[j];
        //         mp[j%set_size].second -= 1;
        //     }
        // }
        for(int j=0; j<cpt.size(); j++){
            if(cpt[j] == -1){
                cpt[j] = 1/num_sets;
            }
        }
        g->set_CPT(cpt);
    }
    update_data(alarm,alldata);
    while(chrono::system_clock::now() - start_time < total_time){
        update_cpt(alarm,alldata);
        update_data(alarm,alldata);
    }
    ifstream data_file(file_alarm);
    ofstream output_file;
    string l,temp;
    output_file.open(out);
    if(data_file.is_open()){
        while(!data_file.eof()){
            stringstream ss;
            getline(data_file,l);
            ss.str(l);
            ss>>temp;
            if(temp.compare("probability")==0){
                ss>>temp;
                ss>>temp;
                Graph_Node* currNode = alarm->search_node(temp);
                output_file<<l<<endl;
                getline(data_file,l);
                output_file<<"  table ";
                vector<float> cpt = currNode->get_CPT();
                for(int i=0; i<cpt.size(); i++){
                    if(cpt[i]<0.0001) output_file<<"0.0001"<<" ";
                    else output_file<<fixed<<setprecision(4)<<cpt[i]<<" ";
                }
                output_file<<";"<<endl;
            }
            else if(l.compare("")!=0){
                output_file<<l<<endl;
            }
            else output_file<<l;
        }
        data_file.close();
    }
}
int main(int argc , char* argv[]){
    if(argc<2){
        cerr<<"give 2 files"<<endl;
        return 1;        
    }
    string alarmFile = argv[1];
    string recordFile = argv[2];
    auto startTime = chrono::system_clock::now();
    chrono::milliseconds total_time(119998);
	network Alarm;
    network* a = &Alarm;
	Alarm=read_network(alarmFile);
    DataSet record;
    DataSet* r = &record;
    record = read_data(a,recordFile);
    solve(a,r,alarmFile,"solved_alarm.bif",total_time,startTime);
	cout<<"Perfect! Hurrah! \n";
}




