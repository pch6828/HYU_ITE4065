#include <iostream>
#include <random>
#include <cstring>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Please, Give 1 Argument (# of Thread)." << endl;
        return 0;
    }

    // # of threads given by command line argument
    int numThread = atoi(argv[1]);

    if (numThread <= 0)
    {
        cerr << "# of Thread Must Be Positive." << endl;
    }

    return 0;
}