#ifndef SLNN_SEGMENTER_BASE_MODEL_BAREINPUT1_F2I_MODEL_0628_HPP_
#define SLNN_SEGMENTER_BASE_MODEL_BAREINPUT1_F2I_MODEL_0628_HPP_

#include <boost/archive/text_iarchive.hpp>
#include <boost/program_options.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/split_member.hpp>

#include "dynet/dynet.h"
#include "dynet/dict.h"
#include "input1_with_feature_model_0628.hpp"
#include "segmenter/cws_module/cws_feature_layer.h"
namespace slnn{

template<typename RNNDerived>
class CWSBareInput1F2IModel : public CWSInput1WithFeatureModel<RNNDerived>
{
    friend class boost::serialization::access;

public:
    CWSBareInput1F2IModel();
    virtual ~CWSBareInput1F2IModel();
    CWSBareInput1F2IModel(const CWSBareInput1F2IModel &) = delete;
    CWSBareInput1F2IModel& operator()(const CWSBareInput1F2IModel&) = delete;

    virtual void set_model_param_from_outer(const boost::program_options::variables_map &var_map) override;
    virtual void set_model_param_from_inner() override;
    
    virtual void build_model_structure() = 0 ;
    virtual void print_model_info() = 0 ;

    virtual dynet::expr::Expression  build_loss(dynet::ComputationGraph &cg,
        const IndexSeq &input_seq,
        const CWSFeatureDataSeq &feature_data_seq,
        const IndexSeq &gold_seq) override;
    virtual void predict(dynet::ComputationGraph &cg,
        const IndexSeq &input_seq,
        const CWSFeatureDataSeq &feature_data_seq,
        IndexSeq &pred_seq) override;

    template <typename Archive>
    void save(Archive &ar, const unsigned versoin) const; 
    template <typename Archive>
    void load(Archive &ar, const unsigned version);
    template <typename Archive>
    void serialize(Archive & ar, const unsigned version);

protected:
    
    Index2ExprLayer *word_expr_layer;
    CWSFeatureLayer *cws_feature_layer;
    BIRNNLayer<RNNDerived> *birnn_layer;
    BareOutputBase *output_layer;

public:
    unsigned word_embedding_dim,
        word_dict_size,
        nr_rnn_stacked_layer,
        rnn_x_dim,
        rnn_h_dim,
        softmax_layer_input_dim,
        output_dim ;

    dynet::real dropout_rate ; 

};


template<typename RNNDerived>
CWSBareInput1F2IModel<RNNDerived>::CWSBareInput1F2IModel() 
    :CWSInput1WithFeatureModel<RNNDerived>(),
    word_expr_layer(nullptr),
    cws_feature_layer(nullptr),
    birnn_layer(nullptr),
    output_layer(nullptr)
{}

template <typename RNNDerived>
CWSBareInput1F2IModel<RNNDerived>::~CWSBareInput1F2IModel()
{
    delete word_expr_layer;
    delete cws_feature_layer;
    delete birnn_layer;
    delete output_layer;
}

template <typename RNNDerived>
void CWSBareInput1F2IModel<RNNDerived>::set_model_param_from_outer(const boost::program_options::variables_map &var_map)
{
    unsigned replace_freq_threshold = var_map["replace_freq_threshold"].as<unsigned>();
    float replace_prob_threshold = var_map["replace_prob_threshold"].as<float>();
    this->set_replace_threshold(replace_freq_threshold, replace_prob_threshold);

    word_embedding_dim = var_map["word_embedding_dim"].as<unsigned>() ;
    nr_rnn_stacked_layer = var_map["nr_rnn_stacked_layer"].as<unsigned>() ;
    rnn_h_dim = var_map["rnn_h_dim"].as<unsigned>() ;

    dropout_rate = var_map["dropout_rate"].as<dynet::real>() ;

    unsigned start_here_embedding_dim = var_map["start_here_embedding_dim"].as<unsigned>();
    unsigned pass_here_embedding_dim = var_map["pass_here_embedding_dim"].as<unsigned>();
    unsigned end_here_embedding_dim = var_map["end_here_embedding_dim"].as<unsigned>();
    unsigned context_left_size = var_map["context_left_size"].as<unsigned>();
    unsigned context_right_size = var_map["context_right_size"].as<unsigned>();
    unsigned chartype_embedding_dim = var_map["chartype_embedding_dim"].as<unsigned>();
    this->cws_feature.set_feature_parameters(start_here_embedding_dim, pass_here_embedding_dim, end_here_embedding_dim,
        context_left_size, context_right_size, word_embedding_dim, chartype_embedding_dim);

    rnn_x_dim = word_embedding_dim + this->cws_feature.get_feature_dim();
    softmax_layer_input_dim = rnn_h_dim * 2;
}

template <typename RNNDerived>
void CWSBareInput1F2IModel<RNNDerived>::set_model_param_from_inner()
{
    assert(this->is_dict_frozen());
    word_dict_size = this->get_word_dict_size() ;
    output_dim = this->get_tag_dict_size() ;
}

template<typename RNNDerived>
dynet::expr::Expression CWSBareInput1F2IModel<RNNDerived>::build_loss(dynet::ComputationGraph &cg,
                                                                           const IndexSeq &input_seq, 
                                                                           const CWSFeatureDataSeq &cws_feature_seq,
                                                                           const IndexSeq &gold_seq) 
{
    word_expr_layer->new_graph(cg);
    cws_feature_layer->new_graph(cg);
    birnn_layer->new_graph(cg) ;
    output_layer->new_graph(cg) ;

    birnn_layer->set_dropout() ;
    birnn_layer->start_new_sequence() ;

    std::vector<dynet::expr::Expression> word_exprs;
    word_expr_layer->index_seq2expr_seq(input_seq, word_exprs);
    std::vector<dynet::expr::Expression> feature_exprs;
    cws_feature_layer->build_cws_feature(cws_feature_seq, feature_exprs);

    std::vector<dynet::expr::Expression> input_exprs ;
    StaticConcatenateLayer::concatenate_exprs(std::vector<std::vector<dynet::expr::Expression> *>(
    { &word_exprs, &feature_exprs }), input_exprs);

    std::vector<dynet::expr::Expression> l2r_exprs,
        r2l_exprs ;
    birnn_layer->build_graph(input_exprs, l2r_exprs, r2l_exprs) ;
    return output_layer->build_output_loss(std::vector<std::vector<dynet::expr::Expression> *>({ &l2r_exprs, &r2l_exprs}),
        gold_seq) ;
}

template<typename RNNDerived>
void CWSBareInput1F2IModel<RNNDerived>::predict(dynet::ComputationGraph &cg,
                                                      const IndexSeq &input_seq,
                                                      const CWSFeatureDataSeq &cws_feature_seq,
                                                      IndexSeq &pred_seq)
{
    word_expr_layer->new_graph(cg);
    cws_feature_layer->new_graph(cg);
    birnn_layer->new_graph(cg) ;
    output_layer->new_graph(cg) ;

    birnn_layer->disable_dropout() ;
    birnn_layer->start_new_sequence();

    std::vector<dynet::expr::Expression> word_exprs;
    word_expr_layer->index_seq2expr_seq(input_seq, word_exprs);
    std::vector<dynet::expr::Expression> feature_exprs;
    cws_feature_layer->build_cws_feature(cws_feature_seq, feature_exprs);

    std::vector<dynet::expr::Expression> input_exprs ;
    StaticConcatenateLayer::concatenate_exprs(std::vector<std::vector<dynet::expr::Expression> *>(
    { &word_exprs, &feature_exprs }), input_exprs);

    std::vector<dynet::expr::Expression> l2r_exprs,
        r2l_exprs ;
    birnn_layer->build_graph(input_exprs, l2r_exprs, r2l_exprs) ;
    output_layer->build_output(std::vector<std::vector<dynet::expr::Expression> *>({ &l2r_exprs, &r2l_exprs}),
        pred_seq) ;
}

template <typename RNNDerived>template< typename Archive>
void CWSBareInput1F2IModel<RNNDerived>::save(Archive &ar, const unsigned version) const
{
    ar & word_dict_size & word_embedding_dim
        & rnn_x_dim & rnn_h_dim & nr_rnn_stacked_layer
        & softmax_layer_input_dim & output_dim
        & dropout_rate ;
    ar & this->word_dict & this->cws_feature ;
    ar & *this->m ;
}

template <typename RNNDerived>template< typename Archive>
void CWSBareInput1F2IModel<RNNDerived>::load(Archive &ar, const unsigned version)
{
    ar & word_dict_size & word_embedding_dim
        & rnn_x_dim & rnn_h_dim & nr_rnn_stacked_layer
        & softmax_layer_input_dim & output_dim
        & dropout_rate ;
    ar & this->word_dict & this->cws_feature;
    assert(this->get_word_dict_size() == word_dict_size && this->get_tag_dict_size() == output_dim) ;
    build_model_structure() ;
    ar & *this->m ;
}

template <typename RNNDerived>
template<typename Archive>
void CWSBareInput1F2IModel<RNNDerived>::serialize(Archive & ar, const unsigned version)
{
    boost::serialization::split_member(ar, *this, version);
}

} // end of namespace slnn
#endif
