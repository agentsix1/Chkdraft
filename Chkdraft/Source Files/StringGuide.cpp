#include "StringGuide.h"
#include "GuiAccel.h"

bool StringGuideWindow::CreateThis(HWND hParent)
{
	if ( ClassWindow::RegisterWindowClass(NULL, NULL, NULL, CreateSolidBrush(RGB(0, 0, 0)), NULL, "StringGuide", NULL, false) &&
		 ClassWindow::CreateClassWindow(NULL, NULL, WS_OVERLAPPED|WS_CHILD|WS_VISIBLE, 463, 7, 125, 365, hParent, NULL) )
	{
		HWND hStringGuide = getHandle();
		HDC hDC = GetDC(hStringGuide);
		SelectObject(hDC, defaultFont);
		SIZE strSize = { };
		for ( int i=0; i<numStringColors; i++ )
		{
			GetTextExtentPoint32(hDC, stringColorPrefixes[i], strlen(stringColorPrefixes[i]), &strSize);
			CreateStaticText(hStringGuide, 2, i*13, strSize.cx, 13, stringColorPrefixes[i]);
			CreateStaticText(hStringGuide, strSize.cx+3, i*13, 100, 13, stringColorStrings[i], ID_TEXT_COLOR_FIRST+i);
		}
		ReleaseDC(hStringGuide, hDC);
		return true;
	}
	else
		return false;
}

LRESULT StringGuideWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
		case WM_CTLCOLORSTATIC:
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				int id = GetDlgCtrlID((HWND)lParam);
				if ( id >= ID_TEXT_COLOR_FIRST && id <= ID_TEXT_COLOR_LAST )
					SetTextColor((HDC)wParam, stringColors[id-ID_TEXT_COLOR_FIRST]);
				else
					SetTextColor((HDC)wParam, RGB(16, 252, 24));

				return (BOOL)CreateSolidBrush(RGB(0, 0, 0));
			}
			break;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
			break;
	}
	return 0;
}
