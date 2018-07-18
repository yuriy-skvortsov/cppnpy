#include <vector>
#include "cppnpy.h"
#include <stdio.h>
#include <fstream>

int main()
{
    using namespace std;
    vector<int> v1 {1,2,3,4,5,6};
    vector<double> v2 {7,8,9,10,11,12};

    auto v1npy = cppnpy::carr2npy(v1.data(), {2,3});
    {
        auto f = fopen("tst.npy", "wb");
        fwrite(v1npy.data(), sizeof(decltype(v1npy)::value_type), v1npy.size(), f);
        fclose(f);
    }

    auto v12npz = cppnpy::zip({
            {"V1", cppnpy::carr2npy(v1.data(), {2,3})},
            {"V2", cppnpy::carr2npy(v2.data(), {3,2})}
        });

    {
        ofstream ofs("tst.npz", ofstream::binary);
        ofs << v12npz;
    }

    return 0;
}
