/* WAL - Example showing Depth, RGBA and cross-platform support */

#include "WAL.h"

#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>

float angle = 0.0f;

void draw() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, 1.0, 20.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glRotatef(angle, 1.0f, 1.0f, 2.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_QUADS);
    glColor4ub(255, 0, 0, 127);
    glVertex3f(-0.5, -0.5, 0.5);
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(-0.5, 0.5, 0.5);

    glColor4ub(0, 255, 0, 127); 
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(-0.5, 0.5, -0.5);
    glVertex3f(0.5, 0.5, -0.5);
    glVertex3f(0.5, -0.5, -0.5);

    glColor4ub(0, 0, 255, 127);
    glVertex3f(-0.5, 0.5, -0.5);
    glVertex3f(-0.5, 0.5, 0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(0.5, 0.5, -0.5);

    glColor4ub(255, 255, 0, 127);
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(0.5, -0.5, -0.5);
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(-0.5, -0.5, 0.5);

    glColor4ub(255, 0, 255, 127);
    glVertex3f(0.5, -0.5, -0.5);
    glVertex3f(0.5, 0.5, -0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(0.5, -0.5, 0.5);

    glColor4ub(0, 255, 255, 127);
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(-0.5, -0.5, 0.5);
    glVertex3f(-0.5, 0.5, 0.5);
    glVertex3f(-0.5, 0.5, -0.5);
    glEnd();

    angle += 1.0f;
}

int main() {
    WAL_Window win;
    
    if (WAL_Create(&win, "WAL - Test", 800, 600)) {
        fprintf(stderr, "WAL_Create failed.\n");
        return 1;
    }

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (WAL_Update(&win) && win.keyboard[27] == 0) {
        printf("x: %lu | y: %lu | lbtn: %u | rbtn: %u\n", win.mouse.x, win.mouse.y, win.mouse.lbtn, win.mouse.rbtn);
        printf("is 'A' pressed? %d\n", win.keyboard[65]);
        draw();
    }

    WAL_Destroy(&win);
    return 0;
}