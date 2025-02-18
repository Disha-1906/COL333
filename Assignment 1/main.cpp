#include <iostream>
// #include <bits/stdc++.h>
#include <fstream>

#include "state.cpp"

using namespace std;

int main(int argc, char** argv )
{

     // Parse the input.
    if ( argc < 3 )
    {   
        cout<<"Missing arguments\n";
        cout<<"Correct format : \n";
        cout << "./main <input_filename> <output_filename>";
        exit ( 0 );
    }
    chrono::steady_clock::time_point start = chrono::steady_clock::now();
    string inputfilename ( argv[1] );
    string outputfilename ( argv[2] );
    
    Allocation *s  = new Allocation( inputfilename );
    // long long g = s->findScore();
    // cout<<"Start Cost "<<g<<endl;
    // cout<<"hello"<<endl;
    s->allot(start);
    s->write_to_file(outputfilename);

    long long cost = s->findScore ();
    cout<< "cost:"<<cost<<endl;
    s->readOutputFile(outputfilename);
    return 0;

}