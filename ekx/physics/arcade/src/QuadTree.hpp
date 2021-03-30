#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/box.hpp>
#include <ek/math/vec.hpp>
#include <set>

namespace ek {

// based on cool tutorial: https://thatgamesguy.co.uk/cpp-game-dev-16/
// and very good explanation for optimizations: https://stackoverflow.com/a/48330314/4223136
struct QuadTreeNode {

    enum ChildIndex {
        LeftTop = 0,
        RightTop = 1,
        RightBottom = 2,
        LeftBottom = 3
    };

    // used for first of 4 node identifiers,
    // OR next free index in nodes free pool
    int firstChildNode = 0;
    int objects = -1;
    int objectsCount = 0;

    [[nodiscard]] int getChildIndex(const rect_i& objectBounds, rect_i& nodeBounds) const {
        const int subWidth = nodeBounds.width >> 1u;
        const int subHeight = nodeBounds.height >> 1u;
        const int splitX = nodeBounds.x + subWidth;
        const int splitY = nodeBounds.y + subHeight;
        // Right
        if (objectBounds.x >= splitX) {
            // Top
            if (objectBounds.bottom() <= splitY) {
                nodeBounds.set(splitX, nodeBounds.y, subWidth, subHeight);
                return RightTop;
            }
                // Bottom
            else if (objectBounds.y >= splitY) {
                nodeBounds.set(splitX, splitY, subWidth, subHeight);
                return RightBottom;
            }
        }
            // Left
        else if (objectBounds.right() <= splitX) {
            // Top
            if (objectBounds.bottom() <= splitY) {
                nodeBounds.set(nodeBounds.x, nodeBounds.y, subWidth, subHeight);
                return LeftTop;
            }
                // Bottom
            else if (objectBounds.y >= splitY) {
                nodeBounds.set(nodeBounds.x, splitY, subWidth, subHeight);
                return LeftBottom;
            }
        }

        return -1;
    }

//    void remove(const QuadTreeNodeObject& object) {
//        int index = getChildIndexForBounds(object.bounds);
//        // leaf or not found
//        if (index == -1 || children[index] == -1) {
//            for (int i = 0; i < objects.size(); ++i) {
//                if (objects[i].entity == object.entity) {
//                    objects.erase(objects.begin() + i);
//
//                    assertInvariant();
//                    return;
//                }
//            }
//        }
//        return getChild(index).remove(object);
//    }

};

class QuadTree {
public:
    // pools part
    Array<QuadTreeNode> nodes;
    int nextFreeNode = 0;

    Array<int> objectNext;
    Array<ecs::EntityApi> objectEntity;
    Array<rect_i> objectsBoundsArray;
    int nextFreeObject = -1;

    int allocNode() {
        int freeID;
        if (nextFreeNode) {
            freeID = nextFreeNode;
            nextFreeNode = nodes[nextFreeNode].firstChildNode;
            nodes[freeID].firstChildNode = 0;
        } else {
            freeID = static_cast<int>(nodes._size);
            nodes.emplace_back();
            nodes.emplace_back();
            nodes.emplace_back();
            nodes.emplace_back();
        }
        assert(freeID > 0);
        return freeID;
    }

    void releaseNode(int nodeId) {
        assert(nodeId > 0);
        nodes[nodeId].firstChildNode = nextFreeNode;
        nextFreeNode = nodeId;
    }

    int allocObject() {
        int objId;
        if (nextFreeObject >= 0) {
            objId = nextFreeObject;
            nextFreeObject = objectNext[objId];
            //objectNext[objId] = -1;
        } else {
            objId = static_cast<int>(objectNext._size);
            objectNext.push_back(-1);
            objectsBoundsArray.emplace_back();
            objectEntity.emplace_back();
        }
        return objId;
    }

    void releaseObject(int objectId) {
        objectNext[objectId] = nextFreeObject;
        nextFreeObject = objectId;
    }

    // how many objects a node can contain before it splits into child nodes
    int maxObjects = 5;

    // starting from the base node (0) how many times can it (and its children) split
    int maxLevels = 5;

    rect_i bounds;

    explicit QuadTree(rect_i bounds_) : bounds{bounds_} {
        nodes.emplace_back();
    }

    void search(const rect_f& area, std::set<int>& outNodesList) {
        search(0, rect_i{area}, outNodesList, bounds);
    }

    int insert(ecs::EntityApi entity, const rect_f& objectBounds) {
        int objectId = allocObject();
        objectNext[objectId] = -1;
        objectEntity[objectId] = entity;
        objectsBoundsArray[objectId] = rect_i{objectBounds};
        insert(0, objectId, 0, bounds);
        return objectId;
    }

    // optimize full clearing
    void reset() {
        nodes.reduceSize(1);
        nodes[0] = {};
        objectNext.clear();
        objectsBoundsArray.clear();
        objectEntity.clear();
    }

    void clear(int nodeId = 0) {
        auto& node = nodes[nodeId];
        clearNodeObjects(node);
        if (node.firstChildNode) {
            for (int i = 0; i < 4; ++i) {
                clear(node.firstChildNode + i);
            }
            releaseNode(node.firstChildNode);
            node.firstChildNode = 0;
        }
    }

    void queryEntities(const std::set<int>& nodeIds, Array<ecs::EntityApi>& outEntityList) {
        for (auto nodeId : nodeIds) {
            auto& node = nodes[nodeId];
            // add all entities
            auto objectId = node.objects;
            while (objectId >= 0) {
                outEntityList.push_back(objectEntity[objectId]);
                objectId = objectNext[objectId];
            }
        }
    }

    static rect_i getChildBounds(rect_i bb, int childIndex) {
        const int w = bb.width >> 1;
        const int h = bb.height >> 1;
        rect_i result{bb.x, bb.y, w, h};
        switch (childIndex) {
            case QuadTreeNode::RightTop:
                result.x += w;
                break;
            case QuadTreeNode::RightBottom:
                result.x += w;
                result.y += h;
                break;
            case QuadTreeNode::LeftBottom:
                result.y += h;
                break;
        }
        return result;
    }

private:
    void clearNodeObjects(QuadTreeNode& node) {
        auto objectId = node.objects;
        while (objectId >= 0) {
            auto next = objectNext[objectId];
            releaseObject(objectId);
            objectId = next;
        }
        node.objectsCount = 0;
        node.objects = -1;
    }

    void split(int nodeId) {
        nodes[nodeId].firstChildNode = allocNode();
    }

    void insert(int nodeId, int objectId, int nodeLevel, rect_i nodeBounds) {
        auto& node = nodes[nodeId];
        // Needs to check if it has any children nodes.
        if (node.firstChildNode) {
            // We assume if the first child node is present then all four nodes are because
            // when we split the node we create the four children together.
            // If this node has child nodes then we find the index of the node that the object should belong to
            int indexToPlaceObject = node.getChildIndex(objectsBoundsArray[objectId], nodeBounds);
            if (indexToPlaceObject != -1) {
                insert(node.firstChildNode + indexToPlaceObject, objectId, nodeLevel + 1, nodeBounds);
            } else {
                objectNext[objectId] = node.objects;
                node.objects = objectId;
                ++node.objectsCount;
            }
        } else {
            // place
            objectNext[objectId] = node.objects;
            node.objects = objectId;
            ++node.objectsCount;

            // if we could sub-divide
            if (node.objectsCount > maxObjects && nodeLevel < maxLevels) {
                split(nodeId);
                auto& nodeAfterSplit = nodes[nodeId];
                auto objId = nodeAfterSplit.objects;
                auto prevId = -1;
                while (objId >= 0) {
                    const auto nextObj = objectNext[objId];
                    auto subBounds = nodeBounds;
                    int indexToPlaceObject = nodeAfterSplit.getChildIndex(objectsBoundsArray[objId], subBounds);
                    if (indexToPlaceObject != -1) {
                        if (prevId >= 0) {
                            objectNext[prevId] = nextObj;
                        } else {
                            nodeAfterSplit.objects = nextObj;
                        }
                        --nodeAfterSplit.objectsCount;

                        objectNext[objId] = -1;
                        insert(nodeAfterSplit.firstChildNode + indexToPlaceObject, objId, nodeLevel + 1, subBounds);

                        objId = nextObj;
                    } else {
                        prevId = objId;
                        objId = nextObj;
                    }
                }
            }
        }
    }

    void search(int nodeId, const rect_i& area, std::set<int>& outNodesList, rect_i nodeBounds) {
        auto& node = nodes[nodeId];
        if (node.objectsCount) {
            outNodesList.insert(nodeId);
        }
        if (node.firstChildNode) {
            const int childWidth = nodeBounds.width >> 1;
            const int childHeight = nodeBounds.height >> 1;
            const int splitX = nodeBounds.x + childWidth;
            const int splitY = nodeBounds.y + childHeight;
            // top
            if (area.y < splitY) {
                // left
                if (area.x < splitX) {
                    search(node.firstChildNode + QuadTreeNode::LeftTop, area, outNodesList,
                           {nodeBounds.x, nodeBounds.y, childWidth, childHeight});
                }
                // right
                if (area.right() > splitX) {
                    search(node.firstChildNode + QuadTreeNode::RightTop, area, outNodesList,
                           {nodeBounds.x + childWidth, nodeBounds.y, childWidth, childHeight});
                }
            }
            // bottom
            if (area.bottom() > splitY) {
                // right
                if (area.right() > splitX) {
                    search(node.firstChildNode + QuadTreeNode::RightBottom, area, outNodesList,
                           {nodeBounds.x + childWidth, nodeBounds.y + childHeight, childWidth, childHeight});
                }
                // Left
                if (area.x < splitX) {
                    search(node.firstChildNode + QuadTreeNode::LeftBottom, area, outNodesList,
                           {nodeBounds.x, nodeBounds.y + childHeight, childWidth, childHeight});
                }
            }
        }
    }
};

}

