#pragma once

#include <string>
#include <vector>
#include <ek/util/signals.hpp>

namespace billing {

void initialize(const std::string& developerKey);
void _initialize();

void shutdown();

void getPurchases();

void getDetails(const std::vector<std::string>& skuList);

void purchase(const std::string& sku, const std::string& payload);

void consume(const std::string& token);

struct PurchaseData {
    std::string productID;
    std::string token;
    int state;
    std::string payload;
    std::string signature;
    int errorCode;

    // ios
    std::string receipt;
};

struct ProductDetails {
    std::string sku;
    std::string price;
    std::string currencyCode;
};

struct Context {
    ek::signal_t<const PurchaseData&> onPurchaseChanged;
    ek::signal_t<const ProductDetails&> onProductDetails;
};

extern Context& context;

}
