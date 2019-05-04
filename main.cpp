#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <lambdamart/lambdamart.h>

using namespace std;

void demo(LambdaMART::Config* config) {
//    const char* train = "data/mq2008/small.train";
//    const char* train_query = "data/mq2008/small.train.query";
    const char* train = config->train_data.c_str();
    const char* train_query = config->train_query.c_str();
//    const char* train_label = config->train_label.c_str();
    LambdaMART::Log::Info("Loading training dataset %s and query boundaries %s", train, train_query);
    auto* X_train = new LambdaMART::Dataset(config);
    X_train->load_dataset(train, train_query);
//    X_train->load_debug_dataset(train, train_label, train_query, 32);

    LambdaMART::Log::Info("Start training...");
    LambdaMART::Model* model = (new LambdaMART::Booster(X_train, config))->train();
    LambdaMART::Log::Info("Training finished.");

//    const char* vali = "data/mq2008/small.vali";
//    const char* vali_query = "data/mq2008/small.vali.query";
    const char* vali = config->valid_data.c_str();
    const char* vali_query = config->valid_query.c_str();
    LambdaMART::Log::Info("Loading test dataset %s and query boundaries %s", vali, vali_query);
    auto* X_test = new LambdaMART::RawDataset();
    X_test->load_dataset(vali, vali_query);

    LambdaMART::Log::Info("Predicting with validation dataset...");
    vector<double>* predictions = model->predict(X_test, config->output_result);
    LambdaMART::LambdaRank val_ranker(*X_test, *config);
    vector<double> result = val_ranker.eval(predictions->data());
    string tmp = "Validation";
    for (size_t i = 0 ; i < result.size(); ++i) {
        tmp += "\tNDCG@" + to_string(config->eval_at[i]) + "=" + to_string(result[i]);
    }
    LambdaMART::Log::Info(tmp.c_str());
}

int main(int argc, char** argv) {
    cout << LambdaMART::version() << endl;

    if (argc <= 1) {
        cout << LambdaMART::help() << endl;
        exit(0);
    }

    LambdaMART::Log::Info("Using configuration file %s", argv[1]);
    auto* config = new LambdaMART::Config(argv[1]);

    demo(config);

    return 0;
}
