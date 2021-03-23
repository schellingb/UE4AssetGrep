//--------------------------------------------//
// UE4AssetGrep                               //
// License: Public Domain (www.unlicense.org) //
//--------------------------------------------//

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

enum { GREPS_MAX = 64 };
static struct { const unsigned char* Buf; size_t Len; } Greps[GREPS_MAX];
static size_t GrepCount, MaxGrepLen;

static wchar_t* PathCombine(wchar_t* out, const wchar_t* lpFolder, const wchar_t* lpFile)
{
	wsprintfW(out, L"%s\\%s", lpFolder, lpFile);
	return out;
}

static size_t GrepFile(const wchar_t* Path, size_t BasePathLen, LONGLONG FileSize)
{
	enum { BUFLEN = 512 * 1024, LEEWAY = 10 };
	unsigned char buf[BUFLEN];

	memset(buf, 0, LEEWAY + MaxGrepLen);
	FILE* f = _wfopen(Path, L"rb");
	for (size_t offset = 0, read = fread(buf + LEEWAY + MaxGrepLen, 1, BUFLEN - LEEWAY - MaxGrepLen, f); read; read = fread(buf + LEEWAY + MaxGrepLen, 1, BUFLEN - LEEWAY - MaxGrepLen, f), offset += BUFLEN - LEEWAY - MaxGrepLen)
	{
		for (size_t i = 0; i < GrepCount; i++)
		{
			for (const unsigned char *m = Greps[i].Buf, *p = buf + LEEWAY, *pEnd = buf + LEEWAY + Greps[i].Len + read; p != pEnd; p++)
			{
				if (*p != *m) { m = Greps[i].Buf; continue; }
				if (++m != Greps[i].Buf + Greps[i].Len) continue;
				_wprintf_p(L"%80s (Size: %8d) @ Offset: %8d >> ", Path + BasePathLen, (int)FileSize, (int)(offset + (p - buf) - Greps[i].Len - Greps[i].Len + 1));
				for (const unsigned char *o = p - Greps[i].Len - LEEWAY; o != p + 1 + LEEWAY; o++)
					printf("%s%c%s", (o == p - Greps[i].Len + 1 ? "<" : ""), ((*o < ' ' || *o>127) ? '_' : *o), (o == p ? ">" : ""));
				printf("\n");
				fflush(stdout);
				m = Greps[i].Buf;
			}
		}
		memcpy(buf, buf + BUFLEN - LEEWAY - MaxGrepLen, LEEWAY + MaxGrepLen);
	}
	fclose(f);
	return (size_t)-1;
}

static void GrepFilesRecursively(const wchar_t* lpFolder, const wchar_t* lpFilePattern, size_t BasePathLen = (size_t)-1)
{
	wchar_t szFullPattern[MAX_PATH];
	WIN32_FIND_DATAW FindFileData;
	if (BasePathLen == (size_t)-1) BasePathLen = wcslen(lpFolder);

	for (HANDLE hFindFile = FindFirstFileW(PathCombine(szFullPattern, lpFolder, L"*"), &FindFileData); hFindFile != INVALID_HANDLE_VALUE; hFindFile = (FindNextFileW(hFindFile, &FindFileData) ? hFindFile : (FindClose(hFindFile), INVALID_HANDLE_VALUE)))
	{
		if (FindFileData.cFileName[0] == '.') continue;
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
		GrepFilesRecursively(PathCombine(szFullPattern, lpFolder, FindFileData.cFileName), lpFilePattern, BasePathLen);
	}

	for (HANDLE hFindFile = FindFirstFileW(PathCombine(szFullPattern, lpFolder, lpFilePattern), &FindFileData); hFindFile != INVALID_HANDLE_VALUE; hFindFile = (FindNextFileW(hFindFile, &FindFileData) ? hFindFile : (FindClose(hFindFile), INVALID_HANDLE_VALUE)))
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
		LONGLONG FileSize = ((((LONGLONG)(FindFileData.nFileSizeHigh)) << (sizeof(FindFileData.nFileSizeLow) * 8)) | (LONGLONG)FindFileData.nFileSizeLow);
		size_t MatchIndex = GrepFile(PathCombine(szFullPattern, lpFolder, FindFileData.cFileName), BasePathLen, FileSize);
	}
}

int main(size_t argc, const char** argv)
{
	if (argc < 3 || !argv[1][0] || !argv[2][0])
	{
		Explanation:
		fprintf(stderr, "Start with: %s <ContentDirectory> <SearchText>\n\n", (argc ? argv[0] : ""));
		fprintf(stderr, "Use in Visual Studio as External Tool with Arguments '\"$(SolutionDir)Content\" \"$(CurText)\"'\n\n");
		return 1;
	}
	if (strlen(argv[2]) > 255)
	{
		fprintf(stderr, "Search string with %d characters is longer than the maximum allowed 255 characters\n\n", (int)strlen(argv[2]));
		return 1;
	}

	wchar_t wargv2[256];
	wargv2[MultiByteToWideChar(CP_ACP, 0, argv[2], (int)strlen(argv[2]), wargv2, 256)] = 0;

	GrepCount = 0;
	for (const char *pStart = argv[2], *p = pStart+1;;p++)
	{
		if (*p != '\0' && *p != '\r' && *p != '\n' && *p != '|') continue;
		Greps[GrepCount].Len = p - pStart;
		if (Greps[GrepCount].Len)
		{
			if (Greps[GrepCount].Len > MaxGrepLen) MaxGrepLen = Greps[GrepCount].Len;
			Greps[GrepCount].Buf = (const unsigned char*)pStart;
			printf("Searching for [%.*s] in %s ...\n", (int)Greps[GrepCount].Len, pStart, argv[1]);
			GrepCount++;
			Greps[GrepCount].Len = Greps[GrepCount - 1].Len * 2;
			Greps[GrepCount].Buf = (const unsigned char*)(wargv2 + (pStart - argv[2]));
			if (++GrepCount == GREPS_MAX) break;
		}
		if (*p == '\0') break;
		pStart = p + 1;
	}
	if (!GrepCount) goto Explanation;

	printf("\n");
	fflush(stdout);
	wchar_t szDirectory[MAX_PATH];
	szDirectory[MultiByteToWideChar(CP_ACP, 0, argv[1], (int)strlen(argv[1]), szDirectory, MAX_PATH)] = 0;
	GrepFilesRecursively(szDirectory, L"*.u*"); // limit to *.u files (uasset, umap, ...)
	printf("\nFinished search\n");

	//printf(" - Press enter to quit\n");
	//fgetc(stdin);
	return 0;
}
