# thirdparty/

This folder is for vendored third-party libraries (git submodules or source copies).

Pattern:

- Add a submodule under `thirdparty/` (e.g. `thirdparty/SingleApplication`).
- The top-level build includes `thirdparty/CMakeLists.txt` which will automatically
  `add_subdirectory()` for any child that contains a `CMakeLists.txt` file.

Examples:

```bash
# Add the submodule and initialise it
git submodule add https://github.com/itay-grudev/SingleApplication.git thirdparty/SingleApplication
git submodule update --init --recursive
```

CMake will pick up the vendored copy and make its targets available to the rest
of the project so you can `target_link_libraries(... SingleApplication::SingleApplication)`.
