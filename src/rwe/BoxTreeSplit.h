#ifndef RWE_BOXTREE_H
#define RWE_BOXTREE_H

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <memory>
#include <rwe/Grid.h>
#include <rwe/Sprite.h>
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

    enum class Axis
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

        using Union = boost::variant<BoxTreeSplit<T>, BoxTreeLeaf<T>>;

        Union value;

        BoxTreeNode(
            Axis axis,
            std::unique_ptr<BoxTreeNode>&& left,
            std::unique_ptr<BoxTreeNode>&& right);

        BoxTreeNode(unsigned int width, unsigned int height);

        BoxTreeNode(unsigned int width, unsigned int height, const T& value);

        BoxTreeNode(unsigned int width, unsigned int height, T&& value);

        boost::optional<BoxTreeNode<T>*> findNode(unsigned int itemWidth, unsigned int itemHeight);

        std::vector<BoxPackInfoEntry<T>> walk();
    };

    template <typename T>
    struct BoxTreeSplit
    {
        Axis axis;
        std::unique_ptr<BoxTreeNode<T>> leftChild;
        std::unique_ptr<BoxTreeNode<T>> rightChild;

        BoxTreeSplit(Axis axis, std::unique_ptr<BoxTreeNode<T>>&& leftChild, std::unique_ptr<BoxTreeNode<T>>&& rightChild)
            : axis(axis), leftChild(std::move(leftChild)), rightChild(std::move(rightChild))
        {
        }
    };

    template <typename T>
    struct BoxTreeLeaf
    {
        boost::optional<T> value;

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
                auto newRoot = std::make_unique<BoxTreeNode<T>>(Axis::Horizontal, std::move(root), std::move(newNode));
                root = std::move(newRoot);
                return newNodePtr;
            }
            case GrowDirection::Down:
            {
                auto newNode = std::make_unique<BoxTreeNode<T>>(root->width, itemHeight);
                auto newNodePtr = newNode.get();
                auto newRoot = std::make_unique<BoxTreeNode<T>>(Axis::Horizontal, std::move(root), std::move(newNode));
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
        BoxTreeLeaf<T>& leaf = boost::get<BoxTreeLeaf<T>>(node->value);
        if (node->width == itemWidth && node->height == itemHeight)
        {
            leaf.value = item;
        }
        else if (node->width == itemWidth && node->height > itemHeight)
        {
            auto newLeaf = std::make_unique<BoxTreeNode<T>>(itemWidth, itemHeight, item);
            auto freeSpace = std::make_unique<BoxTreeNode<T>>(itemWidth, node->height - itemHeight);
            *node = BoxTreeNode<T>(Axis::Horizontal, std::move(newLeaf), std::move(freeSpace));
        }
        else if (node->height == itemHeight && node->width > itemWidth)
        {
            auto newLeaf = std::make_unique<BoxTreeNode<T>>(itemWidth, itemHeight, item);
            auto freeSpace = std::make_unique<BoxTreeNode<T>>(node->width - itemWidth, itemHeight);
            *node = BoxTreeNode<T>(Axis::Vertical, std::move(newLeaf), std::move(freeSpace));
        }
        else
        {
            auto leftLeaf = std::make_unique<BoxTreeNode<T>>(itemWidth, itemHeight, item);
            auto bottomSpace = std::make_unique<BoxTreeNode<T>>(node->width, node->height - itemHeight);
            auto rightSpace = std::make_unique<BoxTreeNode<T>>(node->width - itemWidth, itemHeight);

            auto vSplit = std::make_unique<BoxTreeNode<T>>(Axis::Vertical, std::move(leftLeaf), std::move(rightSpace));
            *node = BoxTreeNode<T>(Axis::Horizontal, std::move(vSplit), std::move(bottomSpace));
        }
    }

    template <typename T>
    BoxTreeNode<T>::BoxTreeNode(
        Axis axis,
        std::unique_ptr<BoxTreeNode>&& left,
        std::unique_ptr<BoxTreeNode>&& right)
        : width(axis == Axis::Vertical ? left->width + right->width : left->width),
          height(axis == Axis::Horizontal ? left->height + right->height : left->height),
          value(BoxTreeSplit(axis, std::move(left), std::move(right)))
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
    boost::optional<BoxTreeNode<T>*> BoxTreeNode<T>::findNode(unsigned int itemWidth, unsigned int itemHeight)
    {
        if (itemWidth > width || itemHeight > height)
        {
            return boost::none;
        }

        auto leaf = boost::get<BoxTreeLeaf<T>>(&value);
        if (leaf != nullptr)
        {
            if (leaf->value)
            {
                // leaf is already occupied
                return boost::none;
            }

            return this;
        }

        // we must be a split, search both children
        auto split = boost::get<BoxTreeSplit<T>>(&value);
        auto left = split->leftChild->findNode(width, height);
        if (left)
        {
            return left;
        }

        return split->rightChild->findNode(width, height);
    }

    template <typename T>
    std::vector<BoxPackInfoEntry<T>> BoxTreeNode<T>::walk()
    {
        auto leaf = boost::get<BoxTreeLeaf<T>>(&value);
        if (leaf != nullptr)
        {
            if (leaf->value)
            {
                return std::vector<BoxPackInfoEntry<T>>{BoxPackInfoEntry<T>{0, 0, *(leaf->value)}};
            }

            return std::vector<BoxPackInfoEntry<T>>();
        }

        auto split = boost::get<BoxTreeSplit<T>>(&value);
        auto vec = split->leftChild->walk();
        auto rightVec = split->rightChild->walk();
        switch (split->axis)
        {
            case Axis::Horizontal:
                for (const BoxPackInfoEntry<T>& e : rightVec)
                {
                    vec.push_back(BoxPackInfoEntry<T>{e.x, e.y + split->leftChild->height, e.value});
                }
                break;
            case Axis::Vertical:
                for (const BoxPackInfoEntry<T>& e : rightVec)
                {
                    vec.push_back(BoxPackInfoEntry<T>{e.x + split->leftChild->width, e.y, e.value});
                }
                break;
        }

        return vec;
    }

    template <typename T>
    BoxPackInfo<Grid<T>*> packGrids(std::vector<Grid<T>*>& sprites)
    {
        assert(!sprites.empty());

        std::sort(sprites.begin(), sprites.end(), [](const Grid<T>* a, const Grid<T>* b) {
            auto sideA = std::max(a->getWidth(), a->getHeight());
            auto sideB = std::max(b->getWidth(), b->getHeight());
            return sideA > sideB;
        });


        auto it = sprites.begin();
        auto end = sprites.end();

        BoxTree<Grid<T>*> tree((*it)->getWidth(), (*it)->getHeight(), *it);
        ++it;

        for (; it != end; ++it)
        {
            auto width = (*it)->getWidth();
            auto height = (*it)->getHeight();
            tree.insert(width, height, *it);
        }

        auto entries = tree.root->walk();
        return BoxPackInfo<Grid<T>*>{tree.root->width, tree.root->height, std::move(entries)};
    }
}

#endif
