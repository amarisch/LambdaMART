#include <lambdamart/treelearner.h>
#include <numeric>

namespace LambdaMART {


Tree* TreeLearner::build_new_tree()
{
    Tree* root = new TreeNode(1);
    auto* topInfo = new NodeInfoStats(num_samples, std::accumulate(gradients, gradients + num_samples, 0.0));
    node_queue.push(new SplitCandidate(root, topInfo));
    std::fill(sample_to_node.begin(), sample_to_node.end(), 1);
    Log::Debug("build_new_tree: initialized");

    cur_depth = 1;
    while (cur_depth < config->max_depth) {
        if(!select_split_candidates()) break;  // no more nodes to split -> break
        find_best_splits();
        perform_split();
    }

    histograms.clear();
    node_to_score.clear();
    split_candidates.clear();
    node_to_candidate.clear();
    best_splits.clear();
    return root;
}

bool TreeLearner::select_split_candidates() {
    Log::Debug("select_split_candidates");

    std::fill(node_to_candidate.begin(), node_to_candidate.end(), -1);
    sample_to_candidate.clear();
    sample_to_candidate.resize(num_samples);

    num_candidates = 0;
    split_candidates.clear();
    while (!node_queue.empty() && num_candidates < max_splits)
    {
        auto candidate = node_queue.top();
        node_queue.pop();
        split_candidates.push_back(candidate);
        node_info.push_back(candidate->info);
        node_to_candidate[candidate->node->id] = num_candidates++;
    }

    if (!num_candidates) return false;

    for (sample_t sample = 0; sample < num_samples; ++sample) {
        sample_to_candidate[sample] = node_to_candidate[sample_to_node[sample]];
    }
    return true;
}


/**
 * Find best splits and put them into `best_splits'
 */
void TreeLearner::find_best_splits()
{
    Log::Debug("find_best_splits");

    best_splits.resize(num_candidates);
    for (feature_t fid = 0; fid < num_features; ++fid)
    {
        Log::Debug("checking feature %lu", fid);
        histograms.clear(num_candidates);
        const feature& feat = dataset->get_data()[fid];

        //TODO: unrolling
        for (sample_t sample_idx = 0; sample_idx < num_samples; ++sample_idx)
        {
            if (sample_idx % 10 == 0)   Log::Debug("  sample %lu", sample_idx);
            const int node = sample_to_candidate[sample_idx];
            if (node != -1)
            {
                const bin_t bin = feat.bin_index[sample_idx];
                histograms[node][bin].update(1.0, gradients[sample_idx]);
            }
        }

        for (nodeidx_t candidate = 0; candidate < num_candidates; ++candidate)
        {
            Log::Debug("Candidate %lu", candidate);
            histograms.cumulate(candidate);
            auto temptuple = histograms.BestSplit(candidate, fid, feat, node_info[candidate], config->min_data_in_leaf);
            Log::Debug("\t\tbestsplit: %s, bin %u, gain %lf, left %s, right %s",
                    std::get<0>(temptuple).toString().c_str(),
                    std::get<1>(temptuple),
                    std::get<2>(temptuple),
                    std::get<3>(temptuple).toString().c_str(),
                   std::get<4>(temptuple).toString().c_str());
            if (std::get<2>(temptuple) >= std::get<2>(best_splits[candidate]))
            {
                best_splits[candidate] = temptuple;
                Log::Debug("\t\t-> replaced");
            }
        }
    }
}

void TreeLearner::perform_split()
{
    Log::Debug("perform_split");
    //update node_to_score, sample_to_node
    ++cur_depth;
}

}
