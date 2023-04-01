#include "chip8.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  Chip8 chip8{};
  std::string rom_file{"roms/IBM Logo.ch8"};
  chip8.LoadRom(rom_file);
  chip8.Run();
  return 0;
}
