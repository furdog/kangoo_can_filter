# Kangoo can filter

This is repository contains esp32(arduino) version of ElektroBOX can filtering software.

<img width="541" height="794" alt="web interface illustration (ukrainian version)" src="https://github.com/user-attachments/assets/33a05f58-c780-4d94-a413-8594531f1db0" />

The project is not properly documented at the moment. May change in the future.

## CHANGELOG
> v2.13.0

- Added proper CHANGELOG and implementationg notes and TODO.
- Scaled percentage ratios 100x inside input fields for user сonvenience.
- Remaining capacity doesn't need manual confirmation button now (caused confusion).
- Added hint dialogs, very crucial for new users.

## TODO
> Since 07.09.2026 (short term tasks)
- Language selection badge in the top right corner of screen, similar to hint tools.
- Automatic language selection && language saved between sessions.
- CAN FILTER Hardware revision selection option. So there are no longer hardware variants necessary.

> Since 07.09.2026 (long term tasks)
- Automated hardware revision selection
- Service mode to manually edit hardware layout (pinout && features)
- Refactor. Make CAN FILTER fully hardware agnostic and compliant with MISRA requirements
- Provide automated tests
- Generic CAN FILTER interface for more cars and not only kangoo (unify projects)
- Protobuf for websocket communication and dynamic web interface generation.
- Migrate to ESP-IDF or other platforms

## Implementation notes
> 07.09.2026

From this moment i start to document all the implementation details and further notes.
I will add additional sections as needed to explain architectural decisions, etc.

For now i'll try to write changelog and todos as the first step to proper documentation.