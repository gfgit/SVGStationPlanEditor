add_subdirectory(model)

set(SSP_EDITOR_SOURCES
  ${SSP_EDITOR_SOURCES}

  svg_creator/svgcreatormanager.cpp
  svg_creator/svgcreatormanager.h

  PARENT_SCOPE
)
