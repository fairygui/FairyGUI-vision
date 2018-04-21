#ifndef __TREENODE_H__
#define __TREENODE_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class TreeView;
class GComponent;

class FGUI_IMPEXP TreeNode : public Ref

{
public:
    static TreeNode* create(bool isFolder = false);

    TreeNode* getParent() const { return _parent; }
    TreeView* getRoot() const { return _root; }
    GComponent* getCell() const { return _cell; }
    const Value& getData() const { return _data; }
    void setData(const Value& value) { _data = value; }
    bool isExpanded() const { return _expanded; }
    void setExpaned(bool value);
    bool isFolder() const { return _isFolder;}
    const std::string& getText() const;

    TreeNode* addChild(TreeNode* child);
    TreeNode* addChildAt(TreeNode* child, int index);

    void removeChild(TreeNode * child);
    void removeChildAt(int index);
    void removeChildren() { removeChildren(0, -1); }
    void removeChildren(int beginIndex, int endIndex);

    TreeNode* getChildAt(int index) const;
    TreeNode* getPrevSibling() const;
    TreeNode* getNextSibling() const;

    int getChildIndex(const TreeNode* child) const;
    void setChildIndex(TreeNode* child, int index);
    int setChildIndexBefore(TreeNode* child, int index);
    void swapChildren(TreeNode* child1, TreeNode* child2);
    void swapChildrenAt(int index1, int index2);

    int numChildren() const;

protected:
    TreeNode();
    virtual ~TreeNode();

private:
    bool init(bool isFolder);
    int moveChild(TreeNode* child, int oldIndex, int index);
    void setRoot(TreeView* value);
    void setCell(GComponent* value);

    TreeView* _root;
    TreeNode* _parent;
    GComponent* _cell;
    int _level;
    bool _expanded;
    bool _isFolder;
    bool _isRootNode;
    Value _data;
    Vector<TreeNode*> _children;

    friend class TreeView;
};

NS_FGUI_END

#endif
