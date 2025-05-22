# Data Storage

Gamera keeps its persistent data under `$HOME/.gamera`.

- `history` – visited URLs stored by the 9P server.
- `bookmarks` – user bookmark entries written via the 9P `bookmarks` file.

These files are loaded when the browser starts and updated whenever
new history or bookmark entries are recorded.
