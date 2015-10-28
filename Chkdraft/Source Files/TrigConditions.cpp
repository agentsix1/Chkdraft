#include "TrigConditions.h"
#include "Chkdraft.h"
#include <map>

#define TOP_CONDITION_PADDING 50
#define BOTTOM_CONDITION_PADDING 0
#define DEFAULT_COLUMN_WIDTH 50

enum ID
{
	LIST_CONDITIONS = ID_FIRST,
	CHECK_ENABLED_1,
	CHECK_ENABLED_16 = (CHECK_ENABLED_1+15)
};

TrigConditionsWindow::TrigConditionsWindow() : hBlack(NULL), trigIndex(0), listConditions(*this),
	suggestions(listConditions.GetSuggestions())
{
	InitializeArgMaps();
}

bool TrigConditionsWindow::CreateThis(HWND hParent, u32 windowId)
{
	if ( getHandle() != NULL )
		return SetParent(hParent);

	RECT rcCli;
	if ( GetWindowRect(hParent, &rcCli) &&
		 ClassWindow::RegisterWindowClass(NULL, NULL, NULL, NULL, NULL, "TrigConditions", NULL, false) &&
		 ClassWindow::CreateClassWindow(NULL, "TrigConditions", WS_CHILD,
			5, 62, rcCli.right-rcCli.left, rcCli.bottom-rcCli.top,
			hParent, (HMENU)windowId) )
	{
		CreateSubWindows(getHandle());
		return true;
	}
	else
		return false;
}

bool TrigConditionsWindow::DestroyThis()
{
	suggestions.Hide();
	return false;
}

void TrigConditionsWindow::RefreshWindow(u32 trigIndex)
{
	listConditions.ClearItems();
	this->trigIndex = trigIndex;
	Trigger* trig;
	TextTrigGenerator ttg;
	if ( chkd.maps.curr->getTrigger(trig, trigIndex) &&
		 ttg.LoadScenario(chkd.maps.curr) )
	{
		for ( u8 y=0; y<NUM_TRIG_CONDITIONS; y++ )
		{
			Condition& condition = trig->conditions[y];
			if ( condition.condition > 0 && condition.condition <= 23 )
			{
				u8 numArgs = u8(conditionArgMaps[condition.condition].size());
				if ( numArgs > 8 )
					numArgs = 8;

				listConditions.item(1, y).SetText(
					ttg.GetConditionName(condition.condition).c_str()
				);
				for ( u8 x=0; x<numArgs; x++ )
				{
					listConditions.item(x+2, y).SetText(
						ttg.GetConditionArgument(condition, x, conditionArgMaps[condition.condition]).c_str()
					);
				}
				for ( u8 x=numArgs; x<8; x++ )
					listConditions.item(x+2, y).SetText("");
				
				listConditions.SetEnabledCheck(y, !condition.isDisabled());
			}
			else if ( condition.condition == 0 )
			{
				for ( u8 x=0; x<10; x++ )
					listConditions.item(x, y).SetText("");
			}
		}

		listConditions.AutoSizeColumns(DEFAULT_COLUMN_WIDTH, DEFAULT_COLUMN_WIDTH*2);
	}
}

void TrigConditionsWindow::DoSize()
{
	listConditions.SetPos(2, TOP_CONDITION_PADDING, cliWidth()-2, cliHeight()-TOP_CONDITION_PADDING-BOTTOM_CONDITION_PADDING);
}

void TrigConditionsWindow::ProcessKeyDown(WPARAM wParam, LPARAM lParam)
{
	if ( wParam == VK_TAB )
		SendMessage(listConditions.getHandle(), WM_KEYDOWN, wParam, lParam);
	else if ( wParam == VK_RETURN )
		SendMessage(listConditions.getHandle(), WM_KEYDOWN, wParam, lParam);
}

void TrigConditionsWindow::HideSuggestions()
{
	suggestions.Hide();	
}

void TrigConditionsWindow::ConditionEnableToggled(u8 conditionNum)
{
	Trigger* trig;
	if ( conditionNum >= 0 && conditionNum < 16 && chkd.maps.curr->getTrigger(trig, trigIndex) )
	{
		Condition &condition = trig->condition(conditionNum);
		if ( condition.condition != CID_NO_CONDITION )
		{
			condition.ToggleDisabled();

			chkd.maps.curr->notifyChange(false);
			RefreshWindow(trigIndex);
			chkd.trigEditorWindow.triggersWindow.RefreshWindow(false);

			listConditions.SetEnabledCheck(conditionNum, !condition.isDisabled());
		}
	}
}

void TrigConditionsWindow::InitializeArgMaps()
{
	#define ADD_ARRAY(anArray, vector)					\
	for ( u8 i=0; i<sizeof(anArray)/sizeof(u8); i++ )	\
		vector.push_back(anArray[i]);

	u8 accumulate[] = { 0, 1, 2, 3 };
	u8 bring[] = { 0, 3, 4, 1, 2 };
	u8 command[] = { 0, 2, 3, 1 };
	u8 commandTheLeast[] = { 0 };
	u8 commandTheLeastAt[] = { 0, 1 };
	u8 commandTheMost[] = { 0 };
	u8 commandTheMostAt[] = { 0, 1 };
	u8 coundownTimer[] = { 0, 1 };
	u8 deaths[] = { 0, 2, 3, 1 };
	u8 elapsedTime[] = { 0, 1 };
	u8 highestScore[] = { 0, 1, 2, 3 };
	u8 kill[] = { 0, 2, 3, 1 };
	u8 leastKills[] = { 0 };
	u8 leastResources[] = { 0 };
	u8 lowestScore[] = { 0 };
	u8 mostKills[] = { 0 };
	u8 mostResources[] = { 0 };
	u8 opponents[] = { 0, 1, 2 };
	u8 score[] = { 0, 1, 2, 3 };
	u8 switchState[] = { 0, 1 };

	ADD_ARRAY(accumulate, conditionArgMaps[4]);
	ADD_ARRAY(bring, conditionArgMaps[3]);
	ADD_ARRAY(command, conditionArgMaps[2]);
	ADD_ARRAY(commandTheLeast, conditionArgMaps[16]);
	ADD_ARRAY(commandTheLeastAt, conditionArgMaps[17]);
	ADD_ARRAY(commandTheMost, conditionArgMaps[6]);
	ADD_ARRAY(commandTheMostAt, conditionArgMaps[7]);
	ADD_ARRAY(coundownTimer, conditionArgMaps[1]);
	ADD_ARRAY(deaths, conditionArgMaps[15]);
	ADD_ARRAY(elapsedTime, conditionArgMaps[12]);
	ADD_ARRAY(highestScore, conditionArgMaps[9]);
	ADD_ARRAY(kill, conditionArgMaps[5]);
	ADD_ARRAY(leastKills, conditionArgMaps[18]);
	ADD_ARRAY(leastResources, conditionArgMaps[20]);
	ADD_ARRAY(lowestScore, conditionArgMaps[19]);
	ADD_ARRAY(mostKills, conditionArgMaps[8]);
	ADD_ARRAY(mostResources, conditionArgMaps[10]);
	ADD_ARRAY(opponents, conditionArgMaps[14]);
	ADD_ARRAY(score, conditionArgMaps[21]);
	ADD_ARRAY(switchState, conditionArgMaps[11]);
}

void TrigConditionsWindow::CreateSubWindows(HWND hWnd)
{
	TextControl text;
	listConditions.CreateThis(hWnd, 2, 40, 100, 100, LIST_CONDITIONS);

	/*listConditions.Show();

	for ( int y=0; y<16; y++ )
		listConditions.AddRow(9, (y+1)*100);
	*/
	suggestions.CreateThis(hWnd, 0, 0, 200, 100);
	//listConditions.SetUpDownDuringEdit(false);
	//listConditions.SetKeyReceiver(suggestions.getHandle());
	//dropUnits.CreateThis(hWnd, 0, 0, 150, 100, false, true, 0, 233, DefaultUnitDisplayName, defaultFont);

	RefreshWindow(trigIndex);
}

LRESULT TrigConditionsWindow::MeasureItem(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if ( wParam == LIST_CONDITIONS )
	{
		MEASUREITEMSTRUCT* mis = (MEASUREITEMSTRUCT*)lParam;
		mis->itemWidth = DEFAULT_COLUMN_WIDTH;
		mis->itemHeight = 15;
		return TRUE;
	}
	return ClassWindow::WndProc(hWnd, msg, wParam, lParam);
}

LRESULT TrigConditionsWindow::EraseBackground(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = ClassWindow::WndProc(hWnd, msg, wParam, lParam);
	DrawSelectedCondition();
	return result;
}

bool TrigConditionsWindow::TransformCondition(Condition &condition, u8 newId)
{
	if (condition.condition != newId)
	{
		if ( newId == CID_COMMAND || newId == CID_BRING ||
			 newId == CID_COMMAND_THE_MOST_AT || newId == CID_COMMAND_THE_LEAST_AT )
		{
			condition.locationNum = 64;
		}
		else
			condition.locationNum = 0;

		condition.players = 0;
		condition.amount = 0;
		condition.unitID = 0;

		if ( newId == CID_SWITCH )
			condition.comparison = 3;
		else
			condition.comparison = 0;

		condition.condition = newId;
		condition.typeIndex = 0;
		condition.flags = TextTrigCompiler::defaultFlags(newId);
		condition.internalData = 0;
		chkd.maps.curr->notifyChange(false);
		RefreshWindow(trigIndex);
		chkd.trigEditorWindow.triggersWindow.RefreshWindow(false);
		return true;
	}
	return false;
}

BOOL TrigConditionsWindow::GridItemChanging(u16 gridItemX, u16 gridItemY, string& str)
{
	Trigger* trig;
	TextTrigCompiler ttc;
	if ( gridItemX == 1 ) // Condition Name
	{
		cout << '(' << gridItemX << ", " << gridItemY << ") Condition Name: " << str << endl;
		u8 newId = CID_NO_CONDITION;
		if ( ttc.ParseConditionName(str, newId) || ttc.ParseConditionName(suggestions.Take(), newId) )
		{
			if ( chkd.maps.curr->getTrigger(trig, trigIndex) )
			{
				Condition &condition = trig->condition((u8)gridItemY);
				TransformCondition(condition, newId);
			}
		}
		else if ( str.length() == 0 )
		{
			if ( chkd.maps.curr->getTrigger(trig, trigIndex) &&
				 (u8)gridItemY <= NUM_TRIG_CONDITIONS &&
				 trig->conditions[(u8)gridItemY].condition != newId)
			{
				trig->deleteCondition((u8)gridItemY);
				chkd.maps.curr->notifyChange(false);
				RefreshWindow(trigIndex);
				chkd.trigEditorWindow.triggersWindow.RefreshWindow(false);
			}
		}
		else
			cout << "Unrecognized Condition Name" << endl;
	}
	else if ( gridItemX > 1 ) // Condition Arg
	{
		u8 argNum = (u8)gridItemX - 2;
		cout << '(' << gridItemX << ", " << gridItemY << ") Argument " << (int)argNum << ": " << str << endl;
		if ( chkd.maps.curr->getTrigger(trig, trigIndex) &&
			 ( ttc.ParseConditionArg(str, argNum, conditionArgMaps[trig->condition((u8)gridItemY).condition],
				trig->condition((u8)gridItemY), chkd.maps.curr) ||
			   ttc.ParseConditionArg(suggestions.Take(), argNum, conditionArgMaps[trig->condition((u8)gridItemY).condition],
				trig->condition((u8)gridItemY), chkd.maps.curr) ) )
		{
			chkd.maps.curr->notifyChange(false);
			RefreshWindow(trigIndex);
			chkd.trigEditorWindow.triggersWindow.RefreshWindow(false);
		}
	}
	return FALSE;
}

void TrigConditionsWindow::DrawSelectedCondition()
{
	HDC hDC = GetDC(getHandle());
	if ( hDC != NULL )
	{
		Trigger* trig;
		if ( chkd.maps.curr->getTrigger(trig, trigIndex) )
		{
			int focusedX = -1,
				focusedY = -1;

			if ( listConditions.GetFocusedItem(focusedX, focusedY) )
			{
				u8 conditionNum = (u8)focusedY;
				TextTrigGenerator ttg;
				string str;
				ttg.LoadScenario(chkd.maps.curr);
				str = chkd.trigEditorWindow.triggersWindow.GetConditionString(conditionNum, trig, ttg);
				ttg.ClearScenario();

				UINT width = 0, height = 0;
				GetStringDrawSize(hDC, width, height, str);
				HBRUSH hBackground = CreateSolidBrush(GetSysColor(COLOR_MENU));
				RECT rect;
				rect.left = listConditions.Left()+5;
				rect.right = listConditions.Right()-5;
				rect.top = listConditions.Top()-height-1;
				rect.bottom = listConditions.Top()-1;
				FillRect(hDC, &rect, hBackground);
				DeleteBrush(hBackground);

				SetBkMode(hDC, TRANSPARENT);
				DrawString(hDC, listConditions.Left()+6, listConditions.Top()-height-1, 500, RGB(0, 0, 0), str);
			}
		}
		ReleaseDC(hDC);
	}
}

int TrigConditionsWindow::GetGridItemWidth(int gridItemX, int gridItemY)
{
	string text;
	if ( listConditions.item(gridItemX, gridItemY).getText(text) )
	{
		HDC hDC = GetDC(getHandle());
		UINT width = 0, height = 0;
		if ( GetStringDrawSize(hDC, width, height, text) )
			return width+2;
		ReleaseDC(hDC);
	}
	return 0;
}

void TrigConditionsWindow::PreDrawItems()
{
	HDC hDC = GetDC(getHandle());
	EraseBackground(getHandle(), WM_ERASEBKGND, (WPARAM)hDC, NULL);
	ReleaseDC(hDC);
	hBlack = CreateSolidBrush(RGB(0, 0, 0));
}

void TrigConditionsWindow::SysColorRect(HDC hDC, RECT &rect, DWORD color)
{
	SetBkColor(hDC, GetSysColor(color));
	FillRect(hDC, &rect, GetSysColorBrush(color));
}

void TrigConditionsWindow::DrawItemBackground(HDC hDC, int gridItemX, int gridItemY, RECT &rcItem, int width, int xStart)
{
	RECT rcFill;
	rcFill.top = rcItem.top;
	rcFill.bottom = rcItem.bottom - 1;
	rcFill.left = xStart;
	rcFill.right = xStart + width - 1;

	if ( listConditions.isFocused(gridItemX, gridItemY) )
		SysColorRect(hDC, rcFill, COLOR_ACTIVEBORDER);
	else if ( listConditions.item(gridItemX, gridItemY).isSelected() )
		SysColorRect(hDC, rcFill, COLOR_HIGHLIGHT);
	else
		SysColorRect(hDC, rcFill, COLOR_WINDOW);
}

void TrigConditionsWindow::DrawItemFrame(HDC hDC, RECT &rcItem, int width, int &xStart)
{
	RECT rcFill;
	rcFill.top = rcItem.top - 1;
	rcFill.bottom = rcItem.bottom;
	rcFill.left = xStart - 1;
	rcFill.right = xStart + width;

	FrameRect(hDC, &rcFill, hBlack);
}

void TrigConditionsWindow::DrawGridViewItem(HDC hDC, int gridItemX, int gridItemY, RECT &rcItem, int &xStart)
{
	if ( gridItemX == 0 && gridItemY >= 0 && gridItemY < 16 )
		listConditions.checkEnabled[gridItemY].MoveTo(rcItem.left, rcItem.top);

	int width = ListView_GetColumnWidth(listConditions.getHandle(), gridItemX);
	DrawItemBackground(hDC, gridItemX, gridItemY, rcItem, width, xStart);

	string text;
	if ( listConditions.item(gridItemX, gridItemY).getText(text) )
		DrawString(hDC, xStart+1, rcItem.top, width-2, RGB(0, 0, 0), text);

	DrawItemFrame(hDC, rcItem, width, xStart);
	xStart += width;
}

void TrigConditionsWindow::DrawGridViewRow(UINT gridId, PDRAWITEMSTRUCT pdis)
{
	if ( gridId == LIST_CONDITIONS )
	{
		bool isSelected = ((pdis->itemState&ODS_SELECTED) == ODS_SELECTED),
			 drawSelection = ((pdis->itemAction&ODA_SELECT) == ODA_SELECT),
			 drawEntire = ((pdis->itemAction&ODA_DRAWENTIRE) == ODA_DRAWENTIRE);

		if ( pdis->itemID != -1 && ( drawSelection || drawEntire ) )
		{
			RECT &rcItem = pdis->rcItem;
			int itemStart = rcItem.left;

			int numColumns = listConditions.GetNumColumns();
			for ( int x=0; x<numColumns; x++ )
				DrawGridViewItem(pdis->hDC, x, pdis->itemID, rcItem, itemStart);
		}
	}
}

void TrigConditionsWindow::DrawTouchups(HDC hDC)
{
	RECT rect = { };
	if ( listConditions.GetEditItemRect(rect) )
	{
		rect.left -= 1;
		rect.top -= 1;
		HBRUSH hHighlight = CreateSolidBrush(RGB(0, 0, 200));
		FrameRect(hDC, &rect, hHighlight);
		rect.left -= 1;
		rect.top -= 1;
		rect.right += 1;
		rect.bottom += 1;
		FrameRect(hDC, &rect, hHighlight);
		DeleteBrush(hHighlight);
	}
}

void TrigConditionsWindow::PostDrawItems()
{
	DeleteBrush(hBlack);
	hBlack = NULL;
}

void TrigConditionsWindow::SuggestNothing()
{
	cout << "Suggest Nothing" << endl;
	suggestions.ClearStrings();
	suggestions.Hide();
	HDC hDC = GetDC(getHandle());
	HBRUSH hBackground = CreateSolidBrush(GetSysColor(COLOR_MENU));
	RECT rect;
	rect.left = dropUnits.Left();
	rect.right = dropUnits.Right();
	rect.top = dropUnits.Top();
	rect.bottom = dropUnits.Bottom();

	dropUnits.Hide();

	FillRect(hDC, &rect, hBackground);
	DeleteBrush(hBackground);
}

void TrigConditionsWindow::SuggestUnit()
{
	cout << "Suggest Unit" << endl;
	dropUnits.Show();
	if ( chkd.maps.curr != nullptr )
	{
		for ( u16 i = 0; i < NUM_UNIT_NAMES; i++ )
		{
			string str;
			chkd.maps.curr->getUnitName(str, i);
			suggestions.AddString(str);
			if ( str.compare(string(DefaultUnitDisplayName[i])) != 0 )
				suggestions.AddString(string(DefaultUnitDisplayName[i]));
		}
	}
	suggestions.Show();
}

void TrigConditionsWindow::SuggestLocation()
{
	cout << "Suggest Location" << endl;
	Scenario* chk = chkd.maps.curr;
	ChkLocation* loc = nullptr;
	string str;
	if ( chk != nullptr )
	{
		for ( u8 i = 0; i < 255; i++ )
		{
			if ( chk->getLocation(loc, u8(i)) && loc->stringNum > 0 && chk->getLocationName(str, i) )
				suggestions.AddString(str);
		}
	}
	suggestions.Show();
}

void TrigConditionsWindow::SuggestPlayer()
{
	cout << "Suggest Player" << endl;
	Scenario* chk = chkd.maps.curr;
	if ( chk != nullptr )
	{
		for ( u8 i = 0; i < numTriggerPlayers; i++ )
			suggestions.AddString(string(triggerPlayers[i]));
	}
	suggestions.Show();
}

void TrigConditionsWindow::SuggestAmount()
{
	cout << "Suggest Amount" << endl;
	//suggestions.Show();
}

void TrigConditionsWindow::SuggestNumericComparison()
{
	cout << "Suggest Numeric Comparison" << endl;
	suggestions.AddString(string("At Least"));
	suggestions.AddString(string("At Most"));
	suggestions.AddString(string("Exactly"));
	suggestions.Show();
}

void TrigConditionsWindow::SuggestResourceType()
{
	cout << "Suggest Resource Type" << endl;
	suggestions.AddString(string("Ore"));
	suggestions.AddString(string("Ore and Gas"));
	suggestions.AddString(string("Gas"));
	suggestions.Show();
}

void TrigConditionsWindow::SuggestScoreType()
{
	cout << "Suggest Score Type" << endl;
	for ( u8 i = 0; i < numTriggerScores; i++ )
		suggestions.AddString(string(triggerScores[i]));
	suggestions.Show();
}

void TrigConditionsWindow::SuggestSwitch()
{
	Scenario* chk = chkd.maps.curr;
	if ( chk != nullptr )
	{
		for ( u16 i = 0; i < 256; i++ )
		{
			string str;
			if ( chk->getSwitchName(str, (u8)i) )
				suggestions.AddString(str);
			else
			{
				stringstream ss;
				ss << "Switch " << i + 1 << endl;
				suggestions.AddString(ss.str());
			}
		}
	}
	cout << "Suggest Switch" << endl;
	suggestions.Show();
}

void TrigConditionsWindow::SuggestSwitchState()
{
	cout << "Suggest Switch State" << endl;
	suggestions.AddString(string("Cleared"));
	suggestions.AddString(string("Set"));
	suggestions.Show();
}

void TrigConditionsWindow::SuggestComparison()
{
	cout << "Suggest Comparison" << endl;
	suggestions.Show();
}

void TrigConditionsWindow::SuggestConditionType()
{
	cout << "Suggest Condition Type" << endl;
	suggestions.AddStrings(triggerConditions, numTriggerConditions);
	suggestions.Show();
}

void TrigConditionsWindow::SuggestTypeIndex()
{
	cout << "Suggest Type Index" << endl;
	suggestions.Show();
}

void TrigConditionsWindow::SuggestFlags()
{
	cout << "Suggest Flags" << endl;
	suggestions.Show();
}

void TrigConditionsWindow::SuggestInternalData()
{
	cout << "Suggest Internal Data" << endl;
	suggestions.Show();
}

void TrigConditionsWindow::GridEditStart(u16 gridItemX, u16 gridItemY)
{
	cout << "GridEditStart: (" << gridItemX << ", " << gridItemY << ")" << endl;
	Trigger* trig;
	if ( chkd.maps.curr->getTrigger(trig, trigIndex) )
	{
		Condition &condition = trig->condition((u8)gridItemY);
		ConditionArgType argType = ConditionArgType::CndNoType;
		if ( gridItemX == 1 ) // Condition Name
			argType = ConditionArgType::CndConditionType;
		else if ( gridItemX > 1 ) // Condition Arg
		{
			u8 conditionArgNum = (u8)gridItemX - 2;
			if ( condition.condition <= 23 && conditionArgMaps[condition.condition].size() > conditionArgNum )
			{
				u8 textTrigArgNum = conditionArgMaps[condition.condition][conditionArgNum];
				argType = condition.TextTrigArgType(textTrigArgNum);
			}
		}

		if ( argType != ConditionArgType::CndNoType )
		{
			POINT pt = listConditions.GetFocusedBottomRightScreenPt();
			if ( pt.x != -1 || pt.y != -1 )
				suggestions.MoveTo(pt.x, pt.y);
		}

		suggestions.ClearStrings();
		switch ( argType )
		{
			case CndNoType: SuggestNothing(); break;
			case CndUnit: SuggestUnit(); break;
			case CndLocation: SuggestLocation(); break;
			case CndPlayer: SuggestPlayer(); break;
			case CndAmount: SuggestAmount(); break;
			case CndNumericComparison: SuggestNumericComparison(); break;
			case CndResourceType: SuggestResourceType(); break;
			case CndScoreType: SuggestScoreType(); break;
			case CndSwitch: SuggestSwitch(); break;
			case CndSwitchState: SuggestSwitchState(); break;
			case CndComparison: SuggestComparison(); break;
			case CndConditionType: SuggestConditionType(); break;
			case CndTypeIndex: SuggestTypeIndex(); break;
			case CndFlags: SuggestFlags(); break;
			case CndInternalData: SuggestInternalData(); break;
		}
	}
}

void TrigConditionsWindow::NewSuggestion(string &str)
{
	listConditions.SetEditText(str);
}

LRESULT TrigConditionsWindow::ShowWindow(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if ( (BOOL)wParam == FALSE )
		suggestions.Hide();

	return ClassWindow::WndProc(hWnd, msg, wParam, lParam);
}

LRESULT TrigConditionsWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
		case WM_MEASUREITEM: return MeasureItem(hWnd, msg, wParam, lParam); break;
		case WM_ERASEBKGND: return EraseBackground(hWnd, msg, wParam, lParam); break;
		case WM_SHOWWINDOW: return ShowWindow(hWnd, msg, wParam, lParam); break;
		case WM_NEWSELTEXT: NewSuggestion(*(string*)lParam); break;
		case WM_GETGRIDITEMWIDTH: return GetGridItemWidth(LOWORD(wParam), HIWORD(wParam)); break;
		case WM_PREDRAWITEMS: PreDrawItems(); break;
		case WM_DRAWGRIDVIEWITEM: DrawGridViewRow((UINT)wParam, (PDRAWITEMSTRUCT)lParam); break;
		case WM_DRAWTOUCHUPS: DrawTouchups((HDC)wParam); break;
		case WM_POSTDRAWITEMS: PostDrawItems(); break;
		case WM_GETGRIDITEMCARETPOS: return -1; break;
		case WM_GRIDITEMCHANGING: return GridItemChanging(LOWORD(wParam), HIWORD(wParam), *(string*)lParam); break;
		case WM_GRIDEDITSTART: GridEditStart(LOWORD(wParam), HIWORD(wParam)); break;
		case WM_GRIDEDITEND: suggestions.Hide(); break;
		default: return ClassWindow::WndProc(hWnd, msg, wParam, lParam); break;
	}
	return 0;
}