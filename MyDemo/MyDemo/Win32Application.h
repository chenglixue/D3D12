//Win32Application.h
#pragma once

#include "DXSample.h"

class DXSample;

class Win32Application
{
public:
	/*
		not main loop, it has some feature as follows :
	*	1. Parse the command line parameters
	*	2. Initialize the window class to define property of the window
	*	3. Calculates the required size of the window rectangle
	*	4. Rectangle of last step will be passed to the CreateWindow(), which will create a window
	*	5. Init the OnInit() in D3D12Tutorials that init the pipleline and assets of D3D12
	*	6. show the window
	*	7. main loop.It respond to the D3D12 and key value
	*	8. destory D3D12
	*	9. Return this part of the WM_QUIT message to Windows
	*/
	static int Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow);

	//get the handle to the window
	static HWND GetHwnd() { return m_hwnd; }

protected:
	/*
		Processes the information sent to the window
		Calls the default window procedure to provide default processing for any window messages that an application does not process
	*/
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static HWND m_hwnd;	//handle to the window
};