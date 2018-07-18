# CPPNPY
Creation of this small header-only library was inspired by cnpy by https://github.com/rogersce/cnpy
and necessiated by the need to transfer C arrays via non-file media in NumPy .npy/.npz formats.
Unlike cnpy, it generates output in form of byte buffers (std::string), which permits more flexible use.
It also does not atempt to deal with big-endian file format and, therefore, hopefully, contains less bugs.

# Usage
Just put cppnpy.h in your project and include it. Add -lz linker flag. Enjoy.

# Interface
Everything is wrapped into cppnpy namespace.

- `carr2npy(data, shape)` converts c array to .npy-formatted buffer
- `zip(files)` converts a number of files (buffers) to a single zip-formatted buffer
- `zip({{fname1, carr2npy(data1, shape1)}, ...}) creates .npz-packed buffer of arrays

[cppnpy_test.cpp](cppnpy_test.cpp) contains a usage example.
