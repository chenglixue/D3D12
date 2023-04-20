
#include "pch.h"
#include "MyD3D12.h"

static std::wstring GetLatestWinPixGpuCapturerPath();


_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    PIXLoadLatestWinPixGpuCapturerLibrary();

    // Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
    // This may happen if the application is launched through the PIX UI. 
    if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
    {
        LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
    }

    MyD3D12 sample(1280, 720, L"To romantic unfailing Elysia");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
}


//find path to WinPixGpuCapturer.dll from the most-recently installed version of PIX
static std::wstring GetLatestWinPixGpuCapturerPath()
{
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

    std::wstring pixSearchPath = programFilesPath + std::wstring(L"\\Microsoft PIX\\*");

    WIN32_FIND_DATA findData;
    bool foundPixInstallation = false;
    wchar_t newestVersionFound[MAX_PATH];

    HANDLE hFind = FindFirstFile(pixSearchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) &&
                (findData.cFileName[0] != '.'))
            {
                if (!foundPixInstallation || wcscmp(newestVersionFound, findData.cFileName) <= 0)
                {
                    foundPixInstallation = true;
                    StringCchCopy(newestVersionFound, _countof(newestVersionFound), findData.cFileName);
                }
            }
        } while (FindNextFile(hFind, &findData) != 0);
    }

    FindClose(hFind);

    if (!foundPixInstallation)
    {
        // TODO: Error, no PIX installation found
    }

    wchar_t output[MAX_PATH];
    StringCchCopy(output, pixSearchPath.length(), pixSearchPath.data());
    StringCchCat(output, MAX_PATH, &newestVersionFound[0]);
    StringCchCat(output, MAX_PATH, L"\\WinPixGpuCapturer.dll");

    return &output[0];
}