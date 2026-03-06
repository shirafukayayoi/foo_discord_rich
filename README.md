# Discord Rich Presence Integration

![Rich presence in status](/richpresence_status.png)  
![Rich Presence in status youtube](/richpresence_button.png)  
![Rich Presence in Profile local file](/richpresence.png)

This is a fork of foo_discord_rich, a component for the [foobar2000](https://www.foobar2000.org) audio player, which displays currently played track data via Discord Rich Presence.

## Fork Changes

- Elapsed time now uses Discord's new bar format
- Able to display "top" or "middle" line in status (adjustable configuration)
- **Interactive buttons**: Discord Rich Presence displays clickable buttons
  - **YouTube videos**: "Watch on YouTube" button links directly to the video
  - **Local files/other sources**: Button 1 is fixed to "Search on Spotify". Button 2 is fully customizable via rules (see below)

## Button Customization

### Preferences → Button Settings

Open foobar2000 **Preferences → Discord Rich Presence → Button Settings** to manage Button 2 rules.

Each rule has three fields:

| Field               | Description                                                                                                     |
| ------------------- | --------------------------------------------------------------------------------------------------------------- |
| **Match Condition** | `%tag%=value` format (e.g. `%artist%=YOASOBI`). Leave **empty** to match all tracks (default button).           |
| **Button Label**    | Text shown on the Discord button (max ~32 chars).                                                               |
| **URL Template**    | URL shown when the button is clicked. Supports `%artist%`, `%title%`, `%album%`, `%album artist%` placeholders. |

Rules are evaluated **top-to-bottom**; the first matching rule wins. If no rule matches, Button 2 is hidden.

**Example rules:**

| Match Condition         | Label            | URL                                           |
| ----------------------- | ---------------- | --------------------------------------------- |
| `%title%=夜に駆ける`    | 公式サイト       | `https://www.yoasobi-music.jp`                |
| `%artist%=YOASOBI`      | YOASOBI Official | `https://www.yoasobi-music.jp`                |
| _(empty — matches all)_ | Search on Tidal  | `https://tidal.com/search?q=%artist%+%title%` |

---

### Right-click: Set button for a track or album

You can set a custom button directly from the playlist without opening Preferences.

1. **Right-click** one or more tracks in the playlist.
2. Select **Discord Rich Presence** from the context menu.
3. Choose one of:
   - **Set button for this track...** — sets a button that appears only when that specific file is playing. When multiple tracks are selected, each track gets its own rule with the same label/URL.
   - **Set button for this album...** — sets a button that appears for every track on the same album (`%album%` tag).
4. Enter the **Button Label** and **URL** in the dialog, then click **OK**.
   - If a rule already exists for the target, the dialog pre-fills with the current values.
   - Click **Clear Rule** to remove the existing rule for that target.

Rules added via right-click appear at the **top** of the Button Settings list, so they take priority over any general/default rules.

> **Note:** Track-specific rules use the file path (`%path%`) as the identifier. If you move or rename the file, the rule will no longer match.

## Note

This has only been tested on 64-bit fb2k. A 32-bit version is available but it isn't tested.  
I'm neither competent at C++ nor writing fb2k components, I am just modifying their work.

Visit [the original homepage](https://theqwertiest.github.io/foo_discord_rich) for more info.
