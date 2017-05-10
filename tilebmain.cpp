#include <fstream>
#include <assert.h>
#include "picopng.h"

#include <stdio.h>
#include <cmath>
#include <algorithm>



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

unsigned int xtiles;
unsigned int ytiles;
unsigned int numoftiles;



class tile;

class neighbour
{
    public:
    tile* thetile;
    unsigned int howmanyof;

    neighbour(tile* til = 0, unsigned int howm = 0): thetile(til), howmanyof(howm){};
};






class neighbours
{
    public:
    std::vector<neighbour> alltheneighbours;

    void addneighbour(tile* tiletoadd);

    void readneighbours();

};






class tile
{
    public:
    unsigned int* tilestart;
    tile* iamaninstanceof;

    unsigned int tilenum;
    unsigned int tileyval;
    unsigned int tilexval;

    std::vector<tile*> instancesofme;

    neighbours xneighbours[2];
    neighbours yneighbours[2];

    tile() :iamaninstanceof(0){}
};




void neighbours::addneighbour(tile* tiletoadd)
{
    if(tiletoadd)
    {
        tiletoadd = tiletoadd->iamaninstanceof;
    }

    for(unsigned int i = 0;i<alltheneighbours.size();i++)
    {
        if(alltheneighbours[i].thetile == tiletoadd)
        {
            alltheneighbours[i].howmanyof++;
            return;
        }
    }
    alltheneighbours.push_back(neighbour(tiletoadd,1));
}




void neighbours::readneighbours()
{
    for(unsigned int i = 0;i<alltheneighbours.size();i++)
    {
        printf ("neighbour %d, tile: %d, numberof %d \n", i, alltheneighbours[i].thetile, alltheneighbours[i].howmanyof);
    }
}




tile* alltiles;

tile* gettile(int x, int y)
{
    if(x >= 0 && x < (int)xtiles && y >= 0 && y < (int)ytiles)
    {
        return &(alltiles[(y*xtiles)+x]);
    }
    else
    {
        return 0;
    }
}

std::vector<tile*> uniquetiles;


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



unsigned int comparetilesdif(unsigned int* tile1start, unsigned int* tile2start)
{
    unsigned int diffs = 0;
    for(unsigned int pixy = 0;pixy<tileysize;pixy++)
    {
        for(unsigned int pixx = 0;pixx<tilexsize;pixx++)
        {
            if(tile1start[(pixy*widthheight[0])+pixx] !=  tile2start[(pixy*widthheight[0])+pixx])
            {
                diffs++;
            }
        }
    }
    return diffs;
}

unsigned int comparetilesdif(tile* tile1, tile* tile2)
{
    return comparetilesdif(tile1->tilestart,tile2->tilestart);
}







bool sorttilesbyuse (tile* i, tile* j)
{
    return (i->instancesofme.size() <  j->instancesofme.size());
}

void readtileinfo (tile* i)
{
    printf ("# %d Used %d times. \n", i->tilenum+1, i->instancesofme.size());
}


void findexactcopies()
{
    for(unsigned int currenttile = 0;currenttile<numoftiles;currenttile++)
    {
        if(alltiles[currenttile].iamaninstanceof == 0)
        {
            printf ("Doing tile: %d \n", currenttile);
            uniquetiles.push_back(&(alltiles[currenttile]));
            alltiles[currenttile].iamaninstanceof = &(alltiles[currenttile]);
            alltiles[currenttile].instancesofme.push_back(&(alltiles[currenttile]));
            for(unsigned int i = currenttile+1;i<numoftiles;i++)
            {
                if(alltiles[i].iamaninstanceof == 0)
                {
                    if(comparetiles(alltiles[currenttile].tilestart,alltiles[i].tilestart))
                    {
                        alltiles[i].iamaninstanceof = &(alltiles[currenttile]);
                        alltiles[currenttile].instancesofme.push_back(&(alltiles[i]));
                    }
                }
            }
        }
    }
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
            if(widthheight[0] % tilexsize != 0 || widthheight[1] % tileysize != 0)
            {
                printf ("Some pixels left over with image size: %d * %d, exiting. \n", (int)widthheight[0], (int)widthheight[1]);
                return 0;
            }
            xtiles = widthheight[0] / tilexsize;
            ytiles = widthheight[1] / tileysize;
            numoftiles = xtiles * ytiles;
            printf ("Xtiles: %d  Ytiles: %d Totaltiles: %d  \n", xtiles, ytiles, numoftiles);

            alltiles = new tile[numoftiles];
            for(unsigned int i = 0;i<numoftiles;i++)
            {
                alltiles[i].tilenum = i;
                alltiles[i].tileyval = i / (widthheight[0] / tilexsize);
                alltiles[i].tilexval = i - (alltiles[i].tileyval* (widthheight[0] / tilexsize));
                alltiles[i].tilestart = &(allchannels[(alltiles[i].tileyval*widthheight[0]*tileysize)+alltiles[i].tilexval*tilexsize]);
            }


            findexactcopies();



            printf ("Unique tiles found: %d \n", uniquetiles.size());
            printf ("Xtiles: %d  Ytiles: %d Totaltiles: %d  \n", xtiles, ytiles, numoftiles);

            unsigned int testy = sqrt(uniquetiles.size())+1;

            printf ("Could fit in a png %d * %d", testy*tilexsize, testy*tileysize);





            /*unsigned int radius = 20;

            for(unsigned int i = 0;i<uniquetiles.size();i++)
            {
                for(int x = -(int)radius;x<= (int)radius;x++)
                {
                    for(int y = -(int)radius;y<=(int)radius;y++)
                    {
                        if(x && y)//we don't wanna check the middle..
                        {
                            unsigned int matches = 0;
                            unsigned int all = uniquetiles[i]->instancesofme.size();
                            for(unsigned int j = 1;j<all;j++)
                            {
                                tile* tile1 = gettile(uniquetiles[i]->tilexval+x, uniquetiles[i]->tileyval+y);
                                tile* tile2 = gettile(uniquetiles[i]->instancesofme[j]->tilexval+x, uniquetiles[i]->instancesofme[j]->tileyval+y);
                                if(tile1 && tile2)
                                {
                                    if(tile1->iamaninstanceof == tile2->iamaninstanceof)
                                    {
                                        matches++;
                                    }
                                }
                                else
                                {
                                    if(!tile1 && !tile2)
                                    {
                                        //matches++;//no, don't actually care if they both have nothing offscreen
                                    }
                                }
                            }
                            //printf ("tile %d position x:%d y:%d matches %d / %d \n", uniquetiles[i]->tilenum, x, y, matches,all);
                            if(all > 1 && matches == all-1)
                            {
                                printf ("tile %d position x:%d y:%d matches %d / %d \n", uniquetiles[i]->tilenum, x, y, matches,all-1);
                                //printf ("matches for: %d \n", uniquetiles[i]->tilenum);
                            }
                        }

                    }
                }
            }*/


            for(unsigned int i = 0;i<uniquetiles.size();i++)
            {
                for(unsigned int j = i+1;j<uniquetiles.size();j++)
                {
                    if(comparetilesdif(uniquetiles[i],uniquetiles[j]) < 2)
                    {
                        printf ("%d is nearly the same as %d (1 pixel different...)\n", uniquetiles[i]-alltiles,uniquetiles[j]-alltiles);
                        //copy the pixels over - then reassess it all..
                    }
                }
            }



            for(unsigned int currenttile = 0;currenttile<numoftiles;currenttile++)
            {
                tile* above = gettile(alltiles[currenttile].tilexval, alltiles[currenttile].tileyval-1);
                tile* below = gettile(alltiles[currenttile].tilexval, alltiles[currenttile].tileyval+1);
                tile* left = gettile(alltiles[currenttile].tilexval-1, alltiles[currenttile].tileyval);
                tile* right = gettile(alltiles[currenttile].tilexval+1, alltiles[currenttile].tileyval);

                alltiles[currenttile].iamaninstanceof->yneighbours[0].addneighbour(above);
                alltiles[currenttile].iamaninstanceof->yneighbours[1].addneighbour(below);
                alltiles[currenttile].iamaninstanceof->xneighbours[0].addneighbour(left);
                alltiles[currenttile].iamaninstanceof->xneighbours[1].addneighbour(right);
            }




            for(unsigned int i = 0;i<uniquetiles.size();i++)
            {
                /*printf ("tile %d \n", i);
                uniquetiles[i]->yneighbours[0].readneighbours();
                uniquetiles[i]->yneighbours[1].readneighbours();
                uniquetiles[i]->xneighbours[0].readneighbours();
                uniquetiles[i]->xneighbours[1].readneighbours();
*/
            }




            //std::sort (uniquetiles.begin(), uniquetiles.end(), sorttilesbyuse);
            //for_each (uniquetiles.begin(), uniquetiles.end(), readtileinfo);



            //check for ones that are ONLY used with certain relation to other ones, within regionsize tiles away
            //put unique ones on a new image - make a var in tile for newimageposition
            //export to map / png

            //check for rotate/flipped copies?
            //check for different palette version?
            //check for similarity?


            delete[] theimage;
        }
    }
    return 0;
}
