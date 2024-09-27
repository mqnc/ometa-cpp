#pragma once

#include "empty.h"
#include <variant>
#include <memory>
#include <array>
#include <stack>
#include <string_view>

namespace ometa {

template <typename TData>
class ViewTree {
public:
	using TChildren = std::array<std::shared_ptr<ViewTree<TData>>, 2>;

private:
	std::variant<Empty, View<TData>, TChildren> value;
public:

	bool isEmpty() const { return std::holds_alternative<Empty>(value); }
	bool isLeaf() const { return std::holds_alternative<View<TData>>(value); }
	const View<TData>& getView() const { return std::get<View<TData>>(value); }
	bool hasChildren() const { return std::holds_alternative<TChildren>(value); }
	const ViewTree<TData>* getChild(size_t i) const { return std::get<TChildren>(value)[i].get(); }

public:
	ViewTree(): value(Empty {}) {}
	ViewTree(View<TData> view): value(view) {}
	ViewTree(const TData& data): value(View(data)) {}
	ViewTree(TChildren children): value(children) {}
	ViewTree(const ViewTree<TData>& other): value(other.value) {}
	ViewTree(ViewTree<TData>&& other): value(std::move(other.value)) {}
	ViewTree<TData>& operator=(const ViewTree<TData>& other) {
		if (this != &other) { value = other.value; }
		return *this;
	}

	size_t size() const {
		if (isEmpty()) { return 0; }
		if (isLeaf()) return getView().size();
		return getChild(0)->size() + getChild(1)->size();
	}

	class Iterator {

		const ViewTree<TData>* currentViewTree = nullptr;
		View<TData> currentView;
		std::stack<const ViewTree<TData>*> stack;

		void pushToNextLeaf(const ViewTree<TData>* node) {
			while (node->hasChildren()) {
				stack.push(node);
				node = node->getChild(0);
			}
			currentViewTree = node;
			currentView = currentViewTree->getView();
		}

	public:
		Iterator() {}
		Iterator(const ViewTree<TData>* root) { pushToNextLeaf(root); }

		auto operator*() const {
			return *currentView.begin();
		}

		Iterator& operator++() {
			if (currentView.begin() != currentView.end()) {
				currentView = currentView.next();
			}
			while (currentView.begin() == currentView.end()) {
				if (stack.empty()) {
					currentViewTree = nullptr;
					break;
				}
				else {
					const ViewTree<TData>* node = stack.top();
					stack.pop();
					pushToNextLeaf(node->getChild(1));
				}
			}
			return *this;
		}

		bool operator==(const Iterator& other) const {
			if (currentViewTree == nullptr && other.currentViewTree == nullptr) {
				return true;
			}
			else {
				return currentViewTree == other.currentViewTree && currentView.begin() == other.currentView.begin();
			}
		}
	};

	Iterator begin() const {
		if (isEmpty()) return Iterator();
		else { return Iterator(this); }
	}

	Iterator end() const {
		return Iterator();
	}

};

auto operator""_tree_(const char* str, size_t len) {
	return ViewTree<std::string_view>(std::string_view(str, len));
}

template <typename TData>
ViewTree<TData> operator+(const ViewTree<TData>& lhs, const ViewTree<TData>& rhs) {
	if (lhs.isEmpty()) { return rhs; }
	if (rhs.isEmpty()) { return lhs; }
	return typename ViewTree<TData>::TChildren({
		std::make_shared<ViewTree<TData>>(lhs),
		std::make_shared<ViewTree<TData>>(rhs)
	});
}

ViewTree<std::string_view> operator+(const ViewTree<std::string_view>& lhs, const char* rhs) {
	return lhs + ViewTree<std::string_view>(rhs);
}

ViewTree<std::string_view> operator+(const char* lhs, const ViewTree<std::string_view>& rhs) {
	return ViewTree<std::string_view>(lhs) + rhs;
}

template <typename TData>
std::ostream& operator<<(std::ostream& os, const ViewTree<TData>& tree) {
	if (tree.isLeaf()) {
		os << tree.getView();
	} else {
		os << *tree.getChild(0);
		os << *tree.getChild(1);
	}
	return os;
}

template <typename TData, typename TOther>
bool operator==(const ViewTree<TData>& lhs, const TOther& rhs) {
	if (lhs.size() != rhs.size()) return false;
	auto it = lhs.begin();
	for (const auto& elem: rhs) {
		if (elem != *it) return false;
		++it;
	}
	return true;
}

template <typename TData>
bool operator==(const ViewTree<TData>& lhs, const char* rhs) {
	return lhs == std::string_view(rhs);
}

// template <typename TData1, typename TData2>
// bool operator==(const ViewTree<TData1>& lhs, const ViewTree<TData2>& rhs) {
// 	return lhs == rhs;
// }

// template <typename TOther, typename TData>
// bool operator==(const TOther& lhs, const ViewTree<TData>& rhs) {
// 	return rhs == lhs;
// }

}

namespace std {
	// so a ViewTree<std::string_view> can be a key in a map
	template<>
    struct hash<ometa::ViewTree<std::string_view>> {
        std::size_t operator()(const ometa::ViewTree<std::string_view>& tree) const {
            std::string s;
			s.reserve(tree.size());
			for (const char c : tree){s += c;}
			return std::hash<std::string>{}(s);
        }
    };
}
