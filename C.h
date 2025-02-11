#ifndef HEADER
#define HEADER
#pragma once
#include <string>
#include <string_view>

namespace hex {
    inline std::string encode(std::string_view s) {
        std::string res;
        for (unsigned char c : s) {
            unsigned char high = c >> 4;
            unsigned char low = c & 0x0F;
            char hx1, hx2;
            if (high < 10) {
                hx1 = high + '0';
            } else {
                hx1 = high + 'a' - 10;
            }
            if (low < 10) {
                hx2 = low + '0';
            } else {
                hx2 = low + 'a' - 10;
            }
            res.push_back(hx1);
            res.push_back(hx2);
        }
        return res;
    }

    inline std::string decode(std::string_view s) {
        std::string res;
        for (long unsigned int i = 0; i < s.size(); i += 2) {
            char c1 = s[i];
            char c2 = s[i + 1];
            unsigned char fir, sec;
            if (c1 >= '0' && c1 <= '9') {
                fir = c1 - '0';
            } else {
                fir = c1 - 'a' + 10;
            }
            if (c2 >= '0' && c2 <= '9') {
                sec = c2 - '0';
            } else {
                sec = c2 - 'a' + 10;
            }
            char ress = (fir << 4) | sec;
            res.push_back(ress);
        }
        return res;
    }
}

#endif
