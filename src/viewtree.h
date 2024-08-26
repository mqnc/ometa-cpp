#pragma once

#include "empty.h"
#include <variant>
#include <memory>
#include <array>
#include <stack>

namespace ometa {

template <typename TData>
class ViewTree {
public:
	using TChildren = std::array<std::shared_ptr<ViewTree<TData>>, 2>;

	std::variant<Empty, View<TData>, TChildren> value;

	ViewTree(TChildren children): value(children) {}

	bool isEmpty() const { return std::holds_alternative<Empty>(value); }
	bool isLeaf() const { return std::holds_alternative<View<TData>>(value); }
	const View<TData>& getView() const { return std::get<View<TData>>(value); }
	bool hasChildren() const { return std::holds_alternative<TChildren>(value); }
	const ViewTree<TData>* getChild(size_t i) const { return std::get<TChildren>(value)[i].get(); }

public:
	ViewTree(): value(Empty {}) {}
	ViewTree(View<TData> view): value(view) {}
	ViewTree(const ViewTree<TData>& other): value(other.value) {}
	ViewTree(ViewTree<TData>&& other): value(std::move(other.value)) {}
	ViewTree<TData>& operator=(const ViewTree<TData>& other) {
		if (this != &other) { value = other.value; }
		return *this;
	}

	ViewTree<TData> operator+(ViewTree<TData> other) {
		if (isEmpty()) { return other; }
		if (other.isEmpty()) { return *this; }
		return TChildren({
			std::make_shared<ViewTree<TData>>(*this),
			std::make_shared<ViewTree<TData>>(other)
		});
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
			if(currentView.begin() != currentView.end()) {
				currentView = currentView.next();
			}
			while(currentView.begin() == currentView.end()){
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
	for (auto& elem: rhs) {
		if (elem != *it) return false;
		++it;
	}
	return true;
}

template <typename TData>
bool operator==(const ViewTree<TData>& lhs, const char* rhs) {
	return lhs == std::string_view(rhs);
}

template <typename TData1, typename TData2>
bool operator==(const ViewTree<TData1>& lhs, const ViewTree<TData2>& rhs) {
	return lhs == rhs;
}

template <typename TOther, typename TData>
bool operator==(const TOther& lhs, const ViewTree<TData>& rhs) {
	return rhs == lhs;
}



}
