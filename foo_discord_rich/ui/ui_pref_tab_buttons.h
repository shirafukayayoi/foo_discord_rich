#pragma once

#include <discord/button_config.h>
#include <fb2k/config.h>
#include <ui/ui_itab.h>

#include <resource.h>

#include <foobar2000/SDK/coreDarkMode.h>
#include <qwr/fb2k_config_ui_option.h>
#include <qwr/macros.h>
#include <qwr/ui_ddx_option.h>

#include <vector>

namespace drp::ui
{

class PreferenceTabManager;

class PreferenceTabButtons
    : public CDialogImpl<PreferenceTabButtons>
    , public CWinDataExchange<PreferenceTabButtons>
    , public ITab
{
public:
    enum
    {
        IDD = IDD_PREFS_BUTTONS_TAB
    };

    BEGIN_MSG_MAP( PreferenceTabButtons )
        MSG_WM_INITDIALOG( OnInitDialog )
        COMMAND_HANDLER_EX( IDC_BUTTON_ADD_RULE, BN_CLICKED, OnAddRule )
        COMMAND_HANDLER_EX( IDC_BUTTON_EDIT_RULE, BN_CLICKED, OnEditRule )
        COMMAND_HANDLER_EX( IDC_BUTTON_DELETE_RULE, BN_CLICKED, OnDeleteRule )
        NOTIFY_HANDLER( IDC_LIST_RULES, LVN_ITEMCHANGED, OnListSelectionChanged )
        NOTIFY_HANDLER( IDC_LIST_RULES, NM_DBLCLK, OnListDoubleClick )
    END_MSG_MAP()

public:
    PreferenceTabButtons( PreferenceTabManager* pParent );
    ~PreferenceTabButtons() override;

    // IUiTab
    HWND CreateTab( HWND hParent ) override;
    CDialogImplBase& Dialog() override;
    const wchar_t* Name() const override;
    void OnUiChangeRequest( int nID, bool enable ) override;
    t_uint32 GetState() override;
    void Apply() override;
    void Reset() override;

private:
    BOOL OnInitDialog( HWND hwndFocus, LPARAM lParam );
    void OnAddRule( UINT uNotifyCode, int nID, CWindow wndCtl );
    void OnEditRule( UINT uNotifyCode, int nID, CWindow wndCtl );
    void OnDeleteRule( UINT uNotifyCode, int nID, CWindow wndCtl );
    LRESULT OnListSelectionChanged( int idCtrl, LPNMHDR pnmh, BOOL& bHandled );
    LRESULT OnListDoubleClick( int idCtrl, LPNMHDR pnmh, BOOL& bHandled );

    void LoadRulesFromConfig();
    void SaveRulesToConfig();
    void RefreshRulesList();
    void UpdateButtonStates();
    void EditRule( int index );

private:
    PreferenceTabManager* pParent_ = nullptr;
    std::vector<drp::button_config::ButtonRule> rules_;
    CListViewCtrl listRules_;
    bool isModified_ = false;
};

} // namespace drp::ui
