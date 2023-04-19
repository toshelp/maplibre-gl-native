#include <mbgl/gfx/vertex_attribute.hpp>

namespace mbgl {
namespace gfx {

VertexAttributeArray::VertexAttributeArray(int initCapacity)
    : attrs(initCapacity) {
}

VertexAttributeArray::VertexAttributeArray(VertexAttributeArray &&other)
    : attrs(std::move(other.attrs)) {
}

VertexAttributeArray& VertexAttributeArray::operator=(VertexAttributeArray &&other) {
    attrs = std::move(other.attrs);
    return *this;
}

VertexAttributeArray& VertexAttributeArray::operator=(const VertexAttributeArray& other) {
    attrs.clear();
    for (const auto &kv : other.attrs) {
        add(kv.first, copy(*kv.second));
    }
    return *this;
}

VertexAttribute* VertexAttributeArray::get(const std::string& name) const {
    const auto result = attrs.find(name);
    return (result != attrs.end()) ? result->second.get() : nullptr;
}

VertexAttribute* VertexAttributeArray::add(std::string name, int index,
                                           AttributeDataType dataType, int size, std::size_t count) {
    const auto result = attrs.insert(std::make_pair(std::move(name), std::unique_ptr<VertexAttribute>()));
    if (result.second) {
        result.first->second = create(index, dataType, size, count);
        return result.first->second.get();
    } else {
        return nullptr;
    }
}

VertexAttribute* VertexAttributeArray::getOrAdd(std::string name, int index,
                                                AttributeDataType dataType, int size, std::size_t count) {
    //attrs.emplace_back(std::make_unique<VertexAttribute>(dataType, count));
    const auto result = attrs.insert(std::make_pair(std::move(name), std::unique_ptr<VertexAttribute>()));
    if (result.second) {
        result.first->second = create(index, dataType, size, count);
    } else if (result.first->second->getDataType() != dataType ||
               result.first->second->getSize() != size ||
               result.first->second->getCount() != count) {
        return nullptr;
    }
    return result.first->second.get();
}

bool VertexAttributeArray::isDirty() const {
    for (const auto& kv : attrs) {
        if (kv.second->getDirty()) {
            return true;
        }
    }
    return false;
}

void VertexAttributeArray::resetDirty() {
    for (auto& kv : attrs) {
        kv.second->clearDirty();
    }
}

void VertexAttributeArray::resolve(const VertexAttributeArray& overrides,
                                   ResolveDelegate delegate) const {
    for (auto& kv : attrs) {
        const auto overrideAttr = overrides.get(kv.first);
        delegate(kv.first, *kv.second, overrideAttr);
    }
}

} // namespace gfx
} // namespace mbgl