#ifndef _COMMON_H__
#define _COMMON_H__

#include <climits>
#include <utility>
#include <unordered_map>
#include <cstdlib>
#include <iostream>
#include  <memory>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_map>
#include "message.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "amazon.pb.h"
#include <unistd.h>
#include <sys/epoll.h>


using namespace pqxx;
using std::string;
using std::map;
using std::vector;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::unique_ptr;
typedef google::protobuf::io::FileOutputStream socket_out;
typedef google::protobuf::io::FileInputStream socket_in;

#endif
