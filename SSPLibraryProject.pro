TEMPLATE = subdirs

SUBDIRS += editor library viewer

editor.depends = library
viewer.depends = library
