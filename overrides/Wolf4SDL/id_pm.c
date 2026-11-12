/* Modified for RAD-Wolf, 2026-11-12. See docs/CHANGES.md; original notices retained. */
// ID_PM.C

#include "wl_def.h"
#include "radwolf_stdio_file.h" // RADWOLF_PM_FILES

word ChunksInFile;
word PMSpriteStart;
word PMSoundStart;

bool PMSoundInfoPagePadded = false;

word *pageLengths;

byte *PMPageData;
byte **PMPages;


/*
==================
=
= PM_Startup
=
==================
*/

void PM_Startup (void)
{
    int      i;
    int      padding;
    byte     *page;
    uint32_t *pageOffsets;
    uint32_t pagesize;
    int32_t  filesize,datasize;
    radwolf_file file = {0};
    char     fname[13];

    snprintf (fname,sizeof(fname),"vswap.%s",extension);

    int open_result = radwolf_stdio_open(&file, fname);

    if (open_result != 0)
        CA_CannotOpen(fname);

    //
    // read in header variables
    //
    if (radwolf_file_read_exact(&file, &ChunksInFile, sizeof(ChunksInFile)) != 0) Quit("VSWAP header read failed");
    if (radwolf_file_read_exact(&file, &PMSpriteStart, sizeof(PMSpriteStart)) != 0) Quit("VSWAP header read failed");
    if (radwolf_file_read_exact(&file, &PMSoundStart, sizeof(PMSoundStart)) != 0) Quit("VSWAP header read failed");

    //
    // read in the chunk offsets
    //
    pageOffsets = SafeMalloc((ChunksInFile + 1) * sizeof(*pageOffsets));

    if (radwolf_file_read_exact(&file, pageOffsets, (size_t)ChunksInFile * sizeof(*pageOffsets)) != 0) Quit("VSWAP offset read failed");

    //
    // read in the chunk lengths
    //
    pageLengths = SafeMalloc(ChunksInFile * sizeof(*pageLengths));

    if (radwolf_file_read_exact(&file, pageLengths, (size_t)ChunksInFile * sizeof(*pageLengths)) != 0) Quit("VSWAP length read failed");

    uint32_t file_bytes;
    if (radwolf_file_size(&file, &file_bytes) != 0 || file_bytes > INT32_MAX) Quit("VSWAP size failed");
    filesize = (int32_t)file_bytes;
    datasize = filesize - pageOffsets[0];

    if (datasize < 0)
        Quit ("PM_Startup: The page file \"%s\" is too large!",fname);

    pageOffsets[ChunksInFile] = filesize;

    //
    // check that all chunk offsets are valid
    //
    for (i = 0; i < ChunksInFile; i++)
    {
        if (!pageOffsets[i])
            continue;           // sparse page

        if (pageOffsets[i] < pageOffsets[0] || pageOffsets[i] >= (size_t)filesize)
            Quit ("PM_Startup: Illegal page offset for page %i: %u (filesize: %u)",i,pageOffsets[i],filesize);
    }

    //
    // calculate total amount of padding needed for sprites and sound info page
    //
    padding = 0;

    for (i = PMSpriteStart; i < PMSoundStart; i++)
    {
        if (!pageOffsets[i])
            continue;           // sparse page

        if (((pageOffsets[i] - pageOffsets[0]) + padding) & 1)
            padding++;
    }

    if (((pageOffsets[ChunksInFile - 1] - pageOffsets[0]) + padding) & 1)
        padding++;

    //
    // allocate enough memory to hold the whole page file
    //
    PMPageData = SafeMalloc(datasize + padding);

    //
    // [ChunksInFile + 1] pointers to page starts
    // the last pointer points one byte after the last page
    //
    PMPages = SafeMalloc((ChunksInFile + 1) * sizeof(*PMPages));

    //
    // load pages and initialize PMPages pointers
    //
    page = &PMPageData[0];

    for (i = 0; i < ChunksInFile; i++)
    {
        if ((i >= PMSpriteStart && i < PMSoundStart) || i == ChunksInFile - 1)
        {
            //
            // pad with zeros to make it 2-byte aligned
            //
            if ((page - &PMPageData[0]) & 1)
            {
                *page++ = 0;

                if (i == ChunksInFile - 1)
                    PMSoundInfoPagePadded = true;
            }
        }

        PMPages[i] = page;

        if (!pageOffsets[i])
            continue;               // sparse page

        //
        // use specified page length when next page is sparse
        // otherwise, calculate size from the offset difference between this and the next page
        //
        if (!pageOffsets[i + 1])
            pagesize = pageLengths[i];
        else
            pagesize = pageOffsets[i + 1] - pageOffsets[i];

        if (radwolf_file_seek(&file, pageOffsets[i]) != 0) Quit("VSWAP seek failed");
        if (radwolf_file_read_exact(&file, page, pagesize) != 0) Quit("VSWAP page read failed");

        page += pagesize;
    }

    //
    // last page points after page buffer
    //
    PMPages[ChunksInFile] = page;

    free (pageOffsets);

    if (radwolf_file_close(&file) != 0) Quit("VSWAP close failed");
}


/*
==================
=
= PM_Shutdown
=
==================
*/

void PM_Shutdown (void)
{
    free (pageLengths);
    free (PMPages);
    free (PMPageData);

    pageLengths = NULL;
    PMPages = NULL;
    PMPageData = NULL;
}


/*
==================
=
= PM_GetPageSize
=
==================
*/

uint32_t PM_GetPageSize (int page)
{
    if (page < 0 || page >= ChunksInFile)
        Quit ("PM_GetPageSize: Invalid page request: %i",page);

    return (uint32_t)(PMPages[page + 1] - PMPages[page]);
}


/*
==================
=
= PM_GetPage
=
= Returns the address of the page
=
==================
*/

byte *PM_GetPage (int page)
{
    if (page < 0 || page >= ChunksInFile)
        Quit ("PM_GetPage: Invalid page request: %i",page);

    return PMPages[page];
}


/*
==================
=
= PM_GetPageEnd
=
= Returns the address of the last page
=
==================
*/

byte *PM_GetPageEnd (void)
{
    return PMPages[ChunksInFile];
}
