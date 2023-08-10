#include <iostream>

int main(int argc, char** argv, char** envp)
{
    // We need to initialize the Python interpreter when running tests, otherwise calls to the Python C API will fail.
    // There are a few things that we need to take care of when initializing the interpreter for the tests. Primarily,
    // blueExposure - just like blue - doesn't have access to our `pythonInterpreter` files. Therefore, we need to
    // emulate quite a bit of logic that we'd otherwise get for free, specifically around constructing the `PYTHONPATH`.
    // First of all - and this is the easy bit - we add the location of the Python standard library inside the Perforce
    // branch to it. It's straight forward, and doesn't depend on any magic - just on the environment variable that is
    // already required to exist. We can then construct the path based on the variable and the well-known location.
    // The less obvious bit is adding "." to the `PYTHONPATH` as well. This is necessary for a few reasons.
    // The primary reason is that we cannot know the path to the Python runtime and built-in C extensions (it contains a
    // version number). By adding "." we leave it up to the caller to run the tests from a working directory that
    // includes these runtime files.
    PyConfig config;
    static wchar_t pythonPath[2048] = {};
    const char* envBranchPath = std::getenv("CCP_EVE_PERFORCE_BRANCH_PATH");
    if (!envBranchPath) {
        std::cerr << "Could not find `CCP_EVE_PERFORCE_BRANCH_PATH` environment variable, thus Python cannot be initialized correctly.\n";
        return -2;
    }
    swprintf(pythonPath, sizeof(pythonPath) / sizeof(*pythonPath), L"%s/carbon/common/stdlib", envBranchPath);

    PyConfig_InitIsolatedConfig(&config);
    PyWideStringList_Append(&config.module_search_paths, L".");
    PyWideStringList_Append(&config.module_search_paths, pythonPath);
    config.module_search_paths_set = 1;

    Py_InitializeFromConfig(&config);
    if (Py_IsInitialized() == 0) {
        std::cerr << "Failed initializing Python interpreter\n";
        return -1;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
