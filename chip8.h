#ifndef CHIP8_H_
#define CHIP8_H_

#include <SDL2/SDL.h>
#include <array>
#include <cstdint>
#include <stack>
#include <string>

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

  static constexpr std::size_t kMemorySize{4096};
  static constexpr std::size_t kScreenHeight{512};
  static constexpr std::size_t kScreenWidth{1024};
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

  // Constants
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
  SDL_Window *window;
  bool redraw_{true};
};

#endif // CHIP8_H_
