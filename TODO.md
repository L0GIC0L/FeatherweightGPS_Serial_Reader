# TODO.md

These are the objectives that must be accomplished before first release:

### Todo

- [ ] Research Imgui tiling and a backround window transparency
- [ ] Create a dedicated window to reconnect to GPS, set COM port, and set the log file location.
- [ ] Create a window to display the latest data in text form.
- [ ] Add theming.

### In Progress

- [ ] Create plots that properly display velocity and altitude.
- [ ] Create a plot to overlay over the map that displays GPS longitude and latitude.

### Done ✓

- [x] Connect to Featherweight over serial
- [X] Automate build workflow for Linux

- [X] Implement the native serial libraries instead of boost libraries.
  - [X] Fundamental research.

- [X] Implement a cmake file for easier cross-platform compilation.

- [X] Interface and display data on a GUI interface (iMgui?)
  - [X] Fundamental understanding of iMgui established.  

- [X] Sort and break up the recieved string.
  - [X] Serial connection established.
  - [X] Data is saved into a string.
  - [X] Data is stored in a "map" or some other equivalent.
  - [X] Store these function into a separate file for easy use in the GUI program.
- [X] Store parsed string data in auto-saving log file.  
