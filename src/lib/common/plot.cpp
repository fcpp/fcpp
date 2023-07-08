// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "lib/common/plot.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for plot building tools.
namespace plot {

//! @cond INTERNAL
namespace details {
    //! @brief Check if character is a vowel.
    bool isvowel(char c) {
        std::string vowels = "aeiouAEIOU";
        for (char v : vowels) if (c == v) return true;
        return false;
    }
    //! @brief Check if character is alphabetic.
    bool isalpha(char c) {
        if ('a' <= c and c <= 'z') return true;
        if ('A' <= c and c <= 'Z') return true;
        return false;
    }
    //! @brief Shortens a string.
    std::string shorten(std::string s) {
        std::string pre;
        std::string main;
        while (not isalpha(s.back())) s.pop_back();
        bool vowel = false;
        for (size_t i=0; i<s.size(); ++i) {
            if (isalpha(s[i])) {
                if (main.size() < 3) main.push_back(s[i]);
                else if (isvowel(s[i])) vowel = true;
                else if (not vowel) main.push_back(s[i]);
            } else {
                if (main.size()) pre.push_back(main[0]);
                main = "";
                vowel = false;
            }
        }
        return pre + main;
    }
    //! @brief Shortens a title string.
    std::string multi_shorten(std::string s) {
        if (s.empty()) return s;
        std::vector<std::string> tags;
        std::vector<std::string> vals;
        size_t pos = 0, np;
        while (true) {
            np = s.find(" = ", pos);
            tags.push_back(shorten(s.substr(pos, np-pos)));
            pos = np+3;
            np = s.find(", ", pos);
            vals.push_back(s.substr(pos, np-pos));
            if (np == std::string::npos) break;
            pos = np+2;
        }
        s = "";
        for (size_t i=0; i<tags.size(); ++i)
            s += tags[i] + vals[i];
        return s;
    }
}
//! @endcond

}


}
