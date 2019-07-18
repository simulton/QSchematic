TEMPLATE = subdirs

SUBDIRS = \
    qschematic \
    demo

demo.depends = qschematic
