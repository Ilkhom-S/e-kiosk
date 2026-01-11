# EKiosk Applications

This folder contains all application entry points for the EKiosk project.

- Each subfolder is a separate application (e.g., Kiosk, Updater, UpdaterSplashScreen, WatchService, WatchServiceController).
- Each app has its own CMakeLists.txt for building.

## Adding a New Application

1. Create a new subfolder under apps/.
2. Add your app source code and a CMakeLists.txt in that folder.
3. Add `add_subdirectory(<AppFolder>)` to apps/CMakeLists.txt.

## Building All Apps

From the project root, CMake will process apps/CMakeLists.txt and build all applications.

---

See the main project README and docs for more details.
