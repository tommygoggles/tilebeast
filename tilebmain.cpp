#include <fstream>
#include <assert.h>
#include "picopng.h"






bool savefile(const char* filename, char* filedata, unsigned int filelength)
{
    std::fstream outputfile;
    outputfile.open (filename, std::ios::out | std::ios::binary | std::ios::trunc);

    if (outputfile.is_open())
    {
        outputfile.seekg (0, std::ios::beg);
        outputfile.write( (const char*)filedata,filelength);
        outputfile.close();
        return true;
    }
    return false;
}




bool loadfile(const char* filename, char** filedata, unsigned int* filelength)
{
    if (filename)
    {
        if (filename[0] != '\0')
        {
            std::fstream inputfile;
            inputfile.open (filename, std::ios::in | std::ios::binary|std::ios::ate);//|ios::ate - go to end (to get size with size = file.tellg();)

            if (inputfile.is_open())
            {
                *filelength = inputfile.tellg();
                *filedata = new char [*filelength];
                inputfile.seekg (0, std::ios::beg);
                inputfile.read (*filedata, *filelength);
                inputfile.close();
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;//no filename
}




bool loadpng(const char* filename, unsigned long* widthheight, unsigned char** imagedata)
{
    unsigned char* filedata;
    unsigned int filelength;

    if(loadfile(filename,(char**)&filedata,&filelength))
    {
        std::vector<unsigned char> decompressedimagedata;
        decodePNG(decompressedimagedata, widthheight[0], widthheight[1], filedata, filelength, true);

        unsigned int bytesperpixel = 4;
        *imagedata = new unsigned char[widthheight[0] * widthheight[1] * bytesperpixel];

        //copy to image data and don't INVERT_Y

        for (unsigned int yval = 0;yval< (unsigned int)widthheight[1];yval++)
        {
            for (unsigned int xval = 0;xval<(unsigned int)widthheight[0];xval++)
            {
                for (unsigned int bytenumber = 0;bytenumber<bytesperpixel;bytenumber++)
                {
                    (*imagedata)[ (yval*widthheight[0]*bytesperpixel)+(xval*bytesperpixel)+bytenumber ] =
                    //decompressedimagedata[ ( ( (widthheight[1]-1)-yval)*( widthheight[0]*bytesperpixel)  ) + (xval*bytesperpixel) + bytenumber ];
                    decompressedimagedata[  ( (yval)*( widthheight[0]*bytesperpixel)  ) + (xval*bytesperpixel) + bytenumber ];
                }
            }
        }
        delete[] filedata;
        return true;
    }
    return false;
}









unsigned int* allchannels;
unsigned long widthheight[2];
unsigned char* theimage;

unsigned char tilexsize = 8;
unsigned char tileysize = 8;
unsigned int numoftiles;

class tile
{
    public:
    unsigned int* tilestart;

};



bool comparetiles(unsigned int* tile1start, unsigned int* tile2start)
{
    for(unsigned int pixy = 0;pixy<tileysize;pixy++)
    {
        for(unsigned int pixx = 0;pixx<tilexsize;pixx++)
        {
            if(tile1start[(pixy*widthheight[0])+pixx] !=  tile2start[(pixy*widthheight[0])+pixx])
            {
                return false;
            }
        }
    }
    return true;
}


unsigned int* gettilestart(unsigned int tilenum)
{
    assert(tilenum < numoftiles);
    unsigned int tileyval = tilenum / (widthheight[0] / tilexsize);
    unsigned int tilexval = tilenum - (tileyval* (widthheight[0] / tilexsize));

    return &(allchannels[(tileyval*widthheight[0]*tileysize)+tilexval*tilexsize]);
}


int main(int argc, char **argv)
{
    if(argc > 2)
    {

    }
    if(argc > 1)
    {
        if(loadpng(argv[1],widthheight,&theimage))
        {
            allchannels = (unsigned int*)theimage;
            assert(widthheight[0] % tilexsize == 0);
            assert(widthheight[1] % tileysize == 0);
            numoftiles = (widthheight[0] / tilexsize) * (widthheight[1] / tileysize);
            tile* alltiles = new tile[numoftiles];
            for(unsigned int i = 0;i<numoftiles;i++)
            {
                alltiles[i].tilestart = gettilestart(i);
            }


            unsigned int counttest = 0;
            for(unsigned int i = 0;i<numoftiles;i++)
            {
                if(comparetiles(alltiles[11].tilestart,alltiles[i].tilestart))
                {
                    counttest++;
                }
            }

            delete[] theimage;
        }
    }
    return 0;
}
