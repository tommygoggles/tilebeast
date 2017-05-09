#ifndef pico_INCLUDED
#define pico_INCLUDED

#include <vector>

#ifndef size_t
#define size_t std::size_t
#endif

int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32);


#endif
