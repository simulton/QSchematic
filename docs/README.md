# Top-Level Items

When adding an `Item` to the `Scene`, there are two possibilities. If the `Item`
is a "Top-Level" Item (it has no parent), then you should use
`Scene::addItem()`. If the `Item` is the child of another `Item` then you need
to use the superclass' implementation `QGraphicsItem::addItem()`.

> This is needed to determine which Items should be moved by the scene and which
> are moved by their parent