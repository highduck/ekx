#pragma once

#include <ek/log.h>
#include <ek/app_native.h>

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

@interface Billing : NSObject <SKProductsRequestDelegate, SKPaymentTransactionObserver> {
    NSMutableDictionary* _products;
    NSMutableDictionary* _transactionsDict;
    NSMutableArray* _transactionsQueue;
    BOOL _ready;
}
@end

@implementation Billing

- (id)init {
    self = [super init];

    _products = [NSMutableDictionary dictionary];
    _transactionsDict = [NSMutableDictionary dictionary];
    _transactionsQueue = [NSMutableArray array];
    _ready = false;

    [[SKPaymentQueue defaultQueue] addTransactionObserver:self];

    return self;
}

- (void)free {
    _ready = false;
}


- (void)updateProducts:(NSArray*)items {
    SKProductsRequest* productsRequest = [[SKProductsRequest alloc]
            initWithProductIdentifiers:[NSSet setWithArray:items]];
    productsRequest.delegate = self;
    [productsRequest start];
}


- (void)handleQueue:(SKPaymentQueue*)queue
updatedTransactions:(NSArray*)transactions {
    using namespace billing;

    for (SKPaymentTransaction* transaction in transactions) {

        const char* trID = transaction.transactionIdentifier ? transaction.transactionIdentifier.UTF8String : "";
        const char* prodID = transaction.payment.productIdentifier.UTF8String;
        EK_INFO("transaction %li '%s' '%s'", transaction.transactionState, trID, prodID);

        switch (transaction.transactionState) {
            // Call the appropriate custom method for the transaction state.
            case SKPaymentTransactionStatePurchasing:
                break;

            case SKPaymentTransactionStateDeferred:
                break;

            case SKPaymentTransactionStateFailed: {
                [queue finishTransaction:transaction];

                PurchaseData purchase;
                purchase.productID = prodID;
                purchase.token = trID;
                purchase.errorCode = (int) transaction.error.code;
                purchase.state = -1; // failed
                // SKErrorPaymentCancelled
                if (transaction.error.code == SKErrorPaymentCancelled) {
                } else {
                }
                context.onPurchaseChanged(purchase);
            }
                break;

            case SKPaymentTransactionStatePurchased: {
                [_transactionsDict setObject:transaction forKey:transaction.transactionIdentifier];

                PurchaseData purchase;
                purchase.productID = prodID;
                purchase.token = trID;
                purchase.state = 0; // purchased

                //                NSString* str = [[NSString alloc] initWithData:transaction.transactionReceipt encoding:NSUTF8StringEncoding];
                //                NSData *dataReceipt = [NSData dataWithContentsOfURL:[[NSBundle mainBundle] appStoreReceiptURL]];
                //                NSString *receipt = [dataReceipt base64EncodedStringWithOptions:0];
                //                purchase.receipt = [str UTF8String];

                NSString* userData = transaction.payment.applicationUsername;
                if (userData) {
                    purchase.payload = [userData UTF8String];
                }

                context.onPurchaseChanged(purchase);
            }
                break;

            case SKPaymentTransactionStateRestored:
                break;

            default:
                break;
        }
    }
}


- (void)paymentQueue:(SKPaymentQueue*)queue
 updatedTransactions:(NSArray*)transactions {
    if (_ready) {
        [self handleQueue:queue updatedTransactions:transactions];
    } else {
        [_transactionsQueue addObjectsFromArray:transactions];
    }
}

- (void)ready {
    _ready = true;
}

- (void)restore {

    if (!_ready)
        return;

    NSArray* all = [_transactionsDict allValues];
    NSArray* purchased = [all filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id object,
                                                                                                NSDictionary* bindings) {
        return ((SKPaymentTransaction*) object).transactionState == SKPaymentTransactionStatePurchased;
    }]];

    [self handleQueue:[SKPaymentQueue defaultQueue] updatedTransactions:purchased];
    [self handleQueue:[SKPaymentQueue defaultQueue] updatedTransactions:_transactionsQueue];
    [_transactionsQueue removeAllObjects];

    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

- (void)productsRequest:(SKProductsRequest*)request
     didReceiveResponse:(SKProductsResponse*)response {
    using namespace billing;

    //NSArray *products = [response products];
    //NSArray *inv = response.invalidProductIdentifiers;

    //for (NSString *invalidIdentifier in response.invalidProductIdentifiers) {
    // Handle any invalid product identifiers.
    //}

    for (SKProduct* product in response.products) {
        [_products setObject:product forKey:product.productIdentifier];
        ProductDetails details;
        details.sku = [product.productIdentifier UTF8String];

        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        numberFormatter.numberStyle = NSNumberFormatterCurrencyStyle;
        numberFormatter.locale = product.priceLocale;
        NSString* formattedString = [numberFormatter stringFromNumber:product.price];
        details.price = [formattedString UTF8String];
        details.currencyCode = [numberFormatter.currencyCode UTF8String];

        context.onProductDetails(details);
    }
}

- (SKProduct*)getProduct:(const char*)name {
    NSString* str = [NSString stringWithUTF8String:name];

    return _products[str];
}

- (void)purchase:(const char*)prod
        :(const char*)payload {
    SKProduct* product = [self getProduct:prod];
    if (!product)
        return;

    SKMutablePayment* payment = [SKMutablePayment paymentWithProduct:product];
    payment.quantity = 1;
    payment.applicationUsername = [NSString stringWithUTF8String:payload];

    [[SKPaymentQueue defaultQueue] addPayment:payment];
}

- (void)consume:(const char*)token {
    NSString* str = [NSString stringWithUTF8String:token];

    SKPaymentTransaction* trans = _transactionsDict[str];
    if (!trans)
        return;

    [[SKPaymentQueue defaultQueue] finishTransaction:trans];
    [_transactionsDict removeObjectForKey:trans.transactionIdentifier];
}

@end

namespace billing {
Billing* _billing = nullptr;

void initialize(const char* developerKey) {
    (void) developerKey;
    if (_billing != nil) {
        return;
    }
    _initialize();
    _billing = [Billing new];
}

void getPurchases() {
    if (_billing == nil) {
        return;
    }
    [_billing ready];
    [_billing restore];
}

void getDetails(const ek::Array <ek::String>& items) {
    if (_billing == nil) {
        return;
    }

    NSArray* array = [[NSArray alloc] init];

    for (const auto& item: items) {
        NSString* str = [NSString stringWithUTF8String:item.c_str()];
        array = [array arrayByAddingObject:str];
    }

    [_billing updateProducts:array];
}

void purchase(const ek::String& sku, const ek::String& payload) {
    if (_billing == nil) {
        return;
    }

    [_billing purchase:sku.c_str() :payload.c_str()];
}

void consume(const ek::String& token) {
    if (_billing == nil) {
        return;
    }

    [_billing consume:token.c_str()];
}

}
