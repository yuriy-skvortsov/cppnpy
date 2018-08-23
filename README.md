# CPPNPY
Creation of this minimalistic header-only library was inspired by https://github.com/rogersce/cnpy and necessiated by the need to transfer C arrays via non-file media in NumPy .npy/.npz formats.
Unlike cnpy, it generates output in form of byte buffers (std::string), which permits more flexible use. It also does not attempt to deal with big-endian file format and, therefore, hopefully, contains less bugs.

You can also use the provided general purpose zip/unzip implementations. Currently zip provides no (zero) compression but unzip is (hopefully) able to decompress regular zipped files. It also looks like NumPy does not object if you include some other arbitrary files (e.g. some jsons) to you .npz archive and is even able to deal with them as with byte buffers.

# Usage
Just put [cppnpy.h](cppnpy.h) in your project and include it. Add -lz linker flag. Enjoy.
You can also get rid of zlib dependency if you wish, in which case you would need to provide a crc32 implementation.

# Interface
Everything is wrapped into cppnpy namespace.

- `carr2npy(data, shape)` converts c array to .npy-formatted buffer
- `npy2arr(buffer)` converts .npy-formatted buffer to NpArray data structure
- `zip(map<file_name, file_contents>)` converts a number of files (buffers) to a single zip-formatted buffer
- `zip({{fname1, carr2npy(data1, shape1)}, ...})` creates .npz-packed buffer of arrays
- `unzip(buffer)` inflates zipped buffer to map<file_name, file_contents> structure
- `npy2arr(unzip(npzbuffer)['VarName.npy'])` extracts NpArray structure for `VarName` variable from npz buffer

[cppnpy_test.cpp](cppnpy_test.cpp) contains usage examples.
