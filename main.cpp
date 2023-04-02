#include "chip8.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  Chip8 chip8{};
  if (argc >= 2) {
    std::string rom_file{argv[1]};
    chip8.LoadRom(rom_file);
    chip8.Run();
  } else {
    std::cout << "Specify ROM file." << std::endl;
  }
  return 0;
}
