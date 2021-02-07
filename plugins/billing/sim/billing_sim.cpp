#include "billing.hpp"

namespace billing {

void initialize(const std::string& developerKey) {
    (void)developerKey;
}

void getPurchases() {

}

void getDetails(const std::vector<std::string>& skuList) {
    (void)skuList;
}

void purchase(const std::string& sku, const std::string& payload) {
    (void)sku;
    (void)payload;
}

void consume(const std::string& token) {
    (void)token;
}

}