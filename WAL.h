#ifndef __WAL_H__
#define __WAL_H__

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#else
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif

typedef struct _WAL_Window {
    struct {
#ifdef _WIN32
        HWND hW;
        HDC hDC;
        HGLRC hRC;
#else
        Window hW;
        Display *hDC;
        Atom quitMessage;
        GLXContext hRC;
#endif
    } _os;
    unsigned char keyboard[0xFF];
    struct {
        long x, y;
        unsigned char lbtn, rbtn;
    } mouse;
} WAL_Window;

/* WAL_Create():
    + Creates a OpenGL window with depth and RGBA support.
Returns:
    0: Success
    1: Error
Notes:
    If the function returns 1, its safe to assume that it has free'd everything (a clean exit)
Usage:
    WAL_Create(win); while (WAL_Update(win)) { ... } WAL_Destroy(win);
*/
int WAL_Create(WAL_Window* w, char* title, int width, int height);

/* WAL_Update():
    + Handles incomming messages to the window (actions).
    + Swaps the screen buffers.
Returns:
    0: User has pressed the close button.
    1: We should keep running as normal.
Usage:
    WAL_Create(win); while (WAL_Update(win)) { ... } WAL_Destroy(win);
*/
int WAL_Update(WAL_Window* w);

/* WAL_Destroy():
    + Destroys every resource created by WAL_Create
Returns:
    None
Usage:
    WAL_Create(win); while (WAL_Update(win)) { ... } WAL_Destroy(win);
*/
void WAL_Destroy(WAL_Window* w);

#endif /* __WAL_H__ */