//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "i_system.h"
#include "m_misc.h"
#include "m_argv.h"  // haleyjd 20110212: warning fix

int		myargc;
char**		myargv;




//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present
//

int M_CheckParmWithArgs(const char *check, int num_args)
{
    int i;

    for (i = 1; i < myargc - num_args; i++)
    {
	if (!strcasecmp(check, myargv[i]))
	    return i;
    }

    return 0;
}

//
// M_ParmExists
//
// Returns true if the given parameter exists in the program's command
// line arguments, false if not.
//

boolean M_ParmExists(const char *check)
{
    return M_CheckParm(check) != 0;
}

int M_CheckParm(const char *check)
{
    return M_CheckParmWithArgs(check, 0);
}

#define MAXARGVS        100

static void LoadResponseFile(int argv_index, const char *filename)
{
    FILE *handle;
    int size;
    char *infile;
    char *file;
    char **newargv;
    int newargc;
    int i, k;

    // Read the response file into memory
    handle = fopen(filename, "rb");

    if (handle == NULL)
    {
        printf ("\nNo such response file!");
        exit(1);
    }

    printf("Found response file %s!\n", filename);

    size = M_FileLength(handle);

    // Read in the entire file
    // Allocate one byte extra - this is in case there is an argument
    // at the end of the response file, in which case a '\0' will be
    // needed.

    file = malloc(size + 1);

    i = 0;

    while (i < size)
    {
        k = fread(file + i, 1, size - i, handle);

        if (k < 0)
        {
            I_Error("Failed to read full contents of '%s'", filename);
        }

        i += k;
    }

    fclose(handle);

    // Create new arguments list array

    newargv = malloc(sizeof(char *) * MAXARGVS);
    newargc = 0;
    memset(newargv, 0, sizeof(char *) * MAXARGVS);

    // Copy all the arguments in the list up to the response file

    for (i=0; i<argv_index; ++i)
    {
        newargv[i] = myargv[i];
        ++newargc;
    }

    infile = file;
    k = 0;

    while(k < size)
    {
        // Skip past space characters to the next argument

        while(k < size && isspace(infile[k]))
        {
            ++k;
        }

        if (k >= size)
        {
            break;
        }

        // If the next argument is enclosed in quote marks, treat
        // the contents as a single argument.  This allows long filenames
        // to be specified.

        if (infile[k] == '\"')
        {
            // Skip the first character(")
            ++k;

            newargv[newargc++] = &infile[k];

            // Read all characters between quotes

            while (k < size && infile[k] != '\"' && infile[k] != '\n')
            {
                ++k;
            }

            if (k >= size || infile[k] == '\n')
            {
                I_Error("Quotes unclosed in response file '%s'",
                        filename);
            }

            // Cut off the string at the closing quote

            infile[k] = '\0';
            ++k;
        }
        else
        {
            // Read in the next argument until a space is reached

            newargv[newargc++] = &infile[k];

            while(k < size && !isspace(infile[k]))
            {
                ++k;
            }

            // Cut off the end of the argument at the first space

            infile[k] = '\0';

            ++k;
        }
    }

    // Add arguments following the response file argument

    for (i=argv_index + 1; i<myargc; ++i)
    {
        newargv[newargc] = myargv[i];
        ++newargc;
    }

    myargv = newargv;
    myargc = newargc;

#if 0
    // Disabled - Vanilla Doom does not do this.
    // Display arguments

    printf("%d command-line args:\n", myargc);

    for (k=1; k<myargc; k++)
    {
        printf("'%s'\n", myargv[k]);
    }
#endif
}

//
// Find a Response File
//

void M_FindResponseFile(void)
{
    int i;

    for (i = 1; i < myargc; i++)
    {
        if (myargv[i][0] == '@')
        {
            LoadResponseFile(i, myargv[i] + 1);
        }
    }

    for (;;)
    {
        //!
        // @arg <filename>
        //
        // Load extra command line arguments from the given response file.
        // Arguments read from the file will be inserted into the command
        // line replacing this argument. A response file can also be loaded
        // using the abbreviated syntax '@filename.rsp'.
        //
        i = M_CheckParmWithArgs("-response", 1);
        if (i <= 0)
        {
            break;
        }
        // Replace the -response argument so that the next time through
        // the loop we'll ignore it. Since some parameters stop reading when
        // an argument beginning with a '-' is encountered, we keep something
        // that starts with a '-'.
        myargv[i] = "-_";
        LoadResponseFile(i + 1, myargv[i + 1]);
    }
}

// Return the name of the executable used to start the program:

const char *M_GetExecutableName(void)
{
    return M_BaseName(myargv[0]);
}

