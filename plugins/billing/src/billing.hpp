#pragma once

#include <ek/ds/String.hpp>
#include <ek/ds/Array.hpp>
#include <ek/util/Signal.hpp>

namespace billing {

void initialize(const char* developerKey);
void _initialize();

void shutdown();

void getPurchases();

void getDetails(const ek::Array<ek::String>& skuList);

void purchase(const ek::String& sku, const ek::String& payload);

void consume(const ek::String& token);

struct PurchaseData {
    ek::String productID;
    ek::String token;
    int state;
    ek::String payload;
    ek::String signature;
    int errorCode;

    // ios
    ek::String receipt;
};

struct ProductDetails {
    ek::String sku;
    ek::String price;
    ek::String currencyCode;
};

struct Context {
    ek::Signal<const PurchaseData&> onPurchaseChanged;
    ek::Signal<const ProductDetails&> onProductDetails;
};

extern Context& context;

}
