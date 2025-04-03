#include <iostream>

int main(int argc, char** argv, char** envp)
{
	// Initialize a standard python interpreter
    PyConfig config;
    PyConfig_InitIsolatedConfig(&config);
    if(const auto status = PyConfig_Read( &config ); PyStatus_Exception( status ))
    {
	    PyErr_Print();
	    exit( -1 );
    }

	if( const auto status = Py_InitializeFromConfig( &config ); PyStatus_Exception( status ) )
	{
		PyErr_Print();
		exit( -1 );
	}

    if(!Py_IsInitialized())
    {
	    std::cerr << "Failed initializing Python interpreter\n";
	    return -1;
    }

	PyConfig_Clear( &config );

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
