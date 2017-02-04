#include <windows.h>
#include <io.h>

#include "..\inc\respack.h"

char file_list[MAX_FILE_ENTRIES][MAX_FILE_LENGTH];
int entries = 0;

_fsize_t get_file_size(const char* file_name)
{
	struct _finddata_t fdata;
	long hFile;

	if ((hFile = _findfirst(file_name, &fdata)) == -1)
		return -1;

	_findclose(hFile);

	return fdata.size;
}

int load_file_list(FILE* f_list)
{
	char entry[MAX_FILE_LENGTH];
	char* ret;

	if (f_list == NULL)
		return -1;

	do
	{
		ret = fgets(entry, MAX_FILE_LENGTH, f_list);
		
		if (ret != NULL)
		{
			entries++;
			entry[strlen(entry)-1] = '\0';
			strcpy(file_list[entries-1], (const char*)entry);
		}

	} while(ret != NULL);

	return 0;
}

int create_archive(const char* list_file, const char* pak_file)
{
	FILE *f_list = NULL, *f_pak = NULL, *f_entry = NULL;
	int i, j, c; unsigned long lOffset;

	if ((f_list = fopen(list_file, "r")) == NULL)
	{
		printf("Error: Liste Datei konnte nicht geoeffnet werden!\n");
		return -1;
	}

	if ((f_pak = fopen(pak_file, "wb")) == NULL)
	{
		printf("Error: Pak Datei konnte nicht erstellt werden!\n");
		fclose(f_list);
		return -1;
	}

	if (load_file_list(f_list) == -1)
	{
		printf("Error: load_file_list() == -1\n");
		return -1;
	}

	//---

	// PAK
	fputc('P',f_pak);
	fputc('A',f_pak);
	fputc('K',f_pak);

	// Schreibe Anzahl der Dateien
	fputc(entries,f_pak);

	for (i = 0; i != entries; i++)
	{
		printf("\nAdd: %s", file_list[i]);

		// Schreibe Größe des Dateinamens
		fputc(strlen(file_list[i]),f_pak);

		// Schreibe Dateinamen
		for (j = 0; j != (int)strlen(file_list[i]); j++)
			fputc(file_list[i][j],f_pak);

		
		// Ermittle Größe der Datei
		_fsize_t fsize;
		if ((fsize = get_file_size((const char*)file_list[i])) == -1)
		{
			printf("...FAILED\n\nError: Fehler beim Ermitteln der Dateigroesse der Datei %s\n",file_list[i]);
			fclose(f_list);
			fclose(f_pak);
			return -1;
		}

		// Schreibe Größe der Datei
		if (fwrite(&fsize, sizeof(_fsize_t), 1, f_pak) != 1)
		{
			printf("...FAILED\n\nError: Fehler beim Schreiben der Dateigroesse der Datei %s\n",file_list[i]);
			fclose(f_list);
			fclose(f_pak);
			return -1;
		}

		// Öffne Entry Datei
		if ((f_entry = fopen(file_list[i], "rb")) == NULL)
		{
			printf("...FAILED\n\nError: Fehler beim Oeffnen der Datei %s!\n",file_list[i]);
			fclose(f_list);
			fclose(f_pak);
			return -1;
		}

		// Schreibe Inhalt der Datei
		for (lOffset = 0; lOffset != fsize; lOffset++)
		{
			c = fgetc(f_entry);
			fputc(c,f_pak);
		}

		fclose(f_entry);

		printf("...OK");
	}

	//---

	fclose(f_list);
	fclose(f_pak);

	printf("\n\nZelda Archive successfully created !!!\n");
	fflush(stdin); getchar();

	return 0;
}

int extract_file(const char* pak_file, const char* ex_file, const char* file_out)
{
	FILE *f_pak = NULL, *f_file = NULL;
	int c, i, j, n_files = 0;
	char file_name[MAX_FILE_LENGTH];
	_fsize_t fsize;
	unsigned char pData[1024];
	unsigned long readBytes, readSize; 

	printf("\nExtract: %s",ex_file);

	if ((f_pak = fopen(pak_file, "rb")) == NULL)
	{
		printf("...FAILED\n\nError: Archiv Datei %s konnte nich geoeffnet werden!\n", pak_file);
		return -1;
	}

	if ((f_file = fopen(file_out, "wb")) == NULL)
	{
		printf("...FAILED\n\nError: Datei %s konnte nicht erstellt werden!\n", file_out);
		fclose(f_pak);
		return -1;
	}

	c = fgetc(f_pak); // P
	c = fgetc(f_pak); // A
	c = fgetc(f_pak); // K

	// Lese Anzahl der Dateien
	n_files = fgetc(f_pak);

	for (i = 0; i != n_files; i++)
	{
		// Lese Größe des Dateinamens
		int n_file_name = fgetc(f_pak);

		// Lese Dateinamen
		for (j = 0; j != n_file_name; j++)
			file_name[j] = fgetc(f_pak);
		file_name[j] = '\0';

		if (strcmp(file_name, ex_file) == 0)
		{
			// Gefunden
			
			// Lese Größe der Datei
			if ((fread(&fsize, sizeof(fsize), 1, f_pak)) != 1)
			{
				printf("...FAILED\n\nError: Fehler beim Lesen der Dateigroesse!\n");
				fclose(f_pak);
				fclose(f_file);
				DeleteFile(file_out);
				return -1;
			}

			// Lese Dateiinhalt und schreibe es in f_file
			while (fsize > 0)
			{
				if (fsize >= 1024) 
					readSize = 1024;
				else
					readSize = fsize;

				readBytes = fread(pData,sizeof(unsigned char),readSize,f_pak);
				fwrite(pData,sizeof(unsigned char),readBytes,f_file);

				fsize -= readBytes;
			}

			fclose(f_pak);
			fclose(f_file);

			printf("...OK\n\nFile successfully extracted !!!\n");
			fflush(stdin); getchar();

			return 0;
		}

		// Lese Größe der Datei
		if ((fread(&fsize, sizeof(fsize), 1, f_pak)) != 1)
		{
			printf("...FAILED\n\nError: Fehler beim Lesen der Dateigroesse!\n");
			fclose(f_pak);
			fclose(f_file);
			DeleteFile(file_out);
			return -1;
		}

		// Lese Dateiinhalt
		fseek(f_pak,fsize,SEEK_CUR);
	}

	fclose(f_pak);
	fclose(f_file);

	printf("...FAILED\n\nFile does not exist in Zelda Archive !!!\n");
	DeleteFile(file_out);
	fflush(stdin); getchar();

	return -1;
}
