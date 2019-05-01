#ifndef LAMBDAMART_TREELEARNER_H
#define LAMBDAMART_TREELEARNER_H

#include <lambdamart/types.h>
#include <lambdamart/config.h>
#include <lambdamart/dataset.h>
#include <lambdamart/histogram.h>
#include <queue>

namespace LambdaMART {

// TODO: make it a array-based data structure Tree, without using TreeNodes and pointers
class TreeNode {
    friend class TreeLearner;
    friend class Model;

    TreeNode(nodeidx_t id) :
        id(id), predict(0), impurity(0), isLeaf(false),
        split(nullptr), leftNode(nullptr), rightNode(nullptr) {}

    TreeNode(nodeidx_t id, score_t predict, score_t impurity, bool isLeaf) :
        id(id), predict(predict), impurity(impurity), isLeaf(isLeaf),
        split(nullptr), leftNode(nullptr), rightNode(nullptr) {}

private:
    nodeidx_t id;
    score_t predict;
    score_t impurity;
    bool isLeaf;
    SplitInfo* split;
    TreeNode* leftNode;
    TreeNode* rightNode;

    std::string toString(const std::string& prefix = "")
    {
        return prefix + "id = " + std::to_string(id) + ", predict = " + std::to_string(predict) + ", impurity = " + std::to_string(impurity) + ", isLeaf = " + std::to_string(isLeaf)
               + ", split = " + (split != nullptr ? split->toString() : "none") + ", leftNode = " + std::to_string(leftNode != nullptr ? leftNode->id : 0) + ", rightNode = " + std::to_string(rightNode != nullptr ? rightNode->id : 0)
               + (isLeaf ? "\n" : ("\n" + leftNode->toString(prefix + "  ") + rightNode->toString(prefix + "  ")));
    }

    score_t predict_score(const std::vector<featval_t> &features)
    {
        if (isLeaf)
        {
            return predict;
        }
        else
        {
            return (features[split->feature] <= split->threshold) ? leftNode->predict_score(features) : rightNode->predict_score(features);
        }
    }

    /*
    // TODO: utility functions
    static TreeNode* newEmptyNode(nodeidx_t nodeIndex)
    {
        // make it leaf by default
        return new TreeNode(nodeIndex, std::numeric_limits<score_t>::min(), -1.0, false);
    }

    static uint32_t indexToLevel(nodeidx_t nodeIndex)
    {
        return static_cast<uint32_t>(std::floor(std::log2(nodeIndex)));
    }

    static bool isLeftChild(nodeidx_t nodeIndex)
    {
        return (nodeIndex > 1 && (nodeIndex % 2 == 0));
    }

    static nodeidx_t leftChildIndex(nodeidx_t nodeIndex)
    {
        return (nodeIndex << 1);
    }

    static nodeidx_t rightChildIndex(nodeidx_t nodeIndex)
    {
        return (nodeIndex << 1) + 1;
    }

    static nodeidx_t parentIndex(nodeidx_t nodeIndex)
    {
        return (nodeIndex >> 1);
    }

    static nodeidx_t siblingIndex(nodeidx_t nodeIndex)
    {
        return isLeftChild(nodeIndex) ? nodeIndex + 1 : nodeIndex - 1;
    }

    nodeidx_t numDescendants()
    {
        return isLeaf ? 0 : (2 + leftNode->numDescendants() + rightNode->numDescendants());
    }

    nodeidx_t internalNodes()
    {
        // DEBUG_ASSERT_EX(isLeaf || (leftNode != nullptr && rightNode != nullptr), "%u internalNodes counting: no leftnode or rightnode!", id);
        return isLeaf ? 0 : (1 + leftNode->internalNodes() + rightNode->internalNodes());
    }
    */
};
typedef TreeNode Tree;


class TreeLearner {
    friend class Booster;

public:
    TreeLearner() = delete;
    TreeLearner(const Dataset* _dataset, const double* _gradients, const double* _hessians, const Config* _config) :
        dataset(_dataset), gradients(_gradients), hessians(_hessians), config(_config)
    {
        num_samples = dataset->num_samples();
        max_splits = config->max_splits;
        node_to_score.clear();
        sample_to_node.resize(num_samples, 0);
        histograms.resize(config->max_splits);
    }

private:
    struct SplitCandidate
    {
        TreeNode* node;
        NodeInfoStats* info;
        nodeidx_t smallerSibling;  // id

        SplitCandidate() {}
        SplitCandidate(TreeNode* n, NodeInfoStats* i) : node(n), info(i), smallerSibling(0) {}
        SplitCandidate(TreeNode* n, NodeInfoStats* i, nodeidx_t s) : node(n), info(i), smallerSibling(s) {}

        bool operator<(const SplitCandidate& rhs) const
        { // TODO: make smallerSibling == 0 top priority
            return node->impurity < rhs.node->impurity;
        }
    };

    // as input
    const Config*             config;
    const Dataset*            dataset;
    const double*             gradients;
    const double*             hessians;

    // as working set
    uint64_t                            num_samples;
    int                                 max_splits;
    std::vector<Histogram>              histograms;
    std::vector<double>                 node_to_score;
    std::vector<unsigned int>           sample_to_node;
    std::vector<SplitCandidate*>        split_candidates;
    std::vector<int>                    node_to_candidate;
    std::vector<int>                    sample_to_candidate;  // -1: this sample doesn't exist in any candidate node
    int                                 cur_depth;
    std::priority_queue<SplitCandidate> node_queue;
    int                                 num_nodes_to_split;

    Tree* build_new_tree();
    int select_split_candidates();
    void update_candidate_tracker();
    std::vector<SplitInfo>* find_best_splits();
    nodeidx_t perform_split(const std::vector<SplitInfo>& best_splits,
                         std::vector<double>&          node_to_score,
                         std::vector<unsigned int>&    sample_to_node);
    double get_sample_score(sample_t sid) { return node_to_score[sample_to_node[sid]]; };
};

}



#endif //LAMBDAMART_TREELEARNER_H
