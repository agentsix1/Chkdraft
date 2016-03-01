#ifndef CLASSDIALOG_H
#define CLASSDIALOG_H
#include "WindowsItem.h"
#include <CommCtrl.h>

class ClassDialog : public WindowsItem
{
    public:

        ClassDialog();

        ~ClassDialog();

    protected:

        /** Attempts to create a modeless dialog
        The variables are used the same as in CreateDialog */
        bool CreateModelessDialog(LPCSTR lpTemplateName, HWND hWndParent);

        /** Attempts to create a dialog box
        The variables and return value are used the same as in DialogBox */
        INT_PTR CreateDialogBox(LPCSTR lpTemplateName, HWND hWndParent);

        /** This method attempts to destroy an encapsulated modeless dialog */
        bool DestroyDialog();

        /** This method is called when WM_NOTIFY is sent to the dialog
        window, override this to respond to notifications */
        virtual BOOL DlgNotify(HWND hWnd, WPARAM idFrom, NMHDR* nmhdr);

        /** When overidden, these methods serve as easy ways to process
        notifications/commands */
        virtual void NotifyTreeSelChanged(LPARAM newValue); // Sent when a new tree item is selected
        virtual void NotifyButtonClicked(int idFrom, HWND hWndFrom); // Sent when a button or checkbox is clicked
        virtual void NotifyEditUpdated(int idFrom, HWND hWndFrom); // Sent when edit text changes, before redraw
        virtual void NotifyEditFocusLost(int idFrom, HWND hWndFrom); /* Sent when focus changes or the window is hidden;
                                                                        if this is the result of the window being hidden,
                                                                        idFrom = 0 and hWndFrom = NULL */
        virtual void NotifyWindowHidden(); // Sent when the window is hidden
        virtual void NotifyWindowShown(); // Sent when the window is shown

        /** This method is called when WM_COMMAND is sent to the dialog
        window, override this to respond to commands */
        virtual BOOL DlgCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

        /** This method is where all dialog messages are processed
        Override this in descendant classes to handle dialog messages */
        virtual BOOL DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


    private:

        bool allowEditNotify; // Used to prevent edit update recursion

        /* Calls NotifyEditUpdated and ensures the function is not called again until it returns */
        void SendNotifyEditUpdated(int idFrom, HWND hWndFrom);

        /** This method is used by Dialog windows until WM_INITDIALOG is handled, at which point
        ForwardDlgProc is used to forward messages to the WndProc method */
        static BOOL CALLBACK SetupDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        /** This method simply returns the value given by DlgProc with the same parameters
        This replaces SetupDialogProc to remove the overhead of checking for WM_INITDIALOG
        and ensuring that the class pointer is set */
        static BOOL CALLBACK ForwardDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif