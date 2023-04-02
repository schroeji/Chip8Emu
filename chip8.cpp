#include "chip8.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <algorithm>
#include <bitset>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

void UnknownInstruction(Chip8::Instruction instruction) {
  std::cout << "Unkown instruction: 0x" << std::hex << std::setw(2)
            << std::setfill('0')
            << static_cast<std::uint64_t>(instruction.first)
            << static_cast<std::uint64_t>(instruction.second) << std::endl;
}

void Chip8::UpdateTimers() {
  if (delay_timer_ > 0)
    --delay_timer_;
  if (sound_timer_ > 0) {
    Mix_PlayChannel(-1, mix_chunk_, 0);
    sound_timer_ = 0;
  }
}

void Chip8::Init() {
  // Load font sprites to memory
  std::copy(kFontSprites.begin(), kFontSprites.end(), &memory_[kFontLocation]);

  // Initialize SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    running_ = false;
  }
  window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, kScreenWidth,
                            kScreenHeight, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    running_ = false;
  }
  if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512) < 0) {
    printf("Could not open Audio: %s\n", SDL_GetError());
    running_ = false;
  }
  Mix_AllocateChannels(2);
  mix_chunk_ = Mix_LoadWAV(kSoundFile.c_str());
  if (mix_chunk_ == NULL) {
    printf("Could not load wav: %s\n", SDL_GetError());
    running_ = false;
  }

  pc_ = kProgramStart;
}

void Chip8::Run() {
  Init();
  // Create renderer
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_RenderSetLogicalSize(renderer, kScreenWidth, kScreenHeight);
  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING, 64, 32);

  while (running_) {

    Instruction instruction{Fetch()};
    printf("Got instrucion %.2x %.2X\n", instruction.first, instruction.second);
    pc_ += 2;
    Decode(instruction);
    DrawToScreen(renderer, texture);
    UpdateTimers();
    HandleSdlEvents();

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  Mix_FreeChunk(mix_chunk_);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

Chip8::Instruction Chip8::Fetch() { return {memory_[pc_], memory_[pc_ + 1]}; }

void Chip8::Draw(std::uint8_t x, std::uint8_t y, std::uint8_t n) {
  redraw_ = true;

  // printf("X:  %.2X\n", x);
  // printf("Y:  %.2X\n", y);
  // printf("Values from registers %.2X, %.2X \n", x, y);
  std::uint8_t const screen_x{
      static_cast<std::uint8_t>(variable_registers_[x] % 64)};
  std::uint8_t const screen_y{
      static_cast<std::uint8_t>(variable_registers_[y] % 32)};
  // printf("\nDrawing to %.2X, %.2X \n", screen_x, screen_y);
  // printf("screen X:  %.2X\n", screen_x);
  // printf("screen Y:  %.2X\n", screen_y);

  variable_registers_[0xf] = 0;

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

  std::uint8_t const x{static_cast<std::uint8_t>(instruction.first & 0x0F)};

  std::uint8_t const y{static_cast<std::uint8_t>(
      static_cast<std::uint8_t>(instruction.second & 0xF0) >> 4)};

  std::uint8_t const n{static_cast<std::uint8_t>(instruction.second & 0xF)};

  std::uint16_t nnn{static_cast<std::uint16_t>((x << 8) + instruction.second)};

  // printf("First nibble:  %.2X\n", first_nibble);
  // printf("Second nibble:  %.2X\n", x);
  // printf("first nibble second byte:  %.2X\n", y);
  // printf("nnn:  %.2X\n", nnn);
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
    break;
  case 3:
    if (variable_registers_[x] == instruction.second)
      pc_ += 2;
    break;
  case 4:
    if (variable_registers_[x] != instruction.second)
      pc_ += 2;
    break;
  case 5:
    if (variable_registers_[x] == variable_registers_[y])
      pc_ += 2;
    break;

  case 6:
    variable_registers_[x] = instruction.second;
    break;
  case 7:
    variable_registers_[x] += instruction.second;
    break;
  case 8:
    if (n == 0) {
      variable_registers_[x] = variable_registers_[y];
    } else if (n == 1) {
      variable_registers_[x] |= variable_registers_[y];
    } else if (n == 2) {
      variable_registers_[x] &= variable_registers_[y];
    } else if (n == 3) {
      variable_registers_[x] ^= variable_registers_[y];
    } else if (n == 4) {
      if (variable_registers_[x] + variable_registers_[y] > 255) {
        variable_registers_[0xf] = 1;
      } else {
        variable_registers_[0xf] = 0;
      }
      variable_registers_[x] += variable_registers_[y];
    } else if (n == 5) {
      if (variable_registers_[x] >= variable_registers_[y]) {
        variable_registers_[0xf] = 1;
      } else {
        variable_registers_[0xf] = 0;
      }
      variable_registers_[x] -= variable_registers_[y];
    } else if (n == 6) {
      std::bitset<8> vx{variable_registers_[x]};
      if (vx[0])
        variable_registers_[0xf] = 1;
      variable_registers_[x] = (variable_registers_[x] >> 1);
    } else if (n == 7) {
      if (variable_registers_[x] < variable_registers_[y]) {
        variable_registers_[0xf] = 1;
      } else {
        variable_registers_[0xf] = 0;
      }
      variable_registers_[x] = variable_registers_[y] - variable_registers_[x];
    } else if (n == 0xe) {
      std::bitset<8> vx{variable_registers_[x]};
      if (vx[7])
        variable_registers_[0xf] = 1;
      variable_registers_[x] = (variable_registers_[x] << 1);
    } else {
      UnknownInstruction(instruction);
    }
    break;
  case 9:
    if (variable_registers_[x] != variable_registers_[y])
      pc_ += 2;
    break;
  case 10:
    index_register_ = nnn;
    break;
  case 11:
    pc_ = nnn + variable_registers_[0];
    break;
  case 12: {
    std::default_random_engine generator{};
    std::uniform_int_distribution<std::uint8_t> distribution{0, 255};
    variable_registers_[x] = distribution(generator) & instruction.second;
    break;
  }
  case 13:
    Draw(x, y, instruction.second & 0x0F);
    break;
  case 14:
    if (instruction.second == 0x9e) {
      if (keys_pressed_[variable_registers_[x] & 0xf]) {
        pc_ += 2;
      }
    } else if (instruction.second == 0xa1) {
      if (!keys_pressed_[variable_registers_[x] & 0xf]) {
        pc_ += 2;
      }
    } else {
      UnknownInstruction(instruction);
    }
    break;
  case 15:
    if (instruction.second == 0x07) {
      variable_registers_[x] = delay_timer_;
    } else if (instruction.second == 0x0a) {
      if (std::none_of(keys_pressed_.begin(), keys_pressed_.end(),
                       [](bool val) { return val; })) {
        pc_ -= 2;
      } else {
        for (std::size_t i{0}; i < 16; i++) {
          if (keys_pressed_[i])
            variable_registers_[x] = i;
          break;
        }
      }

    } else if (instruction.second == 0x15) {
      delay_timer_ = variable_registers_[x];
    } else if (instruction.second == 0x18) {
      sound_timer_ = variable_registers_[x];
    } else if (instruction.second == 0x1e) {
      if (index_register_ + variable_registers_[x] > 0xFFF) {
        variable_registers_[0xf] = 1;
      } else {
        variable_registers_[0xf] = 0;
      }
      index_register_ += variable_registers_[x];
      index_register_ &= 0xFFF;
    } else if (instruction.second == 0x29) {
      index_register_ = kFontLocation + 5 * (variable_registers_[x] & 0x0f);
    } else if (instruction.second == 0x33) {
      memory_[index_register_] = variable_registers_[x] / 100;
      memory_[index_register_] = (variable_registers_[x] % 100) / 10;
      memory_[index_register_ + 2] = variable_registers_[x] % 10;
    } else if (instruction.second == 0x55) {
      for (std::size_t i{0}; i <= x; i++) {
        memory_[index_register_ + i] = variable_registers_[i];
      }
    } else if (instruction.second == 0x65) {
      for (std::size_t i{0}; i <= x; i++) {
        variable_registers_[i] = memory_[index_register_ + i];
      }
    } else {
      UnknownInstruction(instruction);
    }
    break;
  default:
    UnknownInstruction(instruction);
    break;
  }
}

void Chip8::DrawToScreen(SDL_Renderer *renderer, SDL_Texture *texture) {
  redraw_ = false;
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
    std::size_t size{static_cast<std::size_t>(ifs.tellg())};
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

void Chip8::HandleSdlEvents() {
  SDL_Event e{};
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      running_ = false;
      std::cout << "Quit" << std::endl;
    } else if (e.type == SDL_KEYDOWN) {
      if (e.key.keysym.sym == SDLK_ESCAPE) {
        running_ = false;
      }
      std::size_t index{kKeyMap[e.key.keysym.sym]};
      keys_pressed_[index] = true;
    } else if (e.type == SDL_KEYUP) {
      std::size_t index{kKeyMap[e.key.keysym.sym]};
      keys_pressed_[index] = false;
    }
  }
}
