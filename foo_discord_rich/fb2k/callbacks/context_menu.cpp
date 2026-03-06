#include <stdafx.h>

#include <discord/button_config.h>
#include <fb2k/config.h>

#include <resource.h>

#include <nlohmann/json.hpp>
#include <qwr/unicode.h>

namespace
{

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

std::vector<drp::button_config::ButtonRule> LoadRules()
{
    try
    {
        return nlohmann::json::parse( static_cast<std::string>( drp::config::button2Rules ) )
            .get<std::vector<drp::button_config::ButtonRule>>();
    }
    catch ( ... )
    {
        return {};
    }
}

void SaveRules( const std::vector<drp::button_config::ButtonRule>& rules )
{
    try
    {
        drp::config::button2Rules = nlohmann::json( rules ).dump();
    }
    catch ( ... )
    {
        drp::config::button2Rules = "[]";
    }
}

std::string EvalTf( metadb_handle_ptr handle, const char* expr )
{
    titleformat_object::ptr tf;
    titleformat_compiler::get()->compile_safe( tf, expr );
    pfc::string8_fast result;
    handle->format_title( nullptr, result, tf, nullptr );
    return result.c_str();
}

// ---------------------------------------------------------------------------
// Quick-set dialog  (IDD_DLG_QUICK_BUTTON)
// ---------------------------------------------------------------------------

class QuickButtonDialog : public CDialogImpl<QuickButtonDialog>
{
public:
    enum
    {
        IDD = IDD_DLG_QUICK_BUTTON
    };

    // conditions: list of condition strings to add/remove (one per track, or single for album)
    QuickButtonDialog( std::wstring targetDisplay, std::vector<std::string> conditions )
        : targetDisplay_( std::move( targetDisplay ) )
        , conditions_( std::move( conditions ) )
    {
        // Pre-fill with existing rule for the first condition (if any)
        if ( !conditions_.empty() )
        {
            const auto rules = LoadRules();
            for ( const auto& rule: rules )
            {
                if ( rule.matchCondition == conditions_[0] )
                {
                    initialLabel_ = rule.label;
                    initialUrl_ = rule.url;
                    break;
                }
            }
        }
    }

    BEGIN_MSG_MAP( QuickButtonDialog )
        MSG_WM_INITDIALOG( OnInitDialog )
        COMMAND_ID_HANDLER_EX( IDOK, OnOk )
        COMMAND_ID_HANDLER_EX( IDCANCEL, OnCancel )
        COMMAND_ID_HANDLER_EX( IDC_BUTTON_CLEAR_RULE, OnClear )
    END_MSG_MAP()

private:
    BOOL OnInitDialog( HWND, LPARAM )
    {
        GetDlgItem( IDC_STATIC_TARGET_INFO ).SetWindowTextW( targetDisplay_.c_str() );
        GetDlgItem( IDC_EDIT_QB_LABEL ).SetWindowTextW( qwr::unicode::ToWide( initialLabel_ ).c_str() );
        GetDlgItem( IDC_EDIT_QB_URL ).SetWindowTextW( qwr::unicode::ToWide( initialUrl_ ).c_str() );
        CenterWindow( GetParent() );
        return TRUE;
    }

    void OnOk( UINT, int, CWindow )
    {
        wchar_t labelBuf[512] = {};
        wchar_t urlBuf[1024] = {};
        GetDlgItem( IDC_EDIT_QB_LABEL ).GetWindowTextW( labelBuf, static_cast<int>( std::size( labelBuf ) ) );
        GetDlgItem( IDC_EDIT_QB_URL ).GetWindowTextW( urlBuf, static_cast<int>( std::size( urlBuf ) ) );

        const auto label = qwr::unicode::ToU8( std::wstring_view{ labelBuf } );
        const auto url = qwr::unicode::ToU8( std::wstring_view{ urlBuf } );

        if ( label.empty() || url.empty() )
        {
            MessageBoxW( L"Label and URL are required.", L"Discord Rich Presence", MB_ICONWARNING );
            return;
        }

        // Remove existing rules matching these conditions, then prepend new ones
        auto rules = LoadRules();
        for ( const auto& cond: conditions_ )
        {
            rules.erase(
                std::remove_if( rules.begin(), rules.end(), [&]( const drp::button_config::ButtonRule& r ) { return r.matchCondition == cond; } ),
                rules.end() );
        }
        // Insert at the front so track/album rules take precedence over general defaults
        for ( auto it = conditions_.rbegin(); it != conditions_.rend(); ++it )
        {
            rules.insert( rules.begin(), drp::button_config::ButtonRule{ *it, label, url } );
        }
        SaveRules( rules );
        EndDialog( IDOK );
    }

    void OnCancel( UINT, int, CWindow )
    {
        EndDialog( IDCANCEL );
    }

    void OnClear( UINT, int, CWindow )
    {
        auto rules = LoadRules();
        for ( const auto& cond: conditions_ )
        {
            rules.erase(
                std::remove_if( rules.begin(), rules.end(), [&]( const drp::button_config::ButtonRule& r ) { return r.matchCondition == cond; } ),
                rules.end() );
        }
        SaveRules( rules );
        EndDialog( IDOK );
    }

    std::wstring targetDisplay_;
    std::vector<std::string> conditions_;
    std::string initialLabel_;
    std::string initialUrl_;
};

// ---------------------------------------------------------------------------
// Context menu group: "Discord Rich Presence" submenu
// ---------------------------------------------------------------------------

static contextmenu_group_popup_factory g_drpContextMenuGroup(
    drp::guid::context_menu_group,
    contextmenu_groups::root,
    "Discord Rich Presence",
    0.0 );

// ---------------------------------------------------------------------------
// Context menu items
// ---------------------------------------------------------------------------

class DiscordContextMenuItems : public contextmenu_item_simple
{
public:
    unsigned get_num_items() override
    {
        return 2;
    }

    void get_item_name( unsigned p_index, pfc::string_base& p_out ) override
    {
        switch ( p_index )
        {
        case 0:
            p_out = "Set button for this track...";
            break;
        case 1:
            p_out = "Set button for this album...";
            break;
        default:
            uBugCheck();
        }
    }

    bool get_item_description( unsigned p_index, pfc::string_base& p_out ) override
    {
        switch ( p_index )
        {
        case 0:
            p_out = "Set a custom Discord button for the selected track(s)";
            return true;
        case 1:
            p_out = "Set a custom Discord button for the album of the selected track";
            return true;
        default:
            return false;
        }
    }

    GUID get_item_guid( unsigned p_index ) override
    {
        switch ( p_index )
        {
        case 0:
            return drp::guid::context_menu_set_track_button;
        case 1:
            return drp::guid::context_menu_set_album_button;
        default:
            uBugCheck();
        }
    }

    GUID get_parent() override
    {
        return drp::guid::context_menu_group;
    }

    void context_command( unsigned p_index, metadb_handle_list_cref p_data, const GUID& /*p_caller*/ ) override
    {
        if ( p_data.get_count() == 0 )
        {
            return;
        }

        if ( p_index == 0 )
        {
            // Per-track: condition = %path%=<file path>
            titleformat_object::ptr tfPath;
            titleformat_compiler::get()->compile_safe( tfPath, "%path%" );
            titleformat_object::ptr tfTitle;
            titleformat_compiler::get()->compile_safe( tfTitle, "%title%" );

            std::vector<std::string> conditions;
            conditions.reserve( p_data.get_count() );
            for ( t_size i = 0; i < p_data.get_count(); ++i )
            {
                pfc::string8_fast path;
                p_data[i]->format_title( nullptr, path, tfPath, nullptr );
                conditions.push_back( std::string( "%path%=" ) + path.c_str() );
            }

            std::wstring display;
            if ( p_data.get_count() == 1 )
            {
                pfc::string8_fast title;
                p_data[0]->format_title( nullptr, title, tfTitle, nullptr );
                display = L"Track: " + qwr::unicode::ToWide( std::string_view{ title.c_str() } );
            }
            else
            {
                display = L"Tracks: " + std::to_wstring( p_data.get_count() ) + L" selected";
            }

            QuickButtonDialog dlg( std::move( display ), std::move( conditions ) );
            dlg.DoModal( core_api::get_main_window() );
        }
        else if ( p_index == 1 )
        {
            // Per-album: condition = %album%=<album name>
            titleformat_object::ptr tfAlbum;
            titleformat_compiler::get()->compile_safe( tfAlbum, "%album%" );

            pfc::string8_fast album;
            p_data[0]->format_title( nullptr, album, tfAlbum, nullptr );
            const std::string albumStr = album.c_str();

            if ( albumStr.empty() )
            {
                MessageBoxW( core_api::get_main_window(),
                             L"This track has no album tag.",
                             L"Discord Rich Presence",
                             MB_ICONINFORMATION );
                return;
            }

            const std::string condition = std::string( "%album%=" ) + albumStr;
            const std::wstring display = L"Album: " + qwr::unicode::ToWide( albumStr );

            QuickButtonDialog dlg( display, { condition } );
            dlg.DoModal( core_api::get_main_window() );
        }
    }
};

static contextmenu_item_factory_t<DiscordContextMenuItems> g_drpContextMenuItems;

} // namespace
