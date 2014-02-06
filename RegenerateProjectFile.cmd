@echo off
echo Checking out project and filters file
p4 edit BlueExposure.vcxproj
p4 edit BlueExposure.vcxproj.filters
p4 edit BlueExposure.v110_xp.vcxproj
p4 edit BlueExposure.v110_xp.vcxproj.filters
p4 edit BlueExposure.orbis.vcxproj
p4 edit BlueExposure.orbis.vcxproj.filters
echo Regenerating
..\..\..\..\..\..\shared_tools\python\27\python.exe ..\..\tools\ProjectFileGenerator\ProjectFileGenerator.py -i BlueExposure.ccpproj --toolset=v100
..\..\..\..\..\..\shared_tools\python\27\python.exe ..\..\tools\ProjectFileGenerator\ProjectFileGenerator.py -i BlueExposure.ccpproj --toolset=v110_xp
..\..\..\..\..\..\shared_tools\python\27\python.exe ..\..\tools\ProjectFileGenerator\ProjectFileGenerator.py -i BlueExposure.ccpproj --orbis
pause