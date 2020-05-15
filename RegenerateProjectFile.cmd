@echo off
echo Checking out project and filters file
p4 edit BlueExposure_v141.vcxproj
p4 edit BlueExposure_v141.vcxproj.filters
p4 edit BlueExposureStatic_v141.vcxproj
p4 edit BlueExposureStatic_v141.vcxproj.filters
echo Regenerating
..\..\..\..\..\..\shared_tools\python\27\python.exe ..\..\tools\ProjectFileGenerator\ProjectFileGenerator.py -i BlueExposure.ccpproj --toolset=v141 --outfile=BlueExposure_v141.vcxproj
..\..\..\..\..\..\shared_tools\python\27\python.exe ..\..\tools\ProjectFileGenerator\ProjectFileGenerator.py -i BlueExposureStatic.ccpproj --toolset=v141 --outfile=BlueExposureStatic_v141.vcxproj
pause
