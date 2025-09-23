```markdown
# Elastic Collision π Simulation

A physics simulation and visualization project that demonstrates the fascinating relationship between **elastic collisions** and the mathematical constant **π**.  
By simulating two blocks colliding with each other and with a wall, the total number of collisions approximates:

\[
\text{Collisions} \approx \pi \times 10^{n/2}
\]

where `n` is the exponent of the mass ratio between the two blocks.

---

## ✨ Features

- 🔹 **High-precision physics engine** using `long double` for accurate calculations  
- 🔹 **Adaptive time-stepping** and **continuous collision detection (CCD)** to prevent tunneling  
- 🔹 **Visualization** with EasyX graphics for real-time animation of collisions  
- 🔹 **Dynamic statistics display**: collision count, theoretical value, and error margin  
- 🔹 **Analytical approximation** for large mass ratios to improve performance  
- 🔹 **Educational value**: illustrates the surprising connection between physics and mathematics  

---

## 📂 Project Structure

```
.
├── main.c / main.cpp   # Core simulation code
├── README.md           # Project documentation
└── (other files)       # Supporting headers, configs, etc.
```

---

## 🚀 Getting Started

### Prerequisites
- Windows environment (EasyX graphics library is Windows-only)  
- C/C++ compiler (e.g., MSVC, MinGW)  
- [EasyX graphics library](https://easyx.cn/) installed  

### Build & Run
1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/elastic-collision-pi.git
   cd elastic-collision-pi
   ```
2. Open the project in **Visual Studio** or compile with your preferred compiler.  
3. Run the executable.  
4. Enter the mass ratio exponent `n` when prompted (e.g., `n = 2, 4, 6...`).  

---

## 📖 Usage

- The program will simulate two blocks colliding with each other and the wall.  
- The **red block** has mass = 1, initially at rest.  
- The **blue block** has mass = 10^n, moving towards the red block.  
- The simulation counts collisions and compares them with the theoretical value.  

Example output:
```
Mass ratio exponent n = 4
Theoretical collisions ≈ 314
Simulated collisions   = 314
Error                  = 0.00%
```

---

## 🧮 Theory Behind

This project is based on a well-known mathematical curiosity:  
When two blocks collide elastically with each other and a wall, the number of collisions encodes digits of **π**.  
For a mass ratio of `10^n : 1`, the total number of collisions is approximately:

\[
\pi \times 10^{n/2}
\]

This provides a beautiful bridge between **classical mechanics** and **pure mathematics**.

---

## 📊 Example Visualization

- Wall on the left, ground at the bottom  
- Red block (mass = 1) near the wall  
- Blue block (mass = 10^n) moving left  
- Collisions are animated in real-time  

---

## 📜 License

This project is licensed under the **MIT License** – see the [LICENSE](LICENSE) file for details.  
(You can replace this with Apache 2.0, GPL, or another license depending on your choice.)

---

## 🙌 Acknowledgements

- Inspired by the "Pi by Collisions" problem popularized by Grant Sanderson (3Blue1Brown).  
- Uses [EasyX](https://easyx.cn/) for graphics rendering.  

---

## 🔮 Future Improvements

- Cross-platform support (replace EasyX with SDL/OpenGL)  
- Performance optimization for extremely large `n`  
- Interactive UI with adjustable parameters  
- Export simulation data for analysis  

---
