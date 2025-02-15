#include <iostream>
#include <unistd.h>

int main() {
  for (;;) {
    // sleep 3 sec
    sleep(3);
    std::cout << "hello, this is demo!" << std::endl;
  }
  return 0;
}