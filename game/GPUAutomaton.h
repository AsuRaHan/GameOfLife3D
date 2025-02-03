//#include <GL/glew.h>
#include "../system/GLFunctions.h"
#include <vector>

class GPUAutomaton {
private:
    GLuint computeProgram;
    GLuint cellsBuffer[2]; // Два буфера для двойной буферизации
    GLint bufferIndex; // Индекс текущего буфера (0 или 1)
    int gridWidth, gridHeight;
    int gridSize;

    void CreateComputeShader();
    void SetupBuffers();
    void DispatchCompute();

    void CheckShaderCompilation(GLuint shader, const std::string& name);
    void CheckProgramLinking(GLuint program);
public:
    GPUAutomaton(int width, int height);
    ~GPUAutomaton();
    void Update(); // Метод для обновления мира
    void GetGridState(std::vector<int>& outState); // Метод для извлечения текущего состояния сетки
    void SetGridState(const std::vector<int>& inState); // Метод для установки начального состояния сетки
};