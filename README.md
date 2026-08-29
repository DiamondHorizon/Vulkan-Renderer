# Vulkan Renderer

A high-performance rendering engine built with Vulkan, designed for developing graphics-intensive applications and games. This project demonstrates modern graphics API usage with efficient rendering techniques.

---

## 🎨 Features

- **Vulkan Graphics API** — Modern, low-overhead GPU rendering
- **Cross-Platform Window Management** — GLFW integration for window and input handling
- **RPG Prototype** — Example game engine built on top of the renderer
- **Optimized Performance** — Direct GPU communication for maximum efficiency
- **Visual Studio Support** — Fully integrated solution for Windows development

---

## 🛠️ Tech Stack

- **C** — Core rendering implementation
- **Vulkan** — GPU graphics API
- **GLFW** — Window and input management
- **Visual Studio** — Development environment

---

## 📋 Requirements

- **Windows** (64-bit)
- **Visual Studio 2015 or later** (2022 recommended)
- **Vulkan SDK** — [Download](https://www.lunarg.com/vulkan-sdk/)
- **GPU with Vulkan support**

---

## 🚀 Getting Started

### Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/DiamondHorizon/Vulkan-Renderer.git
   cd Vulkan-Renderer
   ```

2. Install the Vulkan SDK from [LunarG](https://www.lunarg.com/vulkan-sdk/)

3. Open `RPGPrototype.sln` in Visual Studio

4. Build the solution (Build → Build Solution or Ctrl+Shift+B)

### Running

- Press F5 to run with debugging, or Ctrl+F5 to run without debugging

---

## 📁 Project Structure

```
Vulkan-Renderer/
├── RPGPrototype/           # Main game/rendering engine
│   ├── dependencies/       # Third-party libraries
│   │   ├── glfw/          # Window and input library
│   │   └── vulkan/        # Vulkan headers and libraries
│   └── src/               # Engine source code
├── x64/                   # 64-bit build output
├── RPGPrototype.sln       # Visual Studio solution
└── README.md
```

---

## 🎯 Project Goals

- Learn and master modern graphics API design
- Implement efficient rendering pipelines
- Build the foundation for a 3D game engine
- Demonstrate best practices in Vulkan usage

---

## 📚 Resources

- [Vulkan Documentation](https://www.khronos.org/vulkan/)
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- [GLFW Documentation](https://www.glfw.org/documentation.html)
- [Vulkan Tutorials](https://vulkan-tutorial.com/)

---

## 📄 License

This project is open source and available under the terms specified in the repository.

---

**Built by DiamondHorizon**
