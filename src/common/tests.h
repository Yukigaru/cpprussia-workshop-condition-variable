#pragma once
#include <stdexcept>
#include <string>
#include <iostream>


#define EXPECT(expr) \
    if (!(expr)) { \
        std::string msg = "Test " + std::string{__PRETTY_FUNCTION__} + " failed: " #expr " at line " + std::to_string(__LINE__); \
        throw std::runtime_error(msg); \
    }

#define PASS() \
    std::cout << "Test " << __PRETTY_FUNCTION__ << " passed." << std::endl;

