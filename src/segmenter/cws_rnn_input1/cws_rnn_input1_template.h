#ifndef SLNN_SEGMENTER_CWS_RNN_INPUT1_TEMPLATE_H_
#define SLNN_SEGMENTER_CWS_RNN_INPUT1_TEMPLATE_H_
#include <random>
#include <boost/program_options/variables_map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "segmenter/cws_module/token_module/cws_tag_definition.h"
#include "dynet/expr.h"
namespace slnn{
namespace segmenter{
namespace rnn_input1{

template <typename TokenModuleT, typename StructureParamT, typename NnModuleT>
class SegmenterRnnInput1Template
{
public:
    // typename
    using AnnotatedDataProcessedT = typename TokenModuleT::AnnotatedDataProcessedT;
    using AnnotatedDataRawT = typename TokenModuleT::AnnotatedDataRawT;
    using UnannotatedDataProcessedT = typename TokenModuleT::UnannotatedDataProcessedT;
    using UnannotatedDataRawT = typename TokenModuleT::UnannotatedDataRawT;
    using NnExprT = typename NnModuleT::NnExprT;
    using NnValueT = typename NnModuleT::NnValueT;
public:
    const TokenModuleT* get_token_module() const { return &token_module; }
    TokenModuleT* get_token_module() { return &token_module; }
    const NnModuleT* get_nn() const { return &nn; }
    NnModuleT* get_nn() { return &nn; }
    const StructureParamT *get_param() const { return &param; }
    StructureParamT *get_param() { return &param; }
    std::mt19937* get_mt19937_rng() { static std::mt19937 rng(rng_seed); return &rng; }
    unsigned get_rng_seed() const { return rng_seed; }
private:
    explicit SegmenterRnnInput1Template(int argc, char **argv, unsigned seed) ;
public:
    SegmenterRnnInput1Template(const SegmenterRnnInput1Template&) = delete;
    SegmenterRnnInput1Template(SegmenterRnnInput1Template &&) = delete;
    SegmenterRnnInput1Template& operator=(const SegmenterRnnInput1Template&) = delete;
public:
    static std::shared_ptr<SegmenterRnnInput1Template> create_new_model(int argc, char**argv, unsigned seed);
    static void save_model(std::ostream &os, SegmenterRnnInput1Template &m);
    static std::shared_ptr<SegmenterRnnInput1Template> load_and_build_model(std::istream &is, int argc, char **argv);
    void save_model(std::ostream &os) { save_model(os, *this); }
public:
    void set_model_structure_param_from_outer(const boost::program_options::variables_map &args);
    void finish_read_training_data();
    void build_model_structure();
    NnExprT build_training_graph(const AnnotatedDataProcessedT& ann_processed_data);
    std::vector<Index> predict(const UnannotatedDataProcessedT& unann_processed_data);
private:
    TokenModuleT token_module;
    StructureParamT param;
    NnModuleT nn;
    unsigned  rng_seed;
};

/**************************************************
 * Inline Implemnentation
 **************************************************/

template <typename TokenModuleT, typename StructureParamT, typename NnModuleT>
SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>::
SegmenterRnnInput1Template(int argc, char **argv, unsigned seed)
    :token_module(seed),
    nn(argc, argv, seed),
    rng_seed(seed)
{}


template <typename TokenModuleT, typename StructureParamT, typename NnModuleT>
std::shared_ptr<SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>>
SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>::
create_new_model(int argc, char **argv, unsigned seed)
{
    return std::shared_ptr<SegmenterRnnInput1Template>(new SegmenterRnnInput1Template(argc, argv, seed));
}


template <typename TokenModuleT, typename StructureParamT, typename NnModuleT>
void 
SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>::
save_model(std::ostream &os, SegmenterRnnInput1Template &m)
{
    boost::archive::text_oarchive to(os);
    // 1. first save the seed.
    unsigned seed = m.get_rng_seed();
    to << seed; // boost can't directly serialize r-value ?
    // 2. save param
    to << *(m.get_param());
    // 3. save token module
    to << *(m.get_token_module());
    // 4. save nn
    //  - OH! DON't forget to reset the model to it's best state. (BUG FIX.)
    m.get_nn()->reset2stashed_model();
    to << *(m.get_nn());
}

template <typename TokenModuleT, typename StructureParamT, typename NnModuleT>
std::shared_ptr<SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>> 
SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>::
load_and_build_model(std::istream &is, int argc, char **argv)
{
    boost::archive::text_iarchive ti(is);
    // 1. read seed
    unsigned seed;
    ti >> seed;
    // 2. create model by the seed
    std::shared_ptr<SegmenterRnnInput1Template> m = create_new_model(argc, argv, seed);
    // 3. read structure param
    ti >> *m->get_param();
    // 4. read token module
    ti >> *m->get_token_module();
    //// 5. build structure
    m->build_model_structure();
    // 6. read nn
    ti >> *m->get_nn();
    return m;
}


template <typename TokenModuleT, typename StructureParamT, typename NnModuleT>
inline
void SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>::
set_model_structure_param_from_outer(const boost::program_options::variables_map &args)
{
    param.set_param_from_user_defined(args);
    token_module.set_param_before_process_training_data(param);
}


template <typename TokenModuleT, typename StructureParamT, typename NnModuleT>
inline
void SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>::
finish_read_training_data()
{
    token_module.finish_read_training_data();
    param.set_param_from_token_module(token_module);
}


template <typename TokenModuleT, typename StructureParamT, typename NnModuleT>
inline
void SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>::
build_model_structure()
{
    nn.build_model_structure(param);
    std::cerr << param.get_structure_info() << "\n";
}

template <typename TokenModuleT, typename StructureParamT, typename NnModuleT>
inline
typename SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>::NnExprT
SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>::
build_training_graph(const AnnotatedDataProcessedT& ann_processed_data)
{
    AnnotatedDataProcessedT data_after_unk_replace = 
        token_module.replace_low_freq_token2unk(ann_processed_data);
    return nn.build_training_graph(data_after_unk_replace);
}

template <typename TokenModuleT, typename StructureParamT, typename NnModuleT>
inline
std::vector<Index> SegmenterRnnInput1Template<TokenModuleT, StructureParamT, NnModuleT>::
predict(const UnannotatedDataProcessedT & unann_processed_data)
{
    return nn.predict(unann_processed_data);
}


} // enf of namespace rnn-input1
} // end of namespace segmenter
} // end of namespace slnn


#endif
