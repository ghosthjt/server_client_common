#include <string>  

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);
inline bool is_base64(unsigned char c);

