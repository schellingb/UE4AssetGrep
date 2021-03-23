UE4 Asset Grep
==============

# Overview
This is a simple tool to search Unreal Engine 4 project asset files for occurrences of names (i.e. class or method name).

## Download
You can find a Windows 64-bit build on the [Releases page](https://github.com/schellingb/UE4AssetGrep/releases/latest).

## Setup
In Visual Studio, open the `Tools` -> `External Tools...` window. Then add a new external tool and set the title to
"UE4 Asset Grep". Set the command to UE4AssetGrep.exe and set the arguments to the following (including the quotes):
```
"$(SolutionDir)Content" "$(CurText)"
```
Finally check the checkbox "Use Output window" and then press OK.

## Usage
With a UE4 project open in Visual Studio, select any text in the source code then click `Tools` -> `UE4 Asset Grep`.
The output window will automatically open and the assets will be searched for the selected string.

Be aware that blueprint assets refer to class names with the U or F prefix. So make sure to select only `MyActor` 
when searching for occurrences of a class names `UMyActor` (as well as just `MyStruct` instead of `FMyStruct`).

# License
UE4AssetGrep is available under the [Unlicense](http://unlicense.org/) (public domain).
