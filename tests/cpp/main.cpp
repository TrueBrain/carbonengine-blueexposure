#include <iostream>

int main(int argc, char** argv)
{
    // Need to initialize the Python interpreter, otherwise the string conversions will fail.
    Py_Initialize();
    if (Py_IsInitialized() == 0) {
        std::cerr << "Failed initializing Python interpreter" << std::endl;
        return -1;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
