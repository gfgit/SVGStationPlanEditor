add_subdirectory(items)

set(SSP_EDITOR_SOURCES
  ${SSP_EDITOR_SOURCES}

  svg_creator/model/abstractlayoutitem.cpp
  svg_creator/model/abstractlayoutitem.h

  svg_creator/model/itempropertieswidget.cpp
  svg_creator/model/itempropertieswidget.h

  svg_creator/model/svgcreatorscene.cpp
  svg_creator/model/svgcreatorscene.h

  svg_creator/model/svgconnectionsmodel.cpp
  svg_creator/model/svgconnectionsmodel.h

  PARENT_SCOPE
)
