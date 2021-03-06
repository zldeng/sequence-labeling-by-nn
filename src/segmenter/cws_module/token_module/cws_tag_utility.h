/**
 * tag_utility.h, including the utilities for tag system.
 */

#ifndef SLNN_SEGMENTER_CWS_MODULE_TOKEN_MODULE_TAG_UTILITY_H_
#define SLNN_SEGMENTER_CWS_MODULE_TOKEN_MODULE_TAG_UTILITY_H_
#include "cws_tag_definition.h"

namespace slnn{
namespace segmenter{
namespace token_module{



/****************************************************
 * Inline Implementation
 ****************************************************/
inline
Index select_best_tag_constrained(const std::vector<dynet::real> &dist, size_t time, Index pre_time_tag_id)
{
    dynet::real max_prob = std::numeric_limits<dynet::real>::lowest();
    Index tag_with_max_prob = Tag::TAG_NONE_ID;
    constexpr Index max_tag_id = TAG_SIZE - 1;
    for( Index tag_id = 0; tag_id <= max_tag_id; ++tag_id )
    {
        if( !can_emit(time, tag_id) ){ continue; }
        if( time > 0 && !can_trans_unsafe(pre_time_tag_id, tag_id) ){ continue; }
        if( dist[tag_id] >= max_prob )
        {
            tag_with_max_prob = tag_id;
            max_prob = dist[tag_id];
        }
    }
    // assert(tag_with_max_prob != Tag::TAG_NONE_ID);
    return tag_with_max_prob;
}

}
}
} // end of namespace slnn

#endif
