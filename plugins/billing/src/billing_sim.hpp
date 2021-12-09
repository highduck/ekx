#pragma once

#include <ek/ds/String.hpp>
#include <ek/timers.hpp>

namespace billing {

void initialize(const char* developerKey) {
    (void) developerKey;
    _initialize();
}

void getPurchases() {

}

void getDetails(const ek::Array<ek::String>& skuList) {
    double time = 0.5;
    for (const auto& sku : skuList) {
        ek::setTimeout([sku]() {
            context.onProductDetails({sku, "$1.99", "USD"});
            }, time);
        time += 0.5;
    }
}

void purchase(const ek::String& sku, const ek::String& payload) {
    PurchaseData data;
    data.productID = sku;
    data.payload = payload;
    data.state = 0;
    ek::setTimeout([data]() {
        context.onPurchaseChanged(data);
        }, 2.0);
}

void consume(const ek::String& token) {
    (void) token;
}

}