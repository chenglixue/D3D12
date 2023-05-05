
#include "pch.h"
#include "MyD3D12.h"


_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    PIXLoadLatestWinPixGpuCapturerLibrary();

    MyD3D12 sample(1280, 720, L"To romantic unfailing Elysia");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
}