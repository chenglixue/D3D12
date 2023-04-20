#include "pch.h"
#include "Win32Application.h"

HWND Win32Application::m_hwnd = nullptr;

int Win32Application::Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow)
{
    // Parse the command line parameters
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    pSample->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;    //CS_HREDRAW | CS_VREDRAW represent a redrawn window when width or height changes
    windowClass.lpfnWndProc = WindowProc;   //A pointer to the window procedure function associated with this WNDCLASSEX
    windowClass.hInstance = hInstance;      //A handle to the current app
    //Handle to the cursor resource
    //If NULL, the app must explicitly set the cursor shape each time it moves the mouse over the application's window
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DXSampleClass";   //name of window menu
    RegisterClassEx(&windowClass);

    //Calculates the required size of the window rectangle
    RECT windowRect = { 0, 0, static_cast<LONG>(pSample->GetWidth()), static_cast<LONG>(pSample->GetHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and use m_hwnd store this handle
    m_hwnd = CreateWindow(
        windowClass.lpszClassName,  // name of window menu
        pSample->GetTitle(),        // name of window
        // the style of window
        // this is an overlapped window.It means that windows with the same properties can be merged together
        // for example, for win11 if we clik on some folder they will overlap not separate
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,              // init coords of x.for WS_OVERLAPPEDWINDOW,x is the initial x-coor of the window's upper-left corner
        CW_USEDEFAULT,              // In the same way as x
        windowRect.right - windowRect.left, // width of window
        windowRect.bottom - windowRect.top, // height of window
        nullptr,        // Handle to Parent Window.We have no parent window
        nullptr,        // Handle to window menu.We aren't using menus.
        hInstance,      // The associated app handle
        pSample);       // Point to user-defined data. here is D3D12

    // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
    pSample->OnInit();

    ShowWindow(m_hwnd, nCmdShow);

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg); // Conversion of keyboard keys
            DispatchMessage(&msg);  // Dispatch the message to the corresponding window procedure
        }
    }

    // destory of D3D12 
    pSample->OnDestroy();

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DXSample* pSample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
        // send this message when call the CreateWindow()
    case WM_CREATE:
    {
        // Save the DXSample* passed in to CreateWindow
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        //Changes an attribute of the specified window
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    // when a nonsystem key is pressed
    case WM_KEYDOWN:
        if (pSample)
        {
            pSample->OnKeyDown(static_cast<UINT8>(wParam));
        }
        return 0;

        // when a nonsystem key is released 
    case WM_KEYUP:
        if (pSample)
        {
            pSample->OnKeyUp(static_cast<UINT8>(wParam));
        }
        return 0;

        // when system or another app makes a request to paint
    case WM_PAINT:
        if (pSample)
        {
            pSample->OnUpdate();
            pSample->OnRender();
        }
        return 0;

        //  when a window is being destroyed.Although window is closed, thread maybe still run in background
    case WM_DESTROY:
        //Terminate thread
        PostQuitMessage(0);
        return 0;
    }

    // Calls the default window procedure to provide default processing for any window messages that an application does not process
    return DefWindowProc(hWnd, message, wParam, lParam);
}