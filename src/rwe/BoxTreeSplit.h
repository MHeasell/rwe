#ifndef RWE_BOXTREE_H
#define RWE_BOXTREE_H

#include <memory>
#include <optional>
#include <rwe/Grid.h>
#include <variant>
#include <vector>

namespace rwe
{
    template <typename T>
    struct BoxPackInfoEntry
    {
        unsigned int x;
        unsigned int y;
        T value;
    };

    template <typename T>
    struct BoxPackInfo
    {
        unsigned int width;
        unsigned int height;

        std::vector<BoxPackInfoEntry<T>> entries;
    };

    enum class SplitAxis
    {
        Horizontal,
        Vertical
    };

    template <typename T>
    struct BoxTreeSplit;

    template <typename T>
    struct BoxTreeLeaf;

    template <typename T>
    struct BoxTreeNode
    {
        unsigned int width;
        unsigned int height;

        using Union = std::variant<BoxTreeSplit<T>, BoxTreeLeaf<T>>;

        Union value;

        BoxTreeNode(
            SplitAxis axis,
            std::unique_ptr<BoxTreeNode>&& left,
            std::unique_ptr<BoxTreeNode>&& right);

        BoxTreeNode(unsigned int width, unsigned int height);

        BoxTreeNode(unsigned int width, unsigned int height, const T& value);

        BoxTreeNode(unsigned int width, unsigned int height, T&& value);

        std::optional<BoxTreeNode<T>*> findNode(unsigned int itemWidth, unsigned int itemHeight);

        std::vector<BoxPackInfoEntry<T>> walk();
    };

    template <typename T>
    struct BoxTreeSplit
    {
        SplitAxis axis;
        std::unique_ptr<BoxTreeNode<T>> leftChild;
        std::unique_ptr<BoxTreeNode<T>> rightChild;

        BoxTreeSplit(SplitAxis axis, std::unique_ptr<BoxTreeNode<T>>&& leftChild, std::unique_ptr<BoxTreeNode<T>>&& rightChild)
            : axis(axis), leftChild(std::move(leftChild)), rightChild(std::move(rightChild))
        {
        }
    };

    template <typename T>
    struct BoxTreeLeaf
    {
        std::optional<T> value;

        BoxTreeLeaf() = default;

        explicit BoxTreeLeaf(const T& value) : value(value)
        {
        }

        explicit BoxTreeLeaf(T&& value) : value(std::move(value))
        {
        }
    };

    template <typename T>
    struct BoxTree
    {
        std::unique_ptr<BoxTreeNode<T>> root;

        BoxTree(unsigned int width, unsigned int height, const T& value)
            : root(std::make_unique<BoxTreeNode<T>>(width, height, value))
        {
        }

        BoxTree(unsigned int width, unsigned int height, T&& value)
            : root(std::make_unique<BoxTreeNode<T>>(width, height, std::move(value)))
        {
        }

        BoxTreeNode<T>* findOrCreateNode(unsigned int itemWidth, unsigned int itemHeight);

        void insert(unsigned int itemWidth, unsigned int itemHeight, const T& item);
    };

    enum class GrowDirection
    {
        Across,
        Down
    };

    template <typename T>
    BoxTreeNode<T>* BoxTree<T>::findOrCreateNode(unsigned int itemWidth, unsigned int itemHeight)
    {
        // find a leaf node big enough to fit the box
        auto node = root->findNode(itemWidth, itemHeight);
        if (node)
        {
            return *node;
        }

        // we didn't find one, grow the tree
        GrowDirection direction;
        bool canGrowAcross = itemHeight <= root->height;
        bool canGrowDown = itemWidth <= root->width;
        assert(canGrowAcross || canGrowDown);
        direction = canGrowAcross
            ? canGrowDown
                ? root->height > root->width
                    ? GrowDirection::Across
                    : GrowDirection::Down
                : GrowDirection::Across
            : GrowDirection::Down;

        switch (direction)
        {
            case GrowDirection::Across:
            {
                auto newNode = std::make_unique<BoxTreeNode<T>>(itemWidth, root->height);
                auto newNodePtr = newNode.get();
                auto newRoot = std::make_unique<BoxTreeNode<T>>(SplitAxis::Vertical, std::move(root), std::move(newNode));
                root = std::move(newRoot);
                return newNodePtr;
            }
            case GrowDirection::Down:
            {
                auto newNode = std::make_unique<BoxTreeNode<T>>(root->width, itemHeight);
                auto newNodePtr = newNode.get();
                auto newRoot = std::make_unique<BoxTreeNode<T>>(SplitAxis::Horizontal, std::move(root), std::move(newNode));
                root = std::move(newRoot);
                return newNodePtr;
            }
        }

        throw std::logic_error("unreachable");
    }

    template <typename T>
    void BoxTree<T>::insert(unsigned int itemWidth, unsigned int itemHeight, const T& item)
    {
        auto node = findOrCreateNode(itemWidth, itemHeight);

        // insert into the node
        // (it's guaranteed to be a leaf)
        BoxTreeLeaf<T>& leaf = std::get<BoxTreeLeaf<T>>(node->value);
        if (node->width == itemWidth && node->height == itemHeight)
        {
            leaf.value = item;
        }
        else if (node->width == itemWidth && node->height > itemHeight)
        {
            auto newLeaf = std::make_unique<BoxTreeNode<T>>(itemWidth, itemHeight, item);
            auto freeSpace = std::make_unique<BoxTreeNode<T>>(itemWidth, node->height - itemHeight);
            *node = BoxTreeNode<T>(SplitAxis::Horizontal, std::move(newLeaf), std::move(freeSpace));
        }
        else if (node->height == itemHeight && node->width > itemWidth)
        {
            auto newLeaf = std::make_unique<BoxTreeNode<T>>(itemWidth, itemHeight, item);
            auto freeSpace = std::make_unique<BoxTreeNode<T>>(node->width - itemWidth, itemHeight);
            *node = BoxTreeNode<T>(SplitAxis::Vertical, std::move(newLeaf), std::move(freeSpace));
        }
        else
        {
            auto leftLeaf = std::make_unique<BoxTreeNode<T>>(itemWidth, itemHeight, item);
            auto bottomSpace = std::make_unique<BoxTreeNode<T>>(node->width, node->height - itemHeight);
            auto rightSpace = std::make_unique<BoxTreeNode<T>>(node->width - itemWidth, itemHeight);

            auto vSplit = std::make_unique<BoxTreeNode<T>>(SplitAxis::Vertical, std::move(leftLeaf), std::move(rightSpace));
            *node = BoxTreeNode<T>(SplitAxis::Horizontal, std::move(vSplit), std::move(bottomSpace));
        }
    }

    template <typename T>
    BoxTreeNode<T>::BoxTreeNode(
        SplitAxis axis,
        std::unique_ptr<BoxTreeNode>&& left,
        std::unique_ptr<BoxTreeNode>&& right)
        : width(axis == SplitAxis::Vertical ? left->width + right->width : left->width),
          height(axis == SplitAxis::Horizontal ? left->height + right->height : left->height),
          value(BoxTreeSplit<T>(axis, std::move(left), std::move(right)))
    {
    }

    template <typename T>
    BoxTreeNode<T>::BoxTreeNode(unsigned int width, unsigned int height)
        : width(width), height(height), value(BoxTreeLeaf<T>())
    {
    }

    template <typename T>
    BoxTreeNode<T>::BoxTreeNode(unsigned int width, unsigned int height, const T& value)
        : width(width), height(height), value(BoxTreeLeaf<T>(value))
    {
    }

    template <typename T>
    BoxTreeNode<T>::BoxTreeNode(unsigned int width, unsigned int height, T&& value)
        : width(width), height(height), value(BoxTreeLeaf<T>(std::move(value)))
    {
    }

    template <typename T>
    std::optional<BoxTreeNode<T>*> BoxTreeNode<T>::findNode(unsigned int itemWidth, unsigned int itemHeight)
    {
        if (itemWidth > width || itemHeight > height)
        {
            return std::nullopt;
        }

        auto leaf = std::get_if<BoxTreeLeaf<T>>(&value);
        if (leaf != nullptr)
        {
            if (leaf->value)
            {
                // leaf is already occupied
                return std::nullopt;
            }

            return this;
        }

        // we must be a split, search both children
        auto split = std::get_if<BoxTreeSplit<T>>(&value);
        auto left = split->leftChild->findNode(itemWidth, itemHeight);
        if (left)
        {
            return left;
        }

        return split->rightChild->findNode(itemWidth, itemHeight);
    }

    template <typename T>
    std::vector<BoxPackInfoEntry<T>> BoxTreeNode<T>::walk()
    {
        auto leaf = std::get_if<BoxTreeLeaf<T>>(&value);
        if (leaf != nullptr)
        {
            if (leaf->value)
            {
                return std::vector<BoxPackInfoEntry<T>>{BoxPackInfoEntry<T>{0, 0, *(leaf->value)}};
            }

            return std::vector<BoxPackInfoEntry<T>>();
        }

        auto split = std::get_if<BoxTreeSplit<T>>(&value);
        auto vec = split->leftChild->walk();
        auto rightVec = split->rightChild->walk();
        switch (split->axis)
        {
            case SplitAxis::Horizontal:
                for (const BoxPackInfoEntry<T>& e : rightVec)
                {
                    vec.push_back(BoxPackInfoEntry<T>{e.x, e.y + split->leftChild->height, e.value});
                }
                break;
            case SplitAxis::Vertical:
                for (const BoxPackInfoEntry<T>& e : rightVec)
                {
                    vec.push_back(BoxPackInfoEntry<T>{e.x + split->leftChild->width, e.y, e.value});
                }
                break;
        }

        return vec;
    }

    struct Size
    {
        std::size_t width;
        std::size_t height;

        Size() = default;
        Size(std::size_t width, std::size_t height);
    };

    template <typename T>
    BoxPackInfo<T> packGridsGeneric(std::vector<T>& items, const std::function<Size(const T&)>& f)
    {
        assert(!items.empty());

        std::sort(items.begin(), items.end(), [&f](const T& a, const T& b) {
            auto sizeA = f(a);
            auto maxSideA = std::max(sizeA.width, sizeA.height);

            auto sizeB = f(b);
            auto maxSideB = std::max(sizeB.width, sizeB.height);

            return maxSideA > maxSideB;
        });


        auto it = items.begin();
        auto end = items.end();

        auto rootSize = f(*it);
        BoxTree<T> tree(rootSize.width, rootSize.height, *it);
        ++it;

        for (; it != end; ++it)
        {
            auto size = f(*it);
            auto width = size.width;
            auto height = size.height;
            tree.insert(width, height, *it);
        }

        auto entries = tree.root->walk();
        return BoxPackInfo<T>{tree.root->width, tree.root->height, std::move(entries)};
    }


    template <typename T>
    BoxPackInfo<Grid<T>*> packGrids(std::vector<Grid<T>*>& sprites)
    {
        return packGridsGeneric<Grid<T>*>(sprites, [](const auto& s) { return Size(s->getWidth(), s->getHeight()); });
    }
}

#endif
