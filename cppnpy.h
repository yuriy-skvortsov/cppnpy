// Copyright (C) 2018 Yuriy Skvortsov @ Readdle Inc.
// Released under MIT License
// license available in LICENSE file, or at http://www.opensource.org/licenses/mit-license.php

#ifndef LIBCPPNPY_H_
#define LIBCPPNPY_H_

//#include <complex>

#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#include <zlib.h>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <map>

namespace cppnpy {
    template<typename T>
    constexpr char np_type_id_helper = (
        (std::is_same<T, bool>::value        ? 'b' : 0) +

        ((std::is_same<T, uint8_t>::value ||
          std::is_same<T, uint16_t>::value ||
          std::is_same<T, uint32_t>::value ||
          std::is_same<T, uint64_t>::value)  ? 'u' : 0) +

        ((std::is_same<T, int8_t>::value ||
          std::is_same<T, int16_t>::value ||
          std::is_same<T, int32_t>::value ||
          std::is_same<T, int64_t>::value)   ? 'i' : 0) +
        
        ((std::is_same<T, float>::value ||
          std::is_same<T, double>::value ||
          std::is_same<T, long double>::value) ? 'f' : 0)

#if defined(_LIBCPP_COMPLEX_H) || defined(_LIBCPP_COMPLEX)
        +
        ((std::is_same<T, std::complex<float>>::value ||
          std::is_same<T, std::complex<double>>::value ||
          std::is_same<T, std::complex<long double>>::value) ? 'c' : 0)
#endif
        );

    template<typename T>
    constexpr char np_type_id = (np_type_id_helper<T> == 0) ? '?' : np_type_id_helper<T>;

    struct __attribute__((__packed__)) NpHeader
    {
        unsigned char head[8] = {0x93u, 'N', 'U', 'M', 'P', 'Y', 0x01u, 0x00u};
        uint16_t dsize;
        NpHeader(uint16_t dsize_)
            : dsize(dsize_)
        {}
    };
    
    template<typename T> std::string carr2npy(const T* data, const std::vector<size_t> shape) {
        assert(shape.size() != 0);
        assert(data != nullptr);
        
        size_t size = std::accumulate(shape.begin(),shape.end(),1,std::multiplies<size_t>());
        std::string buffer(sizeof(NpHeader), ' ');
        buffer += "{'descr': '";
        buffer += '<';
        buffer += np_type_id<T>;
        buffer += std::to_string(sizeof(T));
        buffer += "', 'fortran_order': False, 'shape': (";
        for(size_t s : shape)
        {
            buffer += std::to_string(s);
            buffer += ", ";
        }
        buffer.insert(buffer.end()-2, ')');
        buffer += "}";
        buffer.insert(buffer.end(), 16 - (buffer.size() % 16),' ');
        buffer.back() = '\n';
        
        NpHeader header(buffer.size() - sizeof(NpHeader));
        buffer.replace(0, sizeof(NpHeader), (const char*) &header, sizeof(NpHeader));

        buffer.insert(buffer.size(), (const char*)data, size * sizeof(T));
        return buffer;
    }

    struct NpyArray
    {
        std::string buffer;
        std::vector<size_t> shape;
        bool fortran_order;
        char type_symbol;
        unsigned type_size;

        template<typename T>
        bool is_of_type() const {
            return np_type_id<T> == type_symbol && sizeof(T) == type_size;
        }

        template<typename T>
        T* data() {
            assert(!buffer.empty());
            assert(is_of_type<T>());
            return reinterpret_cast<T*>(buffer.data());
        }

        size_t size() const {
            return buffer.size() / type_size;
        }
    };

    inline NpyArray npy2arr(std::string const& buffer)
    {
        NpHeader header(0);
        if(buffer.compare(0, sizeof(header.head), (char const*) header.head, sizeof(header.head)) != 0) {
            return {};
        }
        memcpy(&header, buffer.data(), sizeof(header));
        size_t offset = sizeof(header);
        std::string dict(buffer.data()+offset, header.dsize);
        offset += header.dsize;
        
        NpyArray nparr;

        size_t doffset = dict.find("'descr'", 0);
        doffset = dict.find(":", doffset+1);
        doffset = dict.find("'", doffset+1);
        char endianness = dict[doffset+1];
        if(endianness != '<' && endianness != '|') { return {}; }
        nparr.type_symbol = dict[doffset+2];
        doffset += 3;
        nparr.type_size = atoi(dict.substr(doffset, dict.find("'", doffset)).c_str());

        doffset = dict.find("'fortran_order'", 0);
        doffset = dict.find(":", doffset+1);
        doffset = dict.find_first_of("TF", doffset+1);
        nparr.fortran_order = (dict[doffset] == 'T');
        
        doffset = dict.find("'shape'", 0);
        doffset = dict.find(":", doffset+1);
        doffset = dict.find("(", doffset+1) + 1;
        std::stringstream shapestr(dict.substr(doffset,
                                               dict.find(")", doffset) - doffset));
        while(true) {
            constexpr size_t maxlen = 16;
            char numstr[maxlen];
            shapestr.getline(numstr, maxlen, ',');
            if(shapestr.fail()) { break; }
            nparr.shape.push_back(atoi(numstr));            
        }

        nparr.buffer = buffer.substr(offset);
        
        return nparr;
    }
    
    struct __attribute__((__packed__)) ZipLHeader
    {
        unsigned char sig[4] = {'P', 'K', 0x03u, 0x04u};
        uint16_t ver = 20;
        uint16_t gpbf = 0;
        uint16_t comp_method = 0;
        uint16_t last_mod_time = 0;
        uint16_t last_mod_date = 0;
        uint32_t crc;
        uint32_t cfsize;
        uint32_t ufsize;
        uint16_t fname_size;
        uint16_t xtrf_size = 0;
        
        ZipLHeader(uint32_t crc_, uint32_t fsize_, uint32_t fname_size_)
            : crc(crc_)
            , cfsize(fsize_)
            , ufsize(fsize_)
            , fname_size(fname_size_)
        {}
        
        ZipLHeader() {}
    };

    struct __attribute__((__packed__)) ZipGHeader
    {
        unsigned char sig[4] = {'P', 'K', 0x01u, 0x02u};
        uint16_t ver_made = 20;
        uint16_t ver = 20;
        uint16_t gpbf = 0;
        uint16_t comp_method = 0;
        uint16_t last_mod_time = 0;
        uint16_t last_mod_date = 0;
        uint32_t crc;
        uint32_t cfsize;
        uint32_t ufsize;
        uint16_t fname_size;
        uint16_t xtrf_size = 0;
        uint16_t fcomment_size = 0;
        uint16_t disk_number = 0;
        uint16_t internal_attr = 0;
        uint32_t external_attr = 0;
        uint32_t local_offset;
        ZipGHeader(uint32_t crc_, uint32_t fsize_, uint32_t fname_size_, uint32_t local_offset_)
            : crc(crc_)
            , cfsize(fsize_)
            , ufsize(fsize_)
            , fname_size(fname_size_)
            , local_offset(local_offset_)
        {}
    };

    struct __attribute__((__packed__)) ZipFooter
    {
        unsigned char sig[4] = {'P', 'K', 0x05u, 0x06u};
        uint16_t disk_n = 0;
        uint16_t dcisk_cd_n = 0;
        uint16_t nrec_disk;
        uint16_t nrec_total;
        uint32_t header_size;
        uint32_t header_offset;
        uint16_t comment_size = 0;
        ZipFooter(uint16_t nfiles, uint32_t header_size_, uint32_t header_offset_)
            : nrec_disk(nfiles)
            , nrec_total(nfiles)
            , header_size(header_size_)
            , header_offset(header_offset_)
        {}
    };

    inline std::string zip(const std::map<std::string, std::string> &files)
    {
        assert(!files.empty());
        
        std::string buffer;
        std::string global_header;
        
        for(auto const& file : files)
        {
            std::string const& fname = file.first;
            std::string const& fcontents = file.second;
            assert(!fname.empty());
            assert(!fcontents.empty());
            
            size_t local_offset = buffer.size();
            
            uint32_t crc = (uint32_t)crc32(0, (uint8_t*)fcontents.data(), (uint32_t)fcontents.size());

            ZipLHeader lheader(crc, fcontents.size(), fname.size());        
            buffer.insert(buffer.size(), (const char*)&lheader, sizeof(lheader));
            buffer += fname;
            buffer += fcontents;

            ZipGHeader gheader(crc, fcontents.size(), fname.size(), local_offset);
            global_header.insert(global_header.size(), (const char*)&gheader, sizeof(gheader));
            global_header += fname;
        }
        size_t global_offset = buffer.size();

        buffer += global_header;
        
        ZipFooter zfooter(files.size(), global_header.size(), global_offset);
        buffer.insert(buffer.size(), (const char*)&zfooter, sizeof(zfooter));
        return buffer;
    }

    inline std::map<std::string, std::string> unzip(const std::string &zipbuf)
    {
        std::map<std::string, std::string> files;
        size_t offset = 0;
        
        const size_t zipbuf_size = zipbuf.size();
        const char *zipbuf_data = zipbuf.data();
        
        while(zipbuf_size - offset > sizeof(ZipLHeader)) {
            ZipLHeader lheader;
            memcpy(&lheader, zipbuf_data + offset, sizeof(ZipLHeader));
            offset += sizeof(ZipLHeader);

            if(lheader.sig[0] != 'P' ||
               lheader.sig[1] != 'K' ||
               lheader.sig[2] != 0x03u ||
               lheader.sig[3] != 0x04u) {
                break;
            }

            if(zipbuf_size - offset <
               lheader.fname_size + lheader.xtrf_size + lheader.cfsize) {
                break;
            }

            std::string fname(zipbuf_data+offset, lheader.fname_size);
            offset += lheader.fname_size + lheader.xtrf_size;

            if(lheader.comp_method == 0) {
                files[fname] = std::string(zipbuf_data+offset, lheader.cfsize);
            } else {
                std::string inflated(lheader.ufsize, 0);
                int err;
                z_stream d_stream;
                d_stream.zalloc = Z_NULL;
                d_stream.zfree = Z_NULL;
                d_stream.opaque = Z_NULL;
                d_stream.avail_in = lheader.cfsize;
                d_stream.next_in = (unsigned char *)zipbuf_data+offset;
                d_stream.avail_out = lheader.ufsize;
                d_stream.next_out = (unsigned char*)&inflated[0];
                err = inflateInit2(&d_stream, -MAX_WBITS);
                err = inflate(&d_stream, Z_FINISH);
                err = inflateEnd(&d_stream);
                files[fname] = inflated;
            }
            offset += lheader.cfsize;
            uint32_t crc = (uint32_t)crc32(0, (uint8_t*)files[fname].data(), (uint32_t)files[fname].size());
            if(crc != lheader.crc) {
                files.erase(fname);
            }
        }
        return files;
    }
}

#endif
