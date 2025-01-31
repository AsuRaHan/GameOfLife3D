#include "DebugOverlay.h"

DebugOverlay::DebugOverlay() : isSimulationRunning(false), windowWidth(800), windowHeight(600) {}

void DebugOverlay::onWindowResize(float width, float height) {
    windowWidth = width;
    windowHeight = height;
}

void DebugOverlay::setSimulationRunning(bool running) {
    isSimulationRunning = running;
}

void DebugOverlay::draw() const {
    // ������������� � 2D ����� ��� ��������� ����������
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, windowHeight, 0); // ���������� ������� ������� ����
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // ������ ���� ������� (����), ���� ����������� (�����) � ����������� �� ���������
    if (isSimulationRunning) {
        // ����������� ��� "�����"
        glColor3f(0.0f, 1.0f, 0.0f); // ������� ���� ��� �����
        glBegin(GL_TRIANGLES);
        glVertex2f(10, 10);
        glVertex2f(50, 30);
        glVertex2f(10, 50);
        glEnd();
    }
    else {
        // ������� ��� "����"
        glColor3f(1.0f, 0.0f, 0.0f); // ������� ���� ��� ����
        glBegin(GL_QUADS);
        glVertex2f(10, 10);
        glVertex2f(50, 10);
        glVertex2f(50, 50);
        glVertex2f(10, 50);
        glEnd();
    }

    // ��������������� �������
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}