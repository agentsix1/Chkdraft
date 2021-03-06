#include "TrigModifyText.h"
#include "Chkdraft.h"

enum ID {
    TAB_META = 0,
    TAB_PLAYERS,
    TAB_CONDITIONS,
    TAB_ACTIONS,
    TAB_TEXT,

    EDIT_TRIGMODIFYTEXT = ID_FIRST,
    CHECK_AUTOCOMPILE,
    BUTTON_COMPILE,
    BUTTON_COMPILEANDSAVE,
    BUTTON_RELOAD
};

TrigModifyTextWindow::TrigModifyTextWindow() : trigIndex(0), autoCompile(false)
{

}

bool TrigModifyTextWindow::CreateThis(HWND hParent, u32 windowId)
{
    if ( getHandle() != NULL )
    {
        trigIndex = 0;
        return SetParent(hParent);
    }

    RECT rcCli;
    if ( GetWindowRect(hParent, &rcCli) &&
         ClassWindow::RegisterWindowClass(0, NULL, NULL, NULL, NULL, "TrigModifyTextWindow", NULL, false) &&
         ClassWindow::CreateClassWindow(0, "TrigModifyTextWindow", WS_CHILD,
            5, 62, rcCli.right-rcCli.left, rcCli.bottom-rcCli.top,
            hParent, (HMENU)windowId) )
    {
        CreateSubWindows(getHandle());
        return true;
    }
    else
        return false;
}

bool TrigModifyTextWindow::DestroyThis()
{
    return false;
}

void TrigModifyTextWindow::RefreshWindow(u32 trigIndex)
{
    this->trigIndex = trigIndex;
    TextTrigGenerator textTrigs;
    if ( textTrigs.GenerateTextTrigs(CM, trigIndex, trigText) )
        editText.SetText(trigText.c_str());
    else
        mb(trigIndex, "Failed to generate text triggers.");
}

void TrigModifyTextWindow::DoSize()
{
    RECT rect;
    if ( getClientRect(rect) )
    {
        int width = rect.right-rect.left,
            height = rect.bottom-rect.top;
        
        buttonReload.MoveTo(rect.right-buttonReload.Width()-10,
            rect.bottom-buttonCompileAndSave.Height()-10);
        buttonCompileAndSave.MoveTo(buttonReload.Left()-buttonCompileAndSave.Width()-5,
            rect.bottom-buttonCompileAndSave.Height()-10);
        buttonCompile.MoveTo(buttonCompileAndSave.Left()-buttonCompile.Width()-5,
            rect.bottom-buttonCompile.Height()-10);
        checkAutoCompile.MoveTo(buttonCompile.Left()-checkAutoCompile.Width()-5,
            rect.bottom-checkAutoCompile.Height()-10);
        editText.SetPos(3, 1, width-5, rect.bottom-buttonCompile.Height()-20);
    }
}

void TrigModifyTextWindow::ParentHidden()
{
    OnLeave();
}

void TrigModifyTextWindow::CreateSubWindows(HWND hWnd)
{
    editText.CreateThis(hWnd, 0, 0, 100, 100, true, EDIT_TRIGMODIFYTEXT);
    checkAutoCompile.CreateThis(hWnd, 0, 0, 85, 20, false, "Auto-compile", CHECK_AUTOCOMPILE);
    
    buttonReload.CreateThis(hWnd, 0, 0, 75, 23, "Reset", BUTTON_RELOAD);
    buttonCompile.CreateThis(hWnd, 20, 105, 75, 23, "Compile", BUTTON_COMPILE);
    buttonCompileAndSave.CreateThis(hWnd, 75, 105, 93, 23, "Compile && Save", BUTTON_COMPILEANDSAVE);
}

bool TrigModifyTextWindow::unsavedChanges()
{
    std::string newText;
    if ( editText.GetWinText(newText) )
        return trigText.compare(newText) != 0;
    else
        return false;
}

void TrigModifyTextWindow::Compile(bool silent, bool saveAfter)
{
    if ( CM != nullptr )
    {
        std::string newText;
        if ( editText.GetWinText(newText) )
        {
            if ( trigText.compare(newText) != 0 )
            {
                if ( CompileEditText(newText) )
                {
                    trigText = newText;
                    CM->notifyChange(false);
                    CM->refreshScenario();
                    if ( saveAfter )
                    {
                        if ( chkd.maps.SaveCurr(false) )
                            MessageBox(NULL, "Success", "Compiler", MB_OK);
                        else
                            MessageBox(NULL, "Compile Succeeded, Save Failed", "Compiler", MB_OK);
                    }
                    else if ( !silent )
                        MessageBox(NULL, "Success", "Compiler", MB_OK);

                    RefreshWindow(trigIndex);
                }
            }
            else if ( !silent )
                MessageBox(NULL, "No change", NULL, MB_OK);
        }
        else if ( !silent )
            MessageBox(NULL, "Compilation failed, couldn't get edit text.", "Error!", MB_OK);
    }
    else
        Error("No map open!");
}

bool TrigModifyTextWindow::CompileEditText(std::string &newText)
{
    if ( CM != nullptr )
    {
        Trigger* trigger;
        if ( CM->getTrigger(trigger, trigIndex) )
        {
            TextTrigCompiler compiler; // All data for compilation is gathered on-the-fly, no need to check for updates
            if ( compiler.CompileTrigger(newText, trigger, CM, chkd.scData) )
                return true;
            else
                MessageBox(NULL, "Compilation failed.", "Error!", MB_OK);
        }
        else
            MessageBox(NULL, "Compilation failed, couldn't find selected trigger", "Error!", MB_OK);
    }
    return false;
}

void TrigModifyTextWindow::OnLeave()
{
    if ( autoCompile )
        Compile(true, false);
    else if ( unsavedChanges() )
    {
        int result = MessageBox(NULL, "Compile changes?", "Message", MB_YESNO);
        if ( result == IDYES )
            Compile(false, false);
    }
}

LRESULT TrigModifyTextWindow::Command(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch ( LOWORD(wParam) )
    {
    case CHECK_AUTOCOMPILE:
        if ( HIWORD(wParam) == BN_CLICKED )
            autoCompile = checkAutoCompile.isChecked();
        break;
    case BUTTON_RELOAD:
        if ( HIWORD(wParam) == BN_CLICKED )
            RefreshWindow(trigIndex);
        break;
    case BUTTON_COMPILE:
        if ( HIWORD(wParam) == BN_CLICKED )
            Compile(false, false);
        break;
    case BUTTON_COMPILEANDSAVE:
        if ( HIWORD(wParam) == BN_CLICKED )
            Compile(false, true);
        break;
    }
    return ClassWindow::Command(hWnd, wParam, lParam);
}

LRESULT TrigModifyTextWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
        case WM_SHOWWINDOW:
            if ( wParam == FALSE )
                OnLeave();
            else if ( wParam == TRUE )
                RefreshWindow(trigIndex);
            break;

        case WM_CLOSE:
            OnLeave();
            return ClassWindow::WndProc(hWnd, msg, wParam, lParam);
            break;

        default:
            return ClassWindow::WndProc(hWnd, msg, wParam, lParam);
            break;
    }
    return 0;
}
