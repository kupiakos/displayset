
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef char bool;
#define true 1
#define false 0

void usage();
int list();
int best(save);
int current();
int set(long index, bool save);

int main(int argc, char* argv[])
{
	int error;
	int i;
	bool save;

	error = 0;
	save = false;
	
	// remove "save" from arguments given
	for (i=0; i < argc; i++)
	{
		if (strcmp(argv[i], "save") == 0)
		{
			int j;
			for (j=i; (j + 1) < argc; j++)
			{
				argv[j] = argv[j+1];
			}
			argc--;
			save = true;
		}
	}

	switch (argc)
	{
	case 1:
		best(save);
		break;

	case 2:
		if (strcmp(argv[1], "list") == 0 && !save)
			error = list();
		else if (strcmp(argv[1], "best") == 0)
			error = best(save);
		else if (strcmp(argv[1], "current") == 0 && !save)
			error = current();
		else
		{
			usage();
			if (strcmp(argv[1], "help") != 0 && strcmp(argv[1], "/?") != 0)
				error = 1;
		}
		break;

	case 3:
		if (strcmp(argv[1], "set") == 0)
		{
			long index;
			char* end;
			index = strtol(argv[2], &end, 10);
			if (*end == '\0')
			{
				error = set(index, save);
			}
			else
			{
				printf("Invalid index '%s'\n", argv[2]);
				error = 2;
			}
			break;
		}
	default:
		usage();
		error = 1;
	}
	return error;
}

int list()
{
	DEVMODE* mode;
	int i;

	mode = malloc(sizeof(DEVMODE));
	mode->dmSize = sizeof(DEVMODE);

	printf("Index  Mode\n-----  -------------------\n");

	for (i=0; EnumDisplaySettings(NULL, i, mode); i++)
	{
		printf("%-5d  %dx%dx%-2d @ %dHz\n", i, mode->dmPelsWidth, mode->dmPelsHeight, mode->dmBitsPerPel, mode->dmDisplayFrequency);
	}
	
	free(mode);

	return 0;
}

void usage()
{
	printf("Usage:\ndisplayset.exe [save] {best | list | current | help | set <index>}\nbest is the default.\n");
}

int set(long index, bool save)
{
	DWORD error;
	DEVMODE* mode;
	BOOL found;
	int i;

	i = 0;
	error = 0;
	found = FALSE;

	mode = malloc(sizeof(DEVMODE));
	mode->dmSize = sizeof(DEVMODE);

	for (i=0; !found && EnumDisplaySettings(NULL, i, mode); i++)
	{
		if (i == index)
			found = TRUE;
	}

	if (found)
	{
		printf("Setting displaymode to %dx%dx%-2d @ %dHz\n", mode->dmPelsWidth, mode->dmPelsHeight, mode->dmBitsPerPel, mode->dmDisplayFrequency);
		error = ChangeDisplaySettings(mode, 0);
		if (error)
		{
			fprintf(stderr, "Error changing display setting!\n");
		}
		else if (save)
		{
			printf("Saving displaymode\n");
			error = ChangeDisplaySettings(mode, CDS_UPDATEREGISTRY | CDS_RESET | CDS_GLOBAL);
			if (error)
				fprintf(stderr, "Error saving the display setting!\n");
		}
	}
	else
	{
		fprintf(stderr, "Unknown display index '%d'\n", index);
		error = 1;
	}

	free(mode);

	return error;
}

int best(bool save)
{
	DWORD error;
	DEVMODE* mode;
	int i;
	int best;
	int besti;

	error = 0;
	best = 0;
	besti = 0;

	mode = malloc(sizeof(DEVMODE));
	mode->dmSize = sizeof(DEVMODE);

	for (i=0; EnumDisplaySettings(NULL, i, mode); i++)
	{
		int rank;
		rank = mode->dmPelsWidth * mode->dmPelsHeight * mode->dmBitsPerPel + mode->dmDisplayFrequency;
		if (rank > best)
		{
			best = rank;
			besti = i;
		}

	}

	error = set(besti, save);

	free(mode);

	return error;
}

int current()
{
	DEVMODE* mode;

	mode = malloc(sizeof(DEVMODE));
	mode->dmSize = sizeof(DEVMODE);

	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, mode))
	{
		printf("%dx%dx%-2d @ %dHz\n", mode->dmPelsWidth, mode->dmPelsHeight, mode->dmBitsPerPel, mode->dmDisplayFrequency);
	}
	else
	{
		fprintf(stderr, "Could not retrieve the current display settings!\n");
	}

	free(mode);

	return 0;
}
