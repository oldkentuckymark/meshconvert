#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <algorithm>

#include "ffrmath.hpp"

constexpr auto Convert888to555(uint8_t const r, uint8_t const g, uint8_t const b) -> uint16_t
{
    return (((r >> 3) & 31) |
            (((g >> 3) & 31) << 5) |
            (((b >> 3) & 31) << 10) );

}

constexpr auto Convert555to888(uint16_t color) -> ffr::util::array<uint8_t, 4>
{
    uint8_t const red = (color & 31) << 3;
    uint8_t const green = ((color >> 5) & 31) << 3;
    uint8_t const blue = ((color >> 10) & 31) << 3;
    uint8_t const alpha = 255;
    return {red,green,blue,alpha};
}

auto read_file(const std::string& filename) -> std::vector<char>
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    auto fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}


template <typename T>
bool writeVectorToBinaryFile(const std::vector<T>& vec, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary);

    //don't write the size of the vector
    int32_t size = vec.size();
    //outFile.write(reinterpret_cast<const char*>(&size), sizeof(size));

    // Write the raw data
    outFile.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));

    outFile.close();
    return true;
}

struct dvertex
{
    double x,y,z,r,g,b,a;
};

int main(int argc, char *argv[])
{

    std::string infilestring{argv[1]};
    std::string outfileposstring{argv[1]};
    outfileposstring.replace(outfileposstring.size()-3,3,"pos");
    std::string outfilecolstring{argv[1]};
    outfilecolstring.replace(outfilecolstring.size()-3,3,"col");

    //read in csv file to string
    auto inbuff = read_file(argv[1]);
    inbuff.push_back(0);
    std::string instring = inbuff.data();

    //replace all commas with spaces for use with stream library
    std::replace(instring.begin(), instring.end(), ',', ' ');
    std::stringstream ss(instring);

    //extract values from stream into vector
    std::vector<double> dv;
    double d;
    while(ss >> d)
    {
        dv.push_back(d);
    }


    //convert raw doubles into vertex struct
    std::vector<dvertex> dvertvec;
    for(auto i = 0ul; i < dv.size(); i = i + 7)
    {
        dvertex* dvertp = reinterpret_cast<dvertex*>(&dv[i]);
        dvertvec.push_back(*dvertp);
    }


    //split vectors into 2, position, and color
    std::vector<ffr::math::fixed32> f32posvec;
    std::vector<uint16_t> uscolorvec;

    for(auto& i : dvertvec)
    {
        f32posvec.push_back( ffr::math::fixed32{static_cast<float>(i.x)} );
        f32posvec.push_back( ffr::math::fixed32{static_cast<float>(i.y)} );
        f32posvec.push_back( ffr::math::fixed32{static_cast<float>(i.z)} );
    }
    for(auto i = 0ul; i < dvertvec.size(); i = i + 3)
    {
        uint8_t r1 = static_cast<uint8_t>(dvertvec[i].r * 255.0);
        uint8_t g1 = static_cast<uint8_t>(dvertvec[i].g * 255.0);
        uint8_t b1 = static_cast<uint8_t>(dvertvec[i].b * 255.0);
        uscolorvec.push_back(Convert888to555(r1,g1,b1));
    }

    //write 2 files, one for positions, one for color

    writeVectorToBinaryFile(f32posvec, outfileposstring);
    writeVectorToBinaryFile(uscolorvec, outfilecolstring);

    return 0;
}
