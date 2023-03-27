![ОГАС](src/OGAS.jfif)
```C
    void bi_traverse(t_node *root, void (*funptr)(t_node *)) //双序遍历
    {
        if (!root) //若为空子树
            return;
        funptr(root);
        bi_traverse(root->left, funptr); //遍历左子树，可为空
        funptr(root);
        bi_traverse(root->right, funptr); //遍历右子树，可为空
    }
```
$E=mc^2$
















