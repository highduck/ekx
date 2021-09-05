#pragma once

#include <ek/timers.hpp>

namespace billing {

void initialize(const std::string& developerKey) {
    (void) developerKey;
    _initialize();
}

void getPurchases() {

}

void getDetails(const std::vector<std::string>& skuList) {
    double time = 0.5;
    for (const auto& sku : skuList) {
        ek::setTimeout([sku]() {
            context.onProductDetails({sku, "$1.99", "USD"});
            }, time);
        time += 0.5;
    }
}

void purchase(const std::string& sku, const std::string& payload) {
    PurchaseData data;
    data.productID = sku;
    data.payload = payload;
    data.state = 0;
    ek::setTimeout([data]() {
        context.onPurchaseChanged(data);
        }, 2.0);
}

void consume(const std::string& token) {
    (void) token;
}

}