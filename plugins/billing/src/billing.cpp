#include "billing.hpp"

namespace billing {

ek::signal_t<const PurchaseData&> onPurchaseChanged;
ek::signal_t<const ProductDetails&> onProductDetails;

}