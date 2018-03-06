#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>

std::vector<std::string> get_available_ports(std::string file)
{
  std::vector<std::string> l;
  int fd;
  std::string port = "";
  for(int i = 0;i < 256;++i)
  {
    port.clear();
    port.append(file);
    port.append(std::to_string(i));
    fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd != -1)
      l.push_back(port);
  }
  return l;
}
