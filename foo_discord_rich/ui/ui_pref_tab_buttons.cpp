#include <stdafx.h>

#include "ui_pref_tab_buttons.h"

#include "ui_pref_tab_manager.h"

#include <discord/button_config.h>
#include <fb2k/config.h>

#include <nlohmann/json.hpp>

namespace drp::ui
{

namespace
{

class ButtonRuleDialog : public CDialogImpl<ButtonRuleDialog>
{
public:
    enum
    {
        IDD = IDD_DLG_BUTTON_RULE
    };

    ButtonRuleDialog( const drp::button_config::ButtonRule& rule = {} )
        : rule_( rule )
    {
    }

    BEGIN_MSG_MAP( ButtonRuleDialog )
        MSG_WM_INITDIALOG( OnInitDialog )
        COMMAND_ID_HANDLER_EX( IDOK, OnOk )
        COMMAND_ID_HANDLER_EX( IDCANCEL, OnCancel )
    END_MSG_MAP()

    const drp::button_config::ButtonRule& GetRule() const
    {
        return rule_;
    }

private:
    BOOL OnInitDialog( HWND hwndFocus, LPARAM lParam )
    {
        CWindow conditionEdit = GetDlgItem( IDC_EDIT_CONDITION );
        CWindow labelEdit = GetDlgItem( IDC_EDIT_LABEL );
        CWindow urlEdit = GetDlgItem( IDC_EDIT_URL );

        conditionEdit.SetWindowTextW( qwr::unicode::ToWide( rule_.matchCondition ).c_str() );
        labelEdit.SetWindowTextW( qwr::unicode::ToWide( rule_.label ).c_str() );
        urlEdit.SetWindowTextW( qwr::unicode::ToWide( rule_.url ).c_str() );

        // Center dialog relative to parent
        CenterWindow( GetParent() );

        return TRUE;
    }

    void OnOk( UINT uNotifyCode, int nID, CWindow wndCtl )
    {
        // Get text from edits
        wchar_t conditionBuffer[512] = {};
        wchar_t labelBuffer[512] = {};
        wchar_t urlBuffer[1024] = {};

        GetDlgItem( IDC_EDIT_CONDITION ).GetWindowTextW( conditionBuffer, std::size( conditionBuffer ) );
        GetDlgItem( IDC_EDIT_LABEL ).GetWindowTextW( labelBuffer, std::size( labelBuffer ) );
        GetDlgItem( IDC_EDIT_URL ).GetWindowTextW( urlBuffer, std::size( urlBuffer ) );

        const auto condition = qwr::unicode::ToU8( std::wstring_view{ conditionBuffer } );
        const auto label = qwr::unicode::ToU8( std::wstring_view{ labelBuffer } );
        const auto url = qwr::unicode::ToU8( std::wstring_view{ urlBuffer } );

        if ( label.empty() || url.empty() )
        {
            MessageBoxW( L"Label and URL are required.", L"Invalid Input", MB_ICONWARNING );
            return;
        }

        // Validate condition format only when a condition is specified
        if ( !condition.empty() && condition.find( '=' ) == qwr::u8string::npos )
        {
            MessageBoxW( L"Condition format: %key%=value (e.g., %artist%=YOASOBI)\nLeave empty to match all tracks.", L"Invalid Condition Format", MB_ICONWARNING );
            return;
        }

        rule_.matchCondition = condition;
        rule_.label = label;
        rule_.url = url;

        EndDialog( IDOK );
    }

    void OnCancel( UINT uNotifyCode, int nID, CWindow wndCtl )
    {
        EndDialog( IDCANCEL );
    }

private:
    drp::button_config::ButtonRule rule_;
};

} // namespace

PreferenceTabButtons::PreferenceTabButtons( PreferenceTabManager* pParent )
    : pParent_( pParent )
{
}

PreferenceTabButtons::~PreferenceTabButtons() = default;

HWND PreferenceTabButtons::CreateTab( HWND hParent )
{
    return CDialogImpl<PreferenceTabButtons>::Create( hParent );
}

CDialogImplBase& PreferenceTabButtons::Dialog()
{
    return *this;
}

const wchar_t* PreferenceTabButtons::Name() const
{
    return L"Button Settings";
}

void PreferenceTabButtons::OnUiChangeRequest( int nID, bool enable )
{
}

t_uint32 PreferenceTabButtons::GetState()
{
    return preferences_state::resettable | ( isModified_ ? preferences_state::changed : 0 );
}

BOOL PreferenceTabButtons::OnInitDialog( HWND hwndFocus, LPARAM lParam )
{
    listRules_ = GetDlgItem( IDC_LIST_RULES );

    // Initialize list columns
    listRules_.InsertColumn( 0, L"Match Condition", LVCFMT_LEFT, 200 );
    listRules_.InsertColumn( 1, L"Button Label", LVCFMT_LEFT, 150 );
    listRules_.InsertColumn( 2, L"URL Template", LVCFMT_LEFT, 300 );

    LoadRulesFromConfig();
    RefreshRulesList();
    UpdateButtonStates();

    return TRUE;
}

void PreferenceTabButtons::OnAddRule( UINT uNotifyCode, int nID, CWindow wndCtl )
{
    ButtonRuleDialog dlg;
    if ( dlg.DoModal( m_hWnd ) == IDOK )
    {
        rules_.push_back( dlg.GetRule() );
        isModified_ = true;
        RefreshRulesList();
        UpdateButtonStates();
        pParent_->OnDataChanged();
    }
}

void PreferenceTabButtons::OnEditRule( UINT uNotifyCode, int nID, CWindow wndCtl )
{
    int selectedIndex = listRules_.GetSelectedIndex();
    if ( selectedIndex >= 0 && selectedIndex < static_cast<int>( rules_.size() ) )
    {
        EditRule( selectedIndex );
    }
}

void PreferenceTabButtons::OnDeleteRule( UINT uNotifyCode, int nID, CWindow wndCtl )
{
    int selectedIndex = listRules_.GetSelectedIndex();
    if ( selectedIndex >= 0 && selectedIndex < static_cast<int>( rules_.size() ) )
    {
        rules_.erase( rules_.begin() + selectedIndex );
        isModified_ = true;
        RefreshRulesList();
        UpdateButtonStates();
        pParent_->OnDataChanged();
    }
}

LRESULT PreferenceTabButtons::OnListSelectionChanged( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
    UpdateButtonStates();
    return 0;
}

LRESULT PreferenceTabButtons::OnListDoubleClick( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
    int selectedIndex = listRules_.GetSelectedIndex();
    if ( selectedIndex >= 0 )
    {
        EditRule( selectedIndex );
    }
    return 0;
}

void PreferenceTabButtons::LoadRulesFromConfig()
{
    try
    {
        const auto rulesJson = nlohmann::json::parse( static_cast<std::string>( drp::config::button2Rules ) );
        rules_ = rulesJson.get<std::vector<drp::button_config::ButtonRule>>();
    }
    catch ( const std::exception& )
    {
        rules_.clear();
    }
}

void PreferenceTabButtons::SaveRulesToConfig()
{
    try
    {
        const auto rulesJson = nlohmann::json( rules_ );
        drp::config::button2Rules = rulesJson.dump();
    }
    catch ( const std::exception& )
    {
        // Fallback to empty rules
        drp::config::button2Rules = "[]";
    }
}

void PreferenceTabButtons::RefreshRulesList()
{
    listRules_.DeleteAllItems();

    for ( size_t i = 0; i < rules_.size(); ++i )
    {
        const auto& rule = rules_[i];
        const auto condition = rule.matchCondition.empty() ? std::wstring( L"(default - all tracks)" ) : qwr::unicode::ToWide( rule.matchCondition );
        int item = listRules_.InsertItem( static_cast<int>( i ), condition.c_str() );
        listRules_.SetItemText( item, 1, qwr::unicode::ToWide( rule.label ).c_str() );
        listRules_.SetItemText( item, 2, qwr::unicode::ToWide( rule.url ).c_str() );
    }
}

void PreferenceTabButtons::UpdateButtonStates()
{
    int selectedIndex = listRules_.GetSelectedIndex();
    BOOL enableEdit = ( selectedIndex >= 0 );

    GetDlgItem( IDC_BUTTON_EDIT_RULE ).EnableWindow( enableEdit );
    GetDlgItem( IDC_BUTTON_DELETE_RULE ).EnableWindow( enableEdit );
}

void PreferenceTabButtons::EditRule( int index )
{
    if ( index < 0 || index >= static_cast<int>( rules_.size() ) )
    {
        return;
    }

    ButtonRuleDialog dlg( rules_[index] );
    if ( dlg.DoModal( m_hWnd ) == IDOK )
    {
        rules_[index] = dlg.GetRule();
        isModified_ = true;
        RefreshRulesList();
        pParent_->OnDataChanged();
    }
}

void PreferenceTabButtons::Apply()
{
    if ( isModified_ )
    {
        SaveRulesToConfig();
        isModified_ = false;
    }
}

void PreferenceTabButtons::Reset()
{
    LoadRulesFromConfig();
    RefreshRulesList();
    isModified_ = false;
}

} // namespace drp::ui
