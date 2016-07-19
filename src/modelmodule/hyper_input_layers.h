#ifndef MODELMODULE_HYPER_INPUT_LAYERS_H_
#define MODELMODULE_HYPER_INPUT_LAYERS_H_

#include <initializer_list>
#include "layers.h"
#include "utils/typedeclaration.h"

namespace slnn{

struct Input1
{
    cnn::LookupParameters *word_lookup_param;
    cnn::ComputationGraph *pcg;
    Input1(cnn::Model *m, unsigned vocab_size, unsigned embedding_dim);
    ~Input1();
    cnn::LookupParameters *get_lookup_param(){ return word_lookup_param; };
    void new_graph(cnn::ComputationGraph &cg);
    void build_inputs(const IndexSeq &sent , std::vector<cnn::expr::Expression> &input_exprs);
};

struct Input1WithFeature
{
    cnn::LookupParameters *word_lookup_param;
    Merge2Layer m2_layer;
    cnn::ComputationGraph *pcg;
    NonLinearFunc *nonlinear_func;
    Input1WithFeature(cnn::Model *m, unsigned vocab_size, unsigned embedding_dim,
        unsigned feature_embedding_dim, unsigned merge_out_dim,
        NonLinearFunc *nonlinear_func=&cnn::expr::rectify);
    cnn::LookupParameters *get_lookup_param(){ return word_lookup_param; }
    void new_graph(cnn::ComputationGraph &cg);
    void build_inputs(const IndexSeq &sent , const std::vector<cnn::expr::Expression> &feature_exprs,
        std::vector<cnn::expr::Expression> &input_exprs);
};

struct Input2D
{
    cnn::LookupParameters *dynamic_lookup_param1,
        *dynamic_lookup_param2;
    Merge2Layer m2_layer;
    cnn::ComputationGraph *pcg;
    NonLinearFunc *nonlinear_func;
    Input2D(cnn::Model *m, unsigned vocab_size1, unsigned embedding_dim1,
        unsigned vocab_size2, unsigned embedding_dim2,
        unsigned mergeout_dim ,
        NonLinearFunc *nonlinear_func=&cnn::expr::rectify);
    ~Input2D();
    void new_graph(cnn::ComputationGraph &cg);
    void build_inputs(const IndexSeq &sent1, const IndexSeq &sent2, std::vector<cnn::expr::Expression> &inputs_exprs);
};

struct Input2
{
    cnn::LookupParameters *dynamic_lookup_param,
        *fixed_lookup_param;
    Merge2Layer m2_layer;
    cnn::ComputationGraph *pcg;
    NonLinearFunc *nonlinear_func;

    Input2(cnn::Model *m, unsigned dynamic_vocab_size, unsigned dynamic_embedding_dim,
        unsigned fixed_vocab_size, unsigned fixed_embedding_dim,
        unsigned mergeout_dim , NonLinearFunc *nonlinear_func=&cnn::expr::rectify);
    ~Input2();
    void new_graph(cnn::ComputationGraph &cg);
    void build_inputs(const IndexSeq &dynamic_sent, const IndexSeq &fixed_sent, 
        std::vector<cnn::expr::Expression> &inputs_exprs);
};

struct Input2WithFeature
{
    cnn::LookupParameters *dynamic_lookup_param,
        *fixed_lookup_param;
    Merge3Layer m3_layer;
    cnn::ComputationGraph *pcg;
    NonLinearFunc *nonlinear_func;

    Input2WithFeature(cnn::Model *m, unsigned dynamic_vocab_size, unsigned dynamic_embedding_dim,
        unsigned fixed_vocab_size, unsigned fixed_embedding_dim,
        unsigned feature_embedding_dim,
        unsigned mergeout_dim , NonLinearFunc *nonlinear_func=&cnn::expr::rectify);
    ~Input2WithFeature();
    void new_graph(cnn::ComputationGraph &cg);
    void build_inputs(const IndexSeq &dynamic_sent, const IndexSeq &fixed_sent, 
        const std::vector<cnn::expr::Expression> &feature_exprs,
        std::vector<cnn::expr::Expression> &inputs_exprs);
};

struct Input3
{
    cnn::LookupParameters *dynamic_lookup_param1,
        *dynamic_lookup_param2 ,
        *fixed_lookup_param;
    Merge3Layer m3_layer;
    cnn::ComputationGraph *pcg;
    NonLinearFunc *nonlinear_func;
    Input3(cnn::Model *m, unsigned dynamic_vocab_size1, unsigned dynamic_embedding_dim1 ,
        unsigned dynamic_vocab_size2 , unsigned dynamic_embedding_dim2 , 
        unsigned fixed_vocab_size, unsigned fixed_embedding_dim,
        unsigned mergeout_dim ,
        NonLinearFunc *nonlinear_func=&cnn::expr::rectify);
    ~Input3();
    void new_graph(cnn::ComputationGraph &cg);
    void build_inputs(const IndexSeq &dynamic_sent1, const IndexSeq &dynamic_sent2 , const IndexSeq &fixed_sent,
        std::vector<cnn::expr::Expression> &inputs_exprs);
};

struct BareInput1
{
    cnn::LookupParameters *word_lookup_param;
    cnn::ComputationGraph *pcg;
    unsigned nr_exprs;
    std::vector<cnn::expr::Expression> exprs;
    BareInput1(cnn::Model *m, unsigned vocabulary_size, unsigned word_embedding_dim, unsigned nr_extra_feature_exprs);
    cnn::LookupParameters *get_lookup_param(){ return word_lookup_param; }
    void new_graph(cnn::ComputationGraph &cg);
    cnn::expr::Expression
        build_input(Index word_idx, const std::vector<cnn::expr::Expression> &extra_feature_exprs);
    void build_inputs(const IndexSeq &sent, const std::vector<std::vector<cnn::expr::Expression>> &extra_feature_exprs_seq,
        std::vector<cnn::expr::Expression> &input_exprs);
};

struct AnotherBareInput1
{
    cnn::LookupParameters *word_lookup_param;
    cnn::ComputationGraph *pcg;
    AnotherBareInput1(cnn::Model *m, unsigned vocabulary_size, unsigned word_embedding_dim);
    void new_graph(cnn::ComputationGraph &cg);
    void build_inputs(const IndexSeq &sent, const std::vector<std::vector<cnn::expr::Expression> *> &extra_feature_ptr_exprs_seq,
        std::vector<cnn::expr::Expression> &input_exprs);
    void build_inputs(const IndexSeq &sent, const std::vector<cnn::expr::Expression> &feature_exprs_seq,
        std::vector<cnn::expr::Expression> &input_exprs);
};

/******* inline function implementation ******/

/****** input1 ******/
inline
void Input1::new_graph(cnn::ComputationGraph &cg)
{
    pcg = &cg;
}

inline
void Input1::build_inputs(const IndexSeq &sent, std::vector<cnn::expr::Expression> &inputs_exprs)
{
    if (nullptr == pcg) throw std::runtime_error("cg should be set .");
    size_t sent_len = sent.size();
    std::vector<cnn::expr::Expression> tmp_inputs(sent_len);
    for (size_t i = 0; i < sent_len; ++i)
    {
        tmp_inputs[i] = lookup(*pcg, word_lookup_param, sent[i]);
    }
    std::swap(inputs_exprs, tmp_inputs);
}

/******* input with feature ******/
inline
void Input1WithFeature::new_graph(cnn::ComputationGraph &cg)
{
    pcg = &cg;
    m2_layer.new_graph(cg);
}

inline
void Input1WithFeature::build_inputs(const IndexSeq &sent, 
    const std::vector<cnn::expr::Expression> &features_exprs,
    std::vector<cnn::expr::Expression> &inputs_exprs)
{
    using std::swap;
    if (nullptr == pcg) throw std::runtime_error("cg should be set .");
    size_t sent_len = sent.size();
    std::vector<cnn::expr::Expression> tmp_inputs(sent_len);
    for (size_t i = 0; i < sent_len; ++i)
    {
        cnn::expr::Expression word_expr = lookup(*pcg, word_lookup_param, sent[i]);
        const cnn::expr::Expression &feature_expr = features_exprs[i];
        cnn::expr::Expression merge_expr = m2_layer.build_graph(word_expr, feature_expr);
        tmp_inputs[i] = (*nonlinear_func)(merge_expr);
    }
    std::swap(inputs_exprs, tmp_inputs);
}
/******* input 2d  *******/
inline
void Input2D::new_graph(cnn::ComputationGraph &cg)
{
    pcg = &cg;
    m2_layer.new_graph(cg);
}

inline
void Input2D::build_inputs(const IndexSeq &seq1, const IndexSeq &seq2, std::vector<cnn::expr::Expression> &inputs_exprs )
{
    size_t seq_len = seq1.size();
    std::vector<cnn::expr::Expression> tmp_inputs(seq_len);
    for (size_t i = 0; i < seq_len; ++i)
    {
        cnn::expr::Expression expr1 = lookup(*pcg, dynamic_lookup_param1, seq1.at(i));
        cnn::expr::Expression expr2 = lookup(*pcg, dynamic_lookup_param2, seq2.at(i));
        cnn::expr::Expression linear_merge_expr = m2_layer.build_graph(expr1, expr2);
        cnn::expr::Expression nonlinear_expr = nonlinear_func(linear_merge_expr);
        tmp_inputs[i] = nonlinear_expr;
    }
    std::swap(inputs_exprs, tmp_inputs);
}

/******** input 2 ***********/
inline
void Input2::new_graph(cnn::ComputationGraph &cg)
{
    pcg = &cg;
    m2_layer.new_graph(cg);
}

inline
void Input2::build_inputs(const IndexSeq &dynamic_seq, const IndexSeq &fixed_seq, std::vector<cnn::expr::Expression> &inputs_exprs)
{
    size_t seq_len = dynamic_seq.size();
    std::vector<cnn::expr::Expression> tmp_inputs(seq_len);
    for (size_t i = 0; i < seq_len; ++i)
    {
        cnn::expr::Expression expr1 = lookup(*pcg, dynamic_lookup_param, dynamic_seq.at(i));
        cnn::expr::Expression expr2 = lookup(*pcg, fixed_lookup_param, fixed_seq.at(i));
        cnn::expr::Expression linear_merge_expr = m2_layer.build_graph(expr1, expr2);
        cnn::expr::Expression nonlinear_expr = nonlinear_func(linear_merge_expr);
        tmp_inputs[i] = nonlinear_expr;
    }
    std::swap(inputs_exprs, tmp_inputs);
}

/* input2 with  feature */
inline
void Input2WithFeature::new_graph(cnn::ComputationGraph &cg)
{
    pcg = &cg;
    m3_layer.new_graph(cg);
}
inline
void Input2WithFeature::build_inputs(const IndexSeq &dynamic_sent, const IndexSeq &fixed_sent, 
    const std::vector<cnn::expr::Expression> &feature_exprs,
    std::vector<cnn::expr::Expression> &inputs_exprs)
{
    size_t seq_len = dynamic_sent.size();
    std::vector<cnn::expr::Expression> tmp_inputs(seq_len);
    for (size_t i = 0; i < seq_len; ++i)
    {
        cnn::expr::Expression expr1 = lookup(*pcg, dynamic_lookup_param, dynamic_sent.at(i));
        cnn::expr::Expression expr2 = lookup(*pcg, fixed_lookup_param, fixed_sent.at(i));
        cnn::expr::Expression linear_merge_expr = m3_layer.build_graph(expr1, expr2, feature_exprs.at(i));
        cnn::expr::Expression nonlinear_expr = (*nonlinear_func)(linear_merge_expr);
        tmp_inputs[i] = nonlinear_expr;
    }
    std::swap(inputs_exprs, tmp_inputs);
}

/******* input 3  ********/
inline
void Input3::new_graph(cnn::ComputationGraph &cg)
{
    pcg = &cg;
    m3_layer.new_graph(cg);
}

inline
void Input3::build_inputs(const IndexSeq &dseq1, const IndexSeq &dseq2, const IndexSeq &fseq,
    std::vector<cnn::expr::Expression> &inputs_exprs)
{
    size_t seq_len = dseq1.size();
    std::vector<cnn::expr::Expression> tmp_inputs(seq_len);
    for (size_t i = 0; i < seq_len; ++i)
    {
        cnn::expr::Expression dexpr1 = lookup(*pcg, dynamic_lookup_param1, dseq1.at(i));
        cnn::expr::Expression dexpr2 = lookup(*pcg, dynamic_lookup_param2, dseq2.at(i));
        cnn::expr::Expression fexpr = const_lookup(*pcg, fixed_lookup_param, fseq.at(i));
        cnn::expr::Expression linear_merge_expr = m3_layer.build_graph(dexpr1, dexpr2, fexpr);
        tmp_inputs[i] = nonlinear_func(linear_merge_expr);
    }
    std::swap(inputs_exprs, tmp_inputs);
}
/******* bare input1  *******/
inline
void BareInput1::new_graph(cnn::ComputationGraph &cg)
{
    pcg = &cg;
}

inline
cnn::expr::Expression
BareInput1::build_input(Index word_idx, const std::vector<cnn::expr::Expression> &extra_feature_exprs)
{
    exprs.at(0) = lookup(*pcg, word_lookup_param, word_idx);
    for( unsigned i = 1 ; i < nr_exprs; ++i )
    {
        exprs.at(i) = extra_feature_exprs.at(i-1);
    }
    return cnn::expr::concatenate(exprs);
}

inline 
void BareInput1::build_inputs(const IndexSeq &sent, const std::vector<std::vector<cnn::expr::Expression>> &extra_feature_exprs_seq,
    std::vector<cnn::expr::Expression> &input_exprs)
{
    using std::swap;
    unsigned seq_len = sent.size();
    std::vector<cnn::expr::Expression> concat_result(seq_len);
    for( unsigned i = 0; i < seq_len ; ++i )
    {
        concat_result[i] = build_input(sent[i], extra_feature_exprs_seq[i]);
    }
    swap(concat_result, input_exprs);
}

/* Another Bare input1 */
inline
void AnotherBareInput1::new_graph(cnn::ComputationGraph &cg)
{
    pcg = &cg;
}

inline 
void AnotherBareInput1::build_inputs(const IndexSeq &sent, const std::vector<std::vector<cnn::expr::Expression> *> &extra_feature_ptr_expr_gp_seq,
    std::vector<cnn::expr::Expression> &input_exprs)
{
    // col-based concatenate
    size_t nr_feature_variety = extra_feature_ptr_expr_gp_seq.size();
    assert(nr_feature_variety > 0);
    size_t feature_seq_len = extra_feature_ptr_expr_gp_seq[0]->size();
    std::vector<cnn::expr::Expression> concated_exprs(feature_seq_len);
    std::vector<cnn::expr::Expression> feature_group(nr_feature_variety);
    for( size_t j = 0; j < feature_seq_len; ++j )
    {
        for( size_t fi = 0; fi < nr_feature_variety; ++fi )
        {
            feature_group.at(fi) = extra_feature_ptr_expr_gp_seq[fi]->at(j);
        }
        concated_exprs.at(j) = cnn::expr::concatenate(feature_group);
    }
    return build_inputs(sent, concated_exprs, input_exprs);
}

inline 
void AnotherBareInput1::build_inputs(const IndexSeq &sent, const std::vector<cnn::expr::Expression> &feature_exprs,
    std::vector<cnn::expr::Expression> &input_exprs)
{
    using std::swap;
    unsigned seq_len = sent.size();
    std::vector<cnn::expr::Expression> tmp_input_exprs(seq_len);
    for( size_t i = 0; i < seq_len; ++i )
    {
        cnn::expr::Expression word_expr = lookup(*pcg, word_lookup_param, sent[i]);
        tmp_input_exprs[i] = cnn::expr::concatenate({ word_expr, feature_exprs[i] });
    }
    swap(input_exprs, tmp_input_exprs);
}

} // end of namespace slnn

#endif