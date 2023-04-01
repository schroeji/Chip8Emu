#include "chip8.h"
#include <SDL2/SDL.h>
#include <bitset>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

void UnknownInstruction(Chip8::Instruction instruction) {
  std::cout << "Unkown instruction: 0x" << std::hex << std::setw(2)
            << std::setfill('0')
            << static_cast<std::uint64_t>(instruction.first)
            << static_cast<std::uint64_t>(instruction.second) << std::endl;
}

void Chip8::Run() {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    exit(1);
  }
  window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, kScreenWidth,
                            kScreenHeight, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    exit(2);
  }
  // Create renderer
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_RenderSetLogicalSize(renderer, kScreenWidth, kScreenHeight);
  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING, 64, 32);

  pc_ = kProgramStart;
  while (true) {
    Instruction instruction{Fetch()};
    printf("Got instruction:  %.2X%.2X\n", instruction.first,
           instruction.second);
    pc_ += 2;
    Decode(instruction);
    DrawToScreen(renderer, texture);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

Chip8::Instruction Chip8::Fetch() { return {memory_[pc_], memory_[pc_ + 1]}; }

void Chip8::Draw(std::uint8_t x, std::uint8_t y, std::uint8_t n) {
  redraw_ = true;

  // printf("X:  %.2X\n", x);
  // printf("Y:  %.2X\n", y);
  printf("Values from registers %.2X, %.2X \n", x, y);
  std::uint8_t const screen_x{
      static_cast<std::uint8_t>(variable_registers_[x] % 64)};
  std::uint8_t const screen_y{
      static_cast<std::uint8_t>(variable_registers_[y] % 32)};
  printf("\nDrawing to %.2X, %.2X \n", screen_x, screen_y);
  std::uint8_t pixel;
  // printf("screen X:  %.2X\n", screen_x);
  // printf("screen Y:  %.2X\n", screen_y);

  variable_registers_[0xf] = 0;
  // for (int yline = 0; yline < n; yline++) {
  //   std::bitset<8> const sprite_byte{memory_[index_register_ + yline]};
  //   pixel = memory_[index_register_ + yline];
  //   for (int xline = 0; xline < 8; xline++) {
  //     // bool const bit_set{static_cast<bool>((pixel & (0x80 >> xline))
  //     != 0)}; if (sprite_byte[7 - xline]) {
  //       if (display_[(screen_x + xline + ((screen_y + yline) * 64))] ==
  //       1) {
  //         variable_registers_[0xF] = 1;
  //       }
  //       display_[screen_x + xline + (screen_y + yline) * 64] ^= 1;
  //     }
  //   }
  // }

  for (std::size_t row{0}; row < n; row++) {
    // printf("index register:  %.2X\n", index_register_);
    std::bitset<8> const sprite_byte{memory_[index_register_ + row]};
    if (screen_y + row >= 32)
      break;
    for (std::size_t col{0}; col < 8; col++) {
      if (screen_x + col >= 64)
        break;
      // printf("screen X:  %d", screen_x + row);
      // printf("screen Y:  %d", screen_y + col);
      bool pixel{display_[screen_y + row][screen_x + col]};
      // if (sprite_byte[7 - col]) {
      //   if (pixel)
      //     variable_registers_[0xf] = 1;
      //   pixel ^= 1;
      //   display_[(screen_y + row) * 64 + (screen_x + col)] = pixel;
      // }
      if (pixel && sprite_byte[7 - col]) {
        variable_registers_[0xf] = 1;
      }
      pixel ^= sprite_byte[7 - col];

      display_[screen_y + row][screen_x + col] = pixel;
    }
  }
}

void Chip8::Decode(Instruction instruction) {
  std::uint8_t const first_nibble{static_cast<std::uint8_t>(
      static_cast<std::uint8_t>(instruction.first & 0xF0) >> 4)};

  std::uint8_t const second_nibble{
      static_cast<std::uint8_t>(instruction.first & 0x0F)};

  std::uint8_t const first_nibble_second_byte{static_cast<std::uint8_t>(
      static_cast<std::uint8_t>(instruction.second & 0xF0) >> 4)};

  std::uint16_t nnn{
      static_cast<std::uint16_t>((second_nibble << 8) + instruction.second)};

  printf("First nibble:  %.2X\n", first_nibble);
  printf("Second nibble:  %.2X\n", second_nibble);
  printf("first nibble second byte:  %.2X\n", first_nibble_second_byte);
  printf("nnn:  %.2X\n", nnn);
  switch (first_nibble) {
  case 0:
    if (instruction == std::pair<std::uint8_t, std::uint8_t>{0x00, 0xE0}) {
      std::memset(display_.data(), 0, 64 * 32);
      // for (auto &array : display_) {
      //   std::memset(array.data(), 0, 64);
      // }
    } else if (instruction ==
               std::pair<std::uint8_t, std::uint8_t>{0x00, 0xEE}) {
      pc_ = stack_.top();
      stack_.pop();
    } else {
      UnknownInstruction(instruction);
    }
    break;

  case 1:
    pc_ = nnn;
    break;

  case 2:
    stack_.push(pc_);
    pc_ = nnn;
  case 6:
    variable_registers_[second_nibble] = instruction.second;
    break;
  case 7:
    variable_registers_[second_nibble] += instruction.second;
    break;
  case 10:
    index_register_ = nnn;
    break;
  case 13:
    Draw(second_nibble, first_nibble_second_byte, instruction.second & 0x0F);
    break;
  default:
    UnknownInstruction(instruction);
    break;
  }
}

void Chip8::DrawToScreen(SDL_Renderer *renderer, SDL_Texture *texture) {
  redraw_ = false;
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 64; col++) {
      if (display_[row][col] == 0) {
        std::cout << "0";
      } else {
        std::cout << "1";
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
  // Create texture that stores frame buffer
  for (int y = 0; y < 32; ++y) {
    for (int x = 0; x < 64; ++x) {
      pixels_[y * 64 + x] = (0x00FFFFFF * display_[y][x]) | 0xFF000000;
    }
  }
  SDL_UpdateTexture(texture, NULL, pixels_.data(), 64 * sizeof(Uint32));
  // Clear screen and render
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void Chip8::LoadRom(std::string &path) {
  std::ifstream ifs(path, std::ios::binary | std::ios::ate);
  if (ifs) {
    long size{ifs.tellg()};
    ifs.seekg(0, std::ios::beg);
    if (size < kMemorySize - kProgramStart) {
      std::vector<char> buffer(size);
      ifs.read(buffer.data(), size);

      std::memcpy(&memory_.data()[kProgramStart], buffer.data(), size);
      std::cout << "Successfully loaded " << buffer.size() << " bytes from "
                << path << std::endl;
    } else {
      std::cout << "ROM file too large. Size " << size << std::endl;
    }
    ifs.close();
  } else {
    std::cout << "Failed to load ROM from " << path << std::endl;
  }
}
