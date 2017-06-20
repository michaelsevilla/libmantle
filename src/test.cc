#include <iostream>
#include <Mantle.h>

int main() {
  std::cout << "Hello, world!" << std::endl;
  Mantle mantle;
  int ret = mantle.start();
  std::cout << "... start()=" << ret << std::endl;
  ret = mantle.execute("BAL_LOG(0, \"Hello world from Lua!\")");
  std::cout << "... execute()=" << ret << std::endl;
}
