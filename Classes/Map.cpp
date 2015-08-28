/****************************************************************************
 Copyright (c) 2015 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "Map.h"

// map 0
static const char* mapBuffer_0[5] =
{
    /*          1         2         3
      0123456789012345678901234567890123456789 */
     "CCC...CCC...CC...CCC...C...C..........",
     "C..C..C....C..C..C..C...C.C...........",
     "CCC...CCC..CCCC..C..C....C............",
     "C..C..C....C..C..C..C....C............",
     "C..C..CCC..C..C..CCC.....C............"
};
static const struct Map map_0 =
{
    &mapBuffer_0[0],
    {37,5},           // size of buffer
    {36,36}           // space between tiles
};

// map 1
static const char* mapBuffer_1[2] =
{
    /*         1         2         3
     0123456789012345678901234567890123456789 */
    "...Bb.....................",
    "..BbBb....Bb.....Bb.....Bb"
};
static const struct Map map_1 =
{
    &mapBuffer_1[0],
    {25,2},           // size of buffer
    {28,44}           // space between tiles
};

// all maps
static const Map* maps[] =
{
    &map_0,
    &map_1,
};
static const int TOTAL_MAPS = sizeof(maps)/sizeof(maps[0]);

const Map* getMap(int mapIndex)
{
    return &map_0;
}

const Map* getRandomMap()
{
    int idx = CCRANDOM_0_1() * TOTAL_MAPS;
    return maps[idx];
}