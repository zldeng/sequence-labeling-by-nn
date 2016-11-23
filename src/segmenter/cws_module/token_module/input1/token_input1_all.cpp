#include "token_input1_all.h"

namespace slnn{
namespace segmenter{
namespace token_module{

std::u32string TokenSegmenterInput1All::EOS_REPR = U"<EOS>";

TokenSegmenterInput1All::TokenSegmenterInput1All(unsigned seed) noexcept
    :unigram_dict(seed),
    bigram_dict(seed),
    lexicon_feat(),
    chartype_feat(),
    state()
{}

std::string TokenSegmenterInput1All::get_module_info() const noexcept
{
    return "";
}

} // end of namespace token_module
} // end of namespace segmenter
} // end of namespace slnn