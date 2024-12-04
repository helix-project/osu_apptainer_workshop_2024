#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[]){

    // parse the command passed
    // Input is in meters
    float input = atof(argv[1]);

    // convert unit to mm
    float output = input * 1e3;

    // output to a text file
    ofstream out_file;
    out_file.open("test.txt");
    out_file << output << endl;

    // Also print
    cout << output << endl;
    return 0;
}