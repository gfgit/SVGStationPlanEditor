set(SSP_LIBRARY_HEADERS
  ${SSP_LIBRARY_HEADERS}
  parsing/editinginfo.h
  parsing/parsinghelpers.h
  parsing/domparser.h
  parsing/stationinfoparser.h
  parsing/streamparser.h

  PARENT_SCOPE
)

set(SSP_LIBRARY_SOURCES
  ${SSP_LIBRARY_SOURCES}
  parsing/editinginfo.cpp
  parsing/parsinghelpers.cpp
  parsing/domparser.cpp
  parsing/stationinfoparser.cpp
  parsing/streamparser.cpp

  PARENT_SCOPE
)
