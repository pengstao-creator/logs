#ifndef LOG_PCH_HPP
#define LOG_PCH_HPP

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <atomic>
#include <sstream>
#include <functional>
#include <condition_variable>
#include <chrono>
#include <queue>
#include <future>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <cstdio>
#include <cstddef>
#include <type_traits>
#include <algorithm>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <glob.h>
#endif

#endif
