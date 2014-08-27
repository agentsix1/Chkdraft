#include "TileProp.h"
#include "Chkdraft.h"

bool TilePropWindow::CreateThis(HWND hParent)
{
	return ClassWindow::CreateModelessDialog(MAKEINTRESOURCE(IDD_DIALOG_SETTILE), hParent);
}

bool TilePropWindow::DestroyThis()
{
	return ClassWindow::DestroyDialog();
}

BOOL TilePropWindow::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
		case WM_INITDIALOG:
			{
				HWND hEdit = GetDlgItem(hWnd, IDC_EDIT_TILEVALUE);
				SendMessage(hEdit, EM_SETLIMITTEXT, 10, 0);
				SendMessage(hWnd, WM_COMMAND, TILE_UPDATE, NULL);
				PostMessage(hWnd, WM_NEXTDLGCTL, (WPARAM)hEdit, true);
				return true;
			}
			break;

		case WM_COMMAND:
			{
				switch ( HIWORD(wParam) )
				{
					case EN_UPDATE:
						RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
						break;
				}
				switch ( LOWORD(wParam) )
				{
					case IDOK:
						{
							TCHAR lpszTile[11];
							int TextLength = (WORD) SendDlgItemMessage(hWnd, IDC_EDIT_TILEVALUE, EM_LINELENGTH, 0, 0);
							*((LPWORD)lpszTile) = TextLength;
							SendDlgItemMessage(hWnd, IDC_EDIT_TILEVALUE, EM_GETLINE, 0, (LPARAM)lpszTile);	
							lpszTile[TextLength] = NULL;
	
							u16 tile = atoi(lpszTile);
							if ( tile > 65535 )
								tile %= 65536;
							
							TileNode* headTile = chkd.maps.curr->selections().getFirstTile();
							headTile->value = tile;
							chkd.maps.curr->SetTile(headTile->xc, headTile->yc, tile);
							chkd.maps.curr->nextUndo();
							EndDialog(hWnd, IDOK);
							break;
						}

					case TILE_UPDATE:
						{
							char title[256];
							TileNode* headTile = chkd.maps.curr->selections().getFirstTile();
							sprintf_s(title, 256, "Tile Properties (%d, %d)", headTile->xc, headTile->yc);
							SetWindowText(hWnd, title);

							HWND hEditTile = GetDlgItem(hWnd, IDC_EDIT_TILEVALUE);
							u16 currTile = headTile->value;
							char tileValue[32];
							_itoa_s(currTile, tileValue, 10);
							SetWindowText(hEditTile, tileValue);
						}
						break;

					case IDCANCEL:
						EndDialog(hWnd, IDCANCEL);
						break;
				}
			}
			break;

		case WM_PAINT:
			{
				RECT rect;
				GetClientRect(hWnd, &rect);
				int width = 32,
					height = 32;
				PAINTSTRUCT ps;
				HDC hDC = BeginPaint(hWnd, &ps),
					MemhDC = CreateCompatibleDC(hDC);
				HBITMAP Membitmap = CreateCompatibleBitmap(hDC, width, height);
				SelectObject(MemhDC, Membitmap);

				TCHAR lpszTile[11];
				int TextLength = WORD(SendDlgItemMessage(hWnd, IDC_EDIT_TILEVALUE, EM_LINELENGTH, 0, 0));
				*((LPWORD)lpszTile) = TextLength;
				SendDlgItemMessage(hWnd, IDC_EDIT_TILEVALUE, EM_GETLINE, 0, (LPARAM)lpszTile);	
				lpszTile[TextLength] = NULL;

				u16 tile = atoi(lpszTile), tileset;
				
				chkd.maps.curr->ERA().get<u16>(tileset, 0);
				TileSet* tiles = &chkd.scData.tilesets.set[tileset];

				HBRUSH brush = CreateSolidBrush(RGB(166, 156, 132));
				FillRect(MemhDC, &rect, brush);
				DeleteObject(brush);

				BITMAPINFO bmi = GetBMI(32, 32);
				DrawTile(MemhDC, tiles, 0, 0, tile, bmi, 0, 0, 0);
				BitBlt(hDC, 55, 50, width, height, MemhDC, 0, 0, SRCCOPY);

				BITMAPINFO bmiMini = GetBMI(8, 8);
				for ( int yMiniTile=0; yMiniTile<4; yMiniTile++ )
				{
					for ( int xMiniTile=0; xMiniTile<4; xMiniTile++ )
						DrawMiniTileElevation(hDC, tiles, 350+xMiniTile*9, 50+yMiniTile*9, tile, xMiniTile, yMiniTile, bmiMini);
				}
				
				DeleteObject(Membitmap);
				DeleteDC    (MemhDC);
				DeleteDC    (hDC);
			}
			break;

		case WM_DESTROY:
			EndDialog(hWnd, IDCANCEL);
			break;

		default:
			return false;
			break;
	}
	return 0;
}