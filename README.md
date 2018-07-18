# CPPNPY
Creation of this minimalistic header-only library was inspired by https://github.com/rogersce/cnpy and necessiated by the need to transfer C arrays via non-file media in NumPy .npy/.npz formats.
Unlike cnpy, it generates output in form of byte buffers (std::string), which permits more flexible use. It also does not attempt to deal with big-endian file format and, therefore, hopefully, contains less bugs.

# Usage
Just put [cppnpy.h](cppnpy.h) in your project and include it. Add -lz linker flag. Enjoy.
You can also get rid of zlib dependency if you wish, in which case you would need to provide a crc32 implementation.

# Interface
Everything is wrapped into cppnpy namespace.

- `carr2npy(data, shape)` converts c array to .npy-formatted buffer
- `zip(files)` converts a number of files (buffers) to a single zip-formatted buffer
- `zip({{fname1, carr2npy(data1, shape1)}, ...})` creates .npz-packed buffer of arrays

[cppnpy_test.cpp](cppnpy_test.cpp) contains a usage example.
