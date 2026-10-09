1. **Remove Artifacts**: Done.
2. **GPU Monitoring**: Add DXGI-based GPU usage tracking to `PerformanceMonitor`.
3. **Benchmark Mode**: Add a `BenchmarkManager` class and a UI tab to run benchmarks and compare profiles.
4. **Tests**: Add basic C++ catch2-style or simple assert-based tests for `RuleEngine`, `DataManager`, and `StateRestorer` in a `tests/` directory and compile them natively.
5. **Windows Startup**: Add a toggle in UI and logic using the Windows Registry (`HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run`) to start with Windows.
