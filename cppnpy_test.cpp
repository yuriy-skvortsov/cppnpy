#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include "cppnpy.h"


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

    {
        // ifstream ifs("zz.zip", ifstream::binary);
        // ifs.seekg(0, std::ios::end);
        // size_t size = ifs.tellg();
        // std::string buffer(size, 0);
        // ifs.seekg(0);
        // ifs.read(&buffer[0], size);
        
        auto uz = cppnpy::unzip(v12npz);
        for(auto el : uz)
        {
            cout <<el.first<<" "<<el.second.size()<<"\n";
        }
    }

    {
        auto v12 = cppnpy::unzip(v12npz);
        auto nparr = cppnpy::npy2arr(v1npy);
        cout << "\nnparr:\n";
        cout << "type_symbol: " << nparr.type_symbol << "\n";
        cout << "type_size: " << nparr.type_size << "\n";
        cout << "fortran_order: " << nparr.fortran_order << "\n";
        cout << "shape: (";
        for(auto s : nparr.shape)
            cout << s <<", ";
        cout << ")\n";
    
        cout << "IsOfType: ";
        cout << "int: " << nparr.is_of_type<int>();
        cout << "; int32_t: " << nparr.is_of_type<int32_t>();
        cout << "; short: " << nparr.is_of_type<short>();
        cout << "\nSize: " << nparr.size() << "\n";
    }       
    return 0;
}
