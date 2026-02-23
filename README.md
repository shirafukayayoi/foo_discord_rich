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
  - **Local files/other sources**: "Search on Spotify" and "Search on Apple Music" buttons for finding the track

## Note

This has only been tested on 64-bit fb2k. A 32-bit version is available but it isn't tested.  
I'm neither competent at C++ nor writing fb2k components, I am just modifying their work.

Visit [the original homepage](https://theqwertiest.github.io/foo_discord_rich) for more info.
