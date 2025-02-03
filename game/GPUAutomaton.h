//#include <GL/glew.h>
#include "../system/GLFunctions.h"
#include <vector>

class GPUAutomaton {
private:
    GLuint computeProgram;
    GLuint cellsBuffer[2]; // ��� ������ ��� ������� �����������
    GLint bufferIndex; // ������ �������� ������ (0 ��� 1)
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
    void Update(); // ����� ��� ���������� ����
    void GetGridState(std::vector<int>& outState); // ����� ��� ���������� �������� ��������� �����
    void SetGridState(const std::vector<int>& inState); // ����� ��� ��������� ���������� ��������� �����
};