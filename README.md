# NT-VER

## D-D-D-Deprecated

Undoubtedly, most people have seen this message during a build:
```
warning C4996: 'GetVersionExA': was declared deprecated
```
The information returned by that API is stored in the Process Environment block, or PEB, created for each process. Deprecating the API that accesses that data *may* be a first step towards removing the data from the PEB. Some have taken to using the RtlGetVersion API, which uses the same structure as GetVersionEx. A quick check here shows that the rtl API has apparently been removed, at least on my system: version 2004, build 19041.450, so it doesn't seem to be a viable alternative for getting version information.

The [GetVersionEx](https://docs.microsoft.com/en-us/windows/win32/API/sysinfoapi/nf-sysinfoapi-getversionexa) API description speaks to the state of the API. The remarks section says, in part:

>"Identifying the current operating system is usually not the best way to determine whether a particular operating system feature is present."

I agree, but there are other valid reasons that one might want to identify the current operating system, including simple curiosity, so the issue remains: how to get version information. Trying to use VERSIONINFO from one of the system DLLs seems problematic as they often have different information and none typically agree with what the winver utility report or what the Settings|System|About dialog shows.

After a bit of looking, the information appears in the registry under the key:
```
HKLM\Software\Microsoft\Windows NT\CurrentVersion

  CurrentMajorVersionNumber  REG_DWORD = 10
  CurrentMinorVersionNumber  REG_DWORD = 0
  CurrentBuildNumber         REG_SZ    = 19041
  UBR                        REG_DWORD = 450
  ReleaseId                  REG_SZ    = 2004
  BuildLab                   REG_SZ    = 19041.vb_release.191206-1406
  CurrentVersion             REG_SZ    = 6.3
  SystemRoot                 REG_SZ    = C:\WINDOWS
```
Using that data, the os version as presented by cmd would be:
```
Microsoft Windows [Version 10.0.19041.450]
Microsoft Windows [Version $major.$minor.$build.$ubr]
```
And the winver utility would be:
```
Version 2004 (OS Build 19041.450)
Version $release (OS Build $build.$ubr)
```
If you want os version information, this seems to be the place to get it. The code here retrieves (some) of the data, populating a structure similar to the OSVERSIONINFO structure, but not identical to it.

Since the data is in the registry, the win32 registry API's are the logical choice and a function is provided that uses them. for those who simply must use semi-documented bits, another function is included that makes use of the RtlQueryRegistryValues API.

## Build
Open a "vc tools" command prompt, either 32-bit or 64-bit, change to the directory containing the dsw.c file and then:
```
# cl -W4 v0.c -link ntdll.lib advapi32.lib
```

## Files
The following files are included:
```
NT-VER
|   v0.c                        source.
\   README.md                   this.
```
That is all.
```
  SR  15,15
  BR  14
```
