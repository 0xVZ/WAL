#include "WAL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    WAL_Window* w = (WAL_Window*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);

    switch (message) {
    case WM_MOUSEMOVE:
        w->mouse.x = max(0, GET_X_LPARAM(lParam));
        w->mouse.y = max(0, GET_Y_LPARAM(lParam));
        break;
    case WM_KEYDOWN:
        w->keyboard[wParam] = 1;
        break;
    case WM_KEYUP:
        w->keyboard[wParam] = 0;
        break;
    case WM_LBUTTONDOWN:
        w->mouse.lbtn = 1;
        break;
    case WM_LBUTTONUP:
        w->mouse.lbtn = 0;
        break;
    case WM_RBUTTONDOWN:
        w->mouse.rbtn = 1;
        break;
    case WM_RBUTTONUP:
        w->mouse.rbtn = 0;
        break;
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcA(hWnd, message, wParam, lParam);
    }

    return 0;
}
#endif

int WAL_Create(WAL_Window* w, char* title, int width, int height) {
#ifdef _WIN32
    DWORD style = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX;
    WNDCLASSA wc;
    int pixelFormat;
    PIXELFORMATDESCRIPTOR pixelFormatDesc;
#else
    int attribList[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    XSizeHints sizeHints;
    XVisualInfo *xVI;
#endif

    memset(w->keyboard, 0, 0xFF);
    w->mouse.x = 0;
    w->mouse.y = 0;
    w->mouse.lbtn = 0;
    w->mouse.rbtn = 0;

#ifdef _WIN32
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)wndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.hIcon = LoadIconA(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = title;

    if (!RegisterClassA(&wc)) {
        fprintf(stderr, "RegisterClassA failed: Can not register window class.\n");
        return 1;
    }

    if ((w->_os.hW = CreateWindowA(title, title, style, 0, 0, width, height, NULL, NULL, wc.hInstance, NULL)) == NULL) {
        fprintf(stderr, "CreateWindowA failed: Can not create window.\n");
        return 1;
    }

    SetWindowLongPtrA(w->_os.hW, GWLP_USERDATA, (LONG_PTR)w);
    if ((w->_os.hDC = GetDC(w->_os.hW)) == NULL) {
        fprintf(stderr, "GetDC failed: Can not obtain device context.\n");
        goto WAL_Create_ELC;
    }
    
    memset(&pixelFormatDesc, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixelFormatDesc.nVersion = 1;
    pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
    pixelFormatDesc.cColorBits = 32;
    pixelFormatDesc.cAlphaBits = 8;
    pixelFormatDesc.cDepthBits = 24;

    if ((pixelFormat = ChoosePixelFormat(w->_os.hDC, &pixelFormatDesc)) == 0) {
        fprintf(stderr, "ChoosePixelFormat failed: Can not create render context.\n");
        goto WAL_Create_ELB;
    }

    if (!SetPixelFormat(w->_os.hDC, pixelFormat, &pixelFormatDesc)) {
        fprintf(stderr, "SetPixelFormat failed: Can not create render context.\n");
        goto WAL_Create_ELB;
    }

    if ((w->_os.hRC = wglCreateContext(w->_os.hDC)) == NULL) {
        fprintf(stderr, "wglCreateContext failed: Can not create render context.\n");
        goto WAL_Create_ELB;
    }

    if (!wglMakeCurrent(w->_os.hDC, w->_os.hRC)) {
        fprintf(stderr, "wglMakeCurrent failed: Can not create render context.\n");
        goto WAL_Create_ELA;
    }

    ShowWindow(w->_os.hW, SW_SHOW);
    UpdateWindow(w->_os.hW);
    return 0;

WAL_Create_ELA:
    wglDeleteContext(w->_os.hRC);
WAL_Create_ELB:
    ReleaseDC(w->_os.hW, w->_os.hDC);
WAL_Create_ELC:
    DestroyWindow(w->_os.hW);
    return 1;
#else
    if ((w->_os.hDC = XOpenDisplay(NULL)) == NULL) {
        fprintf(stderr, "XOpenDisplay failed: Can not open the display.\n");
        return 1;
    }
    
    if ((xVI = glXChooseVisual(w->_os.hDC, DefaultScreen(w->_os.hDC), attribList)) == NULL) {
        fprintf(stderr, "glXChooseVisual failed: Can not create render context.\n");
        goto WAL_Create_ELC;
    }

    if ((w->_os.hW = XCreateSimpleWindow(w->_os.hDC, RootWindow(w->_os.hDC, DefaultScreen(w->_os.hDC)), 0, 0, width, height, 0, xVI->depth, InputOutput)) == None) {
        fprintf(stderr, "XCreateSimpleWindow failed: Can not create window.\n");
        goto WAL_Create_ELC;
    }

    XStoreName(w->_os.hDC, w->_os.hW, title);
    if (XSelectInput(w->_os.hDC, w->_os.hW, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask) == BadWindow) {
        fprintf(stderr, "XSelectInput failed: Can not set windows properties.\n");
        goto WAL_Create_ELB;
    }

	if ((w->_os.quitMessage = XInternAtom(w->_os.hDC, "WM_DELETE_WINDOW", False)) == None) {
        fprintf(stderr, "XInternAtom failed: Can not create WM_DELETE_WINDOW.\n");
        goto WAL_Create_ELB;
    }

	if (XSetWMProtocols(w->_os.hDC, w->_os.hW, &w->_os.quitMessage, 1) == 0) {
        fprintf(stderr, "XSetWMProtocols failed: Can not set windows properties.\n");
        goto WAL_Create_ELB;
    }

    sizeHints.flags = PMinSize | PMaxSize;
    sizeHints.max_width = sizeHints.min_width = width;
    sizeHints.max_height = sizeHints.min_height = height;
    XSetWMNormalHints(w->_os.hDC, w->_os.hW, &sizeHints);

    if ((w->_os.hRC = glXCreateContext(w->_os.hDC, xVI, None, GL_TRUE)) == NULL) {
        fprintf(stderr, "glXCreateContext failed: Can not create render context.\n");
        goto WAL_Create_ELB;
    }

    if (!glXMakeCurrent(w->_os.hDC, w->_os.hW, w->_os.hRC)) {
        fprintf(stderr, "glXCreateContext failed: Can not create render context.\n");
        goto WAL_Create_ELA;
    }

    XMapWindow(w->_os.hDC, w->_os.hW);
    return 0;

WAL_Create_ELA:
    glXDestroyContext(w->_os.hDC, w->_os.hRC);
WAL_Create_ELB:
    XCloseDisplay(w->_os.hDC);
WAL_Create_ELC:
    XDestroyWindow(w->_os.hDC, w->_os.hW);
    return 1;
#endif
}

int WAL_Update(WAL_Window* w) {
#ifdef _WIN32
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) return 0;
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    SwapBuffers(w->_os.hDC);
#else
    XEvent evt;
    unsigned char vk;
    while (XPending(w->_os.hDC)) {
        XNextEvent(w->_os.hDC, &evt);
        switch (evt.type) {
        case MotionNotify:
            w->mouse.x = evt.xmotion.x;
            w->mouse.y = evt.xmotion.y;
            break;
        case ButtonPress:
            if (evt.xbutton.button == Button1) w->mouse.lbtn = 1;
            if (evt.xbutton.button == Button3) w->mouse.rbtn = 1;
            break;
        case ButtonRelease:
            if (evt.xbutton.button == Button1) w->mouse.lbtn = 0;
            if (evt.xbutton.button == Button3) w->mouse.rbtn = 0;
            break;
        case KeyPress:
            XLookupString(&evt.xkey, &vk, 1, NULL, 0);
            w->keyboard[vk] = 1;
            break;
        case KeyRelease:
            XLookupString(&evt.xkey, &vk, 1, NULL, 0);
            w->keyboard[vk] = 0;
            break;
        case ClientMessage:
			if ((Atom) evt.xclient.data.l[0] == w->_os.quitMessage) return 0;
            break;
		case DestroyNotify:
            return 0;
        }
    }

    glXSwapBuffers(w->_os.hDC, w->_os.hW);
#endif
    return 1;
}

void WAL_Destroy(WAL_Window* w) {
#ifdef _WIN32
    wglMakeCurrent(w->_os.hDC, NULL);
    wglDeleteContext(w->_os.hRC);
    ReleaseDC(w->_os.hW, w->_os.hDC);
    DestroyWindow(w->_os.hW);
#else
    glXMakeCurrent(w->_os.hDC, None, NULL);
    glXDestroyContext(w->_os.hDC, w->_os.hRC);
    XDestroyWindow(w->_os.hDC, w->_os.hW);
    XCloseDisplay(w->_os.hDC);
#endif
}