#include "UnitProperties.h"
#include "MoveTo.h"
#include "Chkdraft.h"
#include "UnitChange.h"
#include "UnitCreateDel.h"
#include "UnitIndexMove.h"
#include <string>

enum class UnitListColumn { Name, Owner, Xc, Yc, Index };

enum class Id
{
    UnitList = ID_FIRST,
    ComboPlayer,

    ButtonMoveUp = IDC_BUTTON_MOVEUP,
    ButtonMoveTop = IDC_BUTTON_MOVETOP,
    ButtonMoveDown = IDC_BUTTON_MOVEDOWN,
    ButtonMoveEnd = IDC_BUTTON_MOVEEND,
    ButtonDelete = IDC_BUTTON_DELETE,
    ButtonMoveTo = IDC_BUTTON_MOVE_TO,
    ButtonClose = IDC_BUTTON_CLOSE,

    EditHp = IDC_EDIT_HP,
    EditMp = IDC_EDIT_MP,
    EditShields = IDC_EDIT_SHIELD,
    EditResources = IDC_EDIT_RESOURCES,
    EditHanger = IDC_EDIT_HANGER,
    EditUnitId = IDC_EDIT_ID,
    EditXc = IDC_EDIT_XC,
    EditYc = IDC_EDIT_YC,

    CheckInvincible = IDC_CHECK_INVINCIBLE,
    CheckHallucinated = IDC_CHECK_HALLUCINATED,
    CheckBurrowed = IDC_CHECK_BURROWED,
    CheckCloaked = IDC_CHECK_CLOAKED,
    CheckLifted = IDC_CHECK_LIFTED,
};

UnitPropertiesWindow::UnitPropertiesWindow() : columnSortedBy(UnitListColumn::Index), flipSort(false), initilizing(true), changeHighlightOnly(false)
{

}

bool UnitPropertiesWindow::CreateThis(HWND hParent)
{
    if ( ClassDialog::CreateModelessDialog(MAKEINTRESOURCE(IDD_UNITPROPERTIES), hParent) )
    {
        CreateSubWindows(getHandle());
        return true;
    }
    return false;
}

bool UnitPropertiesWindow::CreateSubWindows(HWND hWnd)
{
    buttonMoveUp.FindThis(hWnd, (u32)Id::ButtonMoveUp);
    buttonMoveTop.FindThis(hWnd, (u32)Id::ButtonMoveTop);
    buttonMoveDown.FindThis(hWnd, (u32)Id::ButtonMoveDown);
    buttonMoveEnd.FindThis(hWnd, (u32)Id::ButtonMoveEnd);
    buttonDelete.FindThis(hWnd, (u32)Id::ButtonDelete);
    buttonMoveTo.FindThis(hWnd, (u32)Id::ButtonMoveTo);

    editLife.FindThis(hWnd, (u32)Id::EditHp);
    editMana.FindThis(hWnd, (u32)Id::EditMp);
    editShield.FindThis(hWnd, (u32)Id::EditShields);
    editResources.FindThis(hWnd, (u32)Id::EditResources);
    editHanger.FindThis(hWnd, (u32)Id::EditHanger);
    editUnitId.FindThis(hWnd, (u32)Id::EditUnitId);
    editXc.FindThis(hWnd, (u32)Id::EditXc);
    editYc.FindThis(hWnd, (u32)Id::EditYc);

    checkInvincible.FindThis(hWnd, (u32)Id::CheckInvincible);
    checkHallucinated.FindThis(hWnd, (u32)Id::CheckHallucinated);
    checkBurrowed.FindThis(hWnd, (u32)Id::CheckBurrowed);
    checkCloaked.FindThis(hWnd, (u32)Id::CheckCloaked);
    checkLifted.FindThis(hWnd, (u32)Id::CheckLifted);

    dropPlayer.CreateThis(hWnd, 657, 29, 90, 438, (u32)Id::ComboPlayer);

    initilizing = true;

    editLife.CreateNumberBuddy(0, 100);
    editMana.CreateNumberBuddy(0, 100);
    editShield.CreateNumberBuddy(0, 100);
    editResources.CreateNumberBuddy(0, 50000);
    editHanger.CreateNumberBuddy(0, 8);
    editUnitId.CreateNumberBuddy(0, 65535);

    listUnits.CreateThis(hWnd, 9, 10, 549, 449, false, false, (u32)Id::UnitList);
    listUnits.EnableFullRowSelect();
    listUnits.SetFont(CreateFont(13, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma"), false);

    listUnits.AddColumn(0, "Unit Type", 200, LVCFMT_LEFT);
    listUnits.AddColumn(1, "Owner", 100, LVCFMT_LEFT);
    listUnits.AddColumn(2, "X", 75, LVCFMT_RIGHT);
    listUnits.AddColumn(3, "Y", 75, LVCFMT_RIGHT);
    listUnits.AddColumn(4, "Index", 75, LVCFMT_RIGHT);

    RepopulateList();

    listUnits.Show();
    initilizing = false;
    
    return true;
}

bool UnitPropertiesWindow::DestroyThis()
{
    columnSortedBy = UnitListColumn::Index; // Reset column sorted by
    return ClassDialog::DestroyDialog();
}

void UnitPropertiesWindow::SetChangeHighlightOnly(bool changeHighlightOnly)
{
    this->changeHighlightOnly = changeHighlightOnly;
}

void UnitPropertiesWindow::ChangeCurrOwner(u8 newOwner)
{
    auto undoableChanges = ReversibleActions::Make();
    auto &selUnits = CM->GetSelections().getUnits();
    for ( u16 unitIndex : selUnits )
    {
        ChkUnit unit = CM->getUnit(unitIndex);
        if ( unit.owner != newOwner ) // If the current and new owners are different
        {
            u8 prevOwner = unit.owner;
            unit.owner = newOwner;

            if ( CM->ReplaceUnit(unitIndex, unit) )
            {
                ChangeUnitsDisplayedOwner(unitIndex, newOwner);
                undoableChanges->Insert(UnitChange::Make(unitIndex, ChkUnitField::Owner, prevOwner));
            }
        }
    }
    CM->AddUndo(undoableChanges);
    CM->Redraw(true);
}

void UnitPropertiesWindow::ChangeDropdownPlayer(u8 newPlayer)
{
    std::string text;
    if ( newPlayer < 12 )
        dropPlayer.SetSel(newPlayer);
    else
        dropPlayer.SetEditNum<u8>(newPlayer + 1);
}

void UnitPropertiesWindow::ChangeUnitsDisplayedOwner(int index, u8 newPlayer)
{
    char owner[32] = {}, padding[2] = { '\0', '\0' };
    if ( newPlayer < 9 )
        padding[0] = '0';
    else
        padding[0] = '\0';

    std::snprintf(owner, sizeof(owner), "Player %s%i", padding, newPlayer+1);
    listUnits.SetItemText(listUnits.GetItemRow(index), (int)UnitListColumn::Owner, owner);
}

void UnitPropertiesWindow::SetListRedraw(bool allowRedraw)
{
    if ( allowRedraw )
    {
        listUnits.SetRedraw(true);
        ListView_SortItems(listUnits.getHandle(), ForwardCompareLvItems, this);
        listUnits.UpdateWindow();
    }
    else
        listUnits.SetRedraw(false);
}

bool UnitPropertiesWindow::AddUnitItem(u16 index, ChkUnit unit)
{
    char owner[32] = {}, padding[2] = { '\0', '\0' },
        xc[32] = {}, yc[32] = {}, cIndex[32] = {};

    u16 unitId = unit.id;

    if ( unit.owner < 9 )
        padding[0] = '0';
    else
        padding[0] = '\0';

    std::snprintf(owner, sizeof(owner), "Player %s%i", padding, unit.owner + 1);
    std::strcpy(xc, std::to_string(unit.xc).c_str());
    std::strcpy(yc, std::to_string(unit.yc).c_str());
    std::strcpy(cIndex, std::to_string(index).c_str());

    ChkdString unitName;
    CM->getUnitName(unitName, unitId);

    listUnits.AddRow(4, index);
    listUnits.SetItemText(index, (int)UnitListColumn::Name, unitName.c_str());
    listUnits.SetItemText(index, (int)UnitListColumn::Owner, owner);
    listUnits.SetItemText(index, (int)UnitListColumn::Xc, xc);
    listUnits.SetItemText(index, (int)UnitListColumn::Yc, yc);
    listUnits.SetItemText(index, (int)UnitListColumn::Index, index);
    return true;
}

void UnitPropertiesWindow::FocusAndSelectIndex(u16 unitIndex)
{
    LVFINDINFO findInfo = {};
    findInfo.flags = LVFI_PARAM;
    findInfo.lParam = unitIndex;
    s32 lvIndex = ListView_FindItem(listUnits.getHandle(), -1, &findInfo);
    ListView_SetItemState(listUnits.getHandle(), lvIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

void UnitPropertiesWindow::DeselectIndex(u16 unitIndex)
{
    LVFINDINFO findInfo = {};
    findInfo.flags = LVFI_PARAM;
    findInfo.lParam = unitIndex;

    int lvIndex = ListView_FindItem(listUnits.getHandle(), -1, &findInfo);
    ListView_SetItemState(listUnits.getHandle(), lvIndex, 0, LVIS_FOCUSED | LVIS_SELECTED);
}

void UnitPropertiesWindow::UpdateEnabledState()
{
    Selections &selections = CM->GetSelections();
    if ( selections.hasUnits() )
    {
        EnableUnitEditing();
        ChkUnit unit = CM->getUnit(selections.getFirstUnit());
        SetUnitFieldText(unit);
    }
    else
        DisableUnitEditing();
}

void UnitPropertiesWindow::RepopulateList()
{
    initilizing = true;
    SetListRedraw(false);

    listUnits.DeleteAllItems();
    if ( CM != nullptr )
    {
        Selections &selections = CM->GetSelections();

        int numUnits = CM->numUnits();
        for ( int i = 0; i < numUnits; i++ )
        {
            ChkUnit unit = CM->getUnit(i);
            AddUnitItem(i, unit);
        }

        if ( selections.hasUnits() )
        {
            u16 selectedIndex = selections.getFirstUnit();
            listUnits.FocusItem(selectedIndex);

            auto &selUnits = selections.getUnits();
            for ( u16 &unitIndex : selUnits )
                listUnits.SelectRow(unitIndex);
            
            EnableUnitEditing();
            ChkUnit unit = CM->getUnit(selectedIndex);
            SetUnitFieldText(unit);

            ChkdString unitName;
            CM->getUnitName(unitName, unit.id);
            WindowsItem::SetWinText(unitName.c_str());

            int row = listUnits.GetItemRow(selections.getHighestIndex());
            listUnits.EnsureVisible(row, false);
            row = listUnits.GetItemRow(selections.getLowestIndex());
            listUnits.EnsureVisible(row, false);
        }
    }

    SetListRedraw(true);
    initilizing = false;
}

void UnitPropertiesWindow::EnableUnitEditing()
{
    dropPlayer.EnableThis();

    ButtonControl* buttonControls[] = { &buttonMoveUp, &buttonMoveTop, &buttonMoveDown, &buttonMoveEnd, &buttonDelete, &buttonMoveTo };
    EditControl* editControls[] = { &editLife, &editMana, &editShield, &editResources, &editHanger, &editUnitId, &editXc, &editYc };
    CheckBoxControl* checkControls[] = { &checkInvincible, &checkHallucinated, &checkBurrowed, &checkCloaked, &checkLifted };
    int numButtonControls = sizeof(buttonControls) / sizeof(ButtonControl*),
        numEditControls = sizeof(editControls) / sizeof(EditControl*),
        numCheckBoxControls = sizeof(checkControls) / sizeof(CheckBoxControl*);

    for ( int i = 0; i < numButtonControls; i++ )
        buttonControls[i]->EnableThis();

    for ( int i = 0; i < numEditControls; i++ )
        editControls[i]->EnableThis();
    
    for ( int i = 0; i < numCheckBoxControls; i++ )
    {
        checkControls[i]->EnableThis();
        checkControls[i]->SetCheck(false);
    }

    ChkUnit unit = CM->getUnit(CM->GetSelections().getFirstUnit());
    SetUnitFieldText(unit);
}

void UnitPropertiesWindow::DisableUnitEditing()
{
    WindowsItem::SetWinText("");

    dropPlayer.SetSel(-1);
    dropPlayer.DisableThis();

    ButtonControl* buttonControls[] = { &buttonMoveUp, &buttonMoveTop, &buttonMoveDown, &buttonMoveEnd, &buttonDelete, &buttonMoveTo };
    EditControl* editControls[] = { &editLife, &editMana, &editShield, &editResources, &editHanger, &editUnitId, &editXc, &editYc };
    CheckBoxControl* checkControls[] = { &checkInvincible, &checkHallucinated, &checkBurrowed, &checkCloaked, &checkLifted };
    int numButtonControls = sizeof(buttonControls) / sizeof(ButtonControl*),
        numEditControls = sizeof(editControls) / sizeof(EditControl*),
        numCheckBoxControls = sizeof(checkControls) / sizeof(CheckBoxControl*);

    for ( int i = 0; i < numButtonControls; i++ )
        buttonControls[i]->DisableThis();

    for ( int i = 0; i < numEditControls; i++ )
    {
        editControls[i]->SetText("");
        editControls[i]->DisableThis();
    }

    for ( int i = 0; i < numCheckBoxControls; i++ )
    {
        checkControls[i]->SetCheck(false);
        checkControls[i]->DisableThis();
    }
}

void UnitPropertiesWindow::SetUnitFieldText(ChkUnit unit)
{
    initilizing = true;

    if ( unit.owner < 12 )
        dropPlayer.SetSel(unit.owner);
    else
        dropPlayer.SetWinText(std::to_string(unit.owner + 1).c_str());

    editLife.SetText(std::to_string(unit.hitpoints).c_str());
    editMana.SetText(std::to_string(unit.energy).c_str());
    editShield.SetText(std::to_string(unit.shields).c_str());
    editResources.SetText(std::to_string(unit.resources).c_str());
    editHanger.SetText(std::to_string(unit.hanger).c_str());
    editUnitId.SetText(std::to_string(unit.id).c_str());
    editXc.SetText(std::to_string(unit.xc).c_str());
    editYc.SetText(std::to_string(unit.yc).c_str());

    checkInvincible.SetCheck((unit.stateFlags&UNIT_STATE_INVINCIBLE) == UNIT_STATE_INVINCIBLE);
    checkHallucinated.SetCheck((unit.stateFlags&UNIT_STATE_HALLUCINATED) == UNIT_STATE_HALLUCINATED);
    checkBurrowed.SetCheck((unit.stateFlags&UNIT_STATE_BURROWED) == UNIT_STATE_BURROWED);
    checkCloaked.SetCheck((unit.stateFlags&UNIT_STATE_CLOAKED) == UNIT_STATE_CLOAKED);
    checkLifted.SetCheck((unit.stateFlags&UNIT_STATE_LIFTED) == UNIT_STATE_LIFTED);

    initilizing = false;
}

void UnitPropertiesWindow::SwapIndexes(HWND hListView, LPARAM index1, LPARAM index2)
{
    int row1 = listUnits.GetItemRow(index1),
        row2 = listUnits.GetItemRow(index2);

    LVITEM item = { };
    item.mask = LVIF_PARAM;
    item.iItem = row1;
    item.lParam = index2;

    ListView_SetItem(hListView, &item);
    listUnits.SetItemText(row1, (int)UnitListColumn::Index, index2);

    item.iItem = row2;
    item.lParam = index1;

    ListView_SetItem(hListView, &item);
    listUnits.SetItemText(row2, (int)UnitListColumn::Index, index1);
}

void UnitPropertiesWindow::ChangeIndex(HWND hListView, LPARAM oldLParam, LPARAM newLParam)
{
    int row = listUnits.GetItemRow(oldLParam);

    LVITEM item = { };
    item.mask = LVIF_PARAM;
    item.iItem = row;
    item.lParam = newLParam;

    ListView_SetItem(hListView, &item);

    listUnits.SetItemText(row, (int)UnitListColumn::Index, newLParam);
}

int UnitPropertiesWindow::CompareLvItems(LPARAM index1, LPARAM index2)
{
    int sort = 0;

    if ( columnSortedBy != UnitListColumn::Index )
    {
        ChkUnit firstUnit = CM->getUnit((u16)index1),
            secondUnit = CM->getUnit((u16)index2);

        switch ( columnSortedBy )
        {
            case UnitListColumn::Name: // Sort by ID / type
                if ( firstUnit.id < secondUnit.id )
                    sort = -1;
                else if ( firstUnit.id > secondUnit.id )
                    sort = 1;
                break;
            case UnitListColumn::Owner: // Sort by owner
                if ( firstUnit.owner < secondUnit.owner )
                    sort = -1;
                else if ( firstUnit.owner > secondUnit.owner )
                    sort = 1;
                break;
            case UnitListColumn::Xc: // Sort by xc
                if ( firstUnit.xc < secondUnit.xc )
                    sort = -1;
                else if ( firstUnit.xc > secondUnit.xc )
                    sort = 1;
                else if ( firstUnit.yc < secondUnit.yc ) // Try to sort by yc if xc's are equal
                    sort = -1;
                else if ( firstUnit.yc > secondUnit.yc )
                    sort = 1;
                break;
            case UnitListColumn::Yc: // Sort by yc
                if ( firstUnit.yc < secondUnit.yc )
                    sort = -1;
                else if ( firstUnit.yc > secondUnit.yc )
                    sort = 1;
                else if ( firstUnit.xc < secondUnit.xc ) // Try to sort by xc if yc's are equal
                    sort = -1;
                else if ( firstUnit.xc > secondUnit.xc )
                    sort = 1;
                break;
        }
    }
    else if ( columnSortedBy == UnitListColumn::Index )
    {
        if ( index1 < index2 )
            sort = -1;
        else if ( index1 > index2 )
            sort = 1;
    }

    if ( flipSort )
        return -sort;
    else
        return sort;
}

void UnitPropertiesWindow::LvColumnClicked(NMHDR* nmhdr)
{
    UnitListColumn newColumn = (UnitListColumn)((NMLISTVIEW*)nmhdr)->iSubItem;
    if ( newColumn == columnSortedBy )
        flipSort = !flipSort;
    else
        flipSort = false;

    columnSortedBy = newColumn;
    ListView_SortItems(nmhdr->hwndFrom, ForwardCompareLvItems, this);
}

void UnitPropertiesWindow::LvItemChanged(NMHDR* nmhdr)
{
    Selections &selections = CM->GetSelections();
    preservedStats.convertToUndo();
    if ( !changeHighlightOnly )
    {
        NMLISTVIEW* itemInfo = (NMLISTVIEW*)nmhdr;
        u16 index = (u16)itemInfo->lParam;

        if ( itemInfo->uNewState & LVIS_SELECTED && initilizing == false ) // Selected
                                                                           // Add item to selection
        {
            bool firstSelected = !selections.hasUnits();
            selections.addUnit(index);

            if ( firstSelected )
                EnableUnitEditing();

            ChkUnit unit = CM->getUnit(index);
            ChkdString unitName;
            CM->getUnitName(unitName, unit.id);
            WindowsItem::SetWinText(unitName.c_str());
            SetUnitFieldText(unit);

            CM->Redraw(false);
        }
        else if ( itemInfo->uOldState & LVIS_SELECTED ) // From selected to not selected
                                                        // Remove item from selection
        {
            selections.removeUnit(index);

            if ( !selections.hasUnits()
                && !(GetKeyState(VK_DOWN) & 0x8000
                    || GetKeyState(VK_UP) & 0x8000
                    || GetKeyState(VK_LEFT) & 0x8000
                    || GetKeyState(VK_RIGHT) & 0x8000
                    || GetKeyState(VK_LBUTTON) & 0x8000
                    || GetKeyState(VK_RBUTTON) & 0x8000) )
            {
                DisableUnitEditing();
            }

            CM->Redraw(false);
        }
    }
}

void UnitPropertiesWindow::NotifyClosePressed()
{
    EndDialog(getHandle(), IDCLOSE);
}

void UnitPropertiesWindow::NotifyMoveTopPressed()
{
    Selections &selections = CM->GetSelections();

    u16 unitStackTopIndex = selections.getFirstUnit();
    selections.sortUnits(true); // sort with lowest indexes first

    listUnits.SetRedraw(false);

    ChkUnit preserve;

    auto unitChanges = ReversibleActions::Make();
    unitChanges->Insert(UnitIndexMoveBoundary::Make());
    u16 i = 0;
    auto &selUnits = selections.getUnits();
    for ( u16 &unitIndex : selUnits )
    {
        if ( unitIndex != 0 ) // If unit is not at the destination index and unitptr can be retrieved
        {
            preserve = CM->getUnit(unitIndex); // Preserve the unit info
            if ( CM->deleteUnit(unitIndex) )
            {
                if ( CM->insertUnit(i, preserve) )
                {
                    unitChanges->Insert(UnitIndexMove::Make(unitIndex, i));
                    if ( unitIndex == unitStackTopIndex )
                        unitStackTopIndex = i;

                    unitIndex = i; // Modify the index that denotes unit selection
                }
                else // Insertion failed
                {
                    selections.removeUnit(unitIndex);
                    break; // Can't advance to next, exit loop
                }
            }
        }
        i++;
    }

    unitChanges->Insert(UnitIndexMoveBoundary::Make());
    if ( unitChanges->Count() > 2 )
        CM->AddUndo(unitChanges);

    selections.ensureFirst(unitStackTopIndex);
    RepopulateList();
}

void UnitPropertiesWindow::NotifyMoveEndPressed()
{
    Selections &selections = CM->GetSelections();

    u16 unitStackTopIndex = selections.getFirstUnit();
    selections.sortUnits(false); // Highest First

    listUnits.SetRedraw(false);
    u16 numUnits = CM->numUnits();
    u16 numUnitsSelected = selections.numUnits();

    ChkUnit preserve;

    u16 i = 1;
    auto unitChanges = ReversibleActions::Make();
    unitChanges->Insert(UnitIndexMoveBoundary::Make());
    auto &selUnits = selections.getUnits();
    for ( u16 &unitIndex : selUnits )
    {
        if ( unitIndex != numUnits - 1 )
        {
            preserve = CM->getUnit(unitIndex);
            if ( CM->deleteUnit(unitIndex) )
            {
                if ( CM->insertUnit(numUnits - i, preserve) )
                {
                    unitChanges->Insert(UnitIndexMove::Make(unitIndex, numUnits - i));

                    if ( unitIndex == unitStackTopIndex )
                        unitStackTopIndex = numUnits - i;

                    unitIndex = numUnits - i;
                }
                else
                {
                    selections.removeUnit(unitIndex);
                    break;
                }
            }
        }
        i++;
    }

    unitChanges->Insert(UnitIndexMoveBoundary::Make());
    if ( unitChanges->Count() > 2 )
        CM->AddUndo(unitChanges);

    selections.ensureFirst(unitStackTopIndex);
    RepopulateList();
}

void UnitPropertiesWindow::NotifyMoveUpPressed()
{
    Selections &selections = CM->GetSelections();
    HWND hUnitList = listUnits.getHandle();

    selections.sortUnits(true);
    listUnits.SetRedraw(false);

    auto unitChanges = ReversibleActions::Make();
    unitChanges->Insert(UnitIndexMoveBoundary::Make());
    auto &selUnits = selections.getUnits();
    for ( u16 &unitIndex : selUnits )
    {
        if ( unitIndex > 0 && !selections.unitIsSelected(unitIndex - 1) )
        {
            if ( CM->SwapUnits(unitIndex, unitIndex - 1) )
            {
                unitChanges->Insert(UnitIndexMove::Make(unitIndex, unitIndex - 1));
                SwapIndexes(hUnitList, unitIndex, unitIndex - 1);
                unitIndex--;
            }
        }
    }

    ListView_SortItems(hUnitList, ForwardCompareLvItems, this);
    int row = listUnits.GetItemRow(selections.getHighestIndex());
    listUnits.EnsureVisible(row, false);
    row = listUnits.GetItemRow(selections.getLowestIndex());
    listUnits.EnsureVisible(row, false);
    unitChanges->Insert(UnitIndexMoveBoundary::Make());

    if ( unitChanges->Count() > 2 )
        CM->AddUndo(unitChanges);

    listUnits.SetRedraw(true);
}

void UnitPropertiesWindow::NotifyMoveDownPressed()
{
    Selections &selections = CM->GetSelections();
    HWND hUnitList = listUnits.getHandle();

    selections.sortUnits(false);
    listUnits.SetRedraw(false);

    auto unitChanges = ReversibleActions::Make();
    unitChanges->Insert(UnitIndexMoveBoundary::Make());
    auto &selUnits = selections.getUnits();
    for ( u16 &unitIndex : selUnits )
    {
        if ( unitIndex < CM->numUnits() && !selections.unitIsSelected(unitIndex + 1) )
        {
            if ( CM->SwapUnits(unitIndex, unitIndex + 1) )
            {
                unitChanges->Insert(UnitIndexMove::Make(unitIndex, unitIndex + 1));
                SwapIndexes(hUnitList, unitIndex, unitIndex + 1);
                unitIndex++;
            }
        }
    }

    unitChanges->Insert(UnitIndexMoveBoundary::Make());
    if ( unitChanges->Count() > 2 )
        CM->AddUndo(unitChanges);

    ListView_SortItems(hUnitList, ForwardCompareLvItems, this);
    int row = listUnits.GetItemRow(selections.getLowestIndex());
    listUnits.EnsureVisible(row, false);
    row = listUnits.GetItemRow(selections.getHighestIndex());
    listUnits.EnsureVisible(row, false);
    listUnits.SetRedraw(true);
}

void UnitPropertiesWindow::NotifyMoveToPressed()
{
    u32 unitMoveTo = 0;
    if ( MoveToDialog<u32>::GetIndex(unitMoveTo, getHandle()) && unitMoveTo < u32(CM->numUnits()) )
    {
        if ( unitMoveTo == 0 )
        {
            NotifyMoveTopPressed();
        }
        else if ( unitMoveTo + CM->GetSelections().numUnits() >= CM->numUnits() )
        {
            NotifyMoveEndPressed();
        }
        else if ( unitMoveTo > 0 )
        {
            Selections &selections = CM->GetSelections();
            u16 numUnitsSelected = selections.numUnits();
            u16 limit = CM->numUnits() - 1;

            u16 unitStackTopIndex = selections.getFirstUnit();
            u16 numUnits = selections.numUnits(),
                shift = numUnits - 1;

            selections.sortUnits(false); // Highest First
            listUnits.SetRedraw(false);

            ChkUnit* selectedUnits;
            try {
                selectedUnits = new ChkUnit[numUnits];
            }
            catch ( std::bad_alloc ) {
                Error("'Move To' failed.\n\nCould not allocate temporary storage, you may have run out of memory");
                return;
            }

            auto unitCreateDels = ReversibleActions::Make();
            u16 i = 0;
            auto &selUnits = selections.getUnits();
            for ( u16 &unitIndex : selUnits )
            { // Remove each selected unit from the map, store in selectedUnits
                u32 loc = ((u32)unitIndex)*UNIT_STRUCT_SIZE;
                selectedUnits[shift - i] = CM->getUnit(unitIndex);
                if ( CM->deleteUnit(unitIndex) )
                {
                    unitCreateDels->Insert(UnitCreateDel::Make(unitIndex, selectedUnits[shift - i]));
                    unitIndex = u16(unitMoveTo + shift - i);
                }
                i++;
            }

            for ( int i = 0; i < numUnits; i++ )
            {
                if ( CM->insertUnit(unitMoveTo + i, selectedUnits[i]) )
                    unitCreateDels->Insert(UnitCreateDel::Make(unitMoveTo + i));
            }

            selections.finishMove();
            selections.ensureFirst(unitStackTopIndex);
            CM->AddUndo(unitCreateDels);
            RepopulateList();
        }
    }
}

void UnitPropertiesWindow::NotifyDeletePressed()
{
    Selections& selections = CM->GetSelections();
    HWND hUnitList = listUnits.getHandle();
    listUnits.SetRedraw(false);
    auto unitDeletes = ReversibleActions::Make();
    while ( selections.hasUnits() )
    {
        u16 index = selections.getHighestIndex();
        selections.removeUnit(index);
        listUnits.RemoveRow(index);

        int row = listUnits.GetItemRow(index);

        ChkUnit unit = CM->getUnit(index);
        unitDeletes->Insert(UnitCreateDel::Make(index, unit));

        CM->deleteUnit(index);

        for ( int i = index + 1; i <= CM->numUnits(); i++ )
            ChangeIndex(hUnitList, i, i - 1);
    }
    CM->AddUndo(unitDeletes);
    CM->Redraw(true);
    listUnits.SetRedraw(true);
}

void UnitPropertiesWindow::NotifyInvincibleClicked()
{
    auto unitChanges = ReversibleActions::Make();
    auto &selUnits = CM->GetSelections().getUnits();
    for ( u16 &unitIndex : selUnits )
    {
        ChkUnit unit = CM->getUnit(unitIndex);
        unitChanges->Insert(UnitChange::Make(unitIndex, ChkUnitField::StateFlags, unit.stateFlags));

        if ( checkInvincible.isChecked() )
            unit.stateFlags |= UNIT_STATE_INVINCIBLE;
        else
            unit.stateFlags &= (~UNIT_STATE_INVINCIBLE);

        CM->ReplaceUnit(unitIndex, unit);
    }
    CM->AddUndo(unitChanges);
}

void UnitPropertiesWindow::NotifyHallucinatedClicked()
{
    auto unitChanges = ReversibleActions::Make();
    auto &selUnits = CM->GetSelections().getUnits();
    for ( u16 &unitIndex : selUnits )
    {
        ChkUnit unit = CM->getUnit(unitIndex);
        unitChanges->Insert(UnitChange::Make(unitIndex, ChkUnitField::StateFlags, unit.stateFlags));

        if ( checkHallucinated.isChecked() )
            unit.stateFlags |= UNIT_STATE_HALLUCINATED;
        else
            unit.stateFlags &= (~UNIT_STATE_HALLUCINATED);

        CM->ReplaceUnit(unitIndex, unit);
    }
    CM->AddUndo(unitChanges);
}

void UnitPropertiesWindow::NotifyBurrowedClicked()
{
    auto unitChanges = ReversibleActions::Make();
    auto &selUnits = CM->GetSelections().getUnits();
    for ( u16 &unitIndex : selUnits )
    {
        ChkUnit unit = CM->getUnit(unitIndex);
        unitChanges->Insert(UnitChange::Make(unitIndex, ChkUnitField::StateFlags, unit.stateFlags));

        if ( checkBurrowed.isChecked() )
            unit.stateFlags |= UNIT_STATE_BURROWED;
        else
            unit.stateFlags &= (~UNIT_STATE_BURROWED);

        CM->ReplaceUnit(unitIndex, unit);
    }
    CM->AddUndo(unitChanges);
}

void UnitPropertiesWindow::NotifyCloakedClicked()
{
    auto unitChanges = ReversibleActions::Make();
    auto &selUnits = CM->GetSelections().getUnits();
    for ( u16 &unitIndex : selUnits )
    {
        ChkUnit unit = CM->getUnit(unitIndex);
        unitChanges->Insert(UnitChange::Make(unitIndex, ChkUnitField::StateFlags, unit.stateFlags));

        if ( checkCloaked.isChecked() )
            unit.stateFlags |= UNIT_STATE_CLOAKED;
        else
            unit.stateFlags &= (~UNIT_STATE_CLOAKED);

        CM->ReplaceUnit(unitIndex, unit);
    }
    CM->AddUndo(unitChanges);
}

void UnitPropertiesWindow::NotifyLiftedClicked()
{
    auto unitChanges = ReversibleActions::Make();
    auto &selUnits = CM->GetSelections().getUnits();
    for ( u16 &unitIndex : selUnits )
    {
        ChkUnit unit = CM->getUnit(unitIndex);
        unitChanges->Insert(UnitChange::Make(unitIndex, ChkUnitField::StateFlags, unit.stateFlags));

        if ( checkLifted.isChecked() )
            unit.stateFlags |= UNIT_STATE_LIFTED; // Check lifted state
        else
            unit.stateFlags &= (~UNIT_STATE_LIFTED); // Uncheck lifted state

        CM->ReplaceUnit(unitIndex, unit);
    }
    CM->AddUndo(unitChanges);
}

void UnitPropertiesWindow::NotifyHpEditUpdated()
{
    u8 hpPercent = 0;
    if ( editLife.GetEditNum<u8>(hpPercent) )
    {
        auto &selUnits = CM->GetSelections().getUnits();
        for ( u16 &unitIndex : selUnits )
            CM->SetUnitHitpoints(unitIndex, hpPercent);

        CM->Redraw(false);
    }
}

void UnitPropertiesWindow::NotifyMpEditUpdated()
{
    u8 mpPercent = 0;
    if ( editMana.GetEditNum<u8>(mpPercent) )
    {
        auto &selUnits = CM->GetSelections().getUnits();
        for ( u16 &unitIndex : selUnits )
            CM->SetUnitEnergy(unitIndex, mpPercent);

        CM->Redraw(false);
    }
}

void UnitPropertiesWindow::NotifyShieldEditUpdated()
{
    u8 shieldPercent = 0;
    if ( editShield.GetEditNum<u8>(shieldPercent) )
    {
        auto &selUnits = CM->GetSelections().getUnits();
        for ( u16 &unitIndex : selUnits )
            CM->SetUnitShields(unitIndex, shieldPercent);

        CM->Redraw(false);
    }
}

void UnitPropertiesWindow::NotifyResourcesEditUpdated()
{
    u32 resources = 0;
    if ( editResources.GetEditNum<u32>(resources) )
    {
        auto &selUnits = CM->GetSelections().getUnits();
        for ( u16 &unitIndex : selUnits )
            CM->SetUnitResources(unitIndex, resources);

        CM->Redraw(false);
    }
}

void UnitPropertiesWindow::NotifyHangerEditUpdated()
{
    u16 hanger = 0;
    if ( editHanger.GetEditNum<u16>(hanger) )
    {
        auto &selUnits = CM->GetSelections().getUnits();
        for ( u16 &unitIndex : selUnits )
            CM->SetUnitHanger(unitIndex, hanger);

        CM->Redraw(true);
    }
}

void UnitPropertiesWindow::NotifyIdEditUpdated()
{
    u16 unitID = 0;
    if ( editUnitId.GetEditNum<u16>(unitID) )
    {
        auto &selUnits = CM->GetSelections().getUnits();
        for ( u16 &unitIndex : selUnits )
        {
            if ( CM->SetUnitTypeId(unitIndex, unitID) )
            {
                int row = listUnits.GetItemRow(unitIndex);

                ChkdString unitName;
                CM->getUnitName(unitName, unitID);
                listUnits.SetItemText(row, (int)UnitListColumn::Name, unitName.c_str());

                if ( unitIndex == CM->GetSelections().getFirstUnit() )
                    WindowsItem::SetWinText(unitName.c_str());
            }
        }
        CM->Redraw(true);
        ListView_SortItems(listUnits.getHandle(), ForwardCompareLvItems, this);
    }
}

void UnitPropertiesWindow::NotifyXcEditUpdated()
{
    u16 unitXC = 0;
    if ( editXc.GetEditNum<u16>(unitXC) )
    {
        auto &selUnits = CM->GetSelections().getUnits();
        for ( u16 &unitIndex : selUnits )
        {
            if ( CM->SetUnitXc(unitIndex, unitXC) )
            {
                int row = listUnits.GetItemRow(unitIndex);
                listUnits.SetItemText(row, (int)UnitListColumn::Xc, unitXC);
            }
        }
        CM->Redraw(true);
        ListView_SortItems(listUnits.getHandle(), ForwardCompareLvItems, this);
    }
}

void UnitPropertiesWindow::NotifyYcEditUpdated()
{
    u16 unitYC = 0;
    if ( editYc.GetEditNum<u16>(unitYC) )
    {
        auto &selUnits = CM->GetSelections().getUnits();
        for ( u16 &unitIndex : selUnits )
        {
            if ( CM->SetUnitYc(unitIndex, unitYC) )
            {
                int row = listUnits.GetItemRow(unitIndex);
                listUnits.SetItemText(row, (int)UnitListColumn::Yc, unitYC);
            }
        }
        CM->Redraw(true);
        ListView_SortItems(listUnits.getHandle(), ForwardCompareLvItems, this);
    }
}

void UnitPropertiesWindow::NotifyButtonClicked(int idFrom, HWND hWndFrom)
{
    switch ( (Id)idFrom )
    {
        case Id::ButtonClose: NotifyClosePressed(); break;
        case Id::ButtonMoveTop: NotifyMoveTopPressed(); break;
        case Id::ButtonMoveEnd: NotifyMoveEndPressed(); break;
        case Id::ButtonMoveUp: NotifyMoveUpPressed(); break;
        case Id::ButtonMoveDown: NotifyMoveDownPressed(); break;
        case Id::ButtonMoveTo: NotifyMoveToPressed(); break;
        case Id::ButtonDelete: NotifyDeletePressed(); break;
        case Id::CheckInvincible: NotifyInvincibleClicked(); break;
        case Id::CheckHallucinated: NotifyHallucinatedClicked(); break;
        case Id::CheckBurrowed: NotifyBurrowedClicked(); break;
        case Id::CheckCloaked: NotifyCloakedClicked(); break;
        case Id::CheckLifted: NotifyLiftedClicked(); break;
    }
}

void UnitPropertiesWindow::NotifyEditUpdated(int idFrom, HWND hWndFrom)
{
    if ( initilizing )
        return;

    switch ( (Id)idFrom )
    {
        case Id::EditHp: NotifyHpEditUpdated(); break;
        case Id::EditMp: NotifyMpEditUpdated(); break;
        case Id::EditShields: NotifyShieldEditUpdated(); break;
        case Id::EditResources: NotifyResourcesEditUpdated(); break;
        case Id::EditHanger: NotifyHangerEditUpdated(); break;
        case Id::EditUnitId: NotifyIdEditUpdated(); break;
        case Id::EditXc: NotifyXcEditUpdated(); break;
        case Id::EditYc: NotifyYcEditUpdated(); break;
    }
}

void UnitPropertiesWindow::NotifyEditFocused(int idFrom, HWND hWndFrom)
{
    switch ( (Id)idFrom )
    {
        case Id::EditHp: preservedStats.AddStats(CM->GetSelections(), ChkUnitField::Hitpoints); break;
        case Id::EditMp: preservedStats.AddStats(CM->GetSelections(), ChkUnitField::Energy); break;
        case Id::EditShields: preservedStats.AddStats(CM->GetSelections(), ChkUnitField::Shields); break;
        case Id::EditResources: preservedStats.AddStats(CM->GetSelections(), ChkUnitField::Resources); break;
        case Id::EditHanger: preservedStats.AddStats(CM->GetSelections(), ChkUnitField::Hanger); break;
        case Id::EditUnitId: preservedStats.AddStats(CM->GetSelections(), ChkUnitField::Id); break;
        case Id::EditXc: preservedStats.AddStats(CM->GetSelections(), ChkUnitField::Xc); break;
        case Id::EditYc: preservedStats.AddStats(CM->GetSelections(), ChkUnitField::Yc); break;
    }
}

void UnitPropertiesWindow::NotifyEditFocusLost(int idFrom, HWND hWndFrom)
{
    Id editId = (Id)idFrom;

    if ( editId == Id::EditHp || editId == Id::EditMp || editId == Id::EditShields ||
        editId == Id::EditResources || editId == Id::EditHanger || editId == Id::EditUnitId ||
        editId == Id::EditXc || editId == Id::EditYc )
    {
        preservedStats.convertToUndo();
    }
}

void UnitPropertiesWindow::NotifyComboSelChanged(int idFrom, HWND hWndFrom)
{
    if ( (Id)idFrom == Id::ComboPlayer )
        ChangeCurrOwner(dropPlayer.GetPlayerBySelNum());
}

void UnitPropertiesWindow::NotifyComboEditUpdated(int idFrom, HWND hWndFrom)
{
    if ( (Id)idFrom == Id::ComboPlayer )
    {
        u8 newPlayer = 0;
        if ( dropPlayer.GetPlayerNum(newPlayer) )
            ChangeCurrOwner(newPlayer);
    }
}

BOOL UnitPropertiesWindow::Activate(WPARAM wParam, LPARAM lParam)
{
    if ( LOWORD(wParam) != WA_INACTIVE )
        chkd.SetCurrDialog(getHandle());
    
    return FALSE;
}

BOOL UnitPropertiesWindow::ShowWindow(WPARAM wParam, LPARAM lParam)
{
    BOOL result = ClassDialog::DlgProc(getHandle(), WM_SHOWWINDOW, wParam, lParam);
    if ( wParam == TRUE )
        SetFocus(listUnits.getHandle());

    return result;
}

BOOL UnitPropertiesWindow::DlgNotify(HWND hWnd, WPARAM idFrom, NMHDR* nmhdr)
{
    switch ( nmhdr->code )
    {
        case LVN_COLUMNCLICK: LvColumnClicked(nmhdr); break;
        case LVN_ITEMCHANGED: LvItemChanged(nmhdr); break;
    }
    return ClassDialog::DlgNotify(hWnd, idFrom, nmhdr);
}

BOOL UnitPropertiesWindow::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch( msg )
    {
        case WM_ACTIVATE: return Activate(wParam, lParam); break;
        case WM_SHOWWINDOW: return ShowWindow(wParam, lParam); break;
        case WM_CLOSE: DestroyThis(); break;
        default: return ClassDialog::DlgProc(hWnd, msg, wParam, lParam); break;
    }
    return TRUE;
}
