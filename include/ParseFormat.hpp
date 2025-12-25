#pragma once
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class ParseFormat {
public:
  ParseFormat() = default;
  ~ParseFormat() = default;

  template <class... Args> std::string parse(std::string format, Args... args) {
    std::vector<std::string> args_str = {toString(args)...};
    size_t arg_index = 0;
    size_t pos = 0;

    // 替换花括号占位符
    while ((pos = format.find("{}", pos)) != std::string::npos) {
      if (arg_index < args_str.size()) {
        // 有匹配的参数，进行替换
        format.replace(pos, 2, args_str[arg_index]);
        pos += args_str[arg_index].length();
        arg_index++;
      } else {
        // 没有匹配的参数，跳过当前占位符
        pos += 2;
      }
    }

    return format;
  }

private:
  template <class T> std::string toString(T value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
  }

private:
  std::string _formatted;
};

