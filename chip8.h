#ifndef CHIP8_H_
#define CHIP8_H_

#include <SDL2/SDL.h>
#include <array>
#include <cstdint>
#include <map>
#include <stack>
#include <string>
#include <unordered_map>

class Chip8 {
public:
  using Instruction = std::pair<std::uint8_t, std::uint8_t>;

  void Run();

  void LoadRom(std::string &path);

private:
  void Draw(std::uint8_t, std::uint8_t, std::uint8_t);

  Instruction Fetch();

  void Decode(Instruction);

  void DrawToScreen(SDL_Renderer *sdl_renderer, SDL_Texture *sdl_texture);

  void UpdateTimers();

  void HandleSdlEvents();

  // Constants
  static constexpr std::size_t kMemorySize{4096};
  static constexpr std::size_t kScreenHeight{512};
  static constexpr std::size_t kScreenWidth{1024};
  static constexpr std::size_t kFontLocation{0x050};
  static constexpr std::uint16_t kProgramStart{0x200};
  static constexpr std::array<std::uint8_t, 80> kFontSprites{
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };

  std::unordered_map<SDL_Keycode, std::size_t> kKeyMap{
      {SDLK_x, 0},   {SDLK_1, 1},   {SDLK_2, 2},   {SDLK_3, 3},
      {SDLK_q, 4},   {SDLK_w, 5},   {SDLK_e, 6},   {SDLK_a, 7},
      {SDLK_s, 8},   {SDLK_d, 9},   {SDLK_z, 0xa}, {SDLK_c, 0xb},
      {SDLK_4, 0xc}, {SDLK_r, 0xd}, {SDLK_f, 0xe}, {SDLK_v, 0xf}};

  // Registers
  std::uint16_t pc_{};
  std::uint16_t index_register_{};
  std::uint8_t delay_timer_{};
  std::uint8_t sound_timer_{};
  std::array<std::uint8_t, 16> variable_registers_;

  // Memory
  std::stack<std::uint16_t> stack_{};
  std::array<std::uint8_t, kMemorySize> memory_{};
  std::array<std::array<bool, 64>, 32> display_{};
  std::array<std::uint32_t, 64 * 32> pixels_{};
  std::array<bool, 16> keys_pressed_{};

  SDL_Window *window;
  bool redraw_{true};
  bool running_{true};
};

#endif // CHIP8_H_
