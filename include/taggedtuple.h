#include <tuple>
#include "tag.h"

namespace ometa {

template <typename... TaggedValues>
class TaggedTuple {
    std::tuple<TaggedValues...> data;

    // Helper to get element by tag
    template <size_t Index, Tag T, typename TupleElement>
    auto& getTaggedHelper(TupleElement& elem) {
        if constexpr (T == TupleElement::getTag()) {
            return elem.value;
        } else {
            return getTaggedHelper<Index + 1, T>(std::get<Index + 1>(data));
        }
    }

public:
    TaggedTuple(TaggedValues... values) : data(values...) {}

    // Get element by tag
    template <Tag T>
    auto& get() {
        return getTaggedHelper<0, T>(std::get<0>(data));
    }
};

}
